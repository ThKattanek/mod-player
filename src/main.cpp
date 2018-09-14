#include <iostream>
#include <iomanip>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "./mod_class.h"
#include "./level_meter_class.h"

using namespace std;

#define AudioSampleRate 44100

#define FONT_FILENAME "mononoki-Regular.ttf"
#define FONT_HEIGHT 12

#ifdef _WIN32
    #define AudioPufferSize (882*1)    // 882 bei 44.100 Khz
#else
    #define AudioPufferSize (882*2)    // 882 bei 44.100 Khz
#endif

void AudioMix(void* userdat, Uint8 *stream, int length);
void GetStringFromPatterLine(char *output_str, int pattern_nr, int pattern_row_nr);

MODClass* mod = NULL;

#undef main
int main(int argc, char *argv[])
{
    int screensize_w = 370;
    int screensize_h = 300;

    char* filename;

    TTF_Font* font1;
    char str1[1024];

    SDL_Texture* tx[MAX_ROW];

    for(int i=0; i<MAX_ROW; i++)
        tx[i] = NULL;

    int play_row_nr;
    int play_pattern_nr;

    if(argc > 1)
        filename = argv[1];
    else
        filename = NULL;

    mod = new MODClass(filename, AudioSampleRate);

    if(!mod->ModIsLoaded())
    {
        if(filename != NULL)
            cerr << "Mod [" << filename <<  "] cannot loaded." << endl;
        else
            cerr << "Mod cannot loaded." << endl;
        return(0);
    }

    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);

    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "2" );

    SDL_Window *win = SDL_CreateWindow(filename, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screensize_w, screensize_h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC) ;
    SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, screensize_w, screensize_h);

    if (TTF_Init() < 0)
    {
        cerr << "Error: TTF not initialize." << endl;
        SDL_Quit();
        return(0);
    }

    font1 = TTF_OpenFont(FONT_FILENAME,FONT_HEIGHT);
    if(font1 == NULL)
    {
        cerr << "Error: TTF not open font." << endl;
        SDL_Quit();
        return(0);
    }
    TTF_SetFontHinting (font1, 1);

    // Find Font Dimensions
    SDL_Color color{0,0,0,0};
    SDL_Surface* surface = TTF_RenderText_Blended(font1,"1234567890",color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(ren, surface);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);



    LevelMeterClass level_meter(ren,12,120);

    //SDL_Color color_fg = {0,50,150,0};
    SDL_Color color_fg = {255,255,255,0};

    /// SLD Audio Installieren ///
    SDL_AudioSpec want,have;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = AudioSampleRate;
    want.format = AUDIO_S16;
    want.channels = 2;
    want.samples = AudioPufferSize;
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
            if (event.type == SDL_QUIT){
                quit = true;
            }
            if (event.type == SDL_KEYUP){
                switch(event.key.keysym.sym)
                {
                case SDLK_PLUS:
                    break;

                case SDLK_MINUS:
                    break;

                case SDLK_ESCAPE:
                    break;

                default:
                    break;
                }
            }
            if (event.type == SDL_MOUSEBUTTONDOWN){
               // quit = true;
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

                SDL_Surface* sf = TTF_RenderText_Blended(font1,str1,color_fg);
                tx[i] = SDL_CreateTextureFromSurface(ren, sf);
                SDL_FreeSurface(sf);
            }
        }

        // Chaeck of change playing pattern_row
        if(mod->CheckPatternRowChange(&play_row_nr))
        {
            //GetStringFromPatterLine(str1, play_pattern_nr, play_row_nr);
        }

        // Alles folgende Rendern in Textur - tex
        SDL_SetRenderTarget(ren,tex);

        //SDL_SetRenderDrawColor(ren,130,130,200,0);
        SDL_SetRenderDrawColor(ren,0,0,0,0);
        SDL_RenderClear(ren);

        SDL_SetRenderDrawColor(ren,200,0,0,0);

        int w, h;
        SDL_QueryTexture(tx[0], NULL, NULL, &w, &h);
        SDL_Rect rec1;
        rec1.x = 0;
        rec1.y = 0;
        rec1.w = w;
        rec1.h = h;

        SDL_Rect rec2 = rec1;

        rec2.x =5;
        rec2.y = screensize_h / 2 - FONT_HEIGHT / 2;
        rec2.y -= play_row_nr * FONT_HEIGHT;

        for(int i=0; i<MAX_ROW; i++)
        {
            SDL_RenderCopy(ren,tx[i],&rec1,&rec2);
            rec2.y += FONT_HEIGHT;
        }

        int y = screensize_h / 2-FONT_HEIGHT / 2;

        SDL_RenderDrawLine(ren,0,y,screensize_w,y);
        SDL_RenderDrawLine(ren,0,y+FONT_HEIGHT,screensize_w,y+FONT_HEIGHT);

        // Volume Visible
        for(int i=0; i<mod->GetModChannelCount(); i++)
        {
            float vol = mod->GetChannelVolumeVisualValue(i);
            level_meter.Draw(i*84+38, screensize_h/2-FONT_HEIGHT/2, vol);
            //level_meter.Draw(i*60+38, screensize_h/2-FONT_HEIGHT/2, vol);
        }

        // Wieder auf Bildschirm rendern
        SDL_SetRenderTarget(ren, NULL);

        SDL_SetRenderDrawColor(ren,130,130,200,0);
        SDL_RenderClear(ren);

        SDL_RenderCopy(ren,tex,NULL,NULL);

        SDL_RenderPresent(ren);

        // if not VSYNC then
        //SDL_Delay(1);
    }

    delete mod;

    TTF_CloseFont(font1);

    for(int i=0; i<MAX_ROW; i++)
    {
        SDL_DestroyTexture(tx[i]);
    }


    TTF_Quit();
    SDL_Quit();
    return 0;
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
