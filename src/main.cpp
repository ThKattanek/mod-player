#include <iostream>
#include <math.h>
#include <SDL2/SDL.h>

#include "./mod_class.h"

using namespace std;

#define AudioSampleRate 44100

#ifdef _WIN32
    #define AudioPufferSize (882)    // 882 bei 44.100 Khz
#else
    #define AudioPufferSize (882*2)    // 882 bei 44.100 Khz
#endif

void AudioMix(void* userdat, Uint8 *stream, int length);

#define SAMPLE_NR 12
char* sample_data;
int sample_len;
int sample_pos;

bool sample_loop;
int sample_loop_start;
int sample_loop_end;

MODClass* mod = NULL;

#undef main
int main(int argc, char *argv[])
{
    char* filename;

    if(argc > 1)
        filename = argv[1];

    mod = new MODClass(filename, AudioSampleRate);

    if(!mod->ModIsLoaded())
        return(0);

    SAMPLE* sample = mod->GetSample(SAMPLE_NR);

    sample_data = (char*)sample->data;
    sample_len = sample->length;
    sample_pos = 0;

    if(sample->loop_length < 3)
        sample_loop = false;
    else sample_loop = true;

    sample_loop_start = sample->loop_start;
    sample_loop_end = sample->loop_start + sample->loop_length;

    cout << "Demo-01" << endl;
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);

    SDL_Window *win = SDL_CreateWindow("Hello World!", 100, 100, 800, 600, SDL_WINDOW_SHOWN);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    /// SLD Audio Installieren (C64 Emulation) ///
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

    mod->MODPlay();

    SDL_Event event;
    bool quit = false;

    uint8_t clr = 200;

    float r = 290;
    float x = 400;
    float y = 300;
    float N=5;


    int numeric_input_count = 0;
    unsigned char numeric_input_buffer[2];
    unsigned char play_sample_number;

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
                    N++;
                    break;

                case SDLK_MINUS:
                    N--;
                    break;

                case SDLK_0:
                    numeric_input_buffer[numeric_input_count] = 0;
                    numeric_input_count++;
                    break;
                case SDLK_1:
                    numeric_input_buffer[numeric_input_count] = 1;
                    numeric_input_count++;
                    break;
                case SDLK_2:
                    numeric_input_buffer[numeric_input_count] = 2;
                    numeric_input_count++;
                    break;
                case SDLK_3:
                    numeric_input_buffer[numeric_input_count] = 3;
                    numeric_input_count++;
                    break;
                case SDLK_4:
                    numeric_input_buffer[numeric_input_count] = 4;
                    numeric_input_count++;
                    break;
                case SDLK_5:
                    numeric_input_buffer[numeric_input_count] = 5;
                    numeric_input_count++;
                    break;
                case SDLK_6:
                    numeric_input_buffer[numeric_input_count] = 6;
                    numeric_input_count++;
                    break;
                case SDLK_7:
                    numeric_input_buffer[numeric_input_count] = 7;
                    numeric_input_count++;
                    break;
                case SDLK_8:
                    numeric_input_buffer[numeric_input_count] = 8;
                    numeric_input_count++;
                    break;
                case SDLK_9:
                    numeric_input_buffer[numeric_input_count] = 9;
                    numeric_input_count++;
                    break;
                case SDLK_ESCAPE:
                    numeric_input_count = 0;
                    break;

                default:
                    break;
                }
                if(numeric_input_count==2)
                {
                    play_sample_number = numeric_input_buffer[0]*10;
                    play_sample_number += numeric_input_buffer[1];
                    numeric_input_count=0;
                    mod->PlaySample(play_sample_number);
                }
            }
            if (event.type == SDL_MOUSEBUTTONDOWN){
               // quit = true;
            }
        }

        SDL_SetRenderDrawColor(ren,clr,clr,clr,0);


        SDL_RenderClear(ren);

        SDL_SetRenderDrawColor(ren,0,0,0,0);

        int px1=0;
        int py1=0+300;

        signed char* data = (signed char*)sample->data;
        float xmul = sample->length / 800.0;
        float xpos_data = 0.0;

        for(int i=0; i<sample->length; i++)
        {
            int px2 = i;
            int py2 = data[(int)xpos_data]*-1+300;
            xpos_data += xmul;

            SDL_RenderDrawLine(ren,px1,py1,px2,py2);

            px1 = px2;
            py1 = py2;
        }

        /*
        float px1=r*cos(0)+x;
        float py1=r*sin(0)+y;

        for(int i=1; i<N; i++)
        {
            float px2=r*cos(360.0 * i/N)+x;
            float py2=r*sin(360.0 * i/N)+y;

            SDL_RenderDrawLine(ren,px1,py1,px2,py2);

            px1 = px2;
            py1 = py2;
        }

        N++;
        if(N==400)
            N=0;
        */

        SDL_RenderPresent(ren);
        SDL_Delay(1);
    }

    delete mod;
    SDL_Quit();
    return 0;
}

void AudioMix(void* userdat, Uint8 *stream, int length)
{
    if(mod != NULL)
        mod->FillAudioBuffer((signed short*)stream, length/2);
    else
    {
        for(int i=0; i<length; i++)
            stream[i] = 0;
    }
    /*
    signed short* data = (signed short*)stream;
    if(sample_data != NULL)
    {
        for(int i=0; i<length/2; i+=2)
        {
            data[i] = sample_data[sample_pos]*32;
            data[i+1] = sample_data[sample_pos]*32;

            sample_pos++;
            if(sample_loop)
            {
                if(sample_pos == sample_loop_end)
                    sample_pos = sample_loop_start;
            }

            if(sample_pos == sample_len) sample_pos = 0;
        }
    }
    else
    {
        for(int i=0; i<length/2; i+=2)
        {
            data[i] = 0;
            data[i+1] = 0;
        }
    }
    */
}
