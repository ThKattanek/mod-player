#ifndef MODCLASS_H
#define MODCLASS_H

#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

enum MOD_TYPE_ID {_MK, _4CHN, _6CHN, _8CHN, _FLT4, _FLT8, _UNKNOWN};

class MODClass
{
public:
    MODClass(const char *filename);
private:
    void MODRead(const char *filename);

    bool mod_is_loaded;
    ifstream file;

    // MOD
    char mod_name[21];
    unsigned char mod_type[5];
    int mod_type_id;
};

#endif // MODCLASS_H
