/*
hiscores.cpp

Copyright (C) 2001-2022 David Joffe

Conceptually should divide this file into more model/view/controller separation? low prio. dj2022-11
*/

#include "config.h"
#include <stdio.h>
#include <string.h>
#include "djstring.h"
#include <vector>
#include "hiscores.h"
#include "djtypes.h"
#include "menu.h"
#include "graph.h"
#include "djlog.h"
#include "djimage.h"
//------------------------
#ifdef djUNICODE_TTF

#include "datadir.h"

#ifdef __OS2__
	#include <SDL/SDL_ttf.h>//TTF_Init
#else
	#include <SDL_ttf.h>//TTF_Init
#endif

#include <utf8proc.h>//<- for 'utf8 to 32-bit' conversion for TTF_GlyphIsProvided32 to help find closest best matching font

// hm to move obcviously not meant to be in hiscores
#if defined(WIN32) && defined(_MSC_VER)
// Microsoft compiler stuff .. hmm not sure where best .. unless cmake etc.
#pragma comment(lib, "SDL2_ttf.lib")
#pragma comment(lib, "utf8proc.lib")
#endif

#endif
//------------------------

SScore::SScore() : nScore(0)
{
	memset(szName,0,sizeof(szName));//ugh, this is 2016
}
void SScore::SetName(const char* szNameNew)
{
	if (szNameNew == nullptr)
	{
		strcpy(szName, "");
		return;
	}

	snprintf(szName, sizeof(szName), "%s", szNameNew);
}

// Scores, sorted from highest to lowest
std::vector<SScore> g_aScores;

djImage *g_pImgHighScores = NULL;

struct SMenuItem instructionsHighScoreItems[] =
{
   { false, "        " },
   { true,  "   OK   " },
   { false, "        " },
   { false, NULL }
};
unsigned char instructionsHighScoreCursor[] = { 128, 129, 130, 131, 0 };
CMenu HighScoresMenu ( "highscores.cpp:HighScoresMenu" );

void InitHighScores()
{
	g_pImgHighScores = NULL;
}

void KillHighScores()
{
	djDestroyImageHWSurface(g_pImgHighScores);
	djDEL(g_pImgHighScores);
}

#ifdef djUNICODE_TTF
class djUnicodeFontHelpers
{
public:
	// Have two helpers, one for char* one for std::string (as a pip faster if caller already has a std::string as no need to do 'strlen' call) [low - dj2022-11]
	static TTF_Font* FindBestMatchingFontMostChars(const std::vector<TTF_Font*>& apFonts, const char* szUTFstring)
	{
		std::string s;
		if (szUTFstring)
			s = szUTFstring;
		return FindBestMatchingFontMostCharsStr(apFonts, s, s.length());
	}
	static TTF_Font* FindBestMatchingFontMostCharsStr(const std::vector<TTF_Font*>& apFonts, std::string& sText, const size_t uLen)
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
				if (uLen == 0)
					return pFont;
			}

			// NB do NOT modify sText while we're iterating over the string
			const char* szStart = sText.c_str();
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
};
#endif

void ShowHighScores()
{
	const int nYSTART = 20;
	const int nHEIGHTPERROW = 14;
#ifdef djUNICODE_TTF
	//TTF_Font* kosugi = TTF_OpenFont(DATA_DIR "fonts/KosugiMaru-Regular.ttf", 16);
	//TTF_Font* kosugi = TTF_OpenFont(DATA_DIR "fonts/KosugiMaru-Regular.ttf", 11);
	const int nPTFONTSIZE = 12;
	std::vector<TTF_Font*> apFonts;
	apFonts.push_back(TTF_OpenFont(DATA_DIR "fonts/DejaVuSansMono-Bold.ttf", nPTFONTSIZE));
	apFonts.push_back(TTF_OpenFont(DATA_DIR "fonts/DejaVuSansMono.ttf", nPTFONTSIZE));
	apFonts.push_back(TTF_OpenFont(DATA_DIR "fonts/NotoSans-Regular.ttf", nPTFONTSIZE));
	apFonts.push_back(TTF_OpenFont(DATA_DIR "fonts/chinese-mainland/NotoSansSC-Regular.otf", nPTFONTSIZE));

	TTF_Font* pFont = TTF_OpenFont(DATA_DIR "fonts/DejaVuSans.ttf", nPTFONTSIZE);
	TTF_Font* pFont2 = TTF_OpenFont(DATA_DIR "fonts/KosugiMaru-Regular.ttf", nPTFONTSIZE);
	apFonts.push_back(pFont);
	apFonts.push_back(pFont2);
	//apFonts.push_back(TTF_OpenFont("C:\\WINDOWS\\fonts\\Arial.ttf", nPTFONTSIZE));
#endif
	HighScoresMenu.setSize(0);
	HighScoresMenu.setItems(instructionsHighScoreItems);
	HighScoresMenu.setMenuCursor(instructionsHighScoreCursor);
	HighScoresMenu.setClrBack( djColor(48,66,128) );
	HighScoresMenu.setXOffset( 220 );
	HighScoresMenu.setYOffset ( 160 );
	if (g_pImgHighScores==NULL)
	{
		g_pImgHighScores = new djImage;
		g_pImgHighScores->Load(FILE_IMG_HIGHSCORES);
		djCreateImageHWSurface( g_pImgHighScores );
	}
	if (g_pImgHighScores)
	{
		djgDrawImage(pVisBack, g_pImgHighScores, 0, 0, g_pImgHighScores->Width(), g_pImgHighScores->Height());
	}

	{
		for ( int i=0; i<(int)g_aScores.size(); i++ )
		{
			GraphDrawString(pVisBack, g_pFont8x8, 16,          nYSTART + i * nHEIGHTPERROW, (unsigned char*)djIntToString(i + 1).c_str());//i+1 because i is 0-based index but human 1-based
			GraphDrawString(pVisBack, g_pFont8x8, 16 + 8 * 3,  nYSTART + i * nHEIGHTPERROW, (unsigned char*)djIntToString(g_aScores[i].nScore).c_str());
#ifndef djUNICODE_TTF
			GraphDrawString(pVisBack, g_pFont8x8, 24 + 11 * 8, nYSTART + i * nHEIGHTPERROW, (unsigned char*)g_aScores[i].szName);
#else
			std::vector< SDL_Color > aColorGrad;
			aColorGrad.push_back(SDL_Color{ 221, 69, 69, 255 });
			aColorGrad.push_back(SDL_Color{ 223, 90, 81, 255 });
			aColorGrad.push_back(SDL_Color{ 223, 90, 81, 255 });
			aColorGrad.push_back(SDL_Color{ 226, 122, 101, 255 });
			aColorGrad.push_back(SDL_Color{ 226, 122, 101, 255 });
			aColorGrad.push_back(SDL_Color{ 230, 153, 119, 255 });
			aColorGrad.push_back(SDL_Color{ 230, 153, 119, 255 });
			aColorGrad.push_back(SDL_Color{ 233, 185, 139, 255 });
			aColorGrad.push_back(SDL_Color{ 233, 185, 139, 255 });
			aColorGrad.push_back(SDL_Color{ 237, 217, 158, 255 });
			std::string sText = g_aScores[i].szName;
			// Get best matching font [this needs work]
			TTF_Font* pFontMostChars = djUnicodeFontHelpers::FindBestMatchingFontMostCharsStr(apFonts, sText, sText.length());
			const unsigned nXPOS = 24 + 11 * 8;
			if (!pFontMostChars)//<- old fallback for safety in case something went wrong and we have no matching fonts
			{
				GraphDrawString(pVisBack, g_pFont8x8, nXPOS, nYSTART + i * nHEIGHTPERROW, (unsigned char*)g_aScores[i].szName);
				continue;
			}

			//SDL_Surface* sur = TTF_RenderUNICODE_Blended_Wrapped(pFont, (const Uint16*)text.c_str(), SDL_Color{ 255, 255, 255, 255 }, CFG_APPLICATION_RENDER_RES_W - nXPOS);
			//SDL_Surface* sur = TTF_RenderUNICODE_Blended(pFont, (const Uint16*)text.c_str(), SDL_Color{ 255, 255, 255, 255 }, CFG_APPLICATION_RENDER_RES_W - nXPOS);
			//SDL_Surface* sur = TTF_RenderUTF8_Blended(pFontMostChars, sText.c_str(), SDL_Color{ 0, 0, 0, 255 });//black for +1 offset underline 'shadow' effect
			SDL_Surface* sur = TTF_RenderUTF8_Solid(pFontMostChars, sText.c_str(), SDL_Color{ 0, 0, 0, 255 });//black for +1 offset underline 'shadow' effect
			//SDL_Texture* tex = SDL_CreateTextureFromSurface(pVisBack->pRenderer, sur);
			if (sur)
			{
				SDL_Rect rcDest{ nXPOS+1, nYSTART + i * nHEIGHTPERROW - 5 + 1, pVisBack->width, pVisBack->height };
				CdjRect rcSrc(0, 0, sur->w, sur->h);
				SDL_BlitSurface(sur, &rcSrc, pVisBack->pSurface, &rcDest);//blit [is this best way?]
				SDL_BlitSurface(sur, &rcSrc, pVisBack->pSurface, &rcDest);//blit [is this best way?]
				//SDL_FreeSurface(sur);// why does this SDL_FreeSurface crash?SDL_FreeSurface(sur); [dj2022-11 fixme]
				sur = nullptr;
			}
			//sur = TTF_RenderUTF8_Blended(pFontMostChars, sText.c_str(), SDL_Color{ 255, 255, 255, 255 });
			sur = TTF_RenderUTF8_Blended(pFontMostChars, sText.c_str(), aColorGrad[i%aColorGrad.size()]);
			//SDL_Texture* tex = SDL_CreateTextureFromSurface(pVisBack->pRenderer, sur);
			if (sur)
			{
				SDL_Rect rcDest{ nXPOS, nYSTART + i * nHEIGHTPERROW -5, pVisBack->width, pVisBack->height };
				CdjRect rcSrc(0, 0, sur->w, sur->h);
				SDL_BlitSurface(sur, &rcSrc, pVisBack->pSurface, &rcDest);//blit [is this best way?]
				SDL_BlitSurface(sur, &rcSrc, pVisBack->pSurface, &rcDest);//blit [is this best way?]
				//SDL_BlitSurface(sur, &rcSrc, pVisBack->pSurface, &rcDest);//blit [is this best way?]
				//SDL_FreeSurface(sur);// why does this SDL_FreeSurface crash?SDL_FreeSurface(sur); [dj2022-11 fixme]
				sur = nullptr;
			}
			//SDL_RenderCopy(pVisBack->pRenderer, tex, NULL, &rect);
#endif//#ifndef djUNICODE_TTF
		}//i

		GraphFlip(true);

		// Pop up high scores menu
		do_menu( &HighScoresMenu);
	}
#ifdef djUNICODE_TTF
	for (auto pFont : apFonts)
	{
		if (pFont)
			TTF_CloseFont(pFont);
	}
	apFonts.clear();
#endif
}

bool LoadHighScores(const char *szFilename)
{
	g_aScores.clear();

	std::string s = djAppendPathStr(djGetFolderUserSettings().c_str(), szFilename);
	FILE *pIn = fopen(s.c_str(), "r");
	if (pIn==NULL)
	{
		djMSG("LoadHighScores: Failed to open file (%s): Creating default list\n", szFilename);
		// The default high scores in DN1 had firstnames of the DN1 developers, so we add that exactly the same here as a sort of 'hat tip' to them (with the same original default scores). And add myself. [dj2020-07]
		// If we turn this into a generic little game engine this part should not be directly in the core but separated as Gnukem-specific stuff (maybe via derived class or lambda or something)
		AddHighScore("Todd", 40000);//Todd Replogle
		AddHighScore("Scott", 30000);//Scott Miller
		AddHighScore("George", 20000);//George Broussard
		AddHighScore("Al", 10000);//Allen H. Blum III
		AddHighScore("David", 5000);//Me [dj2020-07]
		AddHighScore("John", 500);// Is "John"==Jim Norwood? Not sure. The original DN1 highscores say "John" here but credits say "Jim Norwood" and no John is listed in credits. [dj2020-07]
		return false;
	}

	char buf[2048]={0};

	fgets(buf, sizeof(buf), pIn);
	djStripCRLF(buf); // strip CR/LF characters
	int nEntries = 0;
	sscanf(buf, "%d", &nEntries);
	for ( int i=0; i<nEntries; i++ )
	{
		SScore Score;

		fgets(buf, sizeof(buf), pIn);
		djStripCRLF(buf); // strip CR/LF characters
		//fixme dj2022 small bug here still in loading high scores if very long name in file .. we make it safer but it might not load correct because of newlines still
		Score.SetName(buf);

		fgets(buf, sizeof(buf), pIn);
		djStripCRLF(buf); // strip CR/LF characters
		sscanf(buf, "%d", &Score.nScore);

		AddHighScore(Score.szName, Score.nScore);
	}

	fclose(pIn);

	return true;
}

bool SaveHighScores(const char *szFilename)
{
	std::string s = djAppendPathStr(djGetFolderUserSettings().c_str(), szFilename);
	FILE *pOut = fopen(s.c_str(), "w");
	if (pOut==NULL)
	{
		djMSG("SaveHighScores(%s): Failed to create file\n", szFilename);
		return false;
	}
	fprintf(pOut, "%d\n", MIN((int)g_aScores.size(), (int)MAX_HIGHSCORES));
	for ( int i=0; i<(int)(MIN(g_aScores.size(), MAX_HIGHSCORES)); i++ )
	{
		fprintf(pOut, "%s\n", g_aScores[i].szName);
		fprintf(pOut, "%d\n", g_aScores[i].nScore);
	}
	fclose(pOut);
	return true;
}

bool IsNewHighScore(int nScore)
{
	if (nScore==0)
		return false;
	if (g_aScores.size()<MAX_HIGHSCORES)
		return true;
	for ( int i=0; i<(int)g_aScores.size(); i++ )
	{
		if (nScore>g_aScores[i].nScore)
			return true;
	}
	return false;
}

void AddHighScore(const char *szName, int nScore)
{
	for ( int i=0; i<(int)g_aScores.size(); i++ )
	{
		if (nScore>g_aScores[i].nScore)
		{
			SScore Score;
			Score.nScore = nScore;
			Score.SetName(szName);
			g_aScores.insert(g_aScores.begin()+i, Score);
			goto Leave;
		}
	}
	if (g_aScores.size()<MAX_HIGHSCORES)
	{
		SScore Score;
		Score.nScore = nScore;
		Score.SetName(szName);
		g_aScores.push_back(Score);
	}
Leave:
	while (g_aScores.size()>MAX_HIGHSCORES)
		g_aScores.pop_back();
}

void GetHighScore(int nIndex, SScore &Score)
{
	if (nIndex>=(int)g_aScores.size())
	{
		strcpy(Score.szName, "");
		Score.nScore = 0;
		return;
	}
	snprintf(Score.szName, sizeof(Score.szName), "%s", g_aScores[nIndex].szName);
	Score.nScore = g_aScores[nIndex].nScore;
}
