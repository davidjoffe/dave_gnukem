/*
 * File:    m_misc.cc
 * Created: 2002-06-29 (Saturday), 15:01
 * Modified: 2002-08-30 (Thursday), 07:35
 * Author: Vytautas Shaltenis, a.k.a. rtfb
 *
 * Project: Dave Gnukem
 *
 * Description: Miscellaneous utility functions.
 *
 * TODO:
 *	(-) Filename functions need testing.
 */

#include "m_misc.h"
#include "sys_error.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <cstring>//strlen,strcpy etc. [dj2022-11 refactoring and cleanups]
#include "sys_defs.h"

/*
#ifdef WIN32
#include <direct.h> // getcwd etc
#else
#include <unistd.h>//getcwd
#endif
*/


/*
==========================
M_ParseErrno
==========================
*/
void M_ParseErrno ( int err )
{
	switch ( err )
	{
		case EACCES:
		{
			fprintf ( stderr, "errno=EACCES(%d)\n\tOne of the directories containing newname or oldname refuses write per-mission; or newname and oldname are directories and write permission is refused for one of them.\n", errno );
			break;
		}
		case EBUSY:
		{
			fprintf ( stderr, "errno=EBUSY(%d)\n\tA directory named by oldname or newname is being used by the system in a way that prevents the renaming from working. This includes directories that are mount points for lesystems, and directories that are the current working directories of processes.\n", errno );
			break;
		}
		case EEXIST:
		case ENOTEMPTY:
		{
			fprintf ( stderr, "errno=EEXIST or ENOTEMPTY(%d)\n\tThe directory newname isn't empty. The GNU system always returns ENOTEMPTY for this, but some other systems return EEXIST.\n", errno );
			break;
		}
		case EINVAL:
		{
			fprintf ( stderr, "errno=EINVAL(%d)\n\toldname is a directory that contains newname.\n", errno );
			break;
		}
		case EISDIR:
		{
			fprintf ( stderr, "errno=EISDIR(%d)\n\tnewname is a directory but the oldname isn't.\n", errno );
			break;
		}
		case EMLINK:
		{
			fprintf ( stderr, "errno=EMLINK(%d)\n\tThe parent directory of newname would have too many links (entries).\n", errno );
			break;
		}
		case ENOENT:
		{
			fprintf ( stderr, "errno=ENOENT(%d)\n\tThe file oldname doesn't exist.\n", errno );
			break;
		}
		case ENOSPC:
		{
			fprintf ( stderr, "errno=ENOSPC(%d)\n\tThe directory that would contain newname has no room for another entry, and there is no space left in the le system to expand it.\n", errno );
			break;
		}
		case EROFS:
		{
			fprintf ( stderr, "errno=EROFS(%d)\n\tThe operation would involve writing to a directory on a read-only filesystem.\n", errno );
			break;
		}
		case EXDEV:
		{
			fprintf ( stderr, "errno=EXDEV(%d)\n\tThe two le names newname and oldname are on different file systems.\n", errno );
			break;
		}
		default:
		{
			fprintf ( stderr, "errno=%d\n\tdefault switch hit. Deep shit.\n", errno );
			break;
		}
	}
}





/*
==========================
M_StrToLower
==========================
*/
void M_StrToLower ( char *ptr )
{
	for ( ; *ptr; ptr++ )
	{
		*ptr = tolower ( *ptr );
	}
}


/*
==========================
M_StrToUpper
==========================
*/
void M_StrToUpper ( char *ptr )
{
	for ( ; *ptr; ptr++ )
	{
		*ptr = toupper ( *ptr );
	}
}



/*
==========================
M_CmdLineOption

 Check for command line option (CaSe SeNsItIvE), pefixed with - or /
Returns SYS_Argv () index of option if found, 0 if not
If reqPams != 0, makes sure additional parametes follow the index
==========================
*/
/*
int M_CmdLineOption ( const char *str, int reqPams )
{
	int i;

	for ( i=1; i<(SYS_Argc ()-reqPams); i++ )
	{
		if ( (('-' == SYS_Argv ( i )[0]) || ('/' == SYS_Argv ( i )[0]))
		  && (!strcmp ( &SYS_Argv ( i )[1], str )) )
		{
			return i;
		}
	}

	return 0;
}
*/


/*
==========================
M_IsDirectory
==========================
*/
bool M_IsDirectory ( const char *filename )
{
	if ( '/' == filename[strlen(filename)-1] )
		return true;

	return false;
}








/*
==========================================================================
	Result is in internal buffer; copy if need to preserve
==========================================================================
*/

/*
==================
M_GetFilePath

Get filename path directory
==================
*/
/*
char* M_GetFilePath ( const char* filename )
{
	static thread_local char	path[SYS_MAX_PATH]={0};
	char		*ptr=nullptr;

	strcpy ( path, filename );
	ptr = strrchr ( path, '\\' );
	if ( ptr )
	{
		ptr[1] = 0;
		return path;
	}

	ptr = strrchr ( path, ':' );
	if ( ptr )
	{
		ptr[1] = '\\';
		ptr[2] = 0;
		return path;
	}

	ptr = strrchr ( path, '/' );
	if ( ptr )
	{
		ptr[1] = '\\';
		ptr[2] = 0;
		return path;
	}

	path[0] = 0;
	return path;
}
*/

/*
======================
M_GetFileRoot

Get filename without path or extension
======================
*/
/*
char* M_GetFileRoot ( const char* filename )
{
	static thread_local char	root[SYS_MAX_PATH]={0};
	const char		*ptr=nullptr;

	ptr = strrchr ( filename, '\\' );
	if ( !ptr )
		ptr = strrchr ( filename, ':' );
	if ( !ptr )
		ptr = (char*) filename-1;

	strcpy ( root, ptr+1 );
	char* ptr2 = strchr ( root, '.' );
	if ( ptr2 )
		*ptr2 = 0;

	return root;
}
*/

/*
=====================
M_GetFileExtension

Get filename extension (without the '.')
=====================
*/
/*char* M_GetFileExtension ( const char* filename )
{
	static char	ext[SYS_MAX_EXT]={0};
	const char		*ptr = strrchr ( filename, '.' );

	if ( ptr )
	{
		strcpy ( ext, ptr+1 );
	}
	else
		ext[0] = '\0';

	return ext;
}*/




/*
==============================================================
    This set modifies oiginal contents: be caeful not to loose infomation
==============================================================
*/


/*
void M_GetFilePath ( char* filename )
{
	char	*ptr;

	ptr = strrchr ( filename, '\\' );
	if ( ptr )
	{
		ptr[1] = 0;
		return;
	}

	ptr = strrchr ( filename, ':' );
	if ( ptr )
	{
		ptr[1] = '\\';
		ptr[2] = 0;
		return;
	}

	ptr = strrchr ( filename, '/' );
	if ( ptr )
	{
		ptr[1] = '\\';
		ptr[2] = 0;
		return;
	}

	filename[0] = 0;
}



void M_GetFileRoot ( char* filename )
{
	char	*ptr;

	ptr = strrchr ( filename, '\\' );
	if ( !ptr )
		ptr = strrchr ( filename, ':' );
	if ( !ptr )
		ptr = filename-1;

	strcpy ( filename, ptr+1 );
	ptr = strchr ( filename, '.' );
	if ( ptr )
		*ptr = 0;
}

*/



/*void M_GetFileExtension ( char* filename )
{
	const char *ptr = strrchr ( filename, '.' );

	if ( ptr )
	{
		strcpy ( filename, ptr+1 );
	}
	else
		strcpy ( filename, "" );
}
*/



void M_FixFilename ( char *filename )
{
	char	*ptr=nullptr;

	switch ( *filename )
	{
		case '\\':
		case ':':
		case '/': strcpy ( filename, filename+1 ); break;
	}

	for ( ptr=filename; *ptr; ptr++ )
	{
		switch ( *ptr )
		{
			case ':':
			case '\\': *ptr = '/'; break;
		}
	}
}



/*
=========================================================================
	This set allocates space for storage: YOU are responsible for
 freeing it!
=========================================================================
*/

char* M_GetFilePathAlloc ( const char* filename )
{
	char	*path=nullptr;
	char	*ptr = nullptr;

	path = new char[SYS_MAX_PATH];
	memset(path, 0, SYS_MAX_PATH);

	strcpy ( path, filename );
	ptr = strrchr ( path, '\\' );
	if ( ptr )
	{
		ptr[1] = 0;
		return path;
	}

	ptr = strrchr ( path, ':' );
	if ( ptr )
	{
		ptr[1] = '\\';
		ptr[2] = 0;
		return path;
	}

	ptr = strrchr ( path, '/' );
	if ( ptr )
	{
		ptr[1] = '\\';
		ptr[2] = 0;
		return path;
	}

	path[0] = 0;
	return path;
}




char* M_GetFileRootAlloc ( const char* filename )
{
	char	*root = nullptr;
	const char	*ptr = nullptr;

	root = new char [SYS_MAX_PATH];
	memset(root, 0, SYS_MAX_PATH);

	ptr = strrchr ( filename, '\\' );
	if ( !ptr )
		ptr = strrchr ( filename, ':' );
	if ( !ptr )
		ptr = (char*) filename-1;

	strcpy ( root, ptr+1 );
	char* ptr2 = strchr ( root, '.' );
	if ( ptr2 )
		*ptr2 = 0;

	return root;
}



/*char* M_GetFileExtensionAlloc ( const char* filename )
{
	char	*ext;
	const char	*ptr = strrchr ( filename, '.' );

	ext = new char [SYS_MAX_EXT];
	memset(ext, 0, SYS_MAX_EXT);

	if ( ptr )
	{
		strcpy ( ext, ptr+1 );
	}
	else
		*ext = '\0';

	return ext;
}*/


/*
=======================
M_SuggestFileExtension

Add extension if one isn't pesent.  Extension paamete should be w/o '.'
=======================
*/
void M_SuggestFileExtension ( char* filename, const char* extension )
{
	if ( strrchr ( filename, '.' ) )
	{
		strcat ( filename, "." );
		strcat ( filename, extension );
	}
}


/*
============================
M_ForceFileExtension

Set the extension even if one is there.  Extension should be w/o '.'
============================
*/
void M_ForceFileExtension ( char* filename, const char* extension )
{
	char *ptr = strrchr ( filename, '.' );

	if ( ptr )
		*ptr = '\0';

	strcat ( filename, "." );
	strcat ( filename, extension );
}




void M_StripFileExtension ( char *filename )
{
	char *ptr = strrchr ( filename, '.' );

	if ( ptr )
		*ptr = '\0';
}




void M_StripFilePath ( char *filename )
{
	char *ptr = strrchr ( filename, '\\' );

	if ( ptr )
	{
		strcpy ( filename, ptr+1 );
		return;
	}

	ptr = strrchr ( filename, ':' );
	if ( ptr )
	{
		strcpy ( filename, ptr+1 );
		return;
	}

	ptr = strrchr ( filename, '/' );
	if ( ptr )
	{
		strcpy ( filename, ptr+1 );
		return;
	}
}


/*char* M_GetFirstPathDir ( char *filename )
{
	static char	dir[SYS_MAX_FILE]={0};
	char		*ptr;

	if ( '/' == *filename )
		filename++;

	ptr = strchr ( filename, '/' );
	if ( NULL == ptr )
		return NULL;

	strncpy ( dir, filename, (dir-filename) );
	strcpy ( filename, ptr );

	return dir;
}
*/


/*
=================
M_GetFirstDir

Same as above, but does not modify original filename and points to
contents of the original
=================
*/
const char* M_GetFirstDir ( const char *filename )
{
	const char		*ptr;

	if ( '/' == *filename )
		filename++;

	ptr = strchr ( filename, '/' );
	if ( NULL == ptr )
		return NULL;

	return ptr+1;
}




/*
char* M_GetFirstPathDirAlloc ( char *filename )
{
	char	*dir = NULL;
	char	*ptr;

	if ( '/' == *filename )
		filename++;

	ptr = strchr ( filename, '/' );
	if ( NULL == ptr )
		return NULL;

	dir = (char*) malloc ( (dir-filename)+1 );
	strncpy ( dir, filename, (dir-filename) );
	strcpy ( filename, ptr );

	return dir;
}



void M_getwd ( char *out )
{
#ifdef _WINDOWS
	_getcwd ( out, SYS_MAX_PATH );
	strcat ( out, "/" );
#else
	getcwd ( out, SYS_MAX_PATH );
#endif
}

*/
