#ifndef MODCLASS_H
#define MODCLASS_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>

using namespace std;

enum MOD_TYPE_ID {_MK, _4CHN, _6CHN, _8CHN, _4FLT, _8FLT,_OCTA, _UNKNOWN};

static const char FINETUNETBL[] = {0, 1, 2, 3, 4, 5, 6, 7, -8, -7, -6, -5, -4, -3, -2, -1};
static const int  CHANNEL_PAN[8] = {0,1,1,0,0,1,1,0};   // 0=links, 1=rechts
static const int  CHANNEL_PAN_INV[8] = {1,0,0,1,1,0,0,1};   // 0=links, 1=rechts

// C-C#-D-D#-E-F-F#-G-G#-A-A#-H

static const char* NOTE_STRING[12] = {"C-","C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-"};

static const unsigned short PERIOD_TABLE[16][60] = {                        // thanks ByteRaver for this table ;)
   {1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016, 960 , 906,  // Finetune +0
    856 , 808 , 762 , 720 , 678 , 640 , 604 , 570 , 538 , 508 , 480 , 453,
    428 , 404 , 381 , 360 , 339 , 320 , 302 , 285 , 269 , 254 , 240 , 226,
    214 , 202 , 190 , 180 , 170 , 160 , 151 , 143 , 135 , 127 , 120 , 113,
    107 , 101 , 95  , 90  , 85  , 80  , 75  , 71  , 67  , 63  , 60  , 56 },
   {1700, 1604, 1514, 1430, 1348, 1274, 1202, 1134, 1070, 1010, 954 , 900,  // Finetune +1
    850 , 802 , 757 , 715 , 674 , 637 , 601 , 567 , 535 , 505 , 477 , 450,
    425 , 401 , 379 , 357 , 337 , 318 , 300 , 284 , 268 , 253 , 239 , 225,
    213 , 201 , 189 , 179 , 169 , 159 , 150 , 142 , 134 , 126 , 119 , 113,
    106 , 100 , 94  , 89  , 84  , 79  , 75  , 71  , 67  , 63  , 59  , 56 },
   {1688, 1592, 1504, 1418, 1340, 1264, 1194, 1126, 1064, 1004, 948 , 894,  // Finetune +2
    844 , 796 , 752 , 709 , 670 , 632 , 597 , 563 , 532 , 502 , 474 , 447,
    422 , 398 , 376 , 355 , 335 , 316 , 298 , 282 , 266 , 251 , 237 , 224,
    211 , 199 , 188 , 177 , 167 , 158 , 149 , 141 , 133 , 125 , 118 , 112,
    105 , 99  , 94  , 88  , 83  , 79  , 74  , 70  , 66  , 62  , 59  , 56 },
   {1676, 1582, 1492, 1408, 1330, 1256, 1184, 1118, 1056, 996 , 940 , 888,  // Finetune +3
    838 , 791 , 746 , 704 , 665 , 628 , 592 , 559 , 528 , 498 , 470 , 444,
    419 , 395 , 373 , 352 , 332 , 314 , 296 , 280 , 264 , 249 , 235 , 222,
    209 , 198 , 187 , 176 , 166 , 157 , 148 , 140 , 132 , 125 , 118 , 111,
    104 , 99  , 93  , 88  , 83  , 78  , 74  , 70  , 66  , 62  , 59  , 55 },
   {1664, 1570, 1482, 1398, 1320, 1246, 1176, 1110, 1048, 990 , 934 , 882,  // Finetune +4
    832 , 785 , 741 , 699 , 660 , 623 , 588 , 555 , 524 , 495 , 467 , 441,
    416 , 392 , 370 , 350 , 330 , 312 , 294 , 278 , 262 , 247 , 233 , 220,
    208 , 196 , 185 , 175 , 165 , 156 , 147 , 139 , 131 , 124 , 117 , 110,
    104 , 98  , 92  , 87  , 82  , 78  , 73  , 69  , 65  , 62  , 58  , 55 },
   {1652, 1558, 1472, 1388, 1310, 1238, 1168, 1102, 1040, 982 , 926 , 874,  // Finetune +5
    826 , 779 , 736 , 694 , 655 , 619 , 584 , 551 , 520 , 491 , 463 , 437,
    413 , 390 , 368 , 347 , 328 , 309 , 292 , 276 , 260 , 245 , 232 , 219,
    206 , 195 , 184 , 174 , 164 , 155 , 146 , 138 , 130 , 123 , 116 , 109,
    103 , 97  , 92  , 87  , 82  , 77  , 73  , 69  , 65  , 61  , 58  , 54 },
   {1640, 1548, 1460, 1378, 1302, 1228, 1160, 1094, 1032, 974 , 920 , 868,  // Finetune +6
    820 , 774 , 730 , 689 , 651 , 614 , 580 , 547 , 516 , 487 , 460 , 434,
    410 , 387 , 365 , 345 , 325 , 307 , 290 , 274 , 258 , 244 , 230 , 217,
    205 , 193 , 183 , 172 , 163 , 154 , 145 , 137 , 129 , 122 , 115 , 109,
    102 , 96  , 91  , 86  , 81  , 77  , 72  , 68  , 64  , 61  , 57  , 54 },
   {1628, 1536, 1450, 1368, 1292, 1220, 1150, 1086, 1026, 968 , 914 , 862,  // Finetune +7
    814 , 768 , 725 , 684 , 646 , 610 , 575 , 543 , 513 , 484 , 457 , 431,
    407 , 384 , 363 , 342 , 323 , 305 , 288 , 272 , 256 , 242 , 228 , 216,
    204 , 192 , 181 , 171 , 161 , 152 , 144 , 136 , 128 , 121 , 114 , 108,
    102 , 96  , 90  , 85  , 80  , 76  , 72  , 68  , 64  , 60  , 57  , 54 },
   {1814, 1712, 1616, 1524, 1440, 1356, 1280, 1208, 1140, 1076, 1016, 960,  // Finetune -8
    907 , 856 , 808 , 762 , 720 , 678 , 640 , 604 , 570 , 538 , 508 , 480,
    453 , 428 , 404 , 381 , 360 , 339 , 320 , 302 , 285 , 269 , 254 , 240,
    226 , 214 , 202 , 190 , 180 , 170 , 160 , 151 , 143 , 135 , 127 , 120,
    113 , 107 , 101 , 95  , 90  , 85  , 80  , 75  , 71  , 67  , 63  , 60 },
   {1800, 1700, 1604, 1514, 1430, 1350, 1272, 1202, 1134, 1070, 1010, 954,  // Finetune -7
    900 , 850 , 802 , 757 , 715 , 675 , 636 , 601 , 567 , 535 , 505 , 477,
    450 , 425 , 401 , 379 , 357 , 337 , 318 , 300 , 284 , 268 , 253 , 238,
    225 , 212 , 200 , 189 , 179 , 169 , 159 , 150 , 142 , 134 , 126 , 119,
    112 , 106 , 100 , 94  , 89  , 84  , 79  , 75  , 71  , 67  , 63  , 59 },
   {1788, 1688, 1592, 1504, 1418, 1340, 1264, 1194, 1126, 1064, 1004, 948,  // Finetune -6
    894 , 844 , 796 , 752 , 709 , 670 , 632 , 597 , 563 , 532 , 502 , 474,
    447 , 422 , 398 , 376 , 355 , 335 , 316 , 298 , 282 , 266 , 251 , 237,
    223 , 211 , 199 , 188 , 177 , 167 , 158 , 149 , 141 , 133 , 125 , 118,
    111 , 105 , 99  , 94  , 88  , 83  , 79  , 74  , 70  , 66  , 62  , 59 },
   {1774, 1676, 1582, 1492, 1408, 1330, 1256, 1184, 1118, 1056, 996 , 940,  // Finetune -5
    887 , 838 , 791 , 746 , 704 , 665 , 628 , 592 , 559 , 528 , 498 , 470,
    444 , 419 , 395 , 373 , 352 , 332 , 314 , 296 , 280 , 264 , 249 , 235,
    222 , 209 , 198 , 187 , 176 , 166 , 157 , 148 , 140 , 132 , 125 , 118,
    111 , 104 , 99  , 93  , 88  , 83  , 78  , 74  , 70  , 66  , 62  , 59 },
   {1762, 1664, 1570, 1482, 1398, 1320, 1246, 1176, 1110, 1048, 988 , 934,  // Finetune -4
    881 , 832 , 785 , 741 , 699 , 660 , 623 , 588 , 555 , 524 , 494 , 467,
    441 , 416 , 392 , 370 , 350 , 330 , 312 , 294 , 278 , 262 , 247 , 233,
    220 , 208 , 196 , 185 , 175 , 165 , 156 , 147 , 139 , 131 , 123 , 117,
    110 , 104 , 98  , 92  , 87  , 82  , 78  , 73  , 69  , 65  , 61  , 58 },
   {1750, 1652, 1558, 1472, 1388, 1310, 1238, 1168, 1102, 1040, 982 , 926,  // Finetune -3
    875 , 826 , 779 , 736 , 694 , 655 , 619 , 584 , 551 , 520 , 491 , 463,
    437 , 413 , 390 , 368 , 347 , 328 , 309 , 292 , 276 , 260 , 245 , 232,
    219 , 206 , 195 , 184 , 174 , 164 , 155 , 146 , 138 , 130 , 123 , 116,
    109 , 103 , 97  , 92  , 87  , 82  , 77  , 73  , 69  , 65  , 61  , 58 },
   {1736, 1640, 1548, 1460, 1378, 1302, 1228, 1160, 1094, 1032, 974 , 920,  // Finetune -2
    868 , 820 , 774 , 730 , 689 , 651 , 614 , 580 , 547 , 516 , 487 , 460,
    434 , 410 , 387 , 365 , 345 , 325 , 307 , 290 , 274 , 258 , 244 , 230,
    217 , 205 , 193 , 183 , 172 , 163 , 154 , 145 , 137 , 129 , 122 , 115,
    108 , 102 , 96  , 91  , 86  , 81  , 77  , 72  , 68  , 64  , 61  , 57 },
   {1724, 1628, 1536, 1450, 1368, 1292, 1220, 1150, 1086, 1026, 968 , 914,  // Finetune -1
    862 , 814 , 768 , 725 , 684 , 646 , 610 , 575 , 543 , 513 , 484 , 457,
    431 , 407 , 384 , 363 , 342 , 323 , 305 , 288 , 272 , 256 , 242 , 228,
    216 , 203 , 192 , 181 , 171 , 161 , 152 , 144 , 136 , 128 , 121 , 114,
    108 , 101 , 96  , 90  , 85  , 80  , 76  , 72  , 68  , 64  , 60  , 57 }};

#define MAX_PATTERN 128
#define MAX_ROW 64

#define PAL
//#define NTSC

#define PAL_FPS 50
#define NTSC_FPS 60

#define PAL_CLOCK 7093789.2
#define NTSC_CLOCK 7159090.5

#ifdef PAL
    #define FPS PAL_FPS
    #define CLOCK PAL_CLOCK
#else
        #ifdef NTSC
            #define FPS NTSC_FPS
            #define CLOCK NTSC_CLOCK
    #endif
#endif

struct SAMPLE
{
    char name[23];
    unsigned short  length;
    char            finetune;
    unsigned char   volume;         //  0-64
    unsigned short  loop_start;
    unsigned short  loop_length;
    void*           data = NULL;
};

struct NOTE
{
    unsigned char   sample_number;  //  8-bit
    unsigned short  period;         //  12-bit
    unsigned char   effectcommand;  //  4-bit
    unsigned char   effectdata;     //  8-bit

    // Ermittelt aus period
    int             note_postion_in_table;  // 0-59 0=octave2, 12=octave3, 24=octave4, 36=octave5, 48=octave6
    unsigned char   note_number;            // 0-11 C,C#,D
    unsigned char   oktave_number;          // 1-5
};

struct CHANNEL
{
    bool    play = false;
    float   volume = 1.0f;
    int     volume_slide = 0;   // 0=stop, 1=up, 2=down
    int     volume_slide_value = 0;
    int     note_position_in_table = 0;
    int     period = 0;
    float   frequency = 0.0;
    float   frequ_counter = 0.0;
    bool    loop_enable = false;
    int     loop_start = 0;
    int     loop_length = 0;
    void*   sample_data = NULL;
    int     sample_length = 0;
    int     sample_pos = 2;
    bool    arpeggio = false;
    int     arpeggio_counter = 0;
    float   arpeggio_frequency0 = 0.0;
    float   arpeggio_frequency1 = 0.0;
    float   arpeggio_frequency2 = 0.0;
    bool    slide_up = false;
    int     slide_up_value = 0;
    bool    slide_down = false;
    int     slide_down_value = 0;
};

class MODClass
{

public:
    MODClass(const char *filename, int samplerate);
    ~MODClass();

    ///
    /// \brief GetSample
    /// \param sample_number - number of sample in the modfile
    /// \return a pointer of a SAMPLE object
    ///
    SAMPLE* GetSample(unsigned char sample_number);

    ///
    /// \brief FillAudioBuffer - Fill the AudioStream (16Bit/Stereo) with audio data
    /// \param stream
    /// \param length
    ///
    void FillAudioBuffer(signed short* stream, int length);

    ///
    /// \brief PlaySample - Play a single sample
    /// \param sample_nr - number of playing sample (1-31)
    ///
    void PlaySample(unsigned char sample_nr);

    void MODPlay(void);
    void MODStop(void);
    void MODPause(void);

private:
    ///
    /// \brief MODRead - reading a modfile
    /// \param filename - filename to the modfile
    ///
    void MODRead(const char *filename);

    ///
    /// \brief NoteConvert - convert the note from period to note and octave or note an octave to note
    /// \param note - pointer to a NOTE object
    /// \param direction - false = period to note | true = note to period
    ///
    void NoteConvert(NOTE* note, bool direction);

    void NextLine(void);
    void CalcChannelData(int channel_nr, NOTE* note);

    void CalcNextSamples(signed short *samples);
    void CalcNextThick();

    bool mod_is_loaded;
    ifstream file;

    int samplerate;

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

    // MOD Playing
    bool    mod_is_playing;

    int     time_counter_start;   // Samplerate / 50 for PAL or 60 for NTSC
    int     time_counter;
    int     thick_counter_start;
    int     thick_counter;

    int     song_pos;
    NOTE*   akt_pattern;
    int     akt_pattern_line;

    CHANNEL *channels;

    float   channel_pan;        // 0.0 = strict channel sepeparation, left and right
                                // 1.0 = left and right channel is equal
    bool    pattern_break;
    int     pattern_break_line;

    // Single Sample Play
    bool sample_play_enable;
    int sample_play_nr;
    int sample_play_pos;
};

#endif // MODCLASS_H
