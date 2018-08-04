#include "mod_class.h"

MODClass::MODClass(const char *filename)
{
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

void MODClass::MODRead(const char *filename)
{
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
    case _8CHN: case _8FLT:
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
        cout << "Pattern - " << i << endl;
        if(mod_pattern[i] != NULL)
            delete[] mod_pattern[i];

        mod_pattern[i] = new NOTE[mod_pattern_size];
        for(int j=0; j<mod_pattern_size; j++)
        {
            unsigned char note[4];
            file.read((char*)note, 4);

            mod_pattern[i][j].sample_number = note[0] & 0xF0;
            mod_pattern[i][j].sample_number = note[2] >> 4;
            mod_pattern[i][j].period = 0;
            mod_pattern[i][j].effectcommand = 0;
            mod_pattern[i][j].effectdata = 0;
        }

        int channel = 0;
        for(int j=0; j<mod_pattern_size; j++)
        {
            cout << std::hex << (int)mod_pattern[i][j].sample_number << " | ";
            channel++;
            if (channel == mod_channel_count)
            {
                channel = 0;
                cout << endl;
            }
        }
    }

    file.close();
}
