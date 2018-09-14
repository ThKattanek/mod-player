//////////////////////////////////////////////////
//                                              //
// LevelMeterClass                              //
// by Thorsten Kattanek                         //
//                                              //
// #file: level_meter_class.cpp                 //
//                                              //
// last change: 09-09-2018                      //
// https://github.com/ThKattanek/mod-player     //
//                                              //
//////////////////////////////////////////////////

#include "level_meter_class.h"

LevelMeterClass::LevelMeterClass(SDL_Renderer *ren, int w, int h)
{
    this->ren = ren;
    this->w = w;
    this->h = h;

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

    surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, rmask, gmask, bmask, amask);

    float r,g,b;

    r=1.0;
    g=0.0;
    b=0.0;

    rect = {0,0,w,1};

    unsigned long color1;

    for(int i=0; i<h/2; i++)
    {
        rect.y = i;
        g = i*(1.0/(h/2));
        color1 = 0xE0000000 | (unsigned char)(r*255) | (unsigned char)(g*255)<<8 | (unsigned char)(b*255)<<16;
        SDL_FillRect(surface,&rect,color1);
    }

    for(int i=0; i<h/2; i++)
    {
        rect.y = i+h/2;
        r = (h/2-i)*(1.0/(h/2));
        color1 = 0xE0000000 | (unsigned char)(r*255) | (unsigned char)(g*255)<<8 | (unsigned char)(b*255)<<16;
        SDL_FillRect(surface,&rect,color1);
    }

    rect.y = 0;
    rect.h = h;

    texture = SDL_CreateTextureFromSurface(ren, surface);
}

LevelMeterClass::~LevelMeterClass()
{
    SDL_FreeSurface(surface);
}

void LevelMeterClass::Draw(int x, int y, float value)
{
    SDL_Rect dst_rec, src_rec;

    dst_rec = src_rec = rect;

    src_rec.x = 0;
    src_rec.y =  (1.0-value) * rect.h;

    dst_rec.x = x;
    dst_rec.y = src_rec.y + y - rect.h;
    dst_rec.h = rect.h - (1.0-value) * rect.h;

    SDL_RenderCopy(ren,texture,&src_rec,&dst_rec);

    if(value > 0)
    {
        SDL_SetRenderDrawColor(ren,100,100,100,0);
        SDL_RenderDrawRect(ren,&dst_rec);
    }
}
