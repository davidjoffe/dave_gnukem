/*
instructions.cpp

Copyright (C) 1999-2018 David Joffe
*/
#include "djtypes.h"
#include "instructions.h"
#include "graph.h"
#include "djtime.h"//djTimeGetTime() (for frame rate control in instructions screen loop)
#include <vector>
#include <string>

// I don't quite like these two dependencies, but because keys are re-definable, the
// instructions screen 'must' 'know about' them. Hm, a slightly better design might
// be more MVC-ish where e.g. a controller class that 'knows about' both redefinable keys
// and instructions screens could 'pre-configure' the instruction text or something..
// but that's extremely low, bottom of priority list of things worth worrying about.[dj2017-08]
#include "keys.h"
#include"djinput.h"//GetKeyString()

const std::string sPLOT("STORY: The year is $CURRENTYEAR+8. An evil genius, Dr Proetton, has been hired by the CIA to infect the world's computers with a virus called SystemD, crippling them. Only you can stop him. You must find the floppy disk with the Devuan Antivirus on it, and install it on the master computer, which is hidden in Vault7.\n*Any resemblance to actual persons or entities is purely coincidental.\nTL;DR Shoot anything that moves.");
const std::string sABOUT("Dave Gnukem is an open source retro-style 2D scrolling platform shooter similar to, and inspired by, Duke Nukem 1 (a famous 1991 game). It has no affiliation with the original game; this is a \"spare-time\" project created by David Joffe, with contributions by others. It includes a level editor and cross-platform support.\n\nThe original DN1 had 16-color 320x200 graphics. The aim here is similar but different gameplay & 'look and feel' - kind of a parody.\n\nSOURCE: github.com/davidjoffe/dave_gnukem\n\n### ABOUT DUKE NUKEM 1 ###\nDuke Nukem 1 was a famous original 16-color 320x200 'classic' game released by Apogee Software in 1991 that launched the Duke Nukem series. The original Duke Nukem 1 was created by Todd Replogle (co-creator of the Duke Nukem series), John Carmack (of id Software), Scott Miller (founder of 3D Realms), Allen H. Blum III, George Broussard, and Jim Norwood.");

std::string GetKeyStringS(int nSDLKeyCode)
{
	return std::string( GetKeyString(nSDLKeyCode) );
}

// Wrap long string into multiple lines, at a given width, at word breaks
// This is extremely 'dumb'. Doesn't handle Unicode, doesn't handle non-fixed-width
// fonts, etc. Not important now.
void WrapString(const std::string& sStr, unsigned int uWidth, std::vector<std::string>& asLines)
{
	std::string sBuildLine;
	std::string sBuildWord;
	for ( unsigned int i=0; i<sStr.length(); ++i )
	{
		char c=sStr[i];

		if (c=='\n')
		{
			asLines.push_back( sBuildLine );// Not sure if this is quite right [low] [dj2017-08]
			sBuildLine.clear();
		}
		else if (c==' ')
		{
			int nPos1 = sStr.find(' ',i+1);
			//int nPos2 = sStr.find(i+1,'\n');//Hm
			int nPos = nPos1;
			unsigned int uLenNextWord=0;
			if (nPos<0)
			{
				uLenNextWord = sStr.length() - (i+1);
			}
			else
			{
				uLenNextWord = nPos - (i+1);
			}

			if (sBuildLine.length()+1+uLenNextWord>uWidth)
			{
				asLines.push_back( sBuildLine );
				sBuildLine.clear();
			}
			else
				sBuildLine += c;
		}
		else
		{
			sBuildLine += c;
		}
	}

	if (!sBuildLine.empty())
	{
		asLines.push_back( sBuildLine );
	}
}

// Helper: If string less wide than given width uLen, returns a string that would
// 'pad it out' to that width
std::string PadSpaces(const std::string& sStr, unsigned int uLen)
{
	if (sStr.length()<uLen)
		return std::string(uLen-sStr.length(), ' ');
	return "";
}
//-------------------------------------------------------
void DoShow(const std::string& sLinesOrigText, const std::vector<std::string>& asText,float fApproxFramerateTarget=100.f)
{
	unsigned int FW=8;
	unsigned int FH=8;
	std::string sALPHABET=" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}";
	djImage* pImgFont = NULL;
	djImage* pImgFontS = NULL;//shadow
	pImgFont = new djImage;
	if (pImgFont->Load( "data/fonts/simple_6x8.tga" )>=0)
	{
		djCreateImageHWSurface( pImgFont );
		FW=6;
		FH=8;
	}
	else
	{
		djDEL(pImgFont);
	}
	if (!pImgFont)return;
	pImgFontS = new djImage;
	if (pImgFontS->Load( "data/fonts/simple_6x8_shadow.tga" )>=0)
	{
		djCreateImageHWSurface( pImgFontS );
	}
	else
	{
		djDEL(pImgFontS);
	}
	//CBSU[could be sped up'- fixFUtureMaybe]: Pre-build "map" of characters to their indexes in the fontsprite: std::map<char,unsigned int> mapFontChars;

	unsigned int uWIDTH_NUMCOLS = (320 - 16) / FW;

	std::vector<std::string> asLines;
	WrapString(sLinesOrigText, uWIDTH_NUMCOLS, asLines);

	// Note here some of the units are in pixels, others in '8-pixel-block' units

	const int X = 4;//Pixel offset left (top-left corner)
	const int Y = 4;//Pixel offset top (top-left corner)
	const int H = (int)asText.size() + 1
		+ (int)asLines.size()
		;
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
			GraphDrawString( pVisBack, g_pFont8x8, X+j*8, Y+i*8, szBACKGROUND );
		}
	}

	//left+top 'light' lines
	djgSetColorFore(pVisBack,djColor(80,80,80));
	djgDrawRectangle( pVisBack,
		X,
		Y,
		1,
		H*8);
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
		Y+H*8,
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
		GraphDrawString( pVisBack, g_pFont8x8, X+4, (Y+4)+i*8
			+ asLines.size()*8+4
			, (unsigned char*)asText[i].c_str() );
	}

	// Aim (approximately) for some given framerate, which will also determine the text effect speed
	const float fTimeFrame = (1.0f / fApproxFramerateTarget);

	// Start out by being at next time
	float fTimeNext = djTimeGetTime();
	float fTimeNow = fTimeNext;

	bool bRunning = true;
	bool bTypingFinished = false;
	bool bFinishTyping = false;//If press escape first time, and not finished typing, instant-finish

	// For 'typing' effect: Current line number, and offset into current string within line
	unsigned int uTypingCurLine=0;
	unsigned int uTypingCurStrPos=0;

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

		// Advance 'typing position' by 1
		do
		{
			// Should add some randomness, but [low] this is 'OK' for now
			if (uTypingCurLine<asLines.size())
			{
				if (uTypingCurStrPos>=asLines[uTypingCurLine].length())
				{
					// NEXT LINE
					++uTypingCurLine;
					uTypingCurStrPos = 0;
					if (uTypingCurLine>=asLines.size())
						bTypingFinished = true;
				}
				else
				{
					// future? add a 'cursor'
					if (pImgFont)
					{
						// Find character index using alphabet
						char c = asLines[uTypingCurLine][uTypingCurStrPos];
						int nAlphPos = (int)sALPHABET.find(c);
						if (nAlphPos>=0)
						{
							// Blit from font (first dropshadow version, if available)
							if (pImgFontS) djgDrawImageAlpha( pVisBack, pImgFontS,
								FW*nAlphPos,0,
								X + 4 + uTypingCurStrPos*FW + 1, Y + 4 + uTypingCurLine*FH + 1,
								FW,FH);
							djgDrawImageAlpha( pVisBack, pImgFont,
								FW*nAlphPos,0,
								X + 4 + uTypingCurStrPos*FW, Y + 4 + uTypingCurLine*FH,
								FW,FH);
						}
					}
					else
					{
						// (Not currently used)

						// Need NULL-separated string that's one character. So, two chars,
						// first is the character, and second is a 0 to terminate string.
						unsigned char szCurrentCharacter[2]={0};
						szCurrentCharacter[0] = asLines[uTypingCurLine][uTypingCurStrPos];
						GraphDrawString( pVisBack, g_pFont8x8,
							X+4+uTypingCurStrPos*8, (Y+4)+uTypingCurLine*8,
							szCurrentCharacter );
					}

					++uTypingCurStrPos;
				}
			}
		} while ( bFinishTyping ? !bTypingFinished : false );
		// ^ This do/while condition looks a bit weird, but basically all it's
		// doing is simple: If 'insta-finish-typing' is set, then it types one by
		// one *all at once* until finished typing. Otherwise, it just performs
		// one iteration of the 'loop' (to type a single character). So either loops
		// all, or 1. [dj2017-08]


		// [dj2016-10] Re-implementing this to do own djiPollBegin/djiPollEnd in menu instead of calling djiPoll()
		// because of issue whereby key events get 'entirely' missed if up/down even within one 'frame'.
		//djiPollBegin();
		SDL_Event Event;
		while (SDL_PollEvent(&Event))
		{
			switch (Event.type)
			{
			case SDL_KEYDOWN:
				if (!bTypingFinished &&
					(Event.key.keysym.sym==SDLK_SPACE
						|| Event.key.keysym.sym==SDLK_RETURN)
					)
				{
					bFinishTyping = true;//Insta-finish typing
				}
				else if (Event.key.keysym.sym==SDLK_ESCAPE
					|| Event.key.keysym.sym==SDLK_SPACE
					|| Event.key.keysym.sym==SDLK_RETURN
					)
				{
					if (!bTypingFinished)
						bFinishTyping = true;//Insta-finish typing
					else
						bRunning = false;//Exit instruction screen
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
	if (pImgFontS!=NULL)
	{
		djDestroyImageHWSurface( pImgFontS );
		djDEL(pImgFontS);
	}
}
void ShowInstructions()
{
	std::vector<std::string> asText;
	//asText.push_back("");
	asText.push_back("### INSTRUCTIONS ###");
	unsigned int uWIDTH = (320 - 16) / 8;
	WrapString("Find the exit in each level, while dodging or shooting monsters.", uWIDTH, asText);

	asText.push_back("");
	asText.push_back("### KEYS ###");

	char szBuf[1024] = {0};
	sprintf(szBuf, "%s", GetKeyString(g_anKeys[KEY_LEFT]));
	asText.push_back(std::string("LEFT:  ") + szBuf
		+ PadSpaces(szBuf,18-7) + std::string("SHOOT: ") + GetKeyStringS(g_anKeys[KEY_SHOOT]) );
	sprintf(szBuf, "%s", GetKeyString(g_anKeys[KEY_RIGHT]));
	asText.push_back(std::string("RIGHT: ") + szBuf
		+ PadSpaces(szBuf,18-7) + std::string("JUMP:  ") + GetKeyStringS(g_anKeys[KEY_JUMP]) );
	sprintf(szBuf, "%s", GetKeyString(g_anKeys[KEY_ACTION]));
	//asText.push_back(std::string(" Action: ") + szBuf + std::string(" (Activate doors,"));
	//asText.push_back("  exits, lifts, teleporters etc.)");
	asText.push_back(std::string("ACTION:") + szBuf + std::string(" Activate doors, lifts, etc."));

	// Hrm, these may be platform specific, fixmeLOW:
	asText.push_back("6/7: Volume Dn/Up  INS: Toggle sounds");
	asText.push_back("ESC: In-game menu: Save/Restore/Quit");
	//asText.push_back("F4,F5: Sprite/Level editors");
	//asText.push_back("Use the action key to open doors, activate lifts, teleporters, etc.");
	asText.push_back("SECRET BONUSES:");
	asText.push_back("-Shoot all security cameras in level");
	asText.push_back("-Collect letters G,N,U,K,E,M in order");

	DoShow(sPLOT, asText);
}

void ShowAbout()
{
	std::vector<std::string> asText;//<- Empty for the about page.
	DoShow(sABOUT,asText,200.f);
}
