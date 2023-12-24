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
#include <stdarg.h>//va_start etc.

void _SYS_Error ( const char *file, int line, const char *fmt, ... )
{
	if (NULL == fmt)
		return;

	static thread_local char		text[4096]={0};
	static thread_local char		text2[4096+1024]={0};

	//todo-deprecate:// See issue "Move away from printf-style formatting"
	va_list		args;
	va_start ( args, fmt );
		vsnprintf ( text, sizeof(text), fmt, args );
	va_end ( args );

	snprintf ( text2, sizeof(text2), "[Error] %s line %d: %s", file, line, text);

	djLog::LogStr( text2 );
	//DaveCleanup ();			// main application cleanup
	//exit ( 0 );
}


void _SYS_Warning ( const char *file, int line, const char *fmt, ... )
{
	if (NULL == fmt)
		return;

	static thread_local char		text[4096] = { 0 };
	static thread_local char		text2[4096+1024] = { 0 };

	//todo-deprecate:// See issue "Move away from printf-style formatting"
	va_list		args;
	va_start ( args, fmt );
		vsnprintf ( text, sizeof(text), fmt, args );
	va_end ( args );

	snprintf ( text2, sizeof(text2), "[Warning] %s line %d: %s", file, line, text);
  
	djLog::LogStr(text2);
}


void _SYS_Debug ( const char *file, int line, const char *fmt, ... )
{
	if (NULL == fmt)
		return;

	static thread_local char		text[4096]={0};
	static thread_local char		text2[4096+1024]={0};

	//todo-deprecate:// See issue "Move away from printf-style formatting"
	va_list		args;
	va_start ( args, fmt );
		vsnprintf ( text, sizeof(text), fmt, args );
	va_end ( args );

	snprintf( text2, sizeof(text2), "[Debug] %s line %d: %s", file, line, text);

	djLog::LogStr(text2);
}
