//////////////////////////////////////////////////
//                                              //
// MODClass                                     //
// by Thorsten Kattanek                         //
//                                              //
// #file: mod_class.h                           //
//                                              //
// last change: 09-25-2018                      //
// https://github.com/ThKattanek/mod-player     //
//                                              //
//////////////////////////////////////////////////

#ifndef MODCLASS_H
#define MODCLASS_H

#include "./low_pass_filter.h"

#include <fstream>
#include <cstring>

using namespace std;

// Configure
#define MAX_CHANNELS 64
#define MAX_PATTERN 128
#define MAX_ROW 64

#define BPM_DEFAULT 125
#define SPEED_DEFAULT 6

#define VOLUME_VISUAL_DOWN_TIME 0.7

enum MOD_TYPE_ID {_MK, _4CHN, _6CHN, _8CHN, _4FLT, _8FLT,_OCTA, _CH, _NST};

static const int  CHANNEL_PAN[32] = {0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0};   // 0=links, 1=rechts
static const int  CHANNEL_PAN_INV[32] = {1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1};   // 0=links, 1=rechts

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

static const signed short VIBRATO_TABLE[32] = {0,24,49,74,97,120,141,161,180,197,212,224,235,244,250,253,255,253,250,244,235,224,212,197,180,161,141,120,97,74,49,24};

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
    bool    mute = false;
    int     note_position_in_table = 0;
    int     period = 0;
    float   frequ_counter_start = 0.0;
    float   frequ_counter = 0.0;
    bool    loop_enable = false;
    int     loop_start = 0;
    int     loop_length = 0;
    void*   sample_data = NULL;
    int     sample_length = 0;
    int     sample_pos = 2;
    int     sample_finetune = 0;
    bool    arpeggio = false;
    int     arpeggio_counter = 0;
    float   arpeggio_frequency0 = 0.0;
    float   arpeggio_frequency1 = 0.0;
    float   arpeggio_frequency2 = 0.0;
    bool    slide_up = false;
    int     slide_up_value = 0;
    bool    slide_down = false;
    int     slide_down_value = 0;
    bool    slide_note = false;
    int     slide_note_speed = 0;
    int    slide_note_direction = 0;
    int     slide_note_destination_period = 0;
    bool    cut_sample = false;
    int     cut_sample_counter = 0;
    bool    vibrato = false;
    int     vibrato_speed = 0;
    int     vibrato_depth = 0;
    int     vibrato_pos = 0;
    bool    tremolo = false;
    int     tremolo_speed = 0;
    int     tremolo_depth = 0;
    int     tremolo_pos = 0;
    int     tremolo_volume = 0;
    int     tremolo_final_volume = 0;
    bool    retrigger_sample = false;
    int     retrigger_sample_speed = 0;
    int     retrigger_sample_counter = 0;
    float   volume_visual = 0.0;
};

class MODClass
{

public:
    MODClass(const char *filename, int samplerate);
    ~MODClass();

    ///
    /// \brief ModIsLoaded - Check of mod is loaded
    /// \return mod load status as bool - true = Mod is correct loaded, false = Mod is not loaded
    ///
    bool ModIsLoaded(void);

    ///
    /// \brief GetModName - Get the name from loaded modul
    /// \return mod name as const char*
    ///
    const char* GetModName();

    ///
    /// \brief GetModType - Get mod typ as string
    /// \return mod type as const char*
    ///
    const char* GetModType();

    ///
    /// \brief GetModSampleCount - Get count of samples in this mod
    /// \return sample count as int
    ///
    int GetModSampleCount();

    ///
    /// \brief GetModSongLength - Get song lenght from this mod
    /// \return song length as int
    ///
    int GetModSongLength();

    ///
    /// \brief GetModSongEndJump - get end jump position of mod
    /// \return jump end posistion as int
    ///
    int GetModSongEndJump();

    ///
    /// \brief GetModPatternTable - Get the song pattern Table with max. 128 entries
    /// \return Pointer to the Pattern Table as const char*
    ///
    const unsigned char* GetModPatternTable();

    ///
    /// \brief GetModPatterCount - Get max. used pattern
    /// \return pattern count as int
    ///
    int GetModPatterCount();

    ///
    /// \brief GetModChannelCount - Get sound channels for this mod
    /// \return Count of used sound channels as int
    ///
    int GetModChannelCount();

    ///
    /// \brief GetModSample Get the SAMPLE object from this mod
    /// \param sample_nr Sample Number (1-15) or (1-31)
    /// \return Pointer SAMPLE
    ///
    SAMPLE* GetModSample(int sample_nr);

    ///
    /// \brief GetChannelVolumeVisualValue - Return a value from 0.0 to 1.0 for visual painting
    /// \param channel_nr - channel number
    /// \return volume as float
    ///
    float GetChannelVolumeVisualValue(int channel_nr);

    ///
    /// \brief SetChannelMute
    /// \param channel_nr
    /// \param enable
    ///
    void SetChannelMute(int channel_nr, bool enable);

    ///
    /// \brief GetChannelMute
    /// \param channel_nr
    /// \return
    ///
    bool GetChannelMute(int channel_nr);

    ///
    /// \brief FillAudioBuffer - Fill the AudioStream (16Bit/Stereo) with audio data
    /// \param stream
    /// \param length
    ///
    void FillAudioBuffer(signed short* stream, int length);

    ///
    /// \brief MODRead - reading a modfile
    /// \param filename - filename to the modfile
    ///
    bool LoadMod(const char *filename);

    ///
    /// \brief GetLoadError
    /// \return Errorcode - 0x00 - no error, 0x01 - mod not open, 0x02 - to many Pattern in this mod
    ///
    int  GetLoadError();

    void ModPlay(void);
    void ModStop(void);
    void ModPause(void);

    bool CheckPatternChange(int* pattern_nr);
    bool CheckPatternRowChange(int* row_nr);

    NOTE* GetPatternRow(int pattern_nr, int pattern_row_nr);

    char *GetNoteString(int note_nr, int octave_nr);

    float GetAktPatternProgress();

    void SetScopeBuffer(float* buffer);

private:
    ///
    /// \brief NoteConvert - convert the 'note from period to note' and 'octave or note an octave to note'
    /// \param note - pointer to a NOTE object
    /// \param direction - false = period to note | true = note to period
    ///
    void NoteConvert(NOTE* note, bool direction);

    void NextLine(void);
    void CalcChannelData(int channel_nr, NOTE* note);
    float CalcFrequCounterStart(int period);
    void CalcNextSamples(signed short *samples);
    void CalcNextThick();
    void SetSongSpeed(int bpm, int speed);

    bool mod_is_loaded;
    int  mod_load_error;
    ifstream file;

    int samplerate;

    LowPassFilter* lp_filterL;
    LowPassFilter* lp_filterR;

    // Eventhandling
    bool ChangePattern;
    int  ChangePatternNr;

    bool ChangePatternRow;
    int  ChangePatternRowNr;

    // MOD
    char mod_name[21];
    char mod_type[5];
    unsigned char mod_type_id;
    int mod_sample_count;
    SAMPLE mod_samples[31];
    unsigned char mod_song_length;
    unsigned char mod_song_end_jump;
    unsigned char mod_pattern_tbl[128];
    unsigned short mod_pattern_size;
    unsigned char mod_pattern_count;
    unsigned char mod_channel_count;
    NOTE *mod_pattern[MAX_PATTERN];

    // MOD Playing
    bool    mod_is_playing;

    int     bpm;
    float   time_counter_start;
    float   time_counter;

    int     speed;
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

    bool    position_jump;
    int     position_jump_pos;

    bool    set_song_speed;
    int     set_song_speed_var;

    // For Extern Visuals
    char    note_out_str[5];
    float   volume_visual_counter_value;
    float   akt_pattern_line_progress;  // 0.0f = 0% 1.0f = 100%
    float   akt_pattern_line_progress_add;   
    float   scope_enable;       // true wenn der
    float*  scope_buffer;       // Samplesanordnung bei eienem 4 ChannelMod -> 0,1,2,3,0,1,2,3,0,1,2,3 ...
    int     scope_buffer_pos;   //
};

#endif // MODCLASS_H
