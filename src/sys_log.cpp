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
 *	(+) [It] refused to log anything if no '\n' specified. Also,
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
#include "djstring.h"//djAppendPathStr [should move elsewhere?? dj2018-03]

//#if defined(WIN32) && defined(_DEBUG)
////#include <Windows.h>//For OutputDebugString
//#endif

#define DEFAULT_LOG_FILE	"game.log"

#define MAX_LOGS		32


//dj2022-11 note some of this stuff probably not used anymore, maybe clean up someday .. also don't think we really need the rotating logs stuff just adds complexity at this point that I don't think justifies benefit

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

	std::string sPath = djAppendPathStr(djGetFolderUserSettings().c_str(), "logs");
	sPath = djAppendPathStr(sPath.c_str(), "DaveGnukem.log");
	sys_log = CreateLog ( sPath.c_str(), "System" );
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

	if (log_files[num_logs]!=NULL)
	{
		fprintf ( log_files[num_logs], "+------------------------------------------------------------------+\n" );
		fprintf ( log_files[num_logs], "|          %s log file for %02d/%02d/%04d  %02d:%02d:%02d                |\n", descr, tme->tm_mon + 1, tme->tm_mday, tme->tm_year + 1900, tme->tm_hour, tme->tm_min, tme->tm_sec   );
		fprintf ( log_files[num_logs], "+------------------------------------------------------------------+\n" );
		fprintf ( log_files[num_logs], "\n" );
		fprintf ( log_files[num_logs], "\n" );
		fflush ( log_files[num_logs] );
	}

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
			if (log_files[i] != NULL)
				fclose ( log_files[i] );
			initialised = false;
		}
	}
}


//dj2022-11 refactoring some of the log stuff ..
void djLog::LogStr(const char* szPlainString)
{
	if (!initialised) return;
	if (szPlainString == nullptr) return;

	if (log_files[0] != NULL)
	{
		fprintf(log_files[0], "%s", szPlainString);
		fflush(log_files[0]);
	}

#if defined(WIN32) && defined(_DEBUG)
	//dj2016-10 Log to debugger in Windows
	//::OutputDebugString( szPlainString );
#endif
}


void djLog::LogFormatStr( const char *fmt, ... )
{
	if (!initialised)
		return;
	if (NULL == fmt)
		return;

	static thread_local char		text[4096]={0};
	memset ( text, 0, 4096 );

	va_list		ap;
	va_start ( ap, fmt );
		vsnprintf ( (char*)text, sizeof(text), fmt, ap );
	va_end ( ap );

	djLog::LogStr(text);
}



/*
void djLog::LogFormatStr2( dword log_mask, const char *fmt, ... )
{
	if ( !initialised )
		return;
	if ( NULL == fmt )
		return;

	static thread_local char		text[4096] = { 0 };
	memset(text, 0, 4096);

	va_list		ap;
	va_start ( ap, fmt );
		vsnprintf ( (char*)text, sizeof(text), fmt, ap );
	va_end ( ap );

	for ( unsigned int i=0; i<num_logs; i++ )
	{
		if ( SETBIT(i) == log_mask )
		{
			if (log_files[i]!=NULL)
			{
				fprintf ( log_files[i], "%s", text );
				fflush ( log_files[i] );
			}

			#if defined(WIN32) && defined(_DEBUG)
			//dj2016-10 Log to debugger in Windows
			//::OutputDebugString( text );
			#endif
		}
	}
}
*/



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
	char	file[SYS_MAX_FILE]={0};

	if ( NULL == filename )
		strcpy ( file, DEFAULT_LOG_FILE );
	else
		strcpy ( file, filename );

	strcpy ( log_filename_base, file );
	M_StripFileExtension ( log_filename_base );

	// dj2019-06 Commenting this out to effectively disable rotating of logs to fix this issue as reported by keithbowes:
	// https://github.com/davidjoffe/dave_gnukem/issues/120
	// ("home directory is littered with files like ~/.old0, ~/.old1, ~/.old2")
	// I don't really feel it's worth rotating logs; I seldom if ever go look at old logs. We can maybe add it
	// later (and fix the 'littering' issue) IF it seems in future like it's worth it to have rotating logs.
	//PushBackup ( file, bklevel );

	*f = fopen ( file, "w" );

	if ( NULL == *f )
	{
		SYS_Error ( "Could not ceate log file!\n" );
	}
}




/*void PushBackup2 ( const char *filename, int bklevel )
{
	char		oldname[SYS_MAX_FILE]={0};
	char		newname[SYS_MAX_FILE]={0};
	char		appendix[SYS_MAX_EXT]={0};
	FILE		*ff = NULL;

	strcpy ( oldname, filename );

	ff = fopen ( oldname, "r" );
	if ( ff )
		fclose ( ff );

	while ( ff && log_backup_level != bklevel )
	{
		snprintf ( appendix, sizeof(appendix), "old%d", bklevel );
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
}*/





// dj2019-06 Commenting out call to this function to effectively disable rotating of logs to fix this issue as reported by keithbowes:
// https://github.com/davidjoffe/dave_gnukem/issues/120
// ("home directory is littered with files like ~/.old0, ~/.old1, ~/.old2")
// I don't really feel it's worth rotating logs; I seldom if ever go look at old logs. We can maybe add it
// later (and fix the 'littering' issue) IF it seems in future like it's worth it to have rotating logs.
// Otherwise, later maybe just delete this function (that's my recommendation, to simplify this code)
void PushBackup ( const char *filename, int bklevel )
{
	char		newname[SYS_MAX_FILE]={0};
	char		appendix[SYS_MAX_EXT]={0};
	FILE		*ff=nullptr;

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
	snprintf ( appendix, sizeof(appendix), "old%d", bklevel );
	strcpy ( newname, log_filename_base );
	M_ForceFileExtension ( newname, appendix );

	PushBackup ( newname, bklevel+1 );
	rename ( filename, newname );
}





dword SysLog ()
{
	return sys_log;
}

