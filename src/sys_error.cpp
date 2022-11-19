/*
 * File:    sys_error.cc
 * Created: 2002-08-30 (Friday), 22:38
 * Modified: 2002-08-30 (Friday), 22:38
 * Author: Vytautas Shaltenis, a.k.a. rtfb
 *
 * Project: Dave Gnukem
 *
 * Description: Error, warning and debug output.
 *
 * NOTE: I use quite an intensive and dangerous perprocessor hackery here.
 * Kids, do not try this at home :)
 */

#include "sys_log.h"
#include "sys_defs.h"
#include <string>

//dj2022-11 add thread_local (C++11) for thread-safety here as this looks like the sort of thing that might cause problems if we start adding threads in future:
thread_local std::string g_sFile;
thread_local int	g_nLine = 0;

void _SetFileAndLine ( const char *filename, const int linenum )
{
	// [dj2017-06-20] Make a copy of the passed-in filename string rather than keep pointer to it (in case caller deletes string, then this would crash)
	g_sFile = "";
	if (filename!=NULL)
		g_sFile = filename;
	g_nLine = linenum;
}


void _SYS_Error ( const char *fmt, ... )
{
	if (NULL == fmt)
		return;

	static thread_local char		text[4096]={0};
	static thread_local char		text2[4096]={0};

	va_list		args;
	va_start ( args, fmt );
		vsnprintf ( text, sizeof(text), fmt, args );
	va_end ( args );

	snprintf ( text2, sizeof(text2), "[Error] %s line %d: %s", g_sFile.c_str(), g_nLine, text);

	djLog::LogStr( text2 );
	//DaveCleanup ();			// main application cleanup
	//exit ( 0 );
}


void _SYS_Warning ( const char *fmt, ... )
{
	if (NULL == fmt)
		return;

	static thread_local char		text[4096] = { 0 };
	static thread_local char		text2[4096] = { 0 };

	va_list		args;
	va_start ( args, fmt );
		vsnprintf ( text, sizeof(text), fmt, args );
	va_end ( args );

	snprintf ( text2, sizeof(text2), "[Warning] %s line %d: %s", g_sFile.c_str(), g_nLine, text);

	djLog::LogStr(text2);
}


void _SYS_Debug ( const char *fmt, ... )
{
	if (NULL == fmt)
		return;

	static thread_local char		text[4096]={0};
	static thread_local char		text2[4096]={0};

	va_list		args;
	va_start ( args, fmt );
		vsnprintf ( text, sizeof(text), fmt, args );
	va_end ( args );

	snprintf( text2, sizeof(text2), "[Debug] %s line %d: %s", g_sFile.c_str(), (int)g_nLine, text);

	djLog::LogStr(text2);
}
