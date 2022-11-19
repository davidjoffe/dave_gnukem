/*
hiscores.cpp

Copyright (C) 2001-2022 David Joffe
*/

#include <stdio.h>
#include <string.h>
#include "djstring.h"

#include <vector>
using namespace std;


#include "hiscores.h"
#include "djtypes.h"
#include "menu.h"
#include "graph.h"
#include "djlog.h"

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
vector<SScore> g_aScores;

djImage *g_pImgHighScores = NULL;

struct SMenuItem instructionsHighScoreItems[] =
{
   /*{ false, "{~~~~~~}" },
   { true,  "|  OK  |" },
   { false, "[~~~~~~]" },*/
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
		djgDrawImage( pVisBack, g_pImgHighScores, 0, 0, g_pImgHighScores->Width(), g_pImgHighScores->Height() );

		for ( int i=0; i<(int)g_aScores.size(); i++ )
		{
			char buf[1024]={0};
			snprintf(buf, sizeof(buf), "%d  %d", i, g_aScores[i].nScore);
			GraphDrawString(pVisBack, g_pFont8x8, 24, 24+i*12, (unsigned char*)buf);
			GraphDrawString(pVisBack, g_pFont8x8, 24+11*8, 24+i*12, (unsigned char*)g_aScores[i].szName);
		}

		GraphFlip(true);

		// Pop up high scores menu
		do_menu( &HighScoresMenu);
	}
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
