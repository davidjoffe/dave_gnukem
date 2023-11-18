/*--------------------------------------------------------------------------*/
/* David Joffe '95/07/20 */
/*
menu.cpp

Copyright (C) 1995-2023 David Joffe
*/

#include "graph.h"
#include "console.h"//SetConsoleMessage

#include <string.h>

#include "menu.h"

#include "djlang.h"//djLang::DoTranslations()
#include "djsprite.h"
#include "djimage.h"
#include "djstring.h"
#include "djinput.h"
#include "djtime.h"
#include "sys_error.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

djImage* g_pImgMenuBackground8x8 = NULL;//dj2016-10 background 'noise' image experiment
/*--------------------------------------------------------------------------*/
djMenuCursorSprite* g_pDefaultMenuCursor=nullptr;
int g_nMenuCursorAnimOffset=0;
/*--------------------------------------------------------------------------*/

void menu_move( CMenu *pMenu, int& option, int diff, unsigned char cCursor, int iFirstSelectable, int iLastSelectable)
{
	if (pMenu==nullptr) return;//sanitycheck
	int x=pMenu->getXOffset()+8;
	int y=pMenu->getYOffset()+option*8;
	//djgSetColorFore( pVisBack, pMenu->getClrBack() );
	//djgDrawBox( pVisBack, pMenu->getXOffset()+8, pMenu->getYOffset()+option*8, 8, 8 );
	// Clear cursor at current position
	djgDrawImage( pVisBack, g_pImgMenuBackground8x8, x, y, 8, 8 );

	int iOptionPrev = option;
	option += diff;
	if (option < iFirstSelectable)
		option = iLastSelectable;//wrap
	if (option > iLastSelectable)
		option = iFirstSelectable;//wrap

	// Play a sound
	if (option != iOptionPrev)
	{
		djSoundPlay( pMenu->getSoundMove () );
	}

	y=pMenu->getYOffset()+option*8;

	// Redraw the menu cursor
	// First check if custom menu cursor sprite e.g. the skull one selected for menu; if not, try the default one (if any)
	djSprite* pSprite =  pMenu->GetMenuCursorSprite();
	if (pSprite==nullptr || !pSprite->IsLoaded())
	{
		pSprite = (g_pDefaultMenuCursor!=nullptr && g_pDefaultMenuCursor->m_pSprite!=nullptr && g_pDefaultMenuCursor->m_pSprite->IsLoaded() ? g_pDefaultMenuCursor->m_pSprite : nullptr);
	}
	if (pSprite!=nullptr && pSprite->GetImage()!=nullptr)
	{
		if (g_nMenuCursorAnimOffset>=pSprite->GetNumSpritesX())
			g_nMenuCursorAnimOffset = 0;
		djgDrawImageAlpha( pVisBack, pSprite->GetImage(),
			g_nMenuCursorAnimOffset * pSprite->GetSpriteW(), 0, x, y,
			pSprite->GetSpriteW(),
			pSprite->GetSpriteH());
	}
	else//todo:deprecate: fallback: old way (old hardcoded font.tga):
	djgDrawImageAlpha( pVisBack, g_pFont8x8, ((int)cCursor%32)*8, ((int)cCursor/32)*8, x, y, 8, 8 );
}
/*--------------------------------------------------------------------------*/
#ifdef __EMSCRIPTEN__
struct MenuPumpInfo
{
	CMenu *pMenu;
	int option;
	float fTimeNext;
	float fTimeNow;
	bool bmenurunning;
	const unsigned char *szCursor;
};

MenuPumpInfo menuPumpInfo;

void do_menu_pump()
{
	const float fTimeFrame = 1.0f / 30.0f;

	menuPumpInfo.fTimeNow = djTimeGetTime();
	menuPumpInfo.fTimeNext = menuPumpInfo.fTimeNow + fTimeFrame;
	
	// Sleep a little to not hog CPU to cap menu update (frame rate) at approx 10Hz
	while (menuPumpInfo.fTimeNow<menuPumpInfo.fTimeNext)
	{
		//--dh-- SDL_Delay(1);
		menuPumpInfo.fTimeNow = djTimeGetTime();
	}

	// [dj2016-10] Re-implementing this to do own djiPollBegin/djiPollEnd in menu instead of calling djiPoll()
	// because of issue whereby key events get 'entirely' missed if up/down even within one 'frame'.
	djiPollBegin();
	SDL_Event Event;
	while (SDL_PollEvent(&Event))
	{
		switch (Event.type)
		{
		case SDL_KEYDOWN:

			// 'Global' shortcut keys for adjusting volume [dj2016-10]
			if (Event.key.keysym.sym==SDLK_7)//SDLK_PAGEUP)
			{
				djSoundAdjustVolume(4);
				SetConsoleMessage( djStrPrintf( "Volume: %d%%", (int) ( 100.f * ( (float)djSoundGetVolume()/128.f ) ) ) );
			}
			else if (Event.key.keysym.sym==SDLK_6)//SDLK_PAGEDOWN)
			{
				djSoundAdjustVolume(-4);
				SetConsoleMessage( djStrPrintf( "Volume: %d%%", (int) ( 100.f * ( (float)djSoundGetVolume()/128.f ) ) ) );
			}
			else if (Event.key.keysym.sym==SDLK_INSERT)
			{
				if (djSoundEnabled())
					djSoundDisable();
				else
					djSoundEnable();
				SetConsoleMessage( djSoundEnabled() ? "Sounds ON (Ins)" : "Sounds OFF (Ins)" );
			}

			// up arrow
			else if (Event.key.keysym.sym==SDLK_UP)
				menu_move( menuPumpInfo.pMenu, menuPumpInfo.option, -1, *menuPumpInfo.szCursor );

			// down arrow
			else if (Event.key.keysym.sym==SDLK_DOWN)
				menu_move( menuPumpInfo.pMenu, menuPumpInfo.option, 1, *menuPumpInfo.szCursor );

			// home key
			else if (Event.key.keysym.sym==SDLK_HOME)//g_iKeys[DJKEY_HOME])
				menu_move( menuPumpInfo.pMenu, menuPumpInfo.option, -menuPumpInfo.option + menuPumpInfo.pMenu->getSize() - 1, *menuPumpInfo.szCursor );

			// end key
			else if (Event.key.keysym.sym==SDLK_END)//if (g_iKeys[DJKEY_END])
				menu_move( menuPumpInfo.pMenu, menuPumpInfo.option, -menuPumpInfo.option, *menuPumpInfo.szCursor );

			// enter
			else if (Event.key.keysym.sym==SDLK_RETURN)//if (g_iKeys[DJKEY_ENTER])
				menuPumpInfo.bmenurunning = 0;

			// escape
			else if (Event.key.keysym.sym==SDLK_ESCAPE)//if (g_iKeys[DJKEY_ESC])
			{
				menuPumpInfo.option = -1;
				menuPumpInfo.bmenurunning = 0;
			}

			break;
		case SDL_KEYUP:
			break;
		case SDL_QUIT:
			menuPumpInfo.bmenurunning=0;
			menuPumpInfo.option = -1;//Exit
			break;
		}
	}
	djiPollEnd();

	// [dj2016-10] this if seems silly here to me but if i take it out, then as you press Esc on menu,
	// it draws some 'wrong' stuff for one frame .. whatever, just adding this if back again
	if (menuPumpInfo.bmenurunning)
	{
		// Animate cursor [note this is unfortunately currently a bit 'tied' to the 10Hz frame rate limit ...
		// if want to e.g. increase menu frame rate in future to say 20Hz or whatever, then the cursor will
		// animate two times too fast (say) .. if do that in future then must just make this update slightly
		// 'smarter' on the animation - not a priority now at all. dj2016-10]
		static int nCursorAnimUpdateEvery=3;//<-dj2018-03-30 compensate to try keep cursor animation around ~10Hz
		if (--nCursorAnimUpdateEvery<=0)
		{
			menuPumpInfo.szCursor++;
			nCursorAnimUpdateEvery = 3;
		}
		if (*menuPumpInfo.szCursor == 0)
			menuPumpInfo.szCursor = menuPumpInfo.pMenu->getMenuCursor ();

		menu_move( menuPumpInfo.pMenu, menuPumpInfo.option, 0, *menuPumpInfo.szCursor );//Force redraw of cursor for animation purposes
	}

	GraphFlip(true);
}

#endif//__EMSCRIPTEN__

/*--------------------------------------------------------------------------*/
int do_menu( CMenu *pMenu )
{
	//printf("do_menu\n");fflush(nullptr);
	//dj2016-10-28 trying background image with 'noise' instead of solid background color for menu ..
	if (g_pImgMenuBackground8x8==NULL)
	{
		g_pImgMenuBackground8x8 = new djImage;
		if (g_pImgMenuBackground8x8->Load(djDATAPATHc("menucharbackground.tga")) >= 0)
			djCreateImageHWSurface(g_pImgMenuBackground8x8);
		else
			printf("Warning: Menu failed to load charbackground\n");
	}

	// Initialize cursor animation
	const unsigned char *szCursor = pMenu->getMenuCursor();

	// calculate size of menu
	int size = 0;
	for ( size=0; !pMenu->getItems()[size].IsTerminal(); size++ )
		;

	pMenu->setSize ( size );
	if (size==0)return -1;//error[dj2018-03]


	//dj2022-11 to help make more generic
	int iFirstSelectable = -1;

	// set default option (should auto-detect first 'valid selectable' item)
	int i = 0;
	for (i = 0; i < size; i++)
	{
		if (pMenu->getItems()[i].IsSelectable())
		{
			iFirstSelectable = i;
			break;
		}
	}
	if (iFirstSelectable < 0) return -1;//error no selectable items [hm what we should do, still draw?]

	int iLastSelectable = -1;
	// auto-detect LAST 'valid selectable' item
	for (int i = pMenu->getSize() - 1; i >= 0; --i)
	{
		if (pMenu->getItems()[i].IsSelectable())
		{
			iLastSelectable = i;
			break;
		}
	}

	//dj2022-11 add auto-padding to width of widest string (not sure if good/bad idea but anyway for now probably better)
	size_t lenLongestString = 0;
	for (i = 0; i < size; i++)
	{
		if (pMenu->getItems()[i].GetTextStr().empty()) continue;
		const std::string& sText = pMenu->getItems()[i].GetTextStr();
		// This weird code is to remove the "   " padding from front of items, so that we can get a more accurate width of the text
		// It should be deprecated once we remove all the 'hardcoded' leading 3-space-padding from menu text items ...
		// We want to use this lenLongestString info to calculate things like how wide to draw the menu box and its dropshadow etc.
		size_t len = sText.length();
		if (sText.substr(0,3)=="   ")
			len = len - 3;
		if (len > lenLongestString)
			lenLongestString = len;
	}
	// Since we're using 8x8 font, add 3 pixels padding ... but since we're removing the "   " padding from front of items, add some to leave space for cursor and indent etc.
	lenLongestString += 3;

	// default selected option
	int option = iFirstSelectable;

	// Calculate position of menu on screen, if "asked" to do so (== -1)
	if ( pMenu->getXOffset() < 0)
		pMenu->setXOffset ( 8 * (20 - ((int)lenLongestString / 2)));
	if ( pMenu->getYOffset() < 0)
		pMenu->setYOffset( 8 * (12 - (size / 2)) );

	// Draw dropshadows [dj2018-03-30]
	//DrawDropShadowHelper(pMenu, size,pMenu->getXOffset(),pMenu->getYOffset(),	);
	const unsigned char szTR[2]={(unsigned char)251,0};//top right
	const unsigned char szR [2]={(unsigned char)252,0};//right
	const unsigned char szBR[2]={(unsigned char)250,0};//bottom right
	const unsigned char szB [2]={(unsigned char)249,0};//bottom
	const unsigned char szBL[2]={(unsigned char)248,0};//bottom left
	extern djSprite* g_pShadow;//<- new better more generic shadow in own sprite [dj2023]
	//right and top right
	for ( i=0; i<size; ++i )
	{
		const int x = pMenu->getXOffset() + (int)lenLongestString*8;
		if (g_pShadow)
			djgDrawImageAlpha( pVisBack, g_pShadow->GetImage(), (i==0 ? 3 : 4)*8, 0, x, pMenu->getYOffset()+i*8, 8, 8);
		else
			GraphDrawString( pVisBack, g_pFont8x8, x, pMenu->getYOffset()+i*8, i==0?szTR:szR );
	}
	// bottom
	for (size_t x = 0; x < lenLongestString; ++x)
	{
		if (g_pShadow)
			djgDrawImageAlpha( pVisBack, g_pShadow->GetImage(), 1*8, 0, pMenu->getXOffset() + (int)x * 8, pMenu->getYOffset() + size * 8, 8, 8);
		else
			GraphDrawString(pVisBack, g_pFont8x8, pMenu->getXOffset() + (int)x * 8, pMenu->getYOffset() + size * 8, szB);
	}
	// bottom right
	if (g_pShadow)
		djgDrawImageAlpha( pVisBack, g_pShadow->GetImage(), 2*8, 0, pMenu->getXOffset()+ (int)lenLongestString*8, pMenu->getYOffset()+size*8, 8, 8);
	else
		GraphDrawString( pVisBack, g_pFont8x8, pMenu->getXOffset()+ (int)lenLongestString*8, pMenu->getYOffset()+size*8, szBR );
	// bottom left
	if (g_pShadow)
		djgDrawImageAlpha( pVisBack, g_pShadow->GetImage(), 0*8, 0, pMenu->getXOffset(), pMenu->getYOffset()+size*8, 8, 8);
	else
		GraphDrawString( pVisBack, g_pFont8x8, pMenu->getXOffset(), pMenu->getYOffset()+size*8, szBL );

	// draw menu
	for ( i=0; i<size; i++ )
	{
		// Draw blank underneath menu

		for ( unsigned int j=0; j<(unsigned int)lenLongestString; ++j )
		{
			djgDrawImage( pVisBack, g_pImgMenuBackground8x8, pMenu->getXOffset()+j*8, pMenu->getYOffset()+i*8, 8, 8 );
		}

		//left+top 'light' lines
		djgSetColorFore(pVisBack,djColor(80,80,80));
		djgDrawRectangle( pVisBack,
			pMenu->getXOffset(),
			pMenu->getYOffset(),
			1,
			size*8);
		//top
		djgDrawRectangle( pVisBack,
			pMenu->getXOffset(),
			pMenu->getYOffset(),
			(int)lenLongestString*8,
			1
			);

		//bottom+right 'dark' lines
		djgSetColorFore(pVisBack,djColor(35,35,35));
		djgDrawRectangle( pVisBack,
			pMenu->getXOffset()+2,
			pMenu->getYOffset()+size*8,
			(int)lenLongestString*8-2,
			1);
		//right
		djgDrawRectangle( pVisBack,
			pMenu->getXOffset()+(int)lenLongestString*8,
			pMenu->getYOffset()+2,
			1,
			size*8-1
			);

		const SMenuItem& Item = *(pMenu->getItems() + i);

		// Draw menu text
		// If localized, use utf8 GraphDrawString version, else use the old one (font.tga based which is kind of gross and maybe should be deprecated or refactored or renamed as UTF8 should be regarded as 'the norm' this isn't the 90s anymore, it's 2023, though this code literally comes from the 90s)
		if (!djLang::DoTranslations())
		{
			// If it's selectable item we right-indent it to make space for cursor (unless it already starts with three spaces)
			if (Item.IsSelectable() && Item.GetTextStr().substr(0,3)!="   ")
			{
				// We must leave some space for menu cursor on left of selectable items shoudl do this differently though)
				// TODO: DEPRECATE THIS LEADING SPACES STUFF?
				// now we can start tweaking this e.g. to save space on screen we can shave some pixels, doesn't have to be exact align to 8 anymore once we localize all menu strings and strip leading spaces that are/were hard-baked into strongs
				GraphDrawString( pVisBack, g_pFont8x8, pMenu->getXOffset()+3*8 + Item.m_Pos.x , pMenu->getYOffset()+i*8, (unsigned char*)Item.GetTextStr().c_str() );
			}
			else
				GraphDrawString( pVisBack, g_pFont8x8, pMenu->getXOffset() + Item.m_Pos.x, pMenu->getYOffset()+i*8, (unsigned char*)Item.GetTextStr().c_str() );
		}
		// Draw menu text
		if (djLang::DoTranslations())
		{
			extern djSprite* g_pFont2;
			const SMenuItem& Item = *(pMenu->getItems() + i);
			// Draw menu text
			// If it's selectable item we right-indent it to make space for cursor (unless it already starts with three spaces)
			if (Item.IsSelectable() && Item.GetTextStr().substr(0,3)!="   ")
			{
				// We must leave some space for menu cursor on left of selectable items shoudl do this differently though)
				// TODO: DEPRECATE THIS LEADING SPACES STUFF?
				// now we can start tweaking this e.g. to save space on screen we can shave some pixels, doesn't have to be exact align to 8 anymore once we localize all menu strings and strip leading spaces that are/were hard-baked into strongs
				GraphDrawStringUTF8( pVisBack, g_pFont2->GetImage(), pMenu->getXOffset()+3*8 + Item.m_Pos.x , pMenu->getYOffset()+i*8, 8, 8, (unsigned char*)Item.GetTextStr().c_str() );
			}
			else
				GraphDrawStringUTF8( pVisBack, g_pFont2->GetImage(), pMenu->getXOffset() + Item.m_Pos.x, pMenu->getYOffset()+i*8, 8, 8, (unsigned char*)Item.GetTextStr().c_str() );
		}

	}
	menu_move( pMenu, option, 0, *szCursor, iFirstSelectable, iLastSelectable);


#ifdef __EMSCRIPTEN__
	// initialize menu pump
	menuPumpInfo.pMenu = pMenu;
	menuPumpInfo.option = option;
	menuPumpInfo.fTimeNext = djTimeGetTime();
	menuPumpInfo.fTimeNow = menuPumpInfo.fTimeNext;
	menuPumpInfo.bmenurunning = true;
	menuPumpInfo.szCursor = szCursor;
#endif


	#ifndef __EMSCRIPTEN__
	// Wait for user to let go of escape, if he is pressing it
	djiWaitForKeyUp( DJKEY_ESC );



	// [dj2018-03] Changing this from 10 to 30 for faster key response, but we must maintain ~10 fps on menu cursor animations, so we do some stuff at the cursor animation to compensate for htis.
	const float fTimeFrame = (1.0f / 30.0f);

	// Start out by being at next time
	float fTimeNext = djTimeGetTime();
	float fTimeNow = fTimeNext;

	bool bmenurunning = true;

	// dj2022-11 If an A-Z key pressed, see if we can find a selectable menu item that starts with that letter and select that (Duke Nukem did similar)
	bool bAllowMenuKeyLetterShortcuts = true;
#ifdef djCFG_DISABLE_MENU_KEYLETTERSHORTCUTS
	bAllowMenuKeyLetterShortcuts = false;
#endif

	do
	{
		const float fTimeFirst = djTimeGetTime();
		fTimeNow = fTimeFirst;//djTimeGetTime();
		fTimeNext = fTimeNow + fTimeFrame;
		
		// Sleep a little to not hog CPU to cap menu update (frame rate) at approx 10Hz
		while (fTimeNow<fTimeNext)
		{
#ifdef __EMSCRIPTEN__
			//--dh-- SDL_Delay(1);
#else
			SDL_Delay(1);
#endif
			fTimeNow = djTimeGetTime();
		}

		
		
		// Menu cursor steady animation rate regardless of frame rate ..
		static float fTimePassed = 0;
		fTimePassed += (fTimeNow - fTimeFirst);
		if (fTimePassed>0.1f)//About 10Hz (but do some catch-up if slow FPS)
		{
			while (fTimePassed>0.1f)
				fTimePassed -= 0.1f;
			++g_nMenuCursorAnimOffset;
			//g_nMenuCursorAnimOffset = (g_nMenuCursorAnimOffset % pSprite->GetNumSpritesX());
		}


		// [dj2016-10] Re-implementing this to do own djiPollBegin/djiPollEnd in menu instead of calling djiPoll()
		// because of issue whereby key events get 'entirely' missed if up/down even within one 'frame'.
		djiPollBegin();
		SDL_Event Event;
		while (SDL_PollEvent(&Event))
		{
			switch (Event.type)
			{
			case SDL_KEYDOWN:

				// 'Global' shortcut keys for adjusting volume [dj2016-10]
				if (Event.key.keysym.sym==SDLK_7)//SDLK_PAGEUP)
				{
					djSoundAdjustVolume(4);
					djConsoleMessage::SetConsoleMessage(djStrPrintf( "Volume: %d%%", (int) ( 100.f * ( (float)djSoundGetVolume()/128.f ) ) ) );
				}
				else if (Event.key.keysym.sym==SDLK_6)//SDLK_PAGEDOWN)
				{
					djSoundAdjustVolume(-4);
					djConsoleMessage::SetConsoleMessage(djStrPrintf( "Volume: %d%%", (int) ( 100.f * ( (float)djSoundGetVolume()/128.f ) ) ) );
				}
				else if (Event.key.keysym.sym==SDLK_INSERT)
				{
					if (djSoundEnabled())
						djSoundDisable();
					else
						djSoundEnable();
					djConsoleMessage::SetConsoleMessage(djSoundEnabled() ? "Sounds ON (Ins)" : "Sounds OFF (Ins)" );
				}

				// up arrow
				else if (Event.key.keysym.sym==SDLK_UP)
					menu_move( pMenu, option, -1, *szCursor, iFirstSelectable, iLastSelectable);

				// down arrow
				else if (Event.key.keysym.sym==SDLK_DOWN)
					menu_move( pMenu, option, 1, *szCursor, iFirstSelectable, iLastSelectable);

				// home key
				else if (Event.key.keysym.sym==SDLK_HOME)//g_iKeys[DJKEY_HOME])
					menu_move( pMenu, option, -option + pMenu->getSize() - 1, *szCursor, iFirstSelectable, iLastSelectable);

				// end key
				else if (Event.key.keysym.sym==SDLK_END)//if (g_iKeys[DJKEY_END])
					menu_move( pMenu, option, -option, *szCursor, iFirstSelectable, iLastSelectable);

				// enter
				else if (Event.key.keysym.sym==SDLK_RETURN)//if (g_iKeys[DJKEY_ENTER])
					bmenurunning = 0;

				// escape
				else if (Event.key.keysym.sym==SDLK_ESCAPE)//if (g_iKeys[DJKEY_ESC])
				{
					option = -1;
					bmenurunning = 0;
				}

				// dj2022-11 If an A-Z key pressed, see if we can find a selectable menu item that starts with that letter and select that (Duke Nukem did similar)
				else if (bAllowMenuKeyLetterShortcuts && Event.key.keysym.sym>=SDLK_a && Event.key.keysym.sym<=SDLK_z && option>=0)
				{
					const char cPressed = 'a' + (Event.key.keysym.sym - SDLK_a);
					int nCurPos = option;
					const SMenuItem* paItems = pMenu->getItems();
					for (auto i = 0; i < pMenu->getSize(); ++i)
					{
						nCurPos = (nCurPos + 1) % pMenu->getSize();//<- mod to wrap around
						// Find the first/next menu item that starts with the typed character (after whitespace etc.), with wraparound, and jump to it if found (but I think probably not act as if Enter pressed)
						if (paItems[nCurPos].IsSelectable() && !paItems[nCurPos].GetTextStr().empty())
						{
							int nFindFirstAlphChar = 0;
							char c = 0;
							do
							{
								c = paItems[nCurPos].GetText()[nFindFirstAlphChar];
								// todo support diacritics here? e.g. french access? e.g. if press 'e' then find accented e etc.? [low prio but nice to have]
								// Make lowercase
								if ((c >= 'A') && (c <= 'Z')) c += 32;//32 transfers 'A' to e.g. 'a' in ASCII

								if (c < 'a' || c>'z')
									++nFindFirstAlphChar;
							} while (c != 0 && (c < 'a' || c>'z'));

							if (c == cPressed)
							{
								menu_move(pMenu, option, nCurPos - option, *szCursor, iFirstSelectable, iLastSelectable);
								break;
							}
						}
					}
				}
				break;

			case SDL_KEYUP:
				break;
			case SDL_QUIT:
				bmenurunning=0;
				option = -1;//Exit
				break;
			}
		}
		djiPollEnd();

		// [dj2016-10] this if seems silly here to me but if i take it out, then as you press Esc on menu,
		// it draws some 'wrong' stuff for one frame .. whatever, just adding this if back again
		if (bmenurunning)
		{
			// Animate cursor [note this is unfortunately currently a bit 'tied' to the 10Hz frame rate limit ...
			// if want to e.g. increase menu frame rate in future to say 20Hz or whatever, then the cursor will
			// animate two times too fast (say) .. if do that in future then must just make this update slightly
			// 'smarter' on the animation - not a priority now at all. dj2016-10]
			static int nCursorAnimUpdateEvery=3;//<-dj2018-03-30 compensate to try keep cursor animation around ~10Hz
			if (--nCursorAnimUpdateEvery<=0)
			{
				szCursor++;
				nCursorAnimUpdateEvery = 3;
			}
			if (*szCursor == 0)
				szCursor = pMenu->getMenuCursor ();

			menu_move( pMenu, option, 0, *szCursor, iFirstSelectable, iLastSelectable);//Force redraw of cursor for animation purposes
		}

		GraphFlip(true);
	} while (bmenurunning);
	#else//#ifndef __EMSCRIPTEN__
	emscripten_set_main_loop(do_menu_pump, 30, true);
	#endif

	
	// Wait for user to let go of escape or enter
	if (option == -1)
		djiWaitForKeyUp(DJKEY_ESC);
	else
		//this isn't working [anymore?] for redefine keys???
		djiWaitForKeyUp(DJKEY_ENTER);

	//Mix_FadeOutChannel(1, 1000);


	if (g_pImgMenuBackground8x8!=NULL)
	{
		djDestroyImageHWSurface( g_pImgMenuBackground8x8 );
		djDEL(g_pImgMenuBackground8x8);
	}

	return option;
}
/*--------------------------------------------------------------------------*/

CMenu::CMenu(const char *idstr) :
	m_items(NULL),
	m_szCursor(NULL),
	m_pCursorSprite(nullptr),
	m_xOffset(-1),
	m_yOffset(-1),
	m_iSize(-1),
	m_clrBack(djColor(0, 0, 0)),
	m_iSoundMove(SOUNDHANDLE_INVALID)
{
	idstring = djStrDeepCopy(idstr);
}

CMenu::~CMenu()
{
	SYS_Debug ( "CMenu::~CMenu (%s)\n", idstring );

	djDELV(idstring);

	// Do NOT delete m_szCursor or m_items. At the moment they're pointers to globals ... bad old code.
	// This should change later, and be consistent.

	SYS_Debug ( "CMenu::~CMenu ()ok\n" );
	SYS_Debug ( "TODO: items and cursor are not deleted b/c in most cases the yare static strings. May need some changes later\n" );
}
