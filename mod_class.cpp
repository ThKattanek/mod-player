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
        mod_type_id = _FLT4;
    else if(!strcmp((const char*)mod_type, "FLT8"))
        mod_type_id = _FLT8;
    else
    {
        mod_type_id = _UNKNOWN;
        mod_is_loaded = false;
        file.close();
        return;
    }

    cout << "Modtype: " << mod_type << endl;

    file.close();
}
