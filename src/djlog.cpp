/*
djlog.cpp

Copyright (C) 1998-2001 David Joffe

License: GNU GPL Version 2 (*not* "later versions")
*/

#ifdef WIN32
#include <windows.h>
#endif

#include "djlog.h"
#include <stdio.h>

void log_message( const char * szFormat, ... )
{
   static char buf[1024];

   if ( szFormat == NULL )
      return;

   // print the formatted log string onto buf
   vsprintf( buf, szFormat, (char*)(&szFormat + 1) );

#ifdef WIN32
   // send message to debugger
   OutputDebugString( (LPCTSTR)buf );
#else
   // dump message on stdout
   printf( "%s", buf );
   fflush(NULL);
#endif
}

void log_do_nothing( char * szFormat, ... )
{
	szFormat = szFormat;		// shut up the "unused patameter" warning
}

