/*!
\file    djImageLoad.cpp
\brief   Image load/save helpers
\author  David Joffe

Copyright (C) 1998-2023 David Joffe
*/
/*--------------------------------------------------------------------------*/
#include "config.h"
#include "djimage.h"
#include "djimageload.h"
#include <string>
#ifdef djUSE_SDLIMAGE
// Hm not 100% sure if we should best put here 'SDL2/SDL_image.h' or just 'SDL_image.h' and let makefiles etc. pass the folder in ..
#include <SDL_image.h>
#endif

void djStrToLowerTmpHelper( std::string& s )//urgh
{
	for ( size_t i=0; i<s.length(); ++i )
	{
		if ((s[i] >= 'A') && (s[i] <= 'Z')) s[i] += 32;
	}
}

int djImageLoad::LoadImage(djImage* pImg, const char *szFilename)
{
    if (szFilename==nullptr||szFilename[0]==0) return -1;// empty string for filename?

#ifndef djUSE_SDLIMAGE
    // Just pass through to our own ancient TGA loader as that's all we previously supported
    return pImg->Load(szFilename);
#else
    // Get file type from extension [yes that's not perfect but I think will do]
    std::string filename = szFilename;
    std::string extension = filename.substr(filename.find_last_of(".") + 1);
    //std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    djStrToLowerTmpHelper(extension);

    //djStrToLower(s.c_str());
    // For TGA files pass back to our own ancient TGA loader
    if (extension=="tga")
    {
        return pImg->Load(szFilename);
    }

    // extra redundant copy ...
    SDL_Surface* surface = IMG_Load(filename.c_str());
    if (!surface) {
        // << "IMG_Load Error: " << IMG_GetError() << std::endl;
        return -1;
    }
    // if small endian am i supposed to reverse the masks here? not sure ..
    printf("CreateImage:%d %d %d  pitch=%d  rgba  %08x %08x %08x %08x\n", (int)surface->w, (int)surface->h, (int)surface->format->BitsPerPixel, (int)surface->pitch,
        (int)surface->format->Rmask,
        (int)surface->format->Gmask,
        (int)surface->format->Bmask,
        (int)surface->format->Amask);
    pImg->CreateImage(surface->w, surface->h, surface->format->BitsPerPixel, surface->pitch, surface->pixels,
        surface->format->Rmask,
        surface->format->Gmask,
        surface->format->Bmask,
        surface->format->Amask
    );
    printf("CreateImage:DONE\n");
    
    // Clean up
    SDL_FreeSurface(surface);
    return 0;
#endif
}
