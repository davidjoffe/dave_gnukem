//Copyright (C) 1995-2024 David Joffe / Dave Gnukem project
//
//dj2022-11 refactoring some file stuff into new djfile.h/cpp and maybe adding some more file- and path-related helpers and maybe some

/*--------------------------------------------------------------------------*/
#include "config.h"
#include "djfile.h"

#include <stdio.h>//FILE
#include <errno.h>//errno

//-----------------------------------
// Includes for file-exists stuff etc.:
#ifdef WIN32
#include <Windows.h>//GetFileAttributes [for djFileExists]
#include "Shlobj.h"//SHCreateDirectoryEx
#else
#include <cstdlib>
#include <sys/types.h>//stat [for djFileExists]
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>//mkpath
#include <string.h>//strdup
#endif
//-----------------------------------

// Returns 0 on success, else sets pFile to nullptr and returns an error code [dj2022-11] new safety helper for fopen
int djFile::dj_fopen_s(FILE** ppFile, const char* szFilename, const char* szMode)
{
	// fixme threading? mutex? not sure if 'errno' has thread issues?
	//std::mutex ; apparently errno is threadsafe at least on linux
	*ppFile = nullptr;
	if (szFilename == nullptr || szFilename[0] == 0) return -1;

	// dj2022-12 my new fopen helper is now slightly overly-aggressively showing scary-sounding logging errors for stuff that may normally fail (e.g. the first time you run it and the hiscores and user preferences files aren't there)
	// So not sure if should make a parameter here etc. as to whether to log error in some cases
	// I do want to possibly catch errors during development/testing and porting etc. or if users are doing any game modding to help them find bugs
	// But normal users shouldn't see scary sounding errors for completely harmless stuff.
	// Maybe in future will add a "-v", "-vv", "-vvv" etc. commandline options to maybe increase verbosity of logging.
	// Also parts of the code where it is harmless/correct/normal for the file to not be present yet (e.g. logfile, user preferences file etc.) could/should also check if fileexists before calling.
	// Maybe we could also add a parameter here e.g. 'bLogError true/false' but that may be overkill not sure, will think about. Not a priority now.

#ifdef djHAVE_SAFER_FUNCTIONS
	errno_t errRet = fopen_s(ppFile, szFilename, szMode);
	if (errRet != 0)
	{
		*ppFile = nullptr;
		printf("Warning(fopen_s): File open failed %s (errno=%d)\n", szFilename, (int)errRet);
		if (!djFileExists(szFilename)) printf("(File does not exist)\n");
		return (int)errRet;
	}
	return ((*ppFile)==NULL ? -1 : 0);
#else
	*ppFile = fopen(szFilename, szMode);
	if ((*ppFile)==nullptr)
	{
		//if (((int)errno)!=0)
		printf("Warning(fopen): File open failed %s (errno=%d)\n", szFilename, (int)errno);
		if (!djFileExists(szFilename)) printf("(File does not exist)\n");
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
		// Don't log here because this function is currently just a wrapper to djFile::dj_fopen_s which does the logging.
		//printf("Warning(dj_fopen): File open failed %s %d\n", szFilename, (int)nRet);
	}
	return pFile;
}
 
#ifdef WIN32

bool djFolderExists(const char* szPath)
{
	if (szPath==nullptr || szPath[0]==0)return false;
	DWORD dwAttrib = GetFileAttributes(szPath);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool djFileExists(const char* szPath)
{
	if (szPath==nullptr || szPath[0]==0)return false;
	DWORD fileAttr = ::GetFileAttributes(szPath);
	if (0xFFFFFFFF == fileAttr)
		return false;
	return true;
}

bool djEnsureFolderTreeExists(const char* szPath)
{
	if (szPath==nullptr || szPath[0]==0)return false;
	if (djFolderExists(szPath))return true;
	// "Returns ERROR_SUCCESS if successful"
	if (::SHCreateDirectoryEx(NULL, szPath, NULL) == ERROR_SUCCESS)
		return true;
	return false;
}

#else
//#ifdef __APPLE__
bool djFolderExists(const char* szPath)
{
	if (szPath==nullptr || szPath[0]==0)return false;
	struct stat sb;
	if (stat(szPath, &sb) == 0 && S_ISDIR(sb.st_mode))
	{
		return true;
	}
	return false;
}
bool djFileExists(const char* szPath)
{
	if (szPath==nullptr || szPath[0]==0)return false;
	struct stat sb;
	if (stat(szPath, &sb) == 0 && S_ISREG(sb.st_mode))
	{
		return true;
	}
	return false;
}

//[dj2018-03]https://stackoverflow.com/questions/675039/how-can-i-create-directory-tree-in-c-linux
typedef struct stat Stat;
static int do_mkdir(const char* szPath, mode_t mode)
{
	if (szPath==nullptr || szPath[0]==0)return -1;
	Stat            st;
	int             status = 0;

	if (stat(szPath, &st) != 0)
	{
		/* Directory does not exist. EEXIST for race condition */
		if (mkdir(szPath, mode) != 0 && errno != EEXIST)
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
int mkpath(const char* path, mode_t mode)
{
	if (path==nullptr || path[0]==0)return -1;
	char* pp=nullptr;
	char* sp = nullptr;
	int             status=0;
	char* copypath = strdup(path);

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
	if (szPath==nullptr || szPath[0]==0)return false;
	//debug//printf("djEnsureFolderTreeExists(%s)\n",szPath);fflush(NULL);
	if (djFolderExists(szPath))return true;
	//debug//printf("2");
	mkpath(szPath, 0777);
	//mkdir(szPath, 0777);
	//debug//printf("(Exists?=%s)\n",djFolderExists(szPath)?"YES":"NO");fflush(NULL);
	return djFolderExists(szPath);
}

#endif
