/*!
\file    loadedlevel.cpp
\brief   Some data and metadata of current level being played or loaded for editor
\author  David Joffe

Copyright (C) 1995-2023 David Joffe
*/
/*--------------------------------------------------------------------------*/
// Refactoring some stuff from game.h/cpp and extending ..
/*--------------------------------------------------------------------------*/

#include "config.h"
#include "loadedlevel.h"
#include "djimage.h"
#include "djsprite.h"

//---------------------------------------------------------------------------
tsLoadedLevel g_Level;
//---------------------------------------------------------------------------

