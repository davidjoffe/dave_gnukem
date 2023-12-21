/*!
\file    djlang.h
\brief   Localization / languages
\author  David Joffe

Copyright (C) 2023 David Joffe
*/

//#include <string>

/*
//dj2023-11 to generate initial strings, do something like: 
cd src
xgettext --force-po -c -C *.cpp -o ../lang/locale/en.po
*/


//---------------------------------------------------------------------------
#ifndef _dj_LANG_H_
#define _dj_LANG_H_
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//extern std::string g_sCurLang;
//---------------------------------------------------------------------------

void djSelectLanguage(const char* szNewLang="en");
// Doesn't return nullptr always returns a string
const char* djGetLanguage();

class djLang
{
public:
	// Returns true if translations are enabled and a language for localization is selected
	static bool DoTranslations();

	// 1=left-to-right, -1=right-to-left (e.g. Arabic "ar" or Hebrew "he"). Cached value for speed and update the value only when lang code changes
	// Return value negative means right to left language e.g. Hebrew or Arabic
	static int GetCurLangDirection();
	// 1=left-to-right, -1=right-to-left (e.g. Arabic "ar" or Hebrew "he")
	// Return value negative means right to left language e.g. Hebrew or Arabic
	static int GetLangDirection(const char* szCode, int nLen=-1);
};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
