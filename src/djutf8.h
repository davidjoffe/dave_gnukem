//Copyright (C) 2022 David Joffe / Dave Gnukem project
//
//dj2022-11-30 new utf8 helper(s) (and possibly later other encoding or string conversion helpers or wrappers if/as needed in future)
//NB Design-wise this is conceptually part of the generic reusable code so shouldn't have dependencies to any Dave Gnukem specific parts of the codebase

/*--------------------------------------------------------------------------*/
#ifndef _DJUTF8_H_
#define _DJUTF8_H_
#include "config.h"

//! Simple utf8 string iterator (pass over characters in multi-byte and variable-length-byte utf8 encoding and return 32-bit Unicode codepoint in c)
extern int djutf8iterate(const char* sz, unsigned int uLen, int& c);

#endif