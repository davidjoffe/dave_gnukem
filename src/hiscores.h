/*!
\file    hiscores.h
\brief   High score list
\author  David Joffe

Copyright (C) 2000-2001 David Joffe

License: GNU GPL Version 2 (*not* "later versions")
*/
/*--------------------------------------------------------------------------*/
#ifndef _HISCORES_H_
#define _HISCORES_H_

//! Default high score list file
#define FILE_HIGHSCORES "hiscores.dat"

//! Maximum size of high score list
#define MAX_HIGHSCORES 10

/*
class CHighScoreList
{
public:
*/
	//! A single entry on the high score list
	struct SScore
	{
		//! Name
		char szName[128];
		//! Score
		int nScore;
	};
/*
protected:
};
*/

// per-application init/kill for highscore system
extern void InitHighScores();
extern void KillHighScores();


//! High-scores background "skin" (image file)
#define FILE_IMG_HIGHSCORES "data/hiscores.tga"
//! Display high scores
extern void ShowHighScores();

//! Load high scores from given file
extern bool LoadHighScores(const char *szFilename=FILE_HIGHSCORES);
//! Save high scores to given file
extern bool SaveHighScores(const char *szFilename=FILE_HIGHSCORES);

//! Test if the given score will make the high score list
extern bool IsNewHighScore(int nScore);
//! Add a new entry to the list, automatically sorted. Before calling this, use IsNewHighScore() to see if you should.
extern void AddHighScore(const char *szName, int nScore);

//! Get high score at a particular index. Always succeeds if index is in range [0, MAX_HIGHSCORES)
extern void GetHighScore(int nIndex, SScore &Score);


#endif
