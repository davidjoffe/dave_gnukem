/*!
\file    djimageload.h
\brief   Image load/save helpers
\author  David Joffe
*/
/*--------------------------------------------------------------------------*/
#ifndef _DJIMAGELOAD_H_
#define _DJIMAGELOAD_H_

class djImage;

// loaders should be separate file? tho needs access to procected members - either a friend class, or e.g. maybe rather just expose protected members with set variables etc.
// should we bother or just transition to sdl_image etc.? 
// (dj2022 note: Our TGA image loading code literally harkens from the 90s and was some of the earliest game code, and also because we were on LibSDL1 for a long time we just stuck with it .. I felt for a long time maybe once we're on SDL2 we could start maybe using e.g. libsdl_image to load more modern formats like .png which offer mainly compression)
class djImageLoad
{
public:
	static int LoadImage(djImage* pImg, const char *szFilename);
	static djImage* LoadImage(const char *szFilename);
};

#endif