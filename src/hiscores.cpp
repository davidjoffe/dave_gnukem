/*
hiscores.cpp

Copyright (C) 2001-2002 David Joffe

License: GNU GPL Version 2 (*not* "later versions")
*/

#include <stdio.h>
#include <string.h>

#include "mmgr/nommgr.h"
#include <vector>
using namespace std;
#include "mmgr/mmgr.h"


#include "hiscores.h"
#include "djtypes.h"
#include "menu.h"
#include "graph.h"
#include "djlog.h"

// Scores, sorted from highest to lowest
vector<SScore> g_aScores;

djImage *g_pImgHighScores = NULL;

struct SMenuItem instructionsHighScoreItems[] =
{
   { false, "{~~~~~~}" },
   { true,  "|  OK  |" },
   { false, "[~~~~~~]" },
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
	djDEL(g_pImgHighScores);
}

void ShowHighScores()
{
	HighScoresMenu.setSize(0);
	HighScoresMenu.setItems(instructionsHighScoreItems);
	HighScoresMenu.setMenuCursor(instructionsHighScoreCursor);
	HighScoresMenu.setClrBack( djColor(0,173,173) );
	HighScoresMenu.setXOffset( 220 );
	HighScoresMenu.setYOffset ( 160 );
	if (g_pImgHighScores==NULL)
	{
		g_pImgHighScores = new djImage;
		g_pImgHighScores->Load(FILE_IMG_HIGHSCORES);
	}
	if (g_pImgHighScores)
	{
		djgDrawImage( pVisBack, g_pImgHighScores, 0, 0, g_pImgHighScores->Width(), g_pImgHighScores->Height() );

		for ( int i=0; i<(int)g_aScores.size(); i++ )
		{
			char buf[128];
			sprintf(buf, "%d  %d", i, g_aScores[i].nScore);
			GraphDrawString(pVisBack, g_pFont8x8, 24, 24+i*12, (unsigned char*)buf);
			sprintf(buf, "%s", g_aScores[i].szName);
			GraphDrawString(pVisBack, g_pFont8x8, 24+11*8, 24+i*12, (unsigned char*)buf);
		}

		GraphFlip();

		// Pop up high scores menu
		do_menu( &HighScoresMenu);
	}
}

bool LoadHighScores(const char *szFilename)
{
	g_aScores.clear();

	FILE *pIn = fopen(szFilename, "r");
	if (pIn==NULL)
	{
		djMSG("LoadHighScores: Failed to open file (%s): Creating default list\n", szFilename);
		AddHighScore("Todd", 40000);
		AddHighScore("Scott", 30000);
		AddHighScore("George", 20000);
		AddHighScore("Al", 10000);
		AddHighScore("David", 5000);
		return false;
	}

	char buf[512];

	fgets(buf, sizeof(buf), pIn);
	djStripCRLF(buf); // strip CR/LF characters
	int nEntries = 0;
	sscanf(buf, "%d", &nEntries);
	for ( int i=0; i<nEntries; i++ )
	{
		SScore Score;

		fgets(buf, sizeof(buf), pIn);
		djStripCRLF(buf); // strip CR/LF characters
		strcpy(Score.szName, buf);

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
	FILE *pOut = fopen(szFilename, "w");
	if (pOut==NULL)
	{
		djMSG("SaveHighScores(%s): Failed to create file\n", szFilename);
		return false;
	}
	fprintf(pOut, "%d\n", MIN(g_aScores.size(), MAX_HIGHSCORES));
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
			strcpy(Score.szName, szName);
			g_aScores.insert(g_aScores.begin()+i, Score);
			goto Leave;
		}
	}
	if (g_aScores.size()<MAX_HIGHSCORES)
	{
		SScore Score;
		Score.nScore = nScore;
		strcpy(Score.szName, szName);
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
	strcpy(Score.szName, g_aScores[nIndex].szName);
	Score.nScore = g_aScores[nIndex].nScore;
}

