/*
djstring.cpp

Copyright (C) 1998-2017 David Joffe

License: GNU GPL Version 2
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "mmgr/mmgr.h"

#include "djstring.h"
#include <stdarg.h>//va_list etc. [for djStrPrintf dj2016-10]
#ifdef WIN32
#include <Windows.h>//GetFileAttributes [for djFileExists]
#else
#include <sys/types.h>//stat [for djFileExists]
#include <sys/stat.h>
#include <unistd.h>
#endif

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
	szNewStr[n] = '\0';

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
	for ( unsigned int i=0; i<strlen(str); i++ )
	{
		if ((str[i] >= 'A') && (str[i] <= 'Z')) str[i] += 32;
	}
}

std::string djStrPrintf( const char* szFormat, ... )
{
	if ( szFormat == NULL )
		return "";

	// Print the formatted string onto buf
	char buf[4096]={0};
	va_list args;
	va_start(args, szFormat);
	vsnprintf(buf, 4096, szFormat, args);
	va_end(args);

	return buf;
}


// Should actually have a fileutils cpp/h or somethign [low dj2017-08] these don't belong here

// Append a folder to existing path, 'intelligently' handling
// the trailing slash worries for us.
void djAppendPath(char* szPath,char* szAppend)
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

#ifdef WIN32

bool djFileExists(const char* szPath)
{
	DWORD fileAttr = ::GetFileAttributes(szPath);
	if (0xFFFFFFFF == fileAttr)
		return false;
	return true;
}

#else
//#ifdef __APPLE__
/*bool djFolderExists(const char* szPath)
{
	struct stat sb;
	if (stat(szPath, &sb) == 0 && S_ISDIR(sb.st_mode))
	{
		return true;
	}
	return false;
}*/
bool djFileExists(const char* szPath)
{
	struct stat sb;
	if (stat(szPath, &sb) == 0 && S_ISREG(sb.st_mode))
	{
		return true;
	}
	return false;
}
//#else

#endif
