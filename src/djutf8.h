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

//! Encode a single 32-bit Unicode character codepoint as utf8 and store into buffer szBufUTF8, appending at uLen and incrementing uLen depending on how many characters it must append to the buffer. NB! Doesn't add null-terminator. NB doesn't do any safety-checks, this is just meant to be 'fast and unsafe' - so make sure the buffer you pass is long enough.
//! If we ever have to do insane amounts of text such that it becomes a bottleneck then this should perhaps be inlined also but doesn't seem likely at this stage.
extern void djutf8_encode(unsigned int nUnicodeChar32, char *szBufUTF8, int &uLen);

#endif