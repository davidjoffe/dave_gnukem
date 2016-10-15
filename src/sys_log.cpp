/*
 * File:    sys_log.cc
 * Created: 2002-06-29 (Saturday), 11:40
 * Modified: 2002-08-29 (Thursday), 21:06
 * Author: Vytautas Shaltenis, a.k.a. rtfb
 *
 * Project: Dave Gnukem
 *
 * Description: System logger
 *
 * BUGS:
 *	(+) The bitch refused to log anything if no '\n' specified. Also,
 *	garbage at the end of line appeared sometimes. Seems like it was
 *	a matter of lacking terminating zero. Looks like fixed :)
 *	(-) Still needs testing. Won't take long, i think :)
 */


#include "sys_log.h"
#include "sys_error.h"
#include "m_misc.h"
#include "m_aliases.h"

#include <time.h>
#include "sys_defs.h"

#ifdef WIN32
//#include <windows.h>//OutputDebugString
#endif

#define DEFAULT_LOG_FILE	"game.log"

#define MAX_LOGS		32


static bool	log2screen = false;
static bool	log2console = false;
static bool	initialised = false;
static int	log_backup_level = 5;
static char	log_filename_base[SYS_MAX_FILE] = { 0 };
static dword	sys_log = 0;

static FILE	*log_files[MAX_LOGS] = { NULL };
static dword	masks[MAX_LOGS] = { 0 };
static unsigned int	num_logs = 0;


void BackupAndCreate ( FILE **f, const char *filename, int bklevel );
void PushBackup ( const char *filename, int bklevel );



// Init/Kill
void InitLog ()
{
	for ( unsigned int i=0; i<MAX_LOGS; ++i )
	{
		masks[i] = SETBIT(i);
	}

	num_logs = 0;

	sys_log = CreateLog ( "DaveGnukem.log", "System" );
}



void KillLog ()
{
	initialised = false;
}





dword CreateLog ( const char *filename, const char *descr )
{
	time_t		t 	= time ( NULL );
	struct tm	*tme 	= localtime ( &t );

	if ( MAX_LOGS == num_logs )
	{
		printf ( "CreateLog: Log limit reached (%d)\n", num_logs );
		SYS_Error ( "CreateLog: Log limit reached (%d)\n", num_logs );
	}

	BackupAndCreate ( &log_files[num_logs], filename, 0 );

	fprintf ( log_files[num_logs], "+------------------------------------------------------------------+\n" );
	fprintf ( log_files[num_logs], "|          %s log file for %02d/%02d/%04d  %02d:%02d:%02d                |\n", descr, tme->tm_mon + 1, tme->tm_mday, tme->tm_year + 1900, tme->tm_hour, tme->tm_min, tme->tm_sec   );
	fprintf ( log_files[num_logs], "+------------------------------------------------------------------+\n" );
	fprintf ( log_files[num_logs], "\n" );
	fprintf ( log_files[num_logs], "\n" );
	fflush ( log_files[num_logs] );

	initialised = true;

	num_logs++;
	return masks[num_logs-1];
}



void DisposeLog ( dword log_id )
{
	for ( unsigned int i=0; i<num_logs; ++i )
	{
		if ( SETBIT(i) == log_id )
		{
			fclose ( log_files[i] );
			initialised = false;
		}
	}
}



void Log ( const char *fmt, ... )
{
	char		text[1024]={0};
	va_list		ap;

	if ( !initialised )
		return;

	if ( NULL == fmt )
		return;

	memset ( text, 0, 1024 );

	va_start ( ap, fmt );
		vsprintf ( (char*)text, fmt, ap );
	va_end ( ap );

	fprintf ( log_files[0], "%s", text );
	fflush ( log_files[0] );

	#if defined(WIN32) && defined(_DEBUG)
	//dj2016-10 Log to debugger in Windows
	//::OutputDebugString( text );
	#endif
}




void Log ( dword log_mask, const char *fmt, ... )
{
	char		text[2048]={0};
	va_list		ap;

	if ( !initialised )
		return;

	if ( NULL == fmt )
		return;

	memset ( text, 0, 2048 );

	va_start ( ap, fmt );
		vsprintf ( (char*)text, fmt, ap );
	va_end ( ap );

	for ( unsigned int i=0; i<num_logs; i++ )
	{
		if ( SETBIT(i) == log_mask )
		{
			fprintf ( log_files[i], "%s", text );
			fflush ( log_files[i] );

			#if defined(WIN32) && defined(_DEBUG)
			//dj2016-10 Log to debugger in Windows
			//::OutputDebugString( text );
			#endif
		}
	}
}




// This tells logger whether or not to log to system console
void LogToScreen ( const bool l2s )
{
	log2screen = l2s;
}



// This tells logger whether or not to log to game console
void LogToConsole ( const bool l2c )
{
	log2console = l2c;
}



void BackupAndCreate ( FILE **f, const char *filename, int bklevel )
{
	char	file[SYS_MAX_FILE];

	if ( NULL == filename )
		strcpy ( file, DEFAULT_LOG_FILE );
	else
		strcpy ( file, filename );

	strcpy ( log_filename_base, file );
	M_StripFileExtension ( log_filename_base );

	PushBackup ( file, bklevel );

	*f = fopen ( file, "w" );

	if ( NULL == *f )
	{
		SYS_Error ( "Could not ceate log file!\n" );
	}
}




void PushBackup2 ( const char *filename, int bklevel )
{
	char		oldname[SYS_MAX_FILE];
	char		newname[SYS_MAX_FILE];
	char		appendix[SYS_MAX_EXT];
	FILE		*ff = NULL;

	strcpy ( oldname, filename );

	ff = fopen ( oldname, "r" );
	if ( ff )
		fclose ( ff );

	while ( ff && log_backup_level != bklevel )
	{
		sprintf ( appendix, "old%d", bklevel );
		strcpy ( newname, log_filename_base );
		M_ForceFileExtension ( newname, appendix );

		errno = 0;
		if ( -1 == rename ( oldname, newname ) )
		{
			M_ParseErrno ( errno );
		}
		strcpy ( oldname, newname );
		ff = fopen ( oldname, "r" );
		if ( ff )
			fclose ( ff );
		bklevel++;
	}
}





void PushBackup ( const char *filename, int bklevel )
{
	char		newname[SYS_MAX_FILE];
	char		appendix[SYS_MAX_EXT];
	FILE		*ff;

	ff = fopen ( filename, "r" );
	if ( ff )
		fclose ( ff );

	if ( !ff )
	{
		return;
	}

	if ( ff && log_backup_level == bklevel )
	{
		unlink ( filename );
		return;
	}

// assemble a new name:
	sprintf ( appendix, "old%d", bklevel );
	strcpy ( newname, log_filename_base );
	M_ForceFileExtension ( newname, appendix );

	PushBackup ( newname, bklevel+1 );
	rename ( filename, newname );
}





dword SysLog ()
{
	return sys_log;
}

