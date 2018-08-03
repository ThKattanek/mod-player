#ifndef MODCLASS_H
#define MODCLASS_H

#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

enum MOD_TYPE_ID {_MK, _4CHN, _6CHN, _8CHN, _4FLT, _8FLT, _UNKNOWN};

static const char FINETUNETBL[] = {0, 1, 2, 3, 4, 5, 6, 7, -8, -7, -6, -5, -4, -3, -2, -1};

struct SAMPLE
{
    char name[23];
    unsigned short length;
    char finetune;
    unsigned char volume;
    unsigned short loop_start;
    unsigned short loop_length;
};

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
    unsigned char mod_type_id;
    int mod_sample_count;
    SAMPLE mod_samples[31];
    unsigned char mod_song_length;
    unsigned char mod_song_end_jump;
    unsigned char mod_pattern_tbl[128];
    unsigned char mod_pattern_count;
    unsigned char mod_channel_count;
};

#endif // MODCLASS_H
