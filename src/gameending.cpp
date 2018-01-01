//
// gameending.cpp
//
// Created 2017-08-09
/*
Copyright (C) 1995-2018 David Joffe

License: GNU GPL Version 2
*/
/*--------------------------------------------------------------------------*/
#include "game.h"
#include "graph.h"
#include "djtime.h"//djTimeGetTime() (for frame rate control in instructions screen loop)
#include <vector>
#include <string>

#include"djinput.h"

void ShowEndGameSequence()
{
	unsigned int FW=8;
	unsigned int FH=8;
	std::string sALPHABET=
		" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}";
	djImage* pImgFont = NULL;
	if (pImgFont==NULL)
	{
		pImgFont = new djImage;
		if (pImgFont->Load( "data/fonts/simple_6x8.tga" )>=0)
		{
			djCreateImageHWSurface( pImgFont );
			FW=6;
			FH=8;
		}
		else
		{
			//djDestroyImageHWSurface( pImgFont );
			djDEL(pImgFont);
		}
	}

	std::vector<std::string> asText;
	//asText.push_back("012345678901234567890123456789012345678901234567890123456789");
	//fixme THIS NEEDS to be a bit nicer
	asText.push_back("Well done, you've reached the end");
	asText.push_back("of the game.");
	asText.push_back("");
	asText.push_back("Unfortunately, Dr Proetton escaped!");
	asText.push_back("");
	asText.push_back("To be continued in 'version 2'?");
	asText.push_back("");
	asText.push_back("[Need better game ending text/graphics");
	asText.push_back("etc. here.]");
	asText.push_back("");
	asText.push_back("");

	const int X = 4;//Pixel offset left (top-left corner)
	const int Y = 4;//Pixel offset top (top-left corner)
	const int H = (int)asText.size() + 1;
	int W = (320/8)-1;//5;

	// Draw 'blank' underneath text (pseudo-dialogue-background and border)
	// This is slightly crass .. clear background
	for ( int i=0; i<H; ++i )
	{
		for ( int j=0; j<W; ++j )
		{
			// "Character" 3rd-last from end of [0..255) font character range is for
			// background clear.
			const unsigned char szBACKGROUND[2]={255-2,0};
			GraphDrawString( pVisBack, g_pFont8x8, X+j*8, Y+i*FH, szBACKGROUND );
		}
	}

	//left+top 'light' lines
	djgSetColorFore(pVisBack,djColor(80,80,80));
	djgDrawRectangle( pVisBack,
		X,
		Y,
		1,
		H*FH);
	//top
	djgDrawRectangle( pVisBack,
		X,
		Y,
		W*8,
		1
		);
	//bottom+right 'dark' lines
	djgSetColorFore(pVisBack,djColor(35,35,35));
	djgDrawRectangle( pVisBack,
		X+2,
		Y+H*FH,
		W*8-2,
		1);
	//right
	djgDrawRectangle( pVisBack,
		X+W*8,
		Y+2,
		1,
		H*8-1
		);

	// Draw lines of text
	//GraphDrawString( pVisBack, g_pFont8x8, X+16, 0, (const unsigned char*)"INSTRUCTIONS");
	for ( int i=0; i<(int)asText.size(); ++i )
	{
		GraphDrawString( pVisBack, g_pFont8x8, X+4, (Y+4)+i*FH			+ 8
			, (unsigned char*)asText[i].c_str() );
	}

	// Aim for 60fps
	const float fTimeFrame = (1.0f / 60.0f);

	// Start out by being at next time
	float fTimeNext = djTimeGetTime();
	float fTimeNow = fTimeNext;

	bool bRunning = true;

	do
	{
		fTimeNow = djTimeGetTime();
		fTimeNext = fTimeNow + fTimeFrame;
		
		// Sleep a little to not hog CPU to cap menu update (frame rate) at approx 10Hz
		while (fTimeNow<fTimeNext)
		{
			SDL_Delay(1);
			fTimeNow = djTimeGetTime();
		}

		// [dj2016-10] Re-implementing this to do own djiPollBegin/djiPollEnd in menu instead of calling djiPoll()
		// because of issue whereby key events get 'entirely' missed if up/down even within one 'frame'.
		//djiPollBegin();
		SDL_Event Event;
		while (SDL_PollEvent(&Event))
		{
			switch (Event.type)
			{
			case SDL_KEYDOWN:
				if (
					(Event.key.keysym.sym==SDLK_SPACE
					|| Event.key.keysym.sym==SDLK_RETURN)
					)
				{
					bRunning = false;
				}
				else if (Event.key.keysym.sym==SDLK_ESCAPE
					|| Event.key.keysym.sym==SDLK_SPACE
					|| Event.key.keysym.sym==SDLK_RETURN
					)
				{
					bRunning = false;//Exit screen
				}
				break;
			case SDL_QUIT:
				bRunning=false;
				break;
			}
		}
		djiPollEnd();

		GraphFlip(true);
	} while (bRunning);

	if (pImgFont!=NULL)
	{
		djDestroyImageHWSurface( pImgFont );
		djDEL(pImgFont);
	}

	return;
}
