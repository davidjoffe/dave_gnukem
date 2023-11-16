/*!
\file    djlang.cpp
\brief   Localization / languages
\author  David Joffe

Copyright (C) 2023 David Joffe
*/

#include "config.h"
#include "djlang.h"
#include <string>

std::string g_sCurLang="en";

void djSelectLanguage(const char* szLang="en")
{
    if (szLang==nullptr|| szLang[0]==0)
        g_sCurLang = "en";//default
    else
        g_sCurLang = szLang;
}
