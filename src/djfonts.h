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
#include <string>

// forward? keep definition for .cpp backend?
//struct TTF_Font;
#ifdef __OS2__
#include <SDL/SDL_ttf.h>
#else
#include <SDL_ttf.h>
#endif



/*--------------------------------------------------------------------------*/
class djFontDescriptor
{
public:
	djFontDescriptor(const std::string& sFilenameInit="", int nSizePt=0, bool bRTLInit = false, const std::string& sLangCodeInit = "") : m_sFilename(sFilenameInit), m_nFontSizePt(nSizePt), m_bRTL(bRTLInit), m_sLang(sLangCodeInit), m_pFont(nullptr)
	{
	}

	std::string m_sFilename;
	int m_nFontSizePt = 12;

	bool m_bRTL = false;//Right to left? e.g. Arabic or Hebrew

	// is this meaningful?
	std::string m_sLang;//If this is a preferred font for a specific language? [no that would be a bit wrong it should map code -> font]


	// should TTF_Font* if loaded live in here too? I guess probably? maybe? not sure ... then it's not just a descriptor tho but anyway low prio - dj2022-11
	TTF_Font* m_pFont = nullptr;

};
/*struct djLoadedFont
{
public:
	djFontDescriptor FontDesc;
	TTF_Font* m_pFont = nullptr;
};*/
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
class djFontList
{
public:
	// ctor and dtor should 'do as little as possible' not the "real" init/cleanup
	djFontList();
	virtual ~djFontList();

	// return nullptr on fail
	virtual TTF_Font* LoadFont(const char* szFilename, int nPTSize, bool bRTL = false, const std::string& sLangCodeInit = "");
	virtual TTF_Font* LoadFont(djFontDescriptor& FontDescriptor);

	//virtual void LoadFonts();
	virtual void CleanupFonts();

	std::vector<TTF_Font*> m_apFonts;
	// fixme use poitnnr?
	std::vector<djFontDescriptor> m_aFonts;
};
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
class djUnicodeFontHelpers
{
public:
	// Have two helpers, one for char* one for std::string (as a pip faster if caller already has a std::string as no need to do 'strlen' call) [low - dj2022-11]
	//static TTF_Font* FindBestMatchingFontMostChars(const std::vector<TTF_Font*>& apFonts, const char* szUTF8string);
	//static TTF_Font* FindBestMatchingFontMostCharsStr(const std::vector<TTF_Font*>& apFonts, const std::string& sTextUTF8, const size_t uLen);

	// if nTextDirection is negative it's a right-to-left language which is a 'hint' that it should prefer to return 'right to left fonts' (e.g. Arabic) later we could/should langcode too?
	static TTF_Font* FindBestMatchingFontMostCharsStr(const djFontList* pFontList, const std::vector<TTF_Font*>& apFonts, const char* szTextUTF8, const unsigned int uLen, int nTextDirection = 0);
};
/*--------------------------------------------------------------------------*/
class djUnicodeTextHelpers
{
public:
	// Return value negative means right to left language e.g. Hebrew or Arabic
	static int GuessDirection(const char* szTextUTF8, const unsigned int uLen);
	//static int GuessLang(const char* szTextUTF8, const unsigned int uLen, std::string& sLangCodeRet);
	//static int GuessDirection(const std::string& sTextUTF8);
};
/*--------------------------------------------------------------------------*/



#endif//#ifdef djUNICODE_TTF
#endif
