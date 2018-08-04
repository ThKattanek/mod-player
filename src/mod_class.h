#ifndef MODCLASS_H
#define MODCLASS_H

#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

enum MOD_TYPE_ID {_MK, _4CHN, _6CHN, _8CHN, _4FLT, _8FLT, _UNKNOWN};

static const char FINETUNETBL[] = {0, 1, 2, 3, 4, 5, 6, 7, -8, -7, -6, -5, -4, -3, -2, -1};

#define MAX_PATTERN 128
#define MAX_ROW 64

struct SAMPLE
{
    char name[23];
    unsigned short  length;
    char            finetune;
    unsigned char   volume;         //  0-64
    unsigned short  loop_start;
    unsigned short  loop_length;
};

struct NOTE
{
    unsigned char   sample_number;  //  8-bit
    unsigned short  period;         //  12-bit
    unsigned char   effectcommand;  //  4-bit
    unsigned char   effectdata;     //  8-bit
};

class MODClass
{

public:
    MODClass(const char *filename);
    ~MODClass();
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
    unsigned short mod_pattern_size;
    unsigned char mod_pattern_count;
    unsigned char mod_channel_count;
    NOTE *mod_pattern[128];
};

#endif // MODCLASS_H
