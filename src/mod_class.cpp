//////////////////////////////////////////////////
//                                              //
// MODClass                                     //
// by Thorsten Kattanek                         //
//                                              //
// #file: mod_class.cpp                         //
//                                              //
// last change: 12-11-2022                      //
// https://github.com/ThKattanek/mod-player     //
//                                              //
//////////////////////////////////////////////////

#include "mod_class.h"
#include <iostream>

// C-C#-D-D#-E-F-F#-G-G#-A-A#-H
static const char* NOTE_STRING[12] = {"C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-"};

MODClass::MODClass(const char *filename, int samplerate)
{
    this->samplerate = samplerate;

    filter_is_enable = true;
    lp_filterL = new LowPassFilter(8000, 1.0/samplerate);
    lp_filterR = new LowPassFilter(8000, 1.0/samplerate);

    ChangePattern = false;
    ChangePatternNr = 0;

    ChangePatternRow = false;
    ChangePatternRowNr = 0;

    scope_enable = false;
    scope_buffer = NULL;

    channel_pan = 0.5;
    channels = NULL;

    for(int i=0; i<MAX_PATTERN;i++)
    {
        mod_pattern[i] = NULL;
    }

    if(!LoadMod(filename))
    {
        // Fehler beim laden
        mod_is_loaded = false;
    }
    else
    {
        // Mod wurde geladen
        mod_is_loaded = true;
    }
}

MODClass::~MODClass()
{
    for(int i=0; i<MAX_PATTERN;i++)
    {
       // Pattern delete
       if(mod_pattern != NULL)
           delete[] mod_pattern[i];
    }

    delete lp_filterL;
    delete lp_filterR;
}

bool MODClass::ModIsLoaded()
{
    return mod_is_loaded;
}

const char *MODClass::GetModName()
{
    if(mod_is_loaded)
        return mod_name;
    return NULL;
}

const char *MODClass::GetModType()
{
    if(mod_is_loaded)
        return mod_type;
    return NULL;
}

int MODClass::GetModSampleCount()
{
    if(mod_is_loaded)
        return mod_sample_count;
    return 0;
}

int MODClass::GetModSongLength()
{
    if(mod_is_loaded)
        return mod_song_length;
    return 0;
}

int MODClass::GetModSongEndJump()
{
    if(mod_is_loaded)
        return mod_song_end_jump;
    return 0;
}

int MODClass::GetModSongPos()
{
    return song_pos;
}

const unsigned char *MODClass::GetModPatternTable()
{
    if(mod_is_loaded)
        return mod_pattern_tbl;
    return NULL;
}

int MODClass::GetModPatterCount()
{
    if(mod_is_loaded)
        return mod_pattern_count;
    return 0;
}

int MODClass::GetModChannelCount()
{
    if(mod_is_loaded)
        return mod_channel_count;
    return 0;
}

int MODClass::GetModSpeed()
{
    if(mod_is_loaded)
        return speed;
    else
        return 0;
}

int MODClass::GetModBPM()
{
    if(mod_is_loaded)
        return bpm;
    else
        return 0;
}

SAMPLE *MODClass::GetModSample(int sample_nr)
{
    if(mod_is_loaded)
    {
        if((sample_nr > 0) && (sample_nr <= mod_sample_count))
        {
            return &mod_samples[sample_nr-1];
        }
    }
    return NULL;
}

float MODClass::GetChannelVolumeVisualValue(int channel_nr)
{
    if((channel_nr >= 0 && channel_nr < mod_channel_count))
    {
        return channels[channel_nr].volume_visual;
    }
    return 0;
}

void MODClass::SetChannelMute(int channel_nr, bool enable)
{
    if(channel_nr < mod_channel_count)
    {
        channels[channel_nr].mute = enable;
    }
}

bool MODClass::GetChannelMute(int channel_nr)
{
    if(channel_nr < mod_channel_count)
    {
        return channels[channel_nr].mute;
    }
    else return false;
}

void MODClass::FillAudioBuffer(signed short *stream, int length)
{
    scope_buffer_pos = 0;

    float l_r_sample[2];

    if(mod_is_playing)
    {
        for(int i=0; i<length; i+=2)
        {
            time_counter -= 1.0;
            akt_pattern_line_progress += akt_pattern_line_progress_add;

            if(time_counter <= 0)
            {
                time_counter += time_counter_start;
                thick_counter--;
                if(thick_counter == 0)
                {
                    NextLine();

                    thick_counter = thick_counter_start;
                    akt_pattern_line_progress = 0.0;
                    akt_pattern_line_progress_add = 1.0 / (time_counter_start * thick_counter_start);
                }
                CalcNextThick();
            }
            CalcNextSamples(l_r_sample);
            stream[i] = l_r_sample[0] * 0x7fff;
            stream[i+1] = l_r_sample[1] * 0x7fff;
        }
    }
    else
    {
        for(int i=0; i<length; i+=2)
           stream[i] = stream[i+1] = 0;
    }
}

void MODClass::FillAudioBuffer(float *stream, int length)
{
    scope_buffer_pos = 0;

    float l_r_sample[2];

    if(mod_is_playing)
    {
        for(int i=0; i<length; i+=2)
        {
            time_counter -= 1.0;
            akt_pattern_line_progress += akt_pattern_line_progress_add;

            if(time_counter <= 0)
            {
                time_counter += time_counter_start;
                thick_counter--;
                if(thick_counter == 0)
                {
					NextLine();

                    thick_counter = thick_counter_start;
                    akt_pattern_line_progress = 0.0;
                    akt_pattern_line_progress_add = 1.0 / (time_counter_start * thick_counter_start);
                }
				CalcNextThick();
            }
			CalcNextSamples(l_r_sample);
			stream[i] = l_r_sample[0];
			stream[i+1] = l_r_sample[1];
        }
    }
    else
    {
        for(int i=0; i<length; i+=2)
           stream[i] = stream[i+1] = 0.0f;
	}
}

bool MODClass::LoadMod(const char *filename)
{
    ModStop();

    mod_is_loaded = false;

    file.open(filename, ios::in | ios::binary);
    if(!file.is_open())
    {
        mod_load_error = 0x01;
        return false;
    }

    // MOD Name
    file.read(mod_name,20);
    mod_name[20] = 0;

    // MOD Type
    file.seekg(1080, ios::beg);
    file.read((char*)mod_type, 4);
    file.seekg(20, ios::beg);
    mod_type[4] = 0;

    /*
    'M.K', '4CHN','6CHN','8CHN','FLT4','FLT8
    */

    if(!strcmp((const char*)mod_type, "M.K."))
        mod_type_id = _MK;
    else if(!strcmp((const char*)mod_type, "M!K!"))
        mod_type_id = _MK;
    else if(!strcmp((const char*)mod_type, "4CHN"))
        mod_type_id = _4CHN;
    else if(!strcmp((const char*)mod_type, "6CHN"))
        mod_type_id = _6CHN;
    else if(!strcmp((const char*)mod_type, "8CHN"))
        mod_type_id = _8CHN;
    else if(!strcmp((const char*)mod_type, "FLT4"))
        mod_type_id = _4FLT;
    else if(!strcmp((const char*)mod_type, "FLT8"))
        mod_type_id = _8FLT;
    else if(!strcmp((const char*)mod_type, "OCTA"))
        mod_type_id = _OCTA;
    else
    {
        if(!strcmp((const char*)mod_type+2, "CH"))
        {
            mod_type_id = _CH;

            char mod_id_str[5];
            strcpy(mod_id_str, (const char*)mod_type);
            mod_id_str[2] = 0;
            mod_channel_count = strtol(mod_id_str, NULL, 10);
        }
        else
        {
            mod_type_id = _NST; // The old NST format with 15 Samples and not ID-Tag

			// Check of S3M File
			file.seekg(44, ios::beg);
			file.read((char*)mod_type, 4);
			file.seekg(20, ios::beg);
			mod_type[4] = 0;

			if(!strcmp((const char*)mod_type, "SCRM"))
			{
				cout << "The S3M Format is not supported!" << endl;
				return false;
			}
        }
    }

    // SAMPLES
    if(mod_type_id == _NST)
        mod_sample_count = 15;
    else mod_sample_count = 31;

    for(int i=0; i<mod_sample_count; i++)
    {
        // Sample Name
        file.read(mod_samples[i].name,22);
        mod_samples[i].name[22] = 0;

        // Sample length
        file.read((char*)&mod_samples[i].length,2);
        unsigned char *length = (unsigned char*)&mod_samples[i].length;
        unsigned char tmp = length[0];
        length[0] = length[1];
        length[1] = tmp;
        mod_samples[i].length *=2;

        // Finetune
        unsigned char finetune;
        file.read((char*)&finetune,1);
        mod_samples[i].finetune = finetune & 0x0f;

        // Volume
        file.read((char*)&mod_samples[i].volume,1);

        // Loop start
        file.read((char*)&mod_samples[i].loop_start,2);
        unsigned char *loop_start = (unsigned char*)&mod_samples[i].loop_start;
        tmp = loop_start[0];
        loop_start[0] = loop_start[1];
        loop_start[1] = tmp;
        mod_samples[i].loop_start *=2;

        // Loop length
        file.read((char*)&mod_samples[i].loop_length,2);
        unsigned char *loop_length = (unsigned char*)&mod_samples[i].loop_length;
        tmp = loop_length[0];
        loop_length[0] = loop_length[1];
        loop_length[1] = tmp;
        mod_samples[i].loop_length *=2;

        // Fix Loop Start
        if((mod_samples[i].loop_start >= mod_samples[i].length) && (mod_samples[i].loop_length > 2))
            mod_samples[i].loop_start = mod_samples[i].length - mod_samples[i].loop_length;

		// Warnig output when loop overlay
#ifdef __DEBUG__
		if(((mod_samples[i].loop_start + mod_samples[i].loop_length)) >  mod_samples[i].loop_length)
		{
			cout << "Warning: The sample loop-end is greater as sample-length. (Nr.: " << i+1 << ")" << endl;
		}
#endif
    }

    // MOD Song Length
    file.read((char*)&mod_song_length,1);

    // MOD Song End Jump
    file.read((char*)&mod_song_end_jump,1);

    // MOD Patterntable
    file.read((char*)&mod_pattern_tbl,128);

    int max_pattern_nr = 0;
    for(int i=0; i<128; i++)
    {
        if(mod_pattern_tbl[i] > max_pattern_nr)
            max_pattern_nr = mod_pattern_tbl[i];
    }

    // Pattern Count
    mod_pattern_count = max_pattern_nr+1;

    // Channel Count
    switch(mod_type_id)
    {
    case _MK: case _4CHN: case _4FLT: case _NST:
        mod_channel_count = 4;
        break;
    case _6CHN:
        mod_channel_count = 6;
        break;
    case _8CHN: case _8FLT: case _OCTA:
        mod_channel_count = 8;
        break;
    case _CH:
        // CHANNEL COUNT IS SETTING
        break;
    default:
        mod_channel_count = 0;
    }

    if(channels != NULL)
        delete[] channels;
    else
        channels = new CHANNEL[mod_channel_count];

    for(int i=0; i< mod_channel_count; i++)
    {
        channels[i].sample_data = NULL;
        channels[i].loop_enable = false;
        channels[i].play = false;
        channels[i].volume = 1.0;
    }

    // Read Pattern Data
    if(mod_type_id == _NST)
        file.seekg(600, ios::beg);
    else
        file.seekg(1084, ios::beg);

    mod_pattern_size = MAX_ROW * mod_channel_count;

    if(mod_pattern_count > MAX_PATTERN)
    {
        file.close();
        mod_load_error = 0x02;
        return false;
    }

    for(int i=0; i<mod_pattern_count; i++)
    {
        if(mod_pattern[i] != NULL)
            delete[] mod_pattern[i];

        mod_pattern[i] = new NOTE[mod_pattern_size];
        for(int j=0; j<mod_pattern_size; j++)
        {
            unsigned char note[4];
            file.read((char*)note, 4);

            mod_pattern[i][j].sample_number = note[0] & 0xF0;
            mod_pattern[i][j].sample_number |= note[2] >> 4;
            mod_pattern[i][j].period = note[1];
            mod_pattern[i][j].period |= ((unsigned short)note[0]&0x0F) << 8;
            mod_pattern[i][j].effectcommand = note[2] & 0x0F;
            mod_pattern[i][j].effectdata = note[3];

            NoteConvert(&mod_pattern[i][j],false);
        }
    }

    // Sample Data

    for(int i=0; i<mod_sample_count; i++)
    {
        if(mod_samples[i].length > 0)
        {
            char* smp_buffer = new char[mod_samples[i].length];

            if(mod_samples[i].data != NULL)
                delete[] mod_samples[i].data;

            mod_samples[i].data = new float[mod_samples[i].length];

            file.read(smp_buffer, mod_samples[i].length);

            for(int ii=0; ii<mod_samples[i].length; ii++)
            {
                mod_samples[i].data[ii] = smp_buffer[ii]/127.0; // Bei 8 Bit Samples
                if(mod_samples[i].data[ii] < -1.0)
                    mod_samples[i].data[ii] = -1.0;
            }

            delete[] smp_buffer;
        }
    }

    file.close();

    for(int i=0; i<mod_channel_count; i++)
        channels[i].mute = false;

    mod_load_error = 0x00;
    mod_is_loaded = true;
    return true;
}

int MODClass::GetLoadError()
{
    return mod_load_error;
}

void MODClass::NoteConvert(NOTE *note, bool direction)
{
    if(note != NULL)
    {
        if(!direction)
        {
            // period to note
            if(note->period > 0)
            {
                int i=0;
                do{
                }while ((i<60) && (PERIOD_TABLE [0][i++] != note->period));
                i--;
                if(i<60)
                {
                    note->note_postion_in_table = i;
                    note->note_number = i%12;
                    note->oktave_number = i/12 + 2;
                }
                else
                    note->note_number = 0x0f;   // Ungültige Notennummer
            }
            else
                note->note_number = 0x0f;   // Ungültige Notennummer
        }
        else
        {
            // note to periode
        }
    }
}

void MODClass::NextLine()
{
    if(set_song_speed)
    {
        set_song_speed = false;
        thick_counter_start = set_song_speed_var;
    }

    if(position_jump)
    {
        position_jump = false;
        if(position_jump_pos == mod_song_length)
            song_pos = 0;
        else
            song_pos = position_jump_pos;
        akt_pattern_line = 0;
    }

    if(pattern_break)
    {
        pattern_break = false;

        song_pos++;
        if(song_pos == mod_song_length)
            song_pos = 0;

        int pattern_nr = mod_pattern_tbl[song_pos];
        akt_pattern = mod_pattern[pattern_nr];

        akt_pattern_line = pattern_break_line;

        ChangePatternNr = pattern_nr;
        ChangePattern = true;
    }
    else if(akt_pattern_line == 0)
    {
        int pattern_nr = mod_pattern_tbl[song_pos];
        akt_pattern = mod_pattern[pattern_nr];

        ChangePatternNr = pattern_nr;
        ChangePattern = true;
    }

    int pattern_line_adr = mod_channel_count * akt_pattern_line;

    int channel_nr = 0;
    for(int i=pattern_line_adr; i<pattern_line_adr + mod_channel_count; i++)
    {
        CalcChannelData(channel_nr, &akt_pattern[i]);
        channel_nr++;
    }

    ChangePatternRowNr = akt_pattern_line;
    ChangePatternRow = true;

    akt_pattern_line++;
    if(akt_pattern_line == 64)
    {
        akt_pattern_line = 0;
        song_pos++;
        if(song_pos == mod_song_length)
            song_pos = 0;
    }
}

void MODClass::CalcChannelData(int channel_nr, NOTE *note)
{
    bool note_attack = false;

    // Arpeggio beenden -> gilt nur für eine Line
    channels[channel_nr].arpeggio = false;

    // Slide Up and Down beenden
    channels[channel_nr].slide_up = false;
    channels[channel_nr].slide_down = false;

    // Slide to Note beenden
    channels[channel_nr].slide_note = false;

    // Volume Slide beenden -> gilt nur für eine Line
    channels[channel_nr].volume_slide = 0;

    // Vibrato beenden -> gilt nur für eine Line
    channels[channel_nr].vibrato = false;

    // Tremolo beenden -> gilt nur für eine Line
    channels[channel_nr].tremolo = false;

    // Retrigger Sample -> gilt nur für eine Line
    channels[channel_nr].retrigger_sample = false;

    if(note->sample_number > 0)
    {
        channels[channel_nr].volume = (mod_samples[note->sample_number-1].volume & 0x7f) / 64.0;   // Liniar Volume

        channels[channel_nr].sample_data = mod_samples[note->sample_number-1].data;
        channels[channel_nr].sample_length = mod_samples[note->sample_number-1].length;
        channels[channel_nr].sample_finetune = mod_samples[note->sample_number-1].finetune;

        channels[channel_nr].loop_start = mod_samples[note->sample_number-1].loop_start;
        channels[channel_nr].loop_length = mod_samples[note->sample_number-1].loop_length;
        if(channels[channel_nr].loop_length > 2) channels[channel_nr].loop_enable = true;
        else channels[channel_nr].loop_enable = false;

        if(note->effectcommand != 0x03)
        {
            channels[channel_nr].sample_pos = 2;
        }
        else
        {
            if(channels[channel_nr].loop_enable)
            {
                if(channels[channel_nr].sample_pos >= (channels[channel_nr].loop_start + channels[channel_nr].loop_length))
                {
                    channels[channel_nr].sample_pos = channels[channel_nr].loop_start;
                }
            }
            else
            {
                if(channels[channel_nr].sample_pos >= channels[channel_nr].sample_length)
                {
                    channels[channel_nr].sample_pos = 2;
                }
            }
        }
    }

    // Wenn Period == 0 dann keine Änderung der Periode
    if(note->period > 0)
    {
        channels[channel_nr].play = true;

        channels[channel_nr].note_position_in_table = note->note_postion_in_table;

        if(note->effectcommand != 0x03)
        {
            channels[channel_nr].sample_pos = 2;
            channels[channel_nr].period = PERIOD_TABLE[channels[channel_nr].sample_finetune][channels[channel_nr].note_position_in_table];
            channels[channel_nr].frequ_counter_start = CalcFrequCounterStart(channels[channel_nr].period);
        }
        else
        {
            channels[channel_nr].slide_note_destination_period = PERIOD_TABLE[channels[channel_nr].sample_finetune][channels[channel_nr].note_position_in_table];
        }

        note_attack = true;
    }

    // Effekte
    unsigned char vol;
    unsigned char slide_up;
    unsigned char slide_down;
    unsigned char xxx,yyy;
    int arp_period1, arp_period2;

    xxx = note->effectdata >> 4;
    yyy = note->effectdata & 0x0f;

    switch(note->effectcommand)
    {
    case 0x00:      // Arpeggio
        if(note->effectdata > 0)
        {
            channels[channel_nr].arpeggio = true;
            channels[channel_nr].arpeggio_counter = 0;
            channels[channel_nr].arpeggio_frequency0 = CalcFrequCounterStart(channels[channel_nr].period);

            if((channels[channel_nr].note_position_in_table + xxx) < 60)
            {
                arp_period1 = PERIOD_TABLE[0][channels[channel_nr].note_position_in_table + xxx];
                channels[channel_nr].arpeggio_frequency1 = CalcFrequCounterStart(arp_period1);
            }
            else channels[channel_nr].arpeggio_frequency1 = channels[channel_nr].arpeggio_frequency0;

            if((channels[channel_nr].note_position_in_table + yyy) < 60)
            {
                arp_period2 = PERIOD_TABLE[0][channels[channel_nr].note_position_in_table + yyy];
                channels[channel_nr].arpeggio_frequency2 = CalcFrequCounterStart(arp_period2);
            }
            else channels[channel_nr].arpeggio_frequency2 = channels[channel_nr].arpeggio_frequency0;
        }
        break;

    case 0x01:      // Slide up (Portamento Up)
        channels[channel_nr].slide_up = true;
        channels[channel_nr].slide_up_value = note->effectdata;
        break;

    case 0x02:      // Slide down (Portamento Down)
        channels[channel_nr].slide_down = true;
        channels[channel_nr].slide_down_value = note->effectdata;
        break;

    case 0x03:      // Slide to note
        if(channels[channel_nr].period != channels[channel_nr].slide_note_destination_period)
        {
            if(channels[channel_nr].period < channels[channel_nr].slide_note_destination_period)
                channels[channel_nr].slide_note_direction = 0;  // up
            else
                channels[channel_nr].slide_note_direction = 1;  // down

            if(note->effectdata > 0)
                channels[channel_nr].slide_note_speed = note->effectdata;

            channels[channel_nr].slide_note = true;
        }
        break;

    case 0x04:      // Vibrato
        channels[channel_nr].vibrato = true;

        if((note->effectdata >> 4) != 0)
            channels[channel_nr].vibrato_speed = note->effectdata >> 4;
        if((note->effectdata & 0x0f) != 0)
            channels[channel_nr].vibrato_depth = note->effectdata & 0x0f;
        if(note_attack)
            channels[channel_nr].vibrato_pos = 0;   // Retrigger
        break;

    case 0x05:      // Continue Slide to note + Volume slide
        channels[channel_nr].slide_note = true;

        slide_up = note->effectdata >> 4;
        slide_down = note->effectdata & 0x0f;

        // Volume Slide
        if(slide_up > 0)
        {
            channels[channel_nr].volume_slide = 1;
            channels[channel_nr].volume_slide_value = slide_up;
        }
        else if(slide_down > 0)
        {
            channels[channel_nr].volume_slide = 2;
            channels[channel_nr].volume_slide_value = slide_down;
        }
        else{
            channels[channel_nr].volume_slide = 0;
            channels[channel_nr].volume_slide_value = 0;
        }
        break;

    case 0x06:      // Continue Vibrato + Volume Slide
        channels[channel_nr].vibrato = true;

        slide_up = note->effectdata >> 4;
        slide_down = note->effectdata & 0x0f;

        // Volume Slide
        if(slide_up > 0)
        {
            channels[channel_nr].volume_slide = 1;
            channels[channel_nr].volume_slide_value = slide_up;
        }
        else if(slide_down > 0)
        {
            channels[channel_nr].volume_slide = 2;
            channels[channel_nr].volume_slide_value = slide_down;
        }
        else{
            channels[channel_nr].volume_slide = 0;
            channels[channel_nr].volume_slide_value = 0;
        }
        break;

    case 0x07:      // Tremolo
        channels[channel_nr].tremolo = true;

        if((note->effectdata >> 4) != 0)
            channels[channel_nr].tremolo_speed = note->effectdata >> 4;
        if((note->effectdata & 0x0f) != 0)
            channels[channel_nr].tremolo_depth = note->effectdata & 0x0f;

        channels[channel_nr].tremolo_volume = channels[channel_nr].volume * 255;

        if(note_attack)
            channels[channel_nr].tremolo_pos = 0;   // Retrigger
        break;

    case 0x08:      // Not Used
        break;

    case 0x09:      // Set Sample Offset
            if(note->effectdata > 0)
            {
                channels[channel_nr].sample_pos = note->effectdata * 0x100;
                if(channels[channel_nr].sample_pos >= channels[channel_nr].sample_length)
                {
                    channels[channel_nr].sample_pos = 0;
                    channels[channel_nr].play = false;
                }
            }
        break;

    case 0x0A:      // Volume Slide
        slide_up = note->effectdata >> 4;
        slide_down = note->effectdata & 0x0f;

        if(slide_up > 0)
        {
            channels[channel_nr].volume_slide = 1;
            channels[channel_nr].volume_slide_value = slide_up;
        }
        else if(slide_down > 0)
        {
            channels[channel_nr].volume_slide = 2;
            channels[channel_nr].volume_slide_value = slide_down;
        }
        else{
            channels[channel_nr].volume_slide = 0;
            channels[channel_nr].volume_slide_value = 0;
        }
        break;

    case 0x0B:      // Position Jump
        position_jump_pos = note->effectdata;
        if(position_jump_pos <= 127)
        {
            position_jump = true;
        }
        break;

    case 0x0C:      // Set Volume
        vol = note->effectdata;
        if(vol > 64)
            vol = 64;
        channels[channel_nr].volume = vol / 64.0;
        break;

    case 0x0D:      // Pattern Break
        pattern_break = true;
        pattern_break_line = (note->effectdata >> 4) * 10 + (note->effectdata & 0x0f);
        break;

    case 0x0E:      // Extended Effects
            cout << "Extended Effect: " << (note->effectdata >> 4) << endl;
            switch(note->effectdata >> 4)
            {
            case 0x09:  // Retrigger Sample
                channels[channel_nr].retrigger_sample_counter = channels[channel_nr].retrigger_sample_speed = note->effectdata & 0x0f;
                channels[channel_nr].retrigger_sample = true;
                break;
            case 0x0A:  // Fine Volume Slide Up
                channels[channel_nr].volume += ((note->effectdata & 0x0f) / 64.0f);
                if(channels[channel_nr].volume > 1.0) channels[channel_nr].volume = 1.0;
                break;
            case 0x0B:  // Fine Volume Slide Down
                channels[channel_nr].volume -= ((note->effectdata & 0x0f) / 64.0f);
                if(channels[channel_nr].volume < 0.0) channels[channel_nr].volume = 0.0;
                break;
            case 0x0C:  // Cut Sample
                channels[channel_nr].cut_sample = true;
                channels[channel_nr].cut_sample_counter = note->effectdata & 0x0f;
                break;
            case 0x0E:  // Delay
                if((note->effectdata & 0x0f) > 1)
                    thick_counter = thick_counter_start * ((note->effectdata & 0x0f)-1);
                else thick_counter = thick_counter_start;
                break;
            }
        break;

    case 0x0F:      // SetSpeed

        if(note->effectdata != 0)
        {
            if(note->effectdata < 32)
                speed = note->effectdata;
            else
                bpm = note->effectdata;
            SetSongSpeed(bpm, speed);
        }
        break;

    default:
        break;
    }

    if(note_attack && !channels[channel_nr].mute)
    {
        //if(channels[channel_nr].volume_visual < channels[channel_nr].volume)
        channels[channel_nr].volume_visual = channels[channel_nr].volume;
    }
}

float MODClass::CalcFrequCounterStart(int period)
{
    return period / 81.0f;
}

void MODClass::CalcNextSamples(float *samples)
{
    samples[0] = 0.0f;
    samples[1] = 0.0f;

    for(int i=0; i<mod_channel_count; i++)
    {
        int cp = CHANNEL_PAN[i];
        int cpi = CHANNEL_PAN_INV[i];

        if(channels[i].play)
        {
            float* sample_data = channels[i].sample_data;
            if(sample_data != NULL)
            {
                if(!channels[i].mute)
                {
                    samples[cp] += sample_data[channels[i].sample_pos] * channels[i].volume;
                    samples[cpi] += sample_data[channels[i].sample_pos] * channels[i].volume * channel_pan;
                    if(scope_enable)
                    {
                        scope_buffer[scope_buffer_pos] = sample_data[channels[i].sample_pos] * channels[i].volume;
                        scope_buffer_pos++;
                    }
                }
                else if(scope_enable)
                {
                    scope_buffer[scope_buffer_pos] = 0.0;
                    scope_buffer_pos++;
                }

                if(channels[i].frequ_counter_start > 0.0)
                {
                    channels[i].frequ_counter -= 1.0;
                    if(channels[i].frequ_counter < 0.0)
                    {
                        channels[i].frequ_counter += channels[i].frequ_counter_start;
                        channels[i].sample_pos++;

                        if(channels[i].loop_enable)
                        {
							if((channels[i].sample_pos == (channels[i].loop_start + channels[i].loop_length)) || (channels[i].sample_pos == channels[i].sample_length))
                            {
                                channels[i].sample_pos = channels[i].loop_start;
                            }
                        }
                        else
                        {
                            if(channels[i].sample_pos == channels[i].sample_length)
                            {
                                channels[i].sample_pos = 2;
                                channels[i].play = false;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if(scope_enable)
            {
                scope_buffer[scope_buffer_pos] = 0.0;
                scope_buffer_pos++;
            }
        }
    }

    // simply mix only 8bit
    samples[0] /= mod_channel_count;
    samples[1] /= mod_channel_count;

    if(filter_is_enable)
    {
        float s0 = (float)samples[0];
        float s1 = (float)samples[1];

        s0 = lp_filterL->update(s0);
        if(s0>1.0) s0=1.0;
        if(s0<-1.0) s0=-1.0;

        s1 = lp_filterR->update(s1);
        if(s1>1.0) s1=1;
        if(s1<-1.0) s1=-1.0;

        samples[0] = s0;
        samples[1] = s1;
    }

}

void MODClass::CalcNextThick()
{
    for(int i=0; i<mod_channel_count; i++)
    {
        // Arpeggio
        if(channels[i].arpeggio)
        {
            switch(channels[i].arpeggio_counter % 3)
            {
            case 0:
                channels[i].frequ_counter_start = channels[i].arpeggio_frequency0;
                break;
            case 1:
                channels[i].frequ_counter_start = channels[i].arpeggio_frequency1;
                break;
            case 2:
                channels[i].frequ_counter_start = channels[i].arpeggio_frequency2;
                break;
            }
            channels[i].arpeggio_counter++;
        }

        // Slide Up
        if(channels[i].slide_up)
        {
            if(thick_counter > 1)
            {
                channels[i].period -= channels[i].slide_up_value;
                channels[i].frequ_counter_start = CalcFrequCounterStart(channels[i].period);
            }
        }

        // Slide Down
        if(channels[i].slide_down)
        {
            if(thick_counter > 1)
            {
                channels[i].period += channels[i].slide_down_value;
                channels[i].frequ_counter_start = CalcFrequCounterStart(channels[i].period);
            }
        }

        // Slide to Note
        if(channels[i].slide_note)
        {
            if(channels[i].period != channels[i].slide_note_destination_period)
            {
                switch(channels[i].slide_note_direction)
                {
                case 0: // up
                    channels[i].period += channels[i].slide_note_speed;
                    if(channels[i].period > channels[i].slide_note_destination_period)
                        channels[i].period = channels[i].slide_note_destination_period;
                    break;
                case 1: // down
                    channels[i].period -= channels[i].slide_note_speed;
                    if(channels[i].period < channels[i].slide_note_destination_period)
                        channels[i].period = channels[i].slide_note_destination_period;
                    break;
                }
                channels[i].frequ_counter_start = CalcFrequCounterStart(channels[i].period);
            }
            else channels[i].slide_note = false;
        }

        // Vibrato
        if(channels[i].vibrato)
        {
            if(thick_counter > 1)
            {
                signed short vm = ((VIBRATO_TABLE[channels[i].vibrato_pos & 31] * channels[i].vibrato_depth) >> (7)); // Milky (7-2) -> hier dann größerer Effekt
                if ((channels[i].vibrato_pos & 63) > 31) vm = -vm;

                channels[i].vibrato_pos += channels[i].vibrato_speed;
                channels[i].frequ_counter_start = CalcFrequCounterStart(channels[i].period + vm);
            }
        }

        // Tremolo
        if(channels[i].tremolo)
        {

            int vmp = channels[i].tremolo_volume;

            signed short vm = ((VIBRATO_TABLE[channels[i].tremolo_pos & 31] * channels[i].tremolo_depth) >> (6-2)); // Milky (7-2) -> hier dann größerer Effekt
            if ((channels[i].tremolo_pos & 63) > 31) vm = -vm;

            vmp += vm;

            if(vmp < 0) vmp = 0;
            if(vmp > 255) vmp = 255;

            channels[i].tremolo_pos += channels[i].tremolo_speed;
            channels[i].volume = vmp / 255.0;

        }

        // Volume Silde Up and Down
        if(channels[i].volume_slide == 1)
        {
           channels[i].volume += (channels[i].volume_slide_value / 64.0f);
           if(channels[i].volume > 1.0) channels[i].volume = 1.0;
        }
        else if(channels[i].volume_slide == 2)
        {
            channels[i].volume -= (channels[i].volume_slide_value / 64.0f);
            if(channels[i].volume < 0.0) channels[i].volume = 0.0;
        }

        // Retrigger Sample
        if(channels[i].retrigger_sample)
        {
            channels[i].retrigger_sample_counter--;
            if(channels[i].retrigger_sample_counter == 0)
            {
                channels[i].retrigger_sample_counter = channels[i].retrigger_sample_speed;
                channels[i].sample_pos = 0;
            }
        }

        // Cut Sample
        if(channels[i].cut_sample)
        {
            channels[i].cut_sample_counter--;
            if(channels[i].cut_sample_counter == 0)
            {
                channels[i].volume = 0.0;
                channels[i].cut_sample = false;
            }
        }

        // For Extern Visual Effects

        if(channels[i].volume_visual > 0.0)
        {
            channels[i].volume_visual -= volume_visual_counter_value;
            if(channels[i].volume_visual < 0.0)
                channels[i].volume_visual = 0.0;
        }

    }
}

void MODClass::SetSongSpeed(int bpm, int speed)
{
    this->bpm = bpm;
    this->speed = speed;

    time_counter_start = samplerate / (bpm * 0.4);
    time_counter = time_counter_start;

    thick_counter_start = speed;
    thick_counter = thick_counter_start;

    akt_pattern_line_progress = 0.0;
    akt_pattern_line_progress_add = 1.0 / (time_counter_start * thick_counter_start);

    volume_visual_counter_value = (1.0 / (bpm * 0.4)) / VOLUME_VISUAL_DOWN_TIME;
    akt_pattern_line_progress = 0.0f;
}

void MODClass::ModPlay()
{
    if(mod_is_loaded)
    {
        bpm = BPM_DEFAULT;
        speed = SPEED_DEFAULT;

        SetSongSpeed(bpm, speed);

        for(int i=0; i<mod_channel_count; i++)
        {
            channels[i].period = 0;
            channels[i].frequ_counter = 0;
            channels[i].frequ_counter_start = 0;
        }

        song_pos = 0;
        akt_pattern_line = 0;

        pattern_break = false;

        mod_is_playing = true;
    }
}

void MODClass::ModStop()
{
    mod_is_playing = false;
}

void MODClass::ModPause()
{

}

bool ChangePattern;
int  ChangePatternNr;

bool ChangePatternRow;
int  ChangePatternRowNr;

bool MODClass::CheckPatternChange(int *pattern_nr)
{
    if(ChangePattern)
    {
        if(pattern_nr != NULL)
            *pattern_nr = ChangePatternNr;
        ChangePattern = false;
        return true;
    }
    else
        return false;
}

bool MODClass::CheckPatternRowChange(int *row_nr)
{
    if(ChangePatternRow)
    {
        if(row_nr != NULL)
            *row_nr = ChangePatternRowNr;
        ChangePatternRow = false;
        return true;
    }
    else
        return false;
}

NOTE *MODClass::GetPatternRow(int pattern_nr, int pattern_row_nr)
{
    if(pattern_nr < mod_pattern_count && pattern_row_nr < 64)
    {
        return mod_pattern[pattern_nr] + mod_channel_count * pattern_row_nr;
    }
    else
        return NULL;
}

char *MODClass::GetNoteString(int note_nr, int octave_nr)
{
    if((note_nr >= 0) && (note_nr < 12))
    {
        sprintf(note_out_str,"%s%d ",NOTE_STRING[note_nr],octave_nr);
        return (char*)note_out_str;
    }
    return NULL;
}

float MODClass::GetAktPatternProgress()
{
    return akt_pattern_line_progress;
}

void MODClass::SetScopeBuffer(float *buffer)
{
    if(buffer != NULL)
    {
        scope_buffer = buffer;
        scope_enable = true;
    }
}

void MODClass::SetFilterStatus(bool enable)
{
    filter_is_enable = enable;
}

bool MODClass::GetFilterStatus()
{
    return filter_is_enable;
}

void MODClass::SetLowPassCutOffFrequency(float cut_off_freq)
{
    lp_filterL->setCutOffFrequency(cut_off_freq);
    lp_filterR->setCutOffFrequency(cut_off_freq);
}
