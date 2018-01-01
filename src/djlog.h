/*!
\file    djlog.h
\brief   Debugging message "trace" helpers
\author  David Joffe

modified by Vytautas Shaltenis, a.k.a. rtfb

Copyright (C) 1998-2018 David Joffe

License: GNU GPL Version 2
*/
/*--------------------------------------------------------------------------*/
#ifndef _DJLOG_H_
#define _DJLOG_H_

#define TRACE SYS_Debug
#define djMSG SYS_Debug

#include "sys_error.h"

/*
#if defined(DEBUG) || defined(NDEBUG) || defined(_DEBUG)
#define TRACE log_message
#else
#define TRACE ;;
#endif

#define djMSG log_message

extern void log_message( const char * szFormat, ... );
extern void log_do_nothing( const char * szFormat, ... );
*/

#endif
