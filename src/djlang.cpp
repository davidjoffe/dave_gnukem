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
#include <algorithm>// for make lowercase
#include <cctype>// for make lowercase

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
/*
Right to left languages:
Arabic
ISO 639-1: ar
ISO 639-2: ara
Hebrew
ISO 639-1: he (formerly iw)
ISO 639-2: heb
Persian (Farsi)
ISO 639-1: fa
ISO 639-2: fas (formerly per)
Urdu
ISO 639-1: ur
ISO 639-2: urd
Pashto
ISO 639-1: ps
ISO 639-2: pus
Uyghur
ISO 639-1: ug
ISO 639-2: uig
Kurdish (Sorani)
ISO 639-1: ckb (Central Kurdish)
ISO 639-2: kur (for Kurdish in general)
Syriac
ISO 639-1: Not available
ISO 639-2: syr
Dhivehi
ISO 639-1: dv
ISO 639-2: div
Yiddish
ISO 639-1: yi
ISO 639-2: yid
*/
int djLang::GetLangDirection(const char* szCode, int nLen)
{
    if (szCode==nullptr||szCode[0]==0||nLen==0)
        return 1;

    // We want just the code part, e.g. if someone passed in "he-il" or "ar-countrycode" we want "he" or "ar" part only
    std::string s=szCode;
    if (s.length()>3)
        s = s.substr(0,3);
    // Now if we're left with "he-" make it "he" (likewise "ar" etc.)
    if (s.back()=='-')
        s = s.substr(0,2);
    // NB we want to keep at least 3 symbols even though we only need to check the first two, because if the code is something 'wrong' like
    // 'hee' instead of 'he-' it should probably not return as 'right-to-left' (although I suppose it's debatable how one should handle bad input here)

    // Note this function could be faster and could avoid copying the string but for simplicity's sake it currently implemented like this
    // If you're going to use it anywhere it may be a bottleneck rather cache the result and/or we optimize this implementation
    // GetCurLangDirection() already caches result for 'current selected global overall language direction' so for example if we want to 
    // know the whole application should be right-to-left (e.g. if Arabic or Hebrew) and thus things like menus should be right-to-left
    // use the GetCurLangDirection() function, which is updated when the language is selected initially (and should be updated again if
    // the user selects a different language)

    // Make lowercase
    std::transform(s.begin(), s.end(), s.begin(), 
                [](unsigned char c) { return std::tolower(c); });

    if (s == "ar" || s == "ara" ||
            s == "he" || s == "heb" ||
            s == "fa" || s == "fas" ||
            s == "ur" || s == "urd" ||
            s == "ps" || s == "pus" ||
            s == "ug" || s == "uig" ||
            s == "ckb" || s == "kur" || // Note: "kur" is for Kurdish in general
            s == "syr" || // No ISO 639-1 code for Syriac
            s == "dv" || s == "div" ||
            s == "yi" || s == "yid")
    {
        return -1;
    }

    return 1;
}

int djLang::GetCurLangDirection()
{
    // Return cached value for speed and update the value only when lang code changes
    return g_nLangDirection;
}
//---------------------------------------------------------------------------
