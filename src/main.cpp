//////////////////////////////////////////////////
//                                              //
// ModPlayer                                    //
// by Thorsten Kattanek                         //
//                                              //
// #file: main.cpp                              //
//                                              //
// last change: 10-02-2018                      //
// https://github.com/ThKattanek/mod-player     //
//                                              //
//////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "./mod_class.h"
#include "./level_meter_class.h"

using namespace std;

#define AudioSampleRate 44100

#define FONT_FILENAME "Topaz_a1200_v1.0.ttf"

#ifdef _WIN32
#define AUDIO_BUFFER_SIZE (882)    // 735 bei 44.100 Khz (44100 / 50)
#else
    #define AUDIO_BUFFER_SIZE (882)    // 735 bei 44.100 Khz (44100 / 50)
#endif

void CutBackwartString(char* str, char c);
void AudioMix(void* userdat, Uint8 *stream, int length);
void GetStringFromPatterLine(char *output_str, int pattern_nr, int pattern_row_nr);

MODClass* mod = NULL;

#undef main
int main(int argc, char *argv[])
{
    char* AppPath;
    if(argv[0] != NULL)
    {
        AppPath = new char[strlen(argv[0]) + 1];
        strcpy(AppPath,argv[0]);
        CutBackwartString(AppPath,'\\');
    }
    else AppPath = NULL;

    int screensize_w;
    int screensize_h;

    int window_w;
    int window_h;

    float scale_w;
    float scale_h;

    int font_w, font_h;

    char* filename;

    TTF_Font* font1;
    char str1[1024];

    SDL_Surface* sf_tmp;

    SDL_Texture* tx_mute;
    SDL_Texture* tx[MAX_ROW];

    SDL_Texture* tx_modname = NULL;
    SDL_Texture* tx_songpos = NULL;

    float* scope_buffer;

    for(int i=0; i<MAX_ROW; i++)
        tx[i] = NULL;

    int play_row_nr;
    int play_pattern_nr;

    if(argc > 1)
        filename = argv[1];
    else
        filename = NULL;

    mod = new MODClass(filename, AudioSampleRate);

    int low_pass_cutoff_freq = 24000;   // A1200 ??
    //int low_pass_cutoff_freq = 8000;    // A500 ??
    mod->SetLowPassCutOffFrequency(low_pass_cutoff_freq);

    if(!mod->ModIsLoaded())
    {
        if(filename != NULL)
           switch(mod->GetLoadError())
           {
           case 0x01:
               cerr << "Not open! (" << filename << ") " << endl;
               break;
           case 0x02:
               cerr << "To many Pattern !" << endl;
               break;
           }
        else
            cerr << "Filename ???" << endl;
        return(0);
    }

    // Scopes
    scope_buffer = new float[MAX_CHANNELS * AUDIO_BUFFER_SIZE];
    for(int i=0; i<MAX_CHANNELS * AUDIO_BUFFER_SIZE; i++) scope_buffer[i] = 0.0;

    mod->SetScopeBuffer(scope_buffer);

    if (TTF_Init() < 0)
    {
        cerr << "Error: TTF not initialize." << endl;
        SDL_Quit();
        return(0);
    }

    int FONT_H = 6;

    if(mod->GetModChannelCount() < 9)
    {
        FONT_H = 20;
    }else if(mod->GetModChannelCount() < 17)
    {
        FONT_H = 16;
    }
    else if(mod->GetModChannelCount() < 33)
        {
            FONT_H = 10;
        }

   char font_filename[1024];
#ifdef _WIN32
   sprintf(font_filename,"%s%s",AppPath,FONT_FILENAME);
#endif

#ifdef __linux__
    sprintf(font_filename,"%s%s",DATA_PATH,FONT_FILENAME);
#endif


    font1 = TTF_OpenFont(font_filename,FONT_H);
    if(font1 == NULL)
    {
        cerr << "Error: TTF not open font." << endl;
        SDL_Quit();
        return(0);
    }
    TTF_SetFontHinting (font1, 1);
    TTF_SizeText(font1, "12345678", &font_w, &font_h);
    font_w /= 8;
    font_h = FONT_H;

    screensize_w = (4+12*mod->GetModChannelCount()) * font_w;
    screensize_h = 30 * font_h;

    window_w = screensize_w;
    window_h = screensize_h;

    scale_w = 1.0;
    scale_h = 1.0;

    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "2" );

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    SDL_Window *win = SDL_CreateWindow(filename, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_w, window_h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC) ;
    SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, screensize_w, screensize_h);

    // for Song Title
    SDL_Color color_modname = {220,220,220,0}; // Weiß
    sf_tmp = TTF_RenderText_Blended(font1,mod->GetModName(),color_modname);
    tx_modname = SDL_CreateTextureFromSurface(ren, sf_tmp);
    SDL_FreeSurface(sf_tmp);

    // for mute visusals
    SDL_Color color_mute = {220,0,0,0}; // RED
    sf_tmp = TTF_RenderText_Blended(font1,"x",color_mute);
    tx_mute = SDL_CreateTextureFromSurface(ren, sf_tmp);
    SDL_FreeSurface(sf_tmp);

    LevelMeterClass level_meter(ren,font_w+2,font_h * 6);

    SDL_Color color_fg = {0,50,150,0};

    /// SLD Audio Installieren ///
    SDL_AudioSpec want,have;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = AudioSampleRate;
    want.format = AUDIO_S16;
    want.channels = 2;
    want.samples = AUDIO_BUFFER_SIZE;
    want.callback = AudioMix;
    want.userdata = NULL;

    if( SDL_OpenAudio(&want,&have) > 0 )
    {
        cerr << "<< ERROR: Fehler beim installieren von SDL_Audio" << endl;
    }

    if (want.format != have.format)
    {
        cerr << "Audio Format \"AUDIO_S16 wird nicht unterstuetzt." << endl;
    }

    SDL_PauseAudio(0);

    mod->ModPlay();

    SDL_Event event;
    bool quit = false;

    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if(event.type == SDL_WINDOWEVENT)
            {
                switch(event.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                    window_w = event.window.data1;
                    window_h = event.window.data2;

                    scale_w =  (float)screensize_w / window_w;
                    scale_h =  (float)screensize_h / window_h;

                    break;
                }
            }

            if (event.type == SDL_QUIT){
                quit = true;
            }
            if (event.type == SDL_KEYUP){
                switch(event.key.keysym.sym)
                {
                case SDLK_0:
                    if(mod->GetChannelMute(9))
                        mod->SetChannelMute(9,false);
                    else mod->SetChannelMute(9,true);
                    break;

                case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4: case SDLK_5: case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9:
                    if(mod->GetChannelMute(event.key.keysym.sym - SDLK_1))
                        mod->SetChannelMute(event.key.keysym.sym - SDLK_1,false);
                    else mod->SetChannelMute(event.key.keysym.sym - SDLK_1,true);
                    break;

                case SDLK_f:
                    if(mod->GetFilterStatus())
                        mod->SetFilterStatus(false);
                    else mod->SetFilterStatus(true);
                    break;

                case SDLK_PLUS:
                    low_pass_cutoff_freq += 100;
                    mod->SetLowPassCutOffFrequency(low_pass_cutoff_freq);
                    cout << "LowPass Cut-Off Frequency: " << low_pass_cutoff_freq << endl;
                    break;

                case SDLK_MINUS:
                    low_pass_cutoff_freq -= 100;
                    mod->SetLowPassCutOffFrequency(low_pass_cutoff_freq);
                    cout << "LowPass Cut-Off Frequency: " << low_pass_cutoff_freq << endl;
                    break;

                case SDLK_ESCAPE:
                    break;

                default:
                    break;
                }
            }
            if (event.type == SDL_MOUSEBUTTONDOWN){
                if(event.button.button == SDL_BUTTON_LEFT || event.button.button == SDL_BUTTON_RIGHT)
                {
                    // if push left mousebutton
                   int x = event.button.x * scale_w;
                   int y = event.button.y * scale_h;

                   if(y < font_h * 3)
                   {
                       int i = ((float)x - font_w * 3 - 1) / (font_w * 12);

                       switch(event.button.button)
                       {
                       case SDL_BUTTON_LEFT:

                           if(mod->GetChannelMute(i))
                               mod->SetChannelMute(i,false);
                           else mod->SetChannelMute(i,true);
                           break;

                       case SDL_BUTTON_RIGHT:
                           for(int ch=0; ch<mod->GetModChannelCount(); ch++)
                               mod->SetChannelMute(ch,true);
                           mod->SetChannelMute(i,false);
                           break;
                      }
                   }
                }
            }
        }

        // Check of change playing pattern
        if(mod->CheckPatternChange(&play_pattern_nr))
        {
            for(int i=0; i<MAX_ROW; i++)
            {
                GetStringFromPatterLine(str1, play_pattern_nr, i);

                if(tx[i] != NULL)
                    SDL_DestroyTexture(tx[i]);

                sf_tmp = TTF_RenderText_Blended(font1,str1,color_fg);
                tx[i] = SDL_CreateTextureFromSurface(ren, sf_tmp);
                SDL_FreeSurface(sf_tmp);
            }
        }

        // Chaeck of change playing pattern_row
        if(mod->CheckPatternRowChange(&play_row_nr))
        {
            // Speed[07] BPM[7D] Pos[03/30] Pat[05/33] Row[30/3F] Chn[04/04]
            sprintf(str1,"Speed[%.2x] BPM[%.2x] Pos[%.2x/%.2x] Pat[%.2x/%.2x] Row[%.2x/%.2x]",mod->GetModSpeed(),mod->GetModBPM(),mod->GetModSongPos(),mod->GetModSongLength()-1,play_pattern_nr,mod->GetModPatterCount()-1,play_row_nr,MAX_ROW-1);
            SDL_Color color_sonpos = {220,220,220,0};
            sf_tmp = TTF_RenderText_Blended(font1,str1,color_sonpos);
            if(tx_songpos != NULL)
                SDL_DestroyTexture(tx_songpos);
            tx_songpos = SDL_CreateTextureFromSurface(ren, sf_tmp);
            SDL_FreeSurface(sf_tmp);
        }

        // Alles folgende Rendern in Textur - tex
        SDL_SetRenderTarget(ren,tex);
        SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);

        SDL_SetRenderDrawColor(ren,130,130,200,255);
        SDL_RenderClear(ren);

        int w, h;
        SDL_QueryTexture(tx[0], NULL, NULL, &w, &h);
        SDL_Rect rec1;
        rec1.x = 0;
        rec1.y = 0;
        rec1.w = w;
        rec1.h = h;

        SDL_Rect rec2 = rec1;

        rec2.x =5;
        rec2.y = screensize_h / 2 - font_h / 2;
        rec2.y -= play_row_nr * font_h;
        rec2.y -= int (mod->GetAktPatternProgress() * (font_h));

        for(int i=0; i<MAX_ROW; i++)
        {
            SDL_RenderCopy(ren,tx[i],&rec1,&rec2);
            rec2.y += font_h;
        }

        // HLines
        SDL_SetRenderDrawColor(ren,200,0,0,255);
        int y = screensize_h / 2-font_h / 2;
        SDL_RenderDrawLine(ren,0,y,screensize_w,y);
        SDL_RenderDrawLine(ren,0,y+font_h,screensize_w,y+font_h);

        // Volume Visible
        for(int i=0; i<mod->GetModChannelCount(); i++)
        {
            float vol = mod->GetChannelVolumeVisualValue(i);
            level_meter.Draw(3*font_w+2 + i*12*font_w, screensize_h/2-font_h/2, vol);
        }

        // VLines
        SDL_SetRenderDrawColor(ren,200,200,200,255);
        for(int i=0; i<mod->GetModChannelCount()+1; i++)
        {
            int x = font_w * 3 + i*font_w * 12;
            SDL_RenderDrawLine(ren,x,0,x,screensize_h);
        }

        SDL_SetRenderDrawColor(ren,200,200,200,70);
        for(int i=0; i<mod->GetModChannelCount()+1; i++)
        {
            int x = font_w * 8 + i*font_w * 12;
            SDL_RenderDrawLine(ren,x,0,x,screensize_h);
            x += font_w * 3;
            SDL_RenderDrawLine(ren,x,0,x,screensize_h);
        }

        // Scope Background and size
        rec1.x = rec1.y = 0;
        rec1.w = screensize_w;
        rec1.h = font_h * 3;

        SDL_SetRenderDrawColor(ren,80,80,80,180);
        SDL_RenderFillRect(ren,&rec1);

        SDL_SetRenderDrawColor(ren,0,0,80,255);
        SDL_RenderDrawLine(ren,0,rec1.h,screensize_w,rec1.h);

        // HLINE
        SDL_RenderDrawLine(ren,0,rec1.h,screensize_w,rec1.h);

        // Scopes

        int channels = mod->GetModChannelCount();
        int scope_w = font_w * 12 - 2;
        int scope_h = rec1.h * 0.8;
        int scope_y = rec1.h / 2;

        int scope_x1[channels];
        int scope_y1[channels];
        int scope_x2[channels];
        int scope_y2[channels];

        int idx = 0;
        for(int i=0; i<channels; i++)
        {
            scope_x1[i] = scope_x2[i] = i * font_w * 12 + font_w * 3 + 1;


            scope_y1[i] = scope_buffer[idx++] * scope_h + scope_y ;
        }

        SDL_SetRenderDrawColor(ren,180,180,180,255);

        float t1 = 1.0 / scope_w;

        for(int x=1; x<scope_w; x++)
        {
            int idx = int(t1 * x * AUDIO_BUFFER_SIZE) * channels;
            for(int chn=0; chn < channels; chn++)
            {
                scope_x2[chn]++;
                scope_y2[chn] = scope_buffer[idx] * scope_h + scope_y ;
                SDL_RenderDrawLine(ren,scope_x1[chn],scope_y1[chn],scope_x2[chn],scope_y2[chn]);
                scope_x1[chn] = scope_x2[chn];
                scope_y1[chn] = scope_y2[chn];
                idx++;
            }
        }

        /// Mute
        SDL_QueryTexture(tx_mute, NULL, NULL, &w, &h);
        rec1.w = w;
        rec1.h = h;
        rec1.y = 0;

        for(int i=0; i<channels; i++)
        {
            if(mod->GetChannelMute(i))
            {
                rec1.x = i * font_w * 12 + font_w * 4;
                SDL_RenderCopy(ren, tx_mute,NULL,&rec1);
            }
        }

        // Info Background and size
        rec1.x = 0;
        rec1.y = screensize_h - font_h * 3;
        rec1.w = screensize_w;
        rec1.h = font_h * 3;

        SDL_SetRenderDrawColor(ren,80,80,80,180);
        SDL_RenderFillRect(ren,&rec1);

        // HLINE
        SDL_SetRenderDrawColor(ren,0,0,80,255);
        SDL_RenderDrawLine(ren,0,rec1.y,screensize_w,rec1.y);

        // Draw Songname
        SDL_QueryTexture(tx_modname, NULL, NULL, &w, &h);
        rec1.w = w;
        rec1.h = font_h;
        rec1.x = screensize_w/2 - w/2;
        rec1.y = screensize_h - font_h * 3 + 2;
        SDL_RenderCopy(ren,tx_modname,NULL,&rec1);

        // Song Pos
        SDL_QueryTexture(tx_songpos, NULL, NULL, &w, &h);
        rec1.w = w;
        rec1.h = font_h;
        rec1.x = rec1.x = screensize_w/2 - w/2;
        rec1.y = screensize_h - font_h * 1;
        SDL_RenderCopy(ren,tx_songpos,NULL,&rec1);

        // Wieder auf Bildschirm rendern
        SDL_SetRenderTarget(ren, NULL);

        SDL_SetRenderDrawColor(ren,130,130,200,0);
        SDL_RenderClear(ren);

        SDL_RenderCopy(ren,tex,NULL,NULL);

        SDL_RenderPresent(ren);

        // if not VSYNC then
        // SDL_Delay(1);
    }

    SDL_PauseAudio(1);

    delete mod;
    delete[] scope_buffer;

    TTF_CloseFont(font1);

    SDL_DestroyTexture(tx_mute);

    for(int i=0; i<MAX_ROW; i++)
    {
        SDL_DestroyTexture(tx[i]);
    }

    TTF_Quit();
    SDL_Quit();
    return 0;
}

void CutBackwartString(char* str, char c)
{
    if(str != NULL)
    {
        int len = strlen(str);
        int i=len;
        while(i>-1)
        {
            if(str[i] == c)
            {

                break;
            }
            i--;
        }

        if(i>0 && (i+1) < len)
            str[i+1] = 0;
    }
}

void AudioMix(void* userdat, Uint8 *stream, int length)
{
    if(mod != NULL)
        mod->FillAudioBuffer((signed short*)stream, length/2);
    else
        for(int i=0; i<length; i++) stream[i] = 0;
}

void GetStringFromPatterLine(char* output_str, int pattern_nr, int pattern_row_nr)
{
    NOTE* note = mod->GetPatternRow(pattern_nr, pattern_row_nr);

    char str1[1024];

    output_str[0] = 0;
    str1[0] = 0;

    if(note != NULL)
    {
         //sprintf(str1,"%.2x | ",pattern_row_nr);
         sprintf(str1,"%.2x  ",pattern_row_nr);
         strcat(output_str,str1);
         for(int i=0; i<mod->GetModChannelCount();i++)
         {
             /// NOTE and Octave output
             if(note->note_number < 12)
             {
                 strcat(output_str,mod->GetNoteString(note->note_number,note->oktave_number));
             }
             else
             {
                 strcat(output_str,"... ");
             }

             /// Samplenumber output
             if((int)note->sample_number > 0)
             {
                 sprintf(str1,"%.2d ",(int)note->sample_number);
                 strcat(output_str,str1);
             }
             else
             {
                 strcat(output_str,".. ");
             }

             /// Effectnumber and Effectdata output
             if(note->effectcommand == 0x00 && note->effectdata == 0x00)
             {
                 //strcat(output_str,"... | ");
                 strcat(output_str,"...  ");
             }
             else
             {
                 //sprintf(str1,"%.1x%.2x | ",(int)note->effectcommand,(int)note->effectdata);
                 sprintf(str1,"%.1x%.2x  ",(int)note->effectcommand,(int)note->effectdata);
                 strcat(output_str,str1);
             }
             note++;
         }
    }
}
