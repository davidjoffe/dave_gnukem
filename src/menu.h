/*!
\file    menu.h
\brief   Main menu
\author  David Joffe

Copyright (C) 1995-2022 David Joffe
*/
/*--------------------------------------------------------------------------*/
/* menu.h
 */
/* David Joffe '95/07/20 */
/*--------------------------------------------------------------------------*/
#ifndef _MENU_H_
#define _MENU_H_
/*--------------------------------------------------------------------------*/
#include "djtypes.h"
#include "djsound.h"

/*--------------------------------------------------------------------------*/
// Class forwards (don't include actual headers here for compile speed reasons etc.)
class djSprite;
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
//*
class djMenuCursor
{
public:
	djMenuCursor() {}
	virtual ~djMenuCursor() {}
};
class djMenuCursorSprite : public djMenuCursor
{
public:
	djMenuCursorSprite() : djMenuCursor() {}
	virtual ~djMenuCursorSprite() { /*djDEL(m_pSprite);*/ }

	djSprite* m_pSprite = nullptr;
};
//*/
/*--------------------------------------------------------------------------*/
extern djMenuCursorSprite* g_pDefaultMenuCursor;
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
//! A single item in the menu
struct SMenuItem
{
	SMenuItem(bool bItem = false, const char* szText = nullptr, const char* szRetVal = nullptr) : m_bitem(bItem), m_szText(szText), m_szRetVal(szRetVal)
	{
	}

	//! A real (i.e. selectable) menu item (true), or just a string (false)?
	bool m_bitem=false;
	//! The menu text to display
	const char *m_szText=nullptr;

	//! dj2022-11 [optional] new return by a string identifier so we can try have slightly more sane (and potentially less bug-introducing) handling than gross switch statements with lists of hardcoded numerical values henceforth
	const char* m_szRetVal = nullptr;

	const bool IsSelectable() const { return m_bitem; }
};
/*--------------------------------------------------------------------------*/
/*!
\class CMenu
\nosubgrouping

A menu (e.g. the main menu). Consists of an array of menu items (\ref SMenuItem).
A menu item with \ref SMenuItem::m_szText value NULL indicates the end of the
array.

(Note that using a null-terminator to indicate end of menu is a bit 'risky' as any programmer who naively creates a new menu may not realize .. and then it reads past of end of their memory menu .. so not the best strategy .. this was 20 years ago urg ..~dj2022 Refactor someday.)
*/
class CMenu
{
public:
	CMenu(const char *idstr);
	~CMenu();

	// Really old stuff that uses hardcoded offsets into old main game font .. not generic, getting rid of ..and replacing with new djSprite* stuff (to help support localization so menus can be in more langauges)
	void setMenuCursor ( const unsigned char *cursor ) { m_szCursor = (unsigned char*)cursor; };
	void SetMenuCursor( djSprite* pSprite ) { m_pCursorSprite = pSprite; m_nCursorSpriteAnimOffset = 0; }
	void setClrBack ( const djColor &clr ) { m_clrBack = clr; };
	void setSize ( int sz ) { m_iSize = sz; };
	void setXOffset ( int offs ) { m_xOffset = offs; }
	void setYOffset ( int offs ) { m_yOffset = offs; }
	void setItems ( const SMenuItem *items ) { m_items = items; }
	void setSoundMove ( SOUND_HANDLE h ) { m_iSoundMove = h; }

	const djColor& getClrBack () const { return m_clrBack; }
	int getXOffset () const { return m_xOffset; }
	int getYOffset () const { return m_yOffset; }
	int getSize () const { return m_iSize; }
	const unsigned char* getMenuCursor () const { return m_szCursor; }
	djSprite* GetMenuCursorSprite() { return m_pCursorSprite; }
	SOUND_HANDLE getSoundMove () const { return m_iSoundMove; }
	const SMenuItem* getItems () const { return m_items; }

private:
	const SMenuItem		*m_items = nullptr;	// We DON'T own this!! (m_szText==NULL)-terminated
	const unsigned char	*m_szCursor = nullptr; // We DON'T own this!!
	djSprite		*m_pCursorSprite = nullptr; // We DON'T own this!!
	int				m_nCursorSpriteAnimOffset = 0;
	int				m_xOffset=0;
	int				m_yOffset=0;
	int				m_iSize=0;
	djColor			m_clrBack;	// background color (the above ones are obsolete)
	SOUND_HANDLE	m_iSoundMove;	// Sound to play when cursor moved
	char			*idstring=nullptr;	// for debug purposes
};

//! Pop up a menu and wait for the user to select something from the menu.
extern int do_menu( CMenu *pMenu );

#endif
