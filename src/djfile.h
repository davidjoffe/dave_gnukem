//Copyright (C) 1995-2024 David Joffe / Dave Gnukem project
//
//dj2022-11 refactoring some file stuff into new djfile.h/cpp and maybe adding some more file- and path-related helpers and maybe some

// [dj2022-11] fixing these etc. on my win32 visual studio build system ..
// "warning C4996: 'fopen': This function or variable may be unsafe. Consider using fopen_s instead"
// "warning C4996: 'sscanf': This function or variable may be unsafe. Consider using sscanf_s instead"
// 
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/secure-template-overloads?view=msvc-170

/*--------------------------------------------------------------------------*/
#ifndef _dj_DJFILE_H_
#define _dj_DJFILE_H_

#include "config.h"

// [dj2022-11] want safer versions of file-related functions where available
#ifdef __STDC_LIB_EXT1__
	#define __STDC_WANT_LIB_EXT1__ 1
#endif

#if defined(WIN32) && defined(_MSC_VER)
	#ifndef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
		#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
	#endif
#endif// {{ Windows / Microsoft compiler }}

#if defined(WIN32) && defined(_MSC_VER)
	#define djHAVE_SAFER_FUNCTIONS
#elif __STDC_LIB_EXT1__
	#define djHAVE_SAFER_FUNCTIONS
#endif

#include <stdio.h>

class djFile
{
public:
	// Returns 0 on success, else sets pFile to nullptr and returns an error code
	static int dj_fopen_s(FILE** ppFile, const char* szFilename, const char* szMode);

	static FILE* dj_fopen(const char* szFilename, const char* szMode);
};


// these globals should probably not be globals [low]

extern bool djFolderExists(const char* szPath);
extern bool djFileExists(const char* szPath);
extern bool djEnsureFolderTreeExists(const char* szPath);


// some of these feel a little gross and arbitrary, rethink a little some of these 'helpers' .. it's a start though

// [dj2022-11] This feels slightly dodgy to me like I feel like on some platforms this might just not compile ..
#ifdef djHAVE_SAFER_FUNCTIONS
	#define dj_fscanf fscanf_s
	#define dj_sscanf sscanf_s
	// From https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fscanf-s-fscanf-s-l-fwscanf-s-fwscanf-s-l?view=msvc-170 :
	// "the more secure functions require the size in characters of each c, C, s, S, and [ type field to be passed as an argument immediately following the variable"
	// but NB note also we get: warning C4477: 'fscanf_s' : format string '%s' requires an argument of type 'unsigned int', but variadic argument 2 has type 'size_t'
	#define	dj_fscanf_intline(pStream, nInt)  fscanf_s((pStream), "%d\n", &nInt)
	#define	dj_fscanf_int(pStream, nInt)      fscanf_s((pStream), "%d", &nInt)
	#define	dj_fscanf_line(pStream, szBuf)    fscanf_s((pStream), "%s\n", szBuf, (unsigned int)sizeof(szBuf))
#else
	#define dj_fscanf fscanf
	#define dj_sscanf sscanf
	#define	dj_fscanf_intline(pStream,nInt)   fscanf(pStream, "%d\n", &nInt)
	#define	dj_fscanf_int(pStream,nInt)       fscanf(pStream, "%d", &nInt)
	#define	dj_fscanf_line(pStream, szBuf)    fscanf((pStream), "%s\n", szBuf)
#endif

#endif
