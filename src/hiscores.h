/*!
\file    hiscores.h
\brief   High score list
\author  David Joffe

Copyright (C) 2000-2022 David Joffe
*/
/*--------------------------------------------------------------------------*/
#ifndef _HISCORES_H_
#define _HISCORES_H_

//! Default high score list file
#define FILE_HIGHSCORES "hiscores.dat"

//! Maximum size of high score list
#define MAX_HIGHSCORES 12

/*
class CHighScoreList
{
public:
*/
	//! A single entry on the high score list
	struct SScore
	{
		SScore();

		//! Adding 'set' helper function that's safer [dj2022-11]
		void SetName(const char* szNameNew);

		//! Person's name for high score list entry (dj2022-11 note: if we assume later we try globalized Unicode support then perhaps this would become utf8, so 'effective length' may be shorter than this length of a person's name in that case as it's multibyte so just keep that in mind, but as of 2022/11 we don't support Unicode)
		char szName[1024] = { 0 };
		//! Score achieved
		int nScore = 0;
	};
/*
protected:
};
*/

// per-application init/kill for highscore system
extern void InitHighScores();
extern void KillHighScores();


//! High-scores background "skin" (image file)
#define DATAFILE_IMG_HIGHSCORES "hiscores.tga"
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
