//Copyright (C) 1995-2022 David Joffe / Dave Gnukem project
//
//dj2022-11 refactoring some file stuff into new djfile.h/cpp and maybe adding some more file- and path-related helpers and maybe some

/*--------------------------------------------------------------------------*/
#include "config.h"
#include "djfile.h"

#include <stdio.h>//FILE
#include <errno.h>//errno

/*
#include <stdarg.h>//va_list etc.

int djScan::fscanf(FILE* const pStream, const char* szFormat, ...)
{
	va_list args;
	va_start(args, szFormat);
#ifdef djHAVE_SAFER_FUNCTIONS
	int nRet = vfscanf_s(pStream, szFormat, args);
#else
	//#warning "Safer file functions not availables" hm unfortunately seems not available on my WSL ubuntu .. not sure where these are availables
	int nRet = vfscanf(pStream, szFormat, args);
#endif
	va_end(args);
	if (nRet < 0)
	{
		// breakpoint opportunity...
		int n = 3;
	}
	return nRet;
}
*/

// Returns 0 on success, else sets pFile to nullptr and returns an error code [dj2022-11] new safety helper for fopen
int djFile::dj_fopen_s(FILE** ppFile, const char* szFilename, const char* szMode)
{
	// fixme threading? mutex? not sure if 'errno' has thread issues?
	//std::mutex ; apparently errno is threadsafe at least on linux
	*ppFile = nullptr;
	if (szFilename == nullptr || szFilename[0] == 0) return -1;

#ifdef djHAVE_SAFER_FUNCTIONS
	errno_t errRet = fopen_s(ppFile, szFilename, szMode);
	if (errRet != 0)
	{
		*ppFile = nullptr;
		printf("fopen_s error %s (errno=%d)\n", szFilename, (int)errRet);
		return (int)errRet;
	}
	return ((*ppFile)==NULL ? -1 : 0);
#else
	*ppFile = fopen(szFilename, szMode);
	if ((*ppFile)==nullptr)
	{
		//if (((int)errno)!=0)
		printf("fopen error %s (errno=%d)\n", szFilename, (int)errno);
		return (int)errno;
	}
	return 0;//success
#endif
}

FILE* djFile::dj_fopen(const char* szFilename, const char* szMode)
{
	if (szFilename == nullptr || szFilename[0]==0) return NULL;
	FILE* pFile = NULL;
	int nRet = djFile::dj_fopen_s(&pFile, szFilename, szMode);
	if (nRet != 0)
	{
		printf("dj_fopen error %s %d\n", szFilename, (int)nRet);
		//djMSG"GameLoadSprites(): Unable to load raw sprite data\n");
	}
	return pFile;
}
