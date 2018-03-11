/*
 * File:    m_misc.h
 * Created: 2002-06-29 (Saturday), 15:01
 * Modified: 2002-07-22 (Monday), 04:13
 * Author: Vytautas Shaltenis, a.k.a. rtfb
 *
 * Project: Dave Gnukem
 *
 * Description: Miscellaneous utility functions.
 *
 */

#ifndef M_MISC_H_KRANKLYS__
#define M_MISC_H_KRANKLYS__



#define SETBIT(x) (((unsigned int)1)<<x)


void M_ParseErrno ( int err );



void M_StrToLower ( char *ptr );
void M_StrToUpper ( char *ptr );



// command line utility functions
//int M_CmdLineOption ( const char* str, int reqPams );


//
// filename utility functions
//
bool M_IsDirectory ( const char *filename );

// this set returns pointers to static buffers: copy if needed
char* M_GetFilePath ( const char* filename );
char* M_GetFileRoot ( const char* filename );
//char* M_GetFileExtension ( const char* filename );//dj2016-10 commenting out M_GetFileExtension as they seem to be unused [now] from what i can tell
//char* M_GetFirstPathDir ( char *filename );//dj2016-10 commenting out as it seems to be unused
const char* M_GetFirstDir ( const char *filename );

// this set modifies original contents: be careful not to loose information
void M_GetFilePath ( char* filename );
void M_GetFileRoot ( char* filename );
//void M_GetFileExtension ( char* filename );
/*
 * NOTE: Very important function! Call before any access to files, unless
 * absolutely sure that filename is correct.
 */
void M_FixFilename ( char *filename );

// this set allocates space for storage: YOU are responsible for freeing it!
char* M_GetFilePathAlloc ( const char* filename );
char* M_GetFileRootAlloc ( const char* filename );
//char* M_GetFileExtensionAlloc ( const char* filename );
char* M_GetFirstPathDirAlloc ( char *filename );

void M_SuggestFileExtension ( char* filename, const char* extension );
void M_ForceFileExtension ( char* filename, const char* extension );

void M_StripFileExtension ( char *filename );
void M_StripFilePath ( char *filename );

void M_getwd ( char *out );


#endif 		// #ifndef M_MISC_H_KRANKLYS__
