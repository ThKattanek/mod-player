//////////////////////////////////////////////////
//                                              //
// LevelMeterClass                              //
// by Thorsten Kattanek                         //
//                                              //
// #file: level_meter_class.h                   //
//                                              //
// last change: 09-09-2018                      //
// https://github.com/ThKattanek/mod-player     //
//                                              //
//////////////////////////////////////////////////

#ifndef LEVEL_METER_CLASS_H
#define LEVEL_METER_CLASS_H

#include <SDL2/SDL.h>

class LevelMeterClass
{
public:
    LevelMeterClass(SDL_Renderer *ren, int w, int h);
    ~LevelMeterClass();

    void Draw(int x, int y, float value);

private:
    int x,y,w,h;
    SDL_Renderer *ren;
    SDL_Surface *surface;
    Uint32 rmask, gmask, bmask, amask;
    SDL_Texture* texture;
    SDL_Rect rect;
};

#endif // LEVEL_METER_CLASS_H
