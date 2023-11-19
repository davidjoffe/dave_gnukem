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

};
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
