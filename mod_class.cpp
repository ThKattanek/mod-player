#include "mod_class.h"

MODClass::MODClass(const char *filename)
{
    MODRead(filename);
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

    file.close();
}
