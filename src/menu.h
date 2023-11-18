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
#include <string>

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

// For performance reasons we may want an initialized version of this for graphics stuff? Should we (for safety so programmers are aware) give it a name that indicates such? [low]
class djRectBase
{
public:
	int x;
	int y;
	int w;
	int h;
};
class djRect : public djRectBase
{
public:
	djRect(int nX, int nY, int nW, int nH)
	{
		x = nX;
		y = nY;
		w = nW;
		h = nH;
	}
};

/*--------------------------------------------------------------------------*/
//! A single item in the menu
//! // This is a really gross old class from the 1900s and needs modernizing :/
//! It's gross in so many ways from the strdup to the way CMenu expects a NULL termination to signify end of an array of these :/ .. all like old C style code
//! Can it be rescued? or need rewritng?
struct SMenuItem
{
	SMenuItem(bool bItem = false, const std::string& sText="", const std::string& sRetVal="", int nX=0, int nY=0, int nW=0, int nH=0) : m_bitem(bItem),
		m_sText(sText),
		m_sRetVal(sRetVal),
		m_Pos(nX, nY, nW, nH)
	{

	}

	//! A real (i.e. selectable) menu item (true), or just a string (false)?
	bool m_bitem=false;
	//! The menu text to display
	std::string m_sText;

	// Non-owned pointer presumably ..
	void SetText(const char* szText) { m_sText = szText; }
	const char* GetText() const { return m_sText.c_str(); }
	const std::string& GetTextStr() const { return m_sText; }

	// Old-fashioned C-style 'null terminator' concept is morphing a bit .. [dj2023]
	const bool IsTerminal() const { return m_sText.empty(); }
	void SetTerminal() { m_sText.clear(); m_bitem = false; }

	//! dj2022-11 [optional] new return by a string identifier so we can try have slightly more sane (and potentially less bug-introducing) handling than gross switch statements with lists of hardcoded numerical values henceforth
	std::string m_sRetVal;
	const std::string& GetRetVal() const { return m_sRetVal; }

	const bool IsSelectable() const { return m_bitem; }

	// Optional positional offset for drawing
	djRect m_Pos;//(0,0,0,0);
};

// Rather than a 'menu with text' we should conceptualize this as a general sort of 'widgets' UI system perhaps - then we could add custom things like, say, checkboxes straight in UI
/*class djMenuText : public SMenuItem
{
};*/
/*--------------------------------------------------------------------------*/
/*!
\class CMenu
\nosubgrouping

A menu (e.g. the main menu). Consists of an array of menu items (\ref SMenuItem).
A menu item with \ref SMenuItem::m_sText value empty indicates the end of the
array (slightly old-fashioned and may change in future)

(Note that using a null-terminator to indicate end of menu is a bit 'risky' as any programmer who naively creates a new menu may not realize .. and then it reads past of end of their memory menu .. so not the best strategy .. this was 20 years ago urg ..~dj2022 Refactor someday.)
*/
class CMenu
{
public:
	CMenu(const char *idstr);
	~CMenu();

	// Several things no longer used here, or being phased out:

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
