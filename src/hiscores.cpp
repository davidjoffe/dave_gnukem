/*
hiscores.cpp

Copyright (C) 2001-2023 David Joffe

Conceptually should divide this file into more model/view/controller separation? low prio. dj2022-11
*/

#include "config.h"
#include "datadir.h"
#include "djfile.h"
#include "djtypes.h"
#include <stdio.h>
#include <string.h>
#include "djstring.h"
#include <vector>
#include "hiscores.h"
#include "djtypes.h"
#include "menu.h"
#include "graph.h"
#include "djsprite.h"//todo refactor out of here?
#include "djlog.h"
#include "djimage.h"
//------------------------
#ifdef djUNICODE_TTF
#include "djfonts.h"
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
		szName[0] = 0; //<- Make the buffer an empty string
		return;
	}

	// fixmeUNicode dj2022-11 [low prio] but long names that happen to clip in middle of a utf8 multibyte character may leave invalid partial utf8 chars at end of string
	snprintf(szName, sizeof(szName), "%s", szNameNew);

	// Remove any newlines so it doesn't mess with our loading/saving etc.
	for (size_t i = 0; i < strlen(szName); ++i)
	{
		if (szName[i] == '\r' || szName[i] == '\n')
			szName[i] = ' ';//replace with space
	}
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

void ShowHighScores()
{
	const int nYSTART = 20;
	const int nHEIGHTPERROW = 14;

	HighScoresMenu.setSize(0);
	HighScoresMenu.setItems(instructionsHighScoreItems);
	HighScoresMenu.setMenuCursor(instructionsHighScoreCursor);
	HighScoresMenu.setClrBack( djColor(48,66,128) );
	HighScoresMenu.setXOffset( 220 );
	HighScoresMenu.setYOffset ( 160 );
	if (g_pImgHighScores==NULL)
	{
		g_pImgHighScores = new djImage;
		g_pImgHighScores->Load(djDATAPATHc(DATAFILE_IMG_HIGHSCORES));
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

			const unsigned nXPOS = 24 + 11 * 8;

			bool bNEW = false;
			extern djSprite* g_pFont2;
			djImage* pImg = g_pFont8x8;
			if (g_pFont2!=nullptr && g_pFont2->IsLoaded())
			{
				pImg = g_pFont2->GetImage();
				bNEW=true;
			}

			const std::string sText = g_aScores[i].szName;

#ifndef djUNICODE_TTF
			if (!bNEW)
			GraphDrawString(pVisBack, g_pFont8x8, nXPOS, nYSTART + i * nHEIGHTPERROW, (unsigned char*)sText.c_str());
			else
			{
				GraphDrawStringUTF8(pVisBack, pImg, nXPOS, nYSTART + i * nHEIGHTPERROW, 8, 8, (unsigned char*)sText.c_str(), sText.length());
			}
#else
			if (bNEW)
			{
			// dj2022-11 though it's not so easy to do the same cheesey gradient we have on our 8x8 font, I grabbed the colors from that font to create a sort of a gradient anyway across the list of names that matches the visual color look .. not wonderful but not awful
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
			DrawStringUnicodeHelper(pVisBack, nXPOS, nYSTART + i * nHEIGHTPERROW - 6, aColorGrad[i % aColorGrad.size()], sText.c_str(), sText.length());
		}
#endif//#ifndef djUNICODE_TTF
		}//i

		GraphFlip(true);

		// Pop up high scores menu
		do_menu( &HighScoresMenu);
	}
}

bool LoadHighScores(const char *szFilename)
{
	g_aScores.clear();

	if (szFilename == nullptr || szFilename[0] == 0) return false;//sanitychecks

	FILE* pIn = nullptr;// djFile::dj_fopen(s.c_str(), "r");
	// It's normal on first run of application for this file not to exist yet so check so we don't give slightly scary-sounding messages to log about file not found .. [dj2022-12]
	if ( (!djFileExists(szFilename))
		|| ((pIn = djFile::dj_fopen(szFilename, "r")) == NULL) )
	{
		djMSG("LoadHighScores: Failed to open file (%s): Creating default list (this is normal on first run)\n", szFilename);
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

	char buf[4096]={0};
	int nEntries = 0;
	
	#define djREADLINE() buf[0]=0; if ((fgets(buf, sizeof(buf), pIn) == NULL) && ferror(pIn)) goto error; djStripCRLF(buf)

	djREADLINE();
	if (dj_sscanf(buf, "%d", &nEntries) <= 0) goto error;
	for ( int i=0; i<nEntries; i++ )
	{
		SScore Score;

		djREADLINE();
		//fixme dj2022 small bug here still in loading high scores if very long name in file .. we make it safer but it might not load correct because of newlines still
		Score.SetName(buf);

		// NB if we don't have an extra trailing newline on last line I think the last score may not read ..dj2022-11 that's very low prio
		djREADLINE();
		if (buf[0] == 0) goto error;
		if (dj_sscanf(buf, "%d", &Score.nScore) <= 0) goto error;

		AddHighScore(Score.szName, Score.nScore);
	}

	fclose(pIn);

	return true;

error:
	fclose(pIn);
	return false;
}

bool SaveHighScores(const char *szFilename)
{
	if (szFilename == nullptr || szFilename[0] == 0) return false;//sanitychecks

	// [dj2022] we can hopefully soon start treating this as utf8
	FILE *pOut = djFile::dj_fopen(szFilename, "w");
	if (pOut==NULL)
	{
		djMSG("SaveHighScores(%s): Failed to create file\n", szFilename);
		return false;
	}
	fprintf(pOut, "%d\n", djMIN((int)g_aScores.size(), (int)MAX_HIGHSCORES));
	for ( int i=0; i<(int)(djMIN(g_aScores.size(), MAX_HIGHSCORES)); i++ )
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
		Score.szName[0] = 0;//<- Invalid index so return an empty string
		Score.nScore = 0;
		return;
	}
	snprintf(Score.szName, sizeof(Score.szName), "%s", g_aScores[nIndex].szName);
	Score.nScore = g_aScores[nIndex].nScore;
}
