#include "mod_class.h"

MODClass::MODClass(const char *filename, int samplerate)
{
    this->samplerate = samplerate;

    this->time_counter_start = samplerate / FPS;
    this->time_counter = time_counter_start;

    sample_play_enable = false;

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

SAMPLE *MODClass::GetSample(unsigned char sample_number)
{
    if(sample_number < mod_sample_count)
        return &mod_samples[sample_number];
    return NULL;
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
            }

            stream[i] = stream[i+1] = 0;
        }
    }
    else if(sample_play_enable)
        {
            char* sample_data = (char*)mod_samples[sample_play_nr].data;
            for(int i=0; i<length; i+=2)
            {
                if(sample_play_pos < mod_samples[sample_play_nr].length)
                {
                    stream[i] = sample_data[sample_play_pos]*32;
                    stream[i+1] = stream[i];
                    sample_play_pos++;
                }
                else
                {
                    stream[i] = stream[i+1] = 0;
                }
            }

            if(sample_play_pos == mod_samples[sample_play_nr].length)
            {
                sample_play_enable = false;
                sample_play_pos = 0;
            }
        }
        else
        {
            for(int i=0; i<length; i++)
                stream[i] = 0;
        }
}

void MODClass::PlaySample(unsigned char sample_nr)
{
    if(sample_nr > 0 && sample_nr < 32)
    {
        sample_play_nr = sample_nr-1;
        sample_play_pos = 0;
        sample_play_enable = true;

        cout << "Play Sample Number: " << std::dec << setfill('0') << setw(2)  << (int)sample_nr << endl;
    }
}

void MODClass::MODRead(const char *filename)
{
    MODStop();

    file.open(filename, ios::in | ios::binary);
    if(!file.is_open())
    {
        cerr << "Modfile \"" << filename << "\" canno't open." << endl;
        mod_is_loaded = false;
        return;
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
        mod_type_id = _UNKNOWN;
        mod_is_loaded = false;
        file.close();
        return;
    }
    cout << "Modtype: " << mod_type << endl;

    // SAMPLES
    if(mod_type_id == _UNKNOWN)
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
        mod_samples[i].finetune = FINETUNETBL[finetune];

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
    case _MK: case _4CHN: case _4FLT: case _UNKNOWN:
        mod_channel_count = 4;
        break;
    case _6CHN:
        mod_channel_count = 6;
        break;
    case _8CHN: case _8FLT: case _OCTA:
        mod_channel_count = 8;
        break;
    default:
        mod_channel_count = 0;
    }
    cout << "Chanel Count: " << (int)mod_channel_count << endl;

    // Read Pattern Data
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
            //cout << std::dec << setw(4) << (int)mod_pattern[i][j].period << "-";
            if(mod_pattern[i][j].note_number < 12)
                cout << NOTE_STRING[mod_pattern[i][j].note_number] << (int)mod_pattern[i][j].oktave_number;
            else
                cout << "...";
            cout << " ";

            cout << std::hex << setfill('0') << setw(2) << (int)mod_pattern[i][j].sample_number << " ";
            cout << std::hex << (int)mod_pattern[i][j].effectcommand << "-" << setfill('0') << setw(2) << (int)mod_pattern[i][j].effectdata <<  " | ";
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
    if(akt_pattern_line == 0)
    {
        int pattern_nr = mod_pattern_tbl[song_pos];
        akt_pattern = mod_pattern[pattern_nr];

        cout << "Pattern Nr: " << pattern_nr << endl;
    }

    cout << std::hex << setfill('0') << setw(2) << akt_pattern_line << "  | ";
    int pattern_line_adr = mod_channel_count * akt_pattern_line;

    for(int i=pattern_line_adr; i<pattern_line_adr + mod_channel_count; i++)
    {
        if(akt_pattern[i].note_number < 12)
            cout << NOTE_STRING[akt_pattern[i].note_number] << (int)akt_pattern[i].oktave_number;
        else
            cout << "...";
        cout << " ";

        cout << std::hex << setfill('0') << setw(2) << (int)akt_pattern[i].sample_number << " ";
        cout << std::hex << (int)akt_pattern[i].effectcommand << "-" << setfill('0') << setw(2) << (int)akt_pattern[i].effectdata <<  " | ";
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

void MODClass::MODPlay()
{
    thick_counter_start = 6;
    thick_counter = thick_counter_start;

    song_pos = 0;
    akt_pattern_line = 0;

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
