#include <iostream>
#include <iomanip>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "./mod_class.h"

using namespace std;

#define AudioSampleRate 44100

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
    int screensize_w = 800;
    int screensize_h = 600;

    char* filename;

    TTF_Font* font1;
    char str1[1024];
    char str2[1024];
    SDL_Surface* sf;
    SDL_Texture* tx[MAX_ROW];

    int play_row_nr;
    int play_pattern_nr;
    NOTE* note;

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

    SDL_Window *win = SDL_CreateWindow(filename, 100, 100, screensize_w, screensize_h, SDL_WINDOW_SHOWN);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (TTF_Init() < 0)
    {
        cerr << "Error: TTF not initialize." << endl;
        SDL_Quit();
        return(0);
    }

    font1 = TTF_OpenFont("ConsolaMono-Bold.ttf",16);
    if(font1 == NULL)
    {
        cerr << "Error: TTF not open font." << endl;
        SDL_Quit();
        return(0);
    }

    // Crate Channel Volume Visible Texture
    SDL_Surface *surface;
    Uint32 rmask, gmask, bmask, amask;

    /* SDL interprets each pixel as a 32-bit number, so our masks must depend
       on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    int bar_w = 10;
    int bar_h = 100;

    surface = SDL_CreateRGBSurface(SDL_SWSURFACE, bar_w, bar_h, 32, rmask, gmask, bmask, amask);

    float r,g,b;

    r=1.0;
    g=0.0;
    b=0.0;

    SDL_Rect bar_rec = {0,0,bar_w,1};

    unsigned long color1;

    for(int i=0; i<bar_h/2; i++)
    {
        bar_rec.y = i;
        g = i*(1.0/(bar_h/2));
        color1 = 0xff000000 | (unsigned char)(r*255) | (unsigned char)(g*255)<<8 | (unsigned char)(b*255)<<16;
        SDL_FillRect(surface,&bar_rec,color1);
    }

    for(int i=0; i<bar_h/2; i++)
    {
        bar_rec.y = i+bar_h/2;
        r = (bar_h/2-i)*(1.0/(bar_h/2));
        color1 = 0xff000000 | (unsigned char)(r*255) | (unsigned char)(g*255)<<8 | (unsigned char)(b*255)<<16;
        SDL_FillRect(surface,&bar_rec,color1);
    }

    bar_rec.y = 0;
    bar_rec.h = bar_h;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(ren, surface);
    SDL_FreeSurface(surface);

    ////////////////////////////////////////////////////////////////////////////////////

    SDL_Color color_fg = {0,50,150,0};

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

                SDL_Surface* sf = TTF_RenderText_Blended(font1,str1,color_fg);
                tx[i] = SDL_CreateTextureFromSurface(ren, sf);
                SDL_SetTextureBlendMode(tx[0], SDL_BLENDMODE_BLEND);
                SDL_FreeSurface(sf);
            }
        }

        // Chaeck of change playing pattern_row
        if(mod->CheckPatternRowChange(&play_row_nr))
        {
            //GetStringFromPatterLine(str1, play_pattern_nr, play_row_nr);
        }

        SDL_SetRenderDrawColor(ren,130,130,200,0);
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
        rec2.y = screensize_h / 2 - 16 / 2;
        rec2.y -= play_row_nr * 16;

        for(int i=0; i<MAX_ROW; i++)
        {
            SDL_RenderCopy(ren,tx[i],&rec1,&rec2);
            rec2.y += 16;
        }

        int y = screensize_h / 2-4;

        SDL_RenderDrawLine(ren,0,y,screensize_w,y);
        SDL_RenderDrawLine(ren,0,y+16,screensize_w,y+16);

        // Volume Visible

        SDL_Rect dst_rec, src_rec;

        for(int i=0; i<mod->GetModChannelCount(); i++)
        {
            dst_rec = src_rec = bar_rec;

            src_rec.x = 0;

            float vol = mod->GetChannelVolumeVisualValue(i);

            src_rec.y =  (1.0-vol) * bar_rec.h;

            dst_rec.x = i*117 + 38;
            dst_rec.y = src_rec.y + screensize_h / 2 - 3 - bar_rec.h;
            dst_rec.h = bar_rec.h - (1.0-vol) * bar_rec.h;

            SDL_RenderCopy(ren,texture,&src_rec,&dst_rec);
        }

        SDL_RenderPresent(ren);
        SDL_Delay(1);
    }

    delete mod;

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
         sprintf(str1,"%.2x | ",pattern_row_nr);
         strcat(output_str,str1);
         for(int i=0; i<mod->GetModChannelCount();i++)
         {
             /// NOTE and Octave output
             if(note->note_number < 12)
             {
                 sprintf(str1,"%s%d ",NOTE_STRING[note->note_number],(int)note->oktave_number);
                 strcat(output_str,str1);
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
                 strcat(output_str,"... | ");
             }
             else
             {
                 sprintf(str1,"%.1x%.2x | ",(int)note->effectcommand,(int)note->effectdata);
                 strcat(output_str,str1);
             }
             note++;
         }
    }
}
