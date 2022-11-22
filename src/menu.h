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
//! A single item in the menu
struct SMenuItem
{
	//! A real menu item (true), or just a string (false)?
	bool m_bitem=false;
	//! The menu text to display
	const char *m_szText=nullptr;

	//! dj2022-11 [optional] new return by a string identifier so we can try have slightly more sane (and potentially less bug-introducing) handling than gross switch statements with lists of hardcoded numerical values henceforth
	const char* m_szRetVal = nullptr;
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

	void setMenuCursor ( const unsigned char *cursor ) { m_szCursor = (unsigned char*)cursor; };
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
	SOUND_HANDLE getSoundMove () const { return m_iSoundMove; }
	const SMenuItem* getItems () const { return m_items; }

private:
	const SMenuItem		*m_items;	// We DON'T own this!! (m_szText==NULL)-terminated
	const unsigned char	*m_szCursor; // We DON'T own this!!
	int				m_xOffset;
	int				m_yOffset;
	int				m_iSize;
	djColor			m_clrBack;	// background color (the above ones are obsolete)
	SOUND_HANDLE	m_iSoundMove;	// Sound to play when cursor moved
	char			*idstring;	// for debug purposes
};

//! Pop up a menu and wait for the user to select something from the menu.
extern int do_menu( CMenu *pMenu );

#endif
