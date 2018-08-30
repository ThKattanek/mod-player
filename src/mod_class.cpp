#include "mod_class.h"

MODClass::MODClass(const char *filename, int samplerate)
{
    this->samplerate = samplerate;

    this->time_counter_start = samplerate / FPS;
    this->time_counter = time_counter_start;

    channel_pan = 0.3;
    channels = NULL;

    for(int i=0; i<MAX_PATTERN;i++)
    {
        mod_pattern[i] = NULL;
    }

    MODRead(filename);
}

MODClass::~MODClass()
{
    for(int i=0; i<MAX_PATTERN;i++)
    {
       // Pattern delete
       if(mod_pattern != NULL)
           delete[] mod_pattern[i];
    }
}

bool MODClass::ModIsLoaded()
{
    return mod_is_loaded;
}

void MODClass::FillAudioBuffer(signed short *stream, int length)
{
    if(mod_is_playing)
    {
        for(int i=0; i<length; i+=2)
        {
            time_counter--;
            if(time_counter == 0)
            {
                time_counter = time_counter_start;
                thick_counter--;
                if(thick_counter == 0)
                {
                    thick_counter = thick_counter_start;
                    NextLine();
                }
                CalcNextThick();
            }
            CalcNextSamples(stream+i);
        }
    }
}

bool MODClass::MODRead(const char *filename)
{
    MODStop();

    mod_is_loaded = false;

    file.open(filename, ios::in | ios::binary);
    if(!file.is_open())
    {
        cerr << "Modfile \"" << filename << "\" canno't open." << endl;
        return false;
    }

    // MOD Name
    file.read(mod_name,20);
    mod_name[20] = 0;

    cout << "Songname: " << mod_name << endl;

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
        }
    }

    cout << "Modtype: " << mod_type << endl;

    // SAMPLES
    if(mod_type_id == _NST)
        mod_sample_count = 15;
    else mod_sample_count = 31;

    cout << endl;
    cout << mod_sample_count << " samples " << "used in this MOD" << endl;

    for(int i=0; i<mod_sample_count; i++)
    {
        // Sample Name
        file.read(mod_samples[i].name,22);
        mod_samples[i].name[22] = 0;

        cout << "Nr.: " << i+1 << " - " << mod_samples[i].name << endl;

        // Sample length
        file.read((char*)&mod_samples[i].length,2);
        unsigned char *length = (unsigned char*)&mod_samples[i].length;
        unsigned char tmp = length[0];
        length[0] = length[1];
        length[1] = tmp;
        mod_samples[i].length *=2;

        cout << "\tLength: " << mod_samples[i].length << endl;

        // Finetune
        unsigned char finetune;
        file.read((char*)&finetune,1);
        mod_samples[i].finetune = finetune & 0x0f;

        cout << "\tFinetune: " << (int)mod_samples[i].finetune << endl;

        // Volume
        file.read((char*)&mod_samples[i].volume,1);
        cout << "\tVolume: " << (int)mod_samples[i].volume << endl;

        // Loop start
        file.read((char*)&mod_samples[i].loop_start,2);
        unsigned char *loop_start = (unsigned char*)&mod_samples[i].loop_start;
        tmp = loop_start[0];
        loop_start[0] = loop_start[1];
        loop_start[1] = tmp;
        mod_samples[i].loop_start *=2;

        cout << "\tLoopStart: " << mod_samples[i].loop_start << endl;

        // Loop length
        file.read((char*)&mod_samples[i].loop_length,2);
        unsigned char *loop_length = (unsigned char*)&mod_samples[i].loop_length;
        tmp = loop_length[0];
        loop_length[0] = loop_length[1];
        loop_length[1] = tmp;
        mod_samples[i].loop_length *=2;

        cout << "\tLoopLength: " << mod_samples[i].loop_length << endl;

    }

    cout << endl << endl;

    // MOD Song Length
    file.read((char*)&mod_song_length,1);
    cout << "Song Length: " << (int)mod_song_length << endl;

    // MOD Song End Jump
    file.read((char*)&mod_song_end_jump,1);
    cout << "Song End Jump: " << (int)mod_song_end_jump << endl;

    // MOD Song End Jump
    file.read((char*)&mod_pattern_tbl,128);

    cout << endl << "Pattern Table:" << endl;
    int col_count = 0;

    int max_pattern_nr = 0;

    for(int i=0; i<128; i++)
    {
        if(mod_pattern_tbl[i] > max_pattern_nr)
            max_pattern_nr = mod_pattern_tbl[i];

        cout << std::hex  << (int)mod_pattern_tbl[i] << " ";
        col_count++;
        if(col_count == 16)
        {
            cout << endl;
            col_count=0;
        }
    }

    // Pattern Count
    mod_pattern_count = max_pattern_nr+1;
    cout << endl << "Pattern Count: " << (int)mod_pattern_count << endl;

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
    cout << "Chanel Count: " << (int)mod_channel_count << endl;

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

        // Ausgabe des Pattern auf Konsole
        cout << endl << endl << "--- Pattern - [" << i << "] ---" << endl << endl;

        int channel = 0;
        int row = 0;
        for(int j=0; j<mod_pattern_size; j++)
        {
            if(channel == 0) cout << std::hex << setfill('0') << setw(2) << row << "  | ";
            if(mod_pattern[i][j].note_number < 12)
                cout << NOTE_STRING[mod_pattern[i][j].note_number] << (int)mod_pattern[i][j].oktave_number;
            else
                cout << "...";
            cout << " ";

            if((int)mod_pattern[i][j].sample_number > 0)
            {
                cout << std::hex << setfill('0') << setw(2) << (int)mod_pattern[i][j].sample_number << " ";
            }
            else
                cout << ".. ";

            if(mod_pattern[i][j].effectcommand == 0x00 && mod_pattern[i][j].effectdata == 0x00)
                cout << "... | ";
            else
                cout << std::hex << (int)mod_pattern[i][j].effectcommand << setfill('0') << setw(2) << (int)mod_pattern[i][j].effectdata <<  " | ";
            channel++;
            if (channel == mod_channel_count)
            {
                channel = 0;
                row++;
                cout << endl;
            }
        }
    }

    // Sample Data

    for(int i=0; i<mod_sample_count; i++)
    {
        if(mod_samples[i].length > 0)
        {
            if(mod_samples[i].data != NULL)
                delete[] mod_samples[i].data;
            mod_samples[i].data = new char[mod_samples[i].length];
            file.read((char*)mod_samples[i].data,mod_samples[i].length);
        }
    }

    file.close();
    mod_is_loaded = true;
    return true;
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
    if(pattern_break)
    {
        pattern_break = false;

        song_pos++;
        if(song_pos == mod_song_length)
            song_pos = 0;

        int pattern_nr = mod_pattern_tbl[song_pos];
        akt_pattern = mod_pattern[pattern_nr];

        akt_pattern_line = pattern_break_line;

        cout << endl << "Pattern Nr: " << pattern_nr << endl;
    }
    else if(akt_pattern_line == 0)
    {
        int pattern_nr = mod_pattern_tbl[song_pos];
        akt_pattern = mod_pattern[pattern_nr];

        cout << "Pattern Nr: " << pattern_nr << endl;
    }

    cout << std::hex << setfill('0') << setw(2) << akt_pattern_line << "  | ";
    int pattern_line_adr = mod_channel_count * akt_pattern_line;

    int channel_nr = 0;
    for(int i=pattern_line_adr; i<pattern_line_adr + mod_channel_count; i++)
    {
        CalcChannelData(channel_nr, &akt_pattern[i]);
        if(akt_pattern[i].note_number < 12)
            cout << NOTE_STRING[akt_pattern[i].note_number] << (int)akt_pattern[i].oktave_number;
        else
            cout << "...";
        cout << " ";

        if((int)akt_pattern[i].sample_number > 0)
        {
            cout << std::hex << setfill('0') << setw(2) << (int)akt_pattern[i].sample_number << " ";
        }
        else
            cout << ".. ";

        if(akt_pattern[i].effectcommand == 0x00 && akt_pattern[i].effectdata == 0x00)
            cout << "... | ";
        else
            cout << std::hex << "\033[1;31m" << (int)akt_pattern[i].effectcommand << "\033[0m" << setfill('0') << setw(2) << (int)akt_pattern[i].effectdata <<  " | ";
        channel_nr++;
    }
    cout << endl;

    akt_pattern_line++;
    if(akt_pattern_line == 64)
    {
        cout << endl;
        akt_pattern_line = 0;
        song_pos++;
        if(song_pos == mod_song_length)
            song_pos = 0;
    }
}

void MODClass::CalcChannelData(int channel_nr, NOTE *note)
{
    // Arpeggio beenden -> gilt nur für eine Line
    channels[channel_nr].arpeggio = false;

    // Slide Up and Down beenden
    channels[channel_nr].slide_up = false;
    channels[channel_nr].slide_down = false;

    // Volume Slide beenden -> gilt nur für eine Line
    channels[channel_nr].volume_slide = 0;

    // Vibrato beenden -> gilt nur für eine Line
    channels[channel_nr].vibrato = false;

    if(note->sample_number > 0)
    {
        channels[channel_nr].volume = (mod_samples[note->sample_number-1].volume & 0x7f) / 64.0;   // Liniar Volume

        channels[channel_nr].sample_data = mod_samples[note->sample_number-1].data;
        channels[channel_nr].sample_length = mod_samples[note->sample_number-1].length;
        channels[channel_nr].sample_finetune = mod_samples[note->sample_number-1].finetune;
        channels[channel_nr].sample_pos = 2;

        channels[channel_nr].period = PERIOD_TABLE[channels[channel_nr].sample_finetune][channels[channel_nr].note_position_in_table];
        channels[channel_nr].frequ_counter_start = CalcFrequCounterStart(channels[channel_nr].period);

        channels[channel_nr].loop_start = mod_samples[note->sample_number-1].loop_start;
        channels[channel_nr].loop_length = mod_samples[note->sample_number-1].loop_length;
        if(channels[channel_nr].loop_length > 2) channels[channel_nr].loop_enable = true;
        else channels[channel_nr].loop_enable = false;
    }

    // Wenn Period == 0 dann keine Änderung der Periode
    if(note->period > 0)
    {
        channels[channel_nr].play = true;

        channels[channel_nr].note_position_in_table = note->note_postion_in_table;

        channels[channel_nr].period = PERIOD_TABLE[channels[channel_nr].sample_finetune][channels[channel_nr].note_position_in_table];
        channels[channel_nr].frequ_counter_start = CalcFrequCounterStart(channels[channel_nr].period);

        channels[channel_nr].sample_pos = 2;
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

    case 0x04:      // Vibrato
        channels[channel_nr].vibrato = true;

        if((note->effectdata >> 4) != 0)
            channels[channel_nr].vibrato_speed = note->effectdata >> 4;
        if((note->effectdata & 0x0f) != 0)
            channels[channel_nr].vibrato_depth = note->effectdata & 0x0f;
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
    case 0x0C:
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
            switch(note->effectdata >> 4)
            {
            case 0x0A:  // Fine Volume Slide Up
                channels[channel_nr].volume += ((note->effectdata & 0x0f) / 64.0f);
                if(channels[channel_nr].volume > 1.0) channels[channel_nr].volume = 1.0;
                break;
            case 0x0B:  // Fine Volume Slide Down
                channels[channel_nr].volume -= ((note->effectdata & 0x0f) / 64.0f);
                if(channels[channel_nr].volume < 0.0) channels[channel_nr].volume = 0.0;
                break;
            case 0x0E:  // Delay
                if((note->effectdata & 0x0f) > 1)
                    thick_counter = thick_counter_start * ((note->effectdata & 0x0f)-1);
                else thick_counter = thick_counter_start;
                break;
            }
        break;
    case 0x0F:      // SetSpeed
        if(note->effectdata < 32)
        thick_counter_start = note->effectdata;
        break;
    default:
        break;
    }
}

float MODClass::CalcFrequCounterStart(int period)
{
    return period / 81.0f;
}

void MODClass::CalcNextSamples(signed short *samples)
{
    samples[0] = 0;
    samples[1] = 0;

    for(int i=0; i<mod_channel_count; i++)
    {
        int cp = CHANNEL_PAN[i];
        int cpi = CHANNEL_PAN_INV[i];

        if(channels[i].play)
        {
            signed char *sample_data = (signed char*)channels[i].sample_data;
            if(sample_data != NULL)
            {
                //if(i==0)
                {
                    samples[cp] += sample_data[channels[i].sample_pos] * channels[i].volume;
                    samples[cpi] += sample_data[channels[i].sample_pos] * channels[i].volume * channel_pan;
                }

                channels[i].frequ_counter -= 1.0;
                if(channels[i].frequ_counter < 0.0)
                {
                    channels[i].frequ_counter += channels[i].frequ_counter_start;
                    channels[i].sample_pos++;

                    if(channels[i].loop_enable)
                    {
                        if(channels[i].sample_pos == (channels[i].loop_start + channels[i].loop_length))
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

    // simply mix only 8bit
    samples[0] *= (256 / mod_channel_count);
    samples[1] *= (256 / mod_channel_count);
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
                channels[i].frequ_counter_start = (channels[i].period / 81.0);
            }
        }

        // Slide Down
        if(channels[i].slide_down)
        {
            if(thick_counter > 1)
            {
                channels[i].period += channels[i].slide_down_value;
                channels[i].frequ_counter_start = (channels[i].period / 81.0);
            }
        }

        // Vibrato
        if(channels[i].vibrato)
        {
            if(thick_counter > 1)
            {
                signed short vm = ((VIBRATO_TABLE[channels[i].vibrato_pos & 31] * channels[i].vibrato_depth) >> (7)); // Milky (7-2) -> hier dann größerer Effekt
                if ((channels[i].vibrato_pos & 63) > 31) vm = -vm;

                channels[i].vibrato_pos += channels[i].vibrato_speed;
                channels[i].frequ_counter_start = (channels[i].period + vm) / 81.0;
            }
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
    }
}

void MODClass::MODPlay()
{
    thick_counter_start = 5;
    thick_counter = thick_counter_start;

    song_pos = 0;
    akt_pattern_line = 0;

    pattern_break = false;

    NextLine();

    mod_is_playing = true;
}

void MODClass::MODStop()
{
    mod_is_playing = false;
}

void MODClass::MODPause()
{

}
