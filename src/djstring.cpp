/*
djstring.cpp

Copyright (C) 1998-2023 David Joffe
*/

#include "config.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>//std::to_string

#include "djstring.h"
#include <stdarg.h>//va_list etc. [for djStrPrintf dj2016-10]

char *djStrDeepCopy( const char * src )
{
	if (!src)
		return NULL;
	char *newstring = new char[strlen(src) + 1];
	strcpy(newstring, src);
	return newstring;
}

char *djStrDup(const char *src)
{
	if (!src)
		return NULL;
	char *newstring = new char[strlen(src) + 1];
	strcpy(newstring, src);
	return newstring;
}

char *djStrDeepCopy( const char * src, int n )
{
	if ( NULL == src )
		return NULL;

	// FIXME: If src is shorter than n then allocate less
	char* szNewStr = new char[ n + 1 ];
	strncpy( szNewStr, src, n );
	szNewStr[n] = 0;

	return szNewStr;
}

// i starts counting at 1
char* djStrPart( const char *str, int i, const char *delim )
{
	int pos, count, length;

	if ( i < 1 )
		return NULL;

	pos = 0;
	count = 0;
	while ( ( count < i - 1 ) && ( pos < (int)strlen( str ) ) )
	{
		while ( (NULL == strchr( delim, str[pos] )) && (pos < (int)strlen( str )) )
		{
			//	 printf( "[%c]", str[pos] );
			pos++;
		}
		if ( pos < (int)strlen(str) )
		{
			pos++;
			count++;
		}
		//      printf( "\n" );
	}

	// if end of string reached before part i was reached
	if ( count != i - 1 )
		return NULL;

	// Determine length of substring required
	length = 0;
	while ( ((pos+length) < (int)strlen(str)) && (NULL == strchr( delim, str[pos+length] )) )
	{
		length++;
	}

	char * temp = NULL;
	temp = djStrDeepCopy( &str[pos], length );
	//   printf("String found (length %d) is [%s]\n", length, temp );

	return temp;
}

void djStrToLower( char * str )
{
	for ( unsigned int i=0; i<strlen(str); ++i )
	{
		if ((str[i] >= 'A') && (str[i] <= 'Z')) str[i] += 32;
	}
}

std::string djStrPrintf( const char* szFormat, ... )
{
	if ( szFormat == NULL )
		return "";

	// fixme handle larger / arbitrary lengths? this is gross but you can auto-detect the required length with some effort


	// Print the formatted string onto buf
	static thread_local char buf[16384]={0};
	va_list args;
	va_start(args, szFormat);
	vsnprintf(buf, 4096, szFormat, args);
	va_end(args);

	return buf;
}


// Should actually have a fileutils cpp/h or somethign [low dj2017-08] these don't belong here

// Append a folder to existing path, 'intelligently' handling
// the trailing slash worries for us.
void djAppendPath(char* szPath,const char* szAppend)
{
	if (szPath==NULL)return;
	if (szAppend==NULL||szAppend[0]==0)return;

	// If doesn't have trailing slash, add one (unless szPath is empty string)
	if (strlen(szPath)>0)
	{
		char cLast = szPath[ strlen(szPath)-1 ];
		if (cLast!='/' && cLast!='\\')
		{
			strcat(szPath,"/");
		}
	}
	strcat(szPath,szAppend);
}
void djAppendPathS(std::string& sPath,const char* szAppend)
{
	//if (szPath==NULL)return;
	if (szAppend==NULL||szAppend[0]==0)return;

	// If doesn't have trailing slash, add one (unless szPath is empty string)
	if (!sPath.empty())
	{
		char cLast = sPath.at( sPath.length()-1 );
		if (cLast!='/' && cLast!='\\')
		{
			//strcat(szPath,"/");
			sPath += L'/';
		}
	}
	//strcat(szPath,szAppend);
	sPath += szAppend;
}
std::string djAppendPathStr(const char* szBase,const char* szAppend)
{
	std::string sPath = szBase;
	djAppendPathS(sPath,szAppend);
	return sPath;
}

std::string djGetFolderUserSettings()
{
#ifdef WIN32
	std::string s = getenv("APPDATA");
	djAppendPathS(s, "DaveGnukem");
#else
	std::string s = getenv("HOME");
	djAppendPathS(s, ".gnukem");//?
#endif
	
	//dj2018-03 If it somehow to create/use the 'desired' datafolder, then perhaps fall back on current folder again as default, as it used to be? not sure - think about.
	//if (!djFolderExists(s.c_str()))
		//s.clear();

	return s;
}

// Hardly anything uses this function should it even exist if we now have things like std::to_string? Helpers like this made more sense in the days pre things like std::to_string [dj2023]
std::string djIntToString(int n)
{
	//[dj 2013] todo replace with std::to_string? [low] - it's also more thread-safe - and not prone to issues like e.g. possible overrun or cutoff if sizeof(int) is 64-bits on some platform .. it's good to try phase out these printf-style buffers
#if __cplusplus>=201103L // c++11?
	return std::to_string(n);
#else
	// Note NB this must be large enough for e.g. 128-bit ints 'just in case'. Also this function should be threadsafe so no static buffers etc.
	char buf[128] = { 0 };
	snprintf(buf, sizeof(buf), "%d", n);
	return buf;
#endif
}

void djStripCRLF(char* buf)
{
	// While last character in string is either CR or LF, remove.
	while (strlen(buf) >= 1 && ((buf[strlen(buf) - 1] == '\r') || (buf[strlen(buf) - 1] == '\n')))
		buf[strlen(buf) - 1] = '\0';
}
