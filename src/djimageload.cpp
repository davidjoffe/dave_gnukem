/*!
\file    djImageLoad.cpp
\brief   Image load/save helpers
\author  David Joffe

Copyright (C) 1998-2024 David Joffe
*/
/*--------------------------------------------------------------------------*/
#include "djimageload.h"
#include "config.h"
#include "djimage.h"
#include <SDL3/SDL_endian.h>
#include <string>

#ifdef djUSE_SDLIMAGE
// Hm not 100% sure if we should best put here 'SDL2/SDL_image.h' or just 'SDL_image.h' and let makefiles etc. pass the folder in ..
#include <SDL3_image/SDL_image.h>
#endif

//-----------------------------------------
//-------------------- Testing using stb_image.h
// Uncomment to enable usage of stb_image.h to load images:
// Currently still a bit experimental
#define djUSE_STB_IMAGE
//-----------------------------------------

#ifdef djUSE_STB_IMAGE
#define STB_IMAGE_IMPLEMENTATION
// For now only png? actually no it would be useful to load more formats too
//#define STBI_ONLY_PNG
#include "3rdparty/stb_image.h"
#endif
//-----------------------------------------
//-------------------- Testing using stb_image.h
#ifdef djUSE_STB_IMAGE
// Hm, although this was originally intended to load PNGs it directly loads JPG with no changes, so it may load other formats too, but
// I guess some formats we may end up with different RGB/RGBA byte orders and desired masks? So in future should tweak/extend if/as needed. [~dj2023[
djImage* djImageLoad_STB_LoadImage(const char* szFilename, djImage* pUseThisImage=nullptr)
{
    if (szFilename==nullptr || szFilename[0]==0)
        return nullptr;// Empty filename?
    
	//SYS_Debug ( std::string("djImageLoad_STB_LoadImage:"+szFilename).c_str() );

    int width=0, height=0, channels=0;
    // Request 3 channels for RGB or 4 for RGBA
    unsigned char* data = stbi_load(szFilename, &width, &height, &channels, 0);
    if (data == NULL) {
        printf("stbi_load failed\n");
        return nullptr;//false; // Loading failed
    }

    // Ensure we are dealing with RGB or RGBA formats
    if (channels != 3 && channels != 4) {
        stbi_image_free(data);
        printf("stbi_load failed (currently only 3/4 byte formats supported)\n");
        //todo[low prio] handle more?

        return nullptr;//false;
    }

    // Create a new image in djImage with the appropriate size
    djImage* pImg = pUseThisImage;
    if (pImg==nullptr)
        pImg = new djImage();
    pImg->CreateImage(width, height, channels * 8);
    //djlog("djImage::LoadPNG: %s %d %d %d\n", szFilename, width, height, channels);
    printf("LoadPNG: %s %d %d %d\n", szFilename, width, height, channels);

    // Calculate the size of the image data
    size_t dataSize = width * height * channels;

    // CBSU/SBSU - could we avoid this extra copy? (but, without any other performance hits doing that) [ LOW PRIO ]

    // Assuming djImage has a buffer 'm_pBuffer' or similar to store pixel data
    memcpy(pImg->Data(), data, dataSize);

    // todo - should we convert all images to single game-wide byte order for speed reasons? [larger topic - low prio]

    // todo, double-check these are correct [dj 2023-11]
    // also where is 'correct' place to check for endianness etc.?
    if (channels==3)//"RGB" order in file
    {
        // R,G,B,A masks (specify different masks on PPC etc. - see https://github.com/davidjoffe/dave_gnukem/issues/100)

        //RGB big enddian
#if SDL_BYTEORDER==SDL_BIG_ENDIAN
        // not 100% sure these are correct ...
        pImg->m_Rmask = 0xFF000000;
        pImg->m_Gmask = 0x00FF0000;
        pImg->m_Bmask = 0x0000FF00;
        //?pImg->m_Amask = 0;
#else
        //RGB little endian
        pImg->m_Rmask = 0xFF;
        pImg->m_Gmask = 0xFF00;
        pImg->m_Bmask = 0xFF0000;
        //?pImg->m_Amask = 0;
#endif
    }
    else if (channels==4)//"RGBA" order in file
    {
        // Little endian so if R is first in file the Rmask is 0xFF (I *think* .. double-check this...)
#if SDL_BYTEORDER==SDL_BIG_ENDIAN
        //RGBA big enddian
        pImg->m_Rmask = 0xFF000000;
        pImg->m_Gmask = 0x00FF0000;
        pImg->m_Bmask = 0x0000FF00;
        pImg->m_Amask = 0x000000FF;
#else
        //RGBA little endian
        pImg->m_Rmask = 0xFF;
        pImg->m_Gmask = 0xFF00;
        pImg->m_Bmask = 0xFF0000;
        pImg->m_Amask = 0xFF000000;
#endif
    }

    stbi_image_free(data); // Free the data obtained from stb_image
    return pImg;
}
#endif
//-----------------------------------------


void djStrToLowerTmpHelper( std::string& s )//urgh
{
	for ( size_t i=0; i<s.length(); ++i )
	{
		if ((s[i] >= 'A') && (s[i] <= 'Z')) s[i] += 32;
	}
}

// [dj2023-11] This LoadImage is a pass through the other LoadImage in same class that takes a djImage*
djImage* djImageLoad::LoadImage(const char *szFilename)
{
    djImage* pImg = new djImage();
    if (LoadImage(pImg, szFilename)<0)
    {
        // Error, delete image and return nullptr
        delete pImg;
        return nullptr;
    }
    // Success
    return pImg;
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
#ifdef djUSE_STB_IMAGE
    else if (extension=="png")
    {
        // This 'this' is weird
        djImage* pRet = djImageLoad_STB_LoadImage(szFilename, pImg);
		//if (!pImg->LoadPNG(szFilename))
        if (!pRet)
		{
			// dj2022-11 hm this is maybe slightly gross must rethink where all the various logs "should" go etc. and clean up logging system
			printf("Warning: Image load failed: %s\n", szFilename);
			// fixme add some sort of 'debugassert' stuff here to help with testign?
            return -1;
		}
        return 0;
    }
    else // fallback loader - try anything stb_image.h will handle .. this allows JPGs too :) //if (extension=="jpg"||extension=="jpeg")
    {
        // Fallback loader attempt for various other formats stb_image may support ...
        djImage* pRet = djImageLoad_STB_LoadImage(szFilename, pImg);
        if (!pRet)
        {
            // dj2022-11 hm this is maybe slightly gross must rethink where all the various logs "should" go etc. and clean up logging system
            printf("Warning: Image load failed: %s\n", szFilename);
            // fixme add some sort of 'debugassert' stuff here to help with testign?
            return -1;
        }
        return 0;
    }
#endif//djUSE_STB_IMAGE

#ifdef djUSE_SDLIMAGE
    // extra redundant copy ...
    SDL_Surface* surface = IMG_Load(filename.c_str());
    if (!surface) {
        // << "IMG_Load Error: " << IMG_GetError() << std::endl;
        printf("IMG_Load failed\n");
        return -1;
    }
    // if small endian am i supposed to reverse the masks here? not sure ..
    auto f = SDL_GetPixelFormatDetails(surface->format);
    printf("CreateImage:%d %d %d  pitch=%d  rgba  %08x %08x %08x %08x\n", (int)surface->w, (int)surface->h, (int)f->bits_per_pixel, (int)surface->pitch,
        (int)f->Rmask,
        (int)f->Gmask,
        (int)f->Bmask,
        (int)f->Amask);
    pImg->CreateImage(surface->w, surface->h, f->bits_per_pixel, surface->pitch, surface->pixels,
        f->Rmask,
        f->Gmask,
        f->Bmask,
        f->Amask
    );
    printf("CreateImage:DONE\n");
    
    // Clean up
    SDL_DestroySurface(surface);
    return 0;
#else
    return -1;//unhandled format
#endif
}
