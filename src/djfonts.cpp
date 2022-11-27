//Copyright (C) 2022 David Joffe / Dave Gnukem project
//
//dj2022-11-25 new vector-based font stuff (TTF/OTF) and Unicode font helper stuff
//NB Design-wise this is conceptually part of the generic reusable code so shouldn't have dependencies to any Dave Gnukem specific parts of the codebase

/*--------------------------------------------------------------------------*/
#include "config.h"
#include "djfonts.h"
#ifdef djUNICODE_TTF

#include <utf8proc.h>//<- for 'utf8 to 32-bit' conversion for TTF_GlyphIsProvided32 to help find closest best matching font

// hm to move obcviously not meant to be in hiscores
#if defined(WIN32) && defined(_MSC_VER)
// Microsoft compiler stuff .. hmm not sure where best .. unless cmake etc.
#pragma comment(lib, "SDL2_ttf.lib")
#pragma comment(lib, "utf8proc.lib")
#endif

/*--------------------------------------------------------------------------*/
// djFontList
/*--------------------------------------------------------------------------*/
djFontList::djFontList()
{
}
djFontList::~djFontList()
{
	// This should not actually be called from the destructor, you must properly call at correct time (global cleanup order bug issues coudl result)
#ifdef _DEBUG
	//assert(m_apFonts.size() == 0);
#endif
	//CleanupFonts();
}

TTF_Font* djFontList::LoadFont(const char* szFilename, int nPTSize, bool bRTL, const std::string& sLangCodeInit)
{
	if (szFilename == nullptr || szFilename[0] == 0) return nullptr;

	TTF_Font* pFont = TTF_OpenFont(szFilename, nPTSize);
	if (pFont)
	{
		m_apFonts.push_back(pFont);
		djFontDescriptor FontDescriptor;
		//FontDescriptor.
		FontDescriptor.m_pFont = pFont;
		FontDescriptor.m_sFilename = szFilename;
		FontDescriptor.m_nFontSizePt = nPTSize;
		FontDescriptor.m_bRTL = bRTL;
		FontDescriptor.m_sLang = sLangCodeInit;
		m_aFonts.push_back(FontDescriptor);

#ifdef djTTF_HAVE_HARFBUZZ_EXTENSIONS
		if (FontDescriptor.m_bRTL)
		{
			TTF_SetFontDirection(pFont, TTF_DIRECTION_RTL);
			// hm could be Hebrew too but if Arabic we need the harfbuzz extensions .. should also detect language
			if (FontDescriptor.m_sLang == "ar")
				TTF_SetFontScriptName(pFont, "Arab");
		}
#endif
	}
	else
	{
		//todo log a warning here or something? or caller logs warning?
	}
	return pFont;
}
TTF_Font* djFontList::LoadFont(djFontDescriptor& FontDescriptor)
{
	if (FontDescriptor.m_sFilename.empty()) return nullptr;
	// What if already loaded on descriptor? hm

	TTF_Font* pFont = TTF_OpenFont(FontDescriptor.m_sFilename.c_str(), FontDescriptor.m_nFontSizePt);
	if (pFont)
	{
		m_apFonts.push_back(pFont);
		m_aFonts.push_back(FontDescriptor);
		FontDescriptor.m_pFont = pFont;
	
#ifdef djTTF_HAVE_HARFBUZZ_EXTENSIONS
		if (FontDescriptor.m_bRTL)
		{
			TTF_SetFontDirection(pFont, TTF_DIRECTION_RTL);
			// hm could be Hebrew too but if Arabic we need the harfbuzz extensions .. should also detect language
			if (FontDescriptor.m_sLang=="ar")
				TTF_SetFontScriptName(pFont, "Arab");
		}
#endif
	}
	else
	{
		//todo log a warning here or something? or caller logs warning?
	}
	return pFont;
}

void djFontList::CleanupFonts()
{
	for (auto pFont : m_apFonts)
	{
		if (pFont)
			TTF_CloseFont(pFont);
	}
	m_apFonts.clear();
}
/*--------------------------------------------------------------------------*/






/*--------------------------------------------------------------------------*/
// djUnicodeFontHelpers
/*--------------------------------------------------------------------------*/

// abstract base class .. dj2022-11 just thinking
/*
class djFontSelector
{
public:
	virtual TTF_Font* SelectFont() { return nullptr; }
	virtual djFontDescriptor* SelectFontDesc() { return nullptr; }
};
*/

// Have two helpers, one for char* one for std::string (as a pip faster if caller already has a std::string as no need to do 'strlen' call) [low - dj2022-11]
/*TTF_Font* djUnicodeFontHelpers::FindBestMatchingFontMostChars(const std::vector<TTF_Font*>& apFonts, const char* szUTF8string)
{
	std::string s;
	if (szUTF8string)
		s = szUTF8string;
	return FindBestMatchingFontMostCharsStr(apFonts, s.c_str(), s.length());
}*/
//TTF_Font* djUnicodeFontHelpers::FindBestMatchingFontMostCharsStr(const std::vector<TTF_Font*>& apFonts, const std::string& sTextUTF8, const size_t uLen)
TTF_Font* djUnicodeFontHelpers::FindBestMatchingFontMostCharsStr(const djFontList* pFontList, const std::vector<TTF_Font*>& apFonts, const char* szTextUTF8, const unsigned int uLen, int nDir)
{
	int nMatchesMost = 0;
	TTF_Font* pFontMostChars = nullptr;
	for (auto pFont : apFonts)
	{
		if (pFont == nullptr) continue;//safety

		if (pFontMostChars == nullptr)
		{
			pFontMostChars = pFont;

			// If string has zero length might as well just return first font we find
			if (uLen == 0 || szTextUTF8 == nullptr)
				return pFont;
		}

		// NB! If someone passes in NULL string we do NOT want to utf8proc_iterate etc.
		if (szTextUTF8 == nullptr)
			continue;

		// NB do NOT modify sText while we're iterating over the string
		const char* szStart = szTextUTF8;
		utf8proc_int32_t cp = -1;//codepoint in 32-bit
		size_t uOffset = 0;
		size_t uLen2 = uLen;
		// [dj2022-11] Must convert utf8 to 32-bit Unicode glyphs and iterate over string
		// Remember that utf8 is multi-byte and variable-width encoding so a single Unicode codepoint (i.e. one 32-bit value) could be maybe e.g. 1 byte or 2 bytes or 3 bytes or 4 bytes etc. in the utf8 string (but strlen returns the full number of bytes, not "Unicode Characters")
		//"Reads a single codepoint from the UTF-8 sequence being pointed to by str. The maximum number of bytes read is strlen, unless strlen is negative (in which case up to 4 bytes are read). If a valid codepoint could be read, it is stored in the variable pointed to by codepoint_ref, otherwise that variable will be set to -1. In case of success, the number of bytes read is returned; otherwise, a negative error code is returned."
		utf8proc_ssize_t ret = utf8proc_iterate((const utf8proc_uint8_t*)(szStart + uOffset), uLen2, &cp);
		int nMatches = 0;
		while (ret > 0)
		{
			uLen2 -= (size_t)ret;
			uOffset += (size_t)ret;
			// [dj2022-11] Hmm SDL documentation says "This is the same as TTF_GlyphIsProvided(), but takes a 32-bit character instead of 16-bit, and thus can query a larger range. If you are sure you'll have an SDL_ttf that's version 2.0.18 or newer, there's no reason not to use this function exclusively."
			// That means we may have a problem here supporting specifically (just) the Unicode SURROGATE PAIRS range IF (and only if) a platform has SDL older than this .. is that worth worrying about? [seems low prio to me dj2022-11 .. a few days ago we had no Unicode support at all so full Unicode support on 2.0.18+ and everything but surrogate pairs on older SDL's seems OK]
			// We *definitely* want to use the 32-bit version when and where available as otherwise SURROGATE PAIRS won't work.
#if SDL_VERSION_ATLEAST(2, 0, 18)
			if (cp > 0 && TTF_GlyphIsProvided32(pFont, cp))
#else
			if (cp > 0 && TTF_GlyphIsProvided(pFont, cp))
#endif
			{
				++nMatches;
				if (nMatches > nMatchesMost)
				{
					nMatchesMost = nMatches;
					pFontMostChars = pFont;
				}
			}
			ret = utf8proc_iterate((const utf8proc_uint8_t*)(szStart + uOffset), uLen2, &cp);
		}
	}
	return pFontMostChars;
}
/*--------------------------------------------------------------------------*/
//int djUnicodeTextHelpers::GuessDirection(const std::string& sTextUTF8)
// Return value negative means right to left language e.g. Hebrew or Arabic
int djUnicodeTextHelpers::GuessDirection(const char* szTextUTF8, const unsigned int uLen)
{
	if (szTextUTF8 == nullptr || uLen == 0) return 0;

	size_t nNumChars = 0;
	size_t nNumCharsRTL = 0;
	//int nNumCharsLTR = 0;
	//int nNumCharsDirectionAgnostic = 0;// E.g. things like space are direction-agnostic

	// UTF-32 string literal
	//const char32_t* szHEBREWCHARS = U"\u0591\u0592\u0593\u0594\u0595\u0596\u0597\u0598\u0599\u059a\u059b\u059c\u059d\u059e\u059f\u05a0\u05a1\u05a2\u05a3\u05a4\u05a5\u05a6\u05a7\u05a8\u05a9\u05aa\u05ab\u05ac\u05ad\u05ae\u05af\u05b0\u05b1\u05b2\u05b3\u05b4\u05b5\u05b6\u05b7\u05b8\u05b9\u05ba\u05bb\u05bc\u05bd\u05be\u05bf\u05c0\u05c1\u05c2\u05c3\u05c4\u05c5\u05c6\u05c7\u05d0\u05d1\u05d2\u05d3\u05d4\u05d5\u05d6\u05d7\u05d8\u05d9\u05da\u05db\u05dc\u05dd\u05de\u05df\u05e0\u05e1\u05e2\u05e3\u05e4\u05e5\u05e6\u05e7\u05e8\u05e9\u05ea\u05f0\u05f1\u05f2\u05f3\u05f4\ufb1d\ufb1e\ufb1f\ufb20\ufb21\ufb22\ufb23\ufb24\ufb25\ufb26\ufb27\ufb28\ufb29\ufb2a\ufb2b\ufb2c\ufb2d\ufb2e\ufb2f\ufb30\ufb31\ufb32\ufb33\ufb34\ufb35\ufb36\ufb38\ufb39\ufb3a\ufb3b\ufb3c\ufb3e\ufb40\ufb41\ufb43\ufb44\ufb46\ufb47\ufb48\ufb49\ufb4a\ufb4b\ufb4c\ufb4d\ufb4e\ufb4f";
	
	const char* szStart = szTextUTF8;// sTextUTF8.c_str();
	size_t uOffset = 0;
	size_t uLen2 = (size_t)uLen;
	utf8proc_int32_t c = -1;//character codepoint in 32-bit
	utf8proc_ssize_t ret = utf8proc_iterate((const utf8proc_uint8_t*)(szStart + uOffset), uLen2, &c);
	// For guessing, let's stop somewhere for speed reasons e.g. if someone passes a 100MB text string let's maybe not check the entire thing
	const size_t uMAXCHARSTOCHECK = 512;
	while (ret > 0 && nNumChars < uMAXCHARSTOCHECK)
	{
		++nNumChars;
		uLen2 -= (size_t)ret;
		uOffset += (size_t)ret;

		if (c >= 0x0591 && c <= 0x05f4)//hebrew
			++nNumCharsRTL;
		else if (c >= 0xfb1d && c <= 0xfb4f)//hebrew
			++nNumCharsRTL;
		else if (c >= 0x0600 && c <= 0x06ff)//arabic
			++nNumCharsRTL;
		else if (c >= 0x0750 && c <= 0x077f)// arabic supplemnt
			++nNumCharsRTL;
		//else if (c == U' ' || c == U'\t' || c == U'\r' || c == U'\n' || c == U'(' || c == U')')
			//nNumCharsDirectionAgnostic++;

		ret = utf8proc_iterate((const utf8proc_uint8_t*)(szStart + uOffset), uLen2, &c);
	}
	// if more than half are RTL (this is very crude and quick n dirty guess)
	if (nNumCharsRTL > nNumChars / 2)
		return -1;
	return 0;
}
/*--------------------------------------------------------------------------*/


#endif//#ifdef djUNICODE_TTF
