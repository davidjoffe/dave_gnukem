/*!
\file    djstring.h
\brief   Some string helper functions
\author  David Joffe

Copyright (C) 1998-2024 David Joffe
*/
/*--------------------------------------------------------------------------*/
#ifndef _DJSTRING_H_
#define _DJSTRING_H_

#include <string>

//! Make a deep copy of string. Must be deleted with "delete[]".
extern char * djStrDeepCopy( const char * src );
//! Make a deep copy of string. Must be deleted with "delete[]".
extern char * djStrDup( const char * src );
extern char * djStrDeepCopy( const char * src, int n );

//! Convert string in-place to uppercase
extern void   djStrToLower( char * str );

//! For returning sections of strings using given delimiters. i is 1-based
//! eg djStrPart("a,b;c,d", 3, ",;") should return "c"
//! You are responsible for deleting the string it returns
extern char * djStrPart( const char *str, int i, const char *delim );



// Some quick n dirty file/path helpers [dj2017-08]

extern void djAppendPathS(std::string& sPath,const char* szAppend);
extern void djAppendPath(char* szPath,const char* szAppend);
extern std::string djAppendPathStr(const char* szBase,const char* szAppend);


// This doesn't belong in djstring.h[dj2018-03]
extern std::string djGetFolderUserSettings();

extern std::string djIntToString(int n);

//! Strip newline character from string (to handle both UNIX and stupid DOS text file formats)
extern void djStripCRLF(char* buf);

#endif
