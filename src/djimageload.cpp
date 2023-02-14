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
    if (pImg==nullptr) return -1;
    //debug//printf("Load %s\n", szFilename);
    if (szFilename==nullptr||szFilename[0]==0) return -1;// empty string for filename?
    //debug//printf("Load %s\n", szFilename);

    // Get file type from extension [yes that's not perfect but I think will do]
    std::string filename = szFilename;
    std::string extension = filename.substr(filename.find_last_of(".") + 1);
    //std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    djStrToLowerTmpHelper(extension);

    // For TGA files pass back to our own old TGA loader
    if (extension=="tga")
    {
		int ret = pImg->LoadTGA(szFilename);
		if (ret < 0)
		{
			// dj2022-11 hm this is maybe slightly gross must rethink where all the various logs "should" go etc. and clean up logging system
			printf("Warning: Image load failed: %s\n", szFilename);
			// fixme add some sort of 'debugassert' stuff here to help with testign?
		}
        return ret;
    }

#ifdef djUSE_SDLIMAGE
    // extra redundant copy ...
    SDL_Surface* surface = IMG_Load(filename.c_str());
    if (!surface) {
        // << "IMG_Load Error: " << IMG_GetError() << std::endl;
        printf("IMG_Load failed\n");
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
#else
    return -1;//unhandled format
#endif
}
