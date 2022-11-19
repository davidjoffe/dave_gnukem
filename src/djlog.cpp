/*
djlog.cpp

Copyright (C) 1998-2022 David Joffe
*/

#ifdef WIN32
#include <windows.h>
#endif

#include "djlog.h"
#include <stdio.h>
#include <stdarg.h>//va_list etc.

void log_message( const char * szFormat, ... )
{
	//dj2022-11 Making this static buffer thread_local (C++11 onwards) as static buffers will cause thread problems if we ever want to use threads
	static thread_local char buf[4096]={0};

	if ( szFormat == NULL )
		return;

	// print the formatted log string onto buf
	va_list args;
	va_start(args, szFormat);
	vsnprintf(buf, sizeof(buf), szFormat, args);
	va_end(args);

#ifdef WIN32
   // send message to debugger
   OutputDebugString( (LPCTSTR)buf );
#else
   // dump message on stdout
	// NB! (security) - do NOT do "printf(buf) here because if it contains e.g. a "%s" we'll have a crash - hence the deliberate ("%s", buf) [dj2022-11]
   printf( "%s", buf );
   fflush(NULL);
#endif
}

