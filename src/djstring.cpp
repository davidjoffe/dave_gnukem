/*
djstring.cpp

Copyright (C) 1998-2018 David Joffe
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "djstring.h"
#include <stdarg.h>//va_list etc. [for djStrPrintf dj2016-10]
#ifdef WIN32
#include <Windows.h>//GetFileAttributes [for djFileExists]
#include "Shlobj.h"//SHCreateDirectoryEx
#else
#include <sys/types.h>//stat [for djFileExists]
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>//mkpath
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

#ifdef WIN32

bool djFolderExists(const char* szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool djFileExists(const char* szPath)
{
	DWORD fileAttr = ::GetFileAttributes(szPath);
	if (0xFFFFFFFF == fileAttr)
		return false;
	return true;
}

bool djEnsureFolderTreeExists(const char* szPath)
{
	if (djFolderExists(szPath))return true;
	// "Returns ERROR_SUCCESS if successful"
	if (::SHCreateDirectoryEx(NULL,szPath,NULL)==ERROR_SUCCESS)
		return true;
	return false;
}

#else
//#ifdef __APPLE__
bool djFolderExists(const char* szPath)
{
	struct stat sb;
	if (stat(szPath, &sb) == 0 && S_ISDIR(sb.st_mode))
	{
		return true;
	}
	return false;
}
bool djFileExists(const char* szPath)
{
	struct stat sb;
	if (stat(szPath, &sb) == 0 && S_ISREG(sb.st_mode))
	{
		return true;
	}
	return false;
}

//[dj2018-03]https://stackoverflow.com/questions/675039/how-can-i-create-directory-tree-in-c-linux
typedef struct stat Stat;
static int do_mkdir(const char *path, mode_t mode)
{
	Stat            st;
	int             status = 0;

	if (stat(path, &st) != 0)
	{
		/* Directory does not exist. EEXIST for race condition */
		if (mkdir(path, mode) != 0 && errno != EEXIST)
			status = -1;
	}
	else if (!S_ISDIR(st.st_mode))
	{
		errno = ENOTDIR;
		status = -1;
	}

	return(status);
}
/**
** mkpath - ensure all directories in path exist
** Algorithm takes the pessimistic view and works top-down to ensure
** each directory in path exists, rather than optimistically creating
** the last element and working backwards.
*/
int mkpath(const char *path, mode_t mode)
{
	char           *pp;
	char           *sp;
	int             status;
	char           *copypath = strdup(path);

	status = 0;
	pp = copypath;
	while (status == 0 && (sp = strchr(pp, '/')) != 0)
	{
		if (sp != pp)
		{
			/* Neither root nor double slash in path */
			*sp = '\0';
			status = do_mkdir(copypath, mode);
			*sp = '/';
		}
		pp = sp + 1;
	}
	if (status == 0)
		status = do_mkdir(path, mode);
	free(copypath);
	return (status);
}
bool djEnsureFolderTreeExists(const char* szPath)
{
	//debug//printf("djEnsureFolderTreeExists(%s)\n",szPath);fflush(NULL);
	if (djFolderExists(szPath))return true;
	//debug//printf("2");
	mkpath(szPath, 0777);
	//mkdir(szPath, 0777);
	//debug//printf("(Exists?=%s)\n",djFolderExists(szPath)?"YES":"NO");fflush(NULL);
	return djFolderExists(szPath);
	//return mkpath(szPath);
}
//#else

#endif

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
