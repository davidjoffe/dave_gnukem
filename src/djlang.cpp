/*!
\file    djlang.cpp
\brief   Localization / languages
\author  David Joffe

Copyright (C) 2023 David Joffe
*/

#include "config.h"
#include "djlang.h"
#include "localization/djgettext.h"// djlang.h 'knows about' djgettext but not vice versa currently (this might change if/as I re-think things - dj)
#include <string>

std::string g_sCurLang="en";
// 1=left-to-right, -1=right-to-left (e.g. Arabic "ar" or Hebrew "he"). Cached value for speed and update the value only when lang code changes
int g_nLangDirection=1;

//---------------------------------------------------------------------------

void djSelectLanguage(const char* szLang)
{
    if (szLang==nullptr|| szLang[0]==0)
    {
        g_sCurLang = "en";//default
    }
    else
    {
        g_sCurLang = szLang;
    }
    // Hm what's the longest possible language CODE (with region etc.) hrm?
    if (g_sCurLang.length()>32)// Trim if overly long
        g_sCurLang = g_sCurLang.substr(0, 32);

    // Load and/or select .po file(s) (dj2023 todo still thinking about best place for this etc.)
    if (djLang::DoTranslations())
    {
        select_locale(g_sCurLang);
    }

    // Determine OVERALL lang direction and cache the value
    g_nLangDirection = djLang::GetLangDirection(g_sCurLang.c_str(), (int)g_sCurLang.length());
}

const char* djGetLanguage()
{
    return g_sCurLang.c_str();
}

bool djLang::DoTranslations()
{
    // Hrm, is this correct? Maybe default language should be 'none' and 'en' could host translations .. that way can do things like the pun
    return !g_sCurLang.empty() && g_sCurLang!="en";
}

int djLang::GetLangDirection(const char* szCode, int nLen)
{
    int nRet=1;
    if (szCode==nullptr||szCode[0]==0||nLen==0)
        return 1;

    std::string s=szCode;
    if (s.length()>3)
        s = s.substr(0,3);

    extern void djStrToLowerTmpHelper( std::string& s );//urgh
    djStrToLowerTmpHelper(s);//urgh
    if (s=="he-") return -1;
    if (s=="ar-") return -1;
    if (s=="he") return -1;
    if (s=="ar") return -1;
    return 1;
}

int djLang::GetCurLangDirection()
{
    // Return cached value for speed and update the value only when lang code changes
    return g_nLangDirection;
}
//---------------------------------------------------------------------------
