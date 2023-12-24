/*
djlog.cpp

Copyright (C) 1998-2023 David Joffe
*/

#include "config.h"
#ifdef WIN32
#include <windows.h>
#endif

#include "djlog.h"
#include <stdio.h>
#include <stdarg.h>//va_list etc.

//![todo] Refactor away printf-style formatting and use std::cout with std::string's
void log_message( const char * szFormat, ... )
{
	//todo-deprecate:
	//dj2022-11 Making this static buffer thread_local (C++11 onwards) as static buffers will cause thread problems if we ever want to use threads
	static thread_local char buf[4096]={0};

	if ( szFormat == NULL )
		return;

	//todo-deprecate:// See issue "Move away from printf-style formatting"
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
	//todo-deprecate:
   printf( "%s", buf );
   fflush(NULL);
#endif
}
//![/todo]

