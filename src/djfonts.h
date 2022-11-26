//Copyright (C) 2022 David Joffe / Dave Gnukem project
//
//dj2022-11-25 new vector-based font stuff (TTF/OTF) and Unicode font helper stuff.
//NB Design-wise this is conceptually part of the generic reusable code so shouldn't have dependencies to any Dave Gnukem specific parts of the codebase

/*--------------------------------------------------------------------------*/
#ifndef _DJFONTS_H_
#define _DJFONTS_H_

#include "config.h"
#ifdef djUNICODE_TTF
#include <vector>
//#include <string>

// forward? keep definition for .cpp backend?
//struct TTF_Font;
#ifdef __OS2__
#include <SDL/SDL_ttf.h>
#else
#include <SDL_ttf.h>
#endif



/*--------------------------------------------------------------------------*/
class djFontList
{
public:
	// ctor and dtor should 'do as little as possible' not the "real" init/cleanup
	djFontList();
	virtual ~djFontList();

	// return nullptr on fail
	virtual TTF_Font* LoadFont(const char* szFilename, int nPTSize);

	//virtual void LoadFonts();
	virtual void CleanupFonts();

	std::vector<TTF_Font*> m_apFonts;
};
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
class djUnicodeFontHelpers
{
public:
	// Have two helpers, one for char* one for std::string (as a pip faster if caller already has a std::string as no need to do 'strlen' call) [low - dj2022-11]
	//static TTF_Font* FindBestMatchingFontMostChars(const std::vector<TTF_Font*>& apFonts, const char* szUTF8string);
	//static TTF_Font* FindBestMatchingFontMostCharsStr(const std::vector<TTF_Font*>& apFonts, const std::string& sTextUTF8, const size_t uLen);

	static TTF_Font* FindBestMatchingFontMostCharsStr(const std::vector<TTF_Font*>& apFonts, const char* szTextUTF8, const unsigned int uLen);
};
/*--------------------------------------------------------------------------*/
class djUnicodeTextHelpers
{
public:
	static int GuessDirection(const char* szTextUTF8, const unsigned int uLen);
	//static int GuessDirection(const std::string& sTextUTF8);
};
/*--------------------------------------------------------------------------*/



#endif//#ifdef djUNICODE_TTF
#endif
