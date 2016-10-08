/*
 * File:    sys_error.h
 * Created: 2002-08-30 (Friday), 22:38
 * Modified: 2002-08-30 (Friday), 22:38
 * Author: Vytautas Shaltenis, a.k.a. rtfb
 *
 * Project: Dave Gnukem
 *
 * Description: Error, warning and debug output.
 *
 * NOTE: I use quite an intensive and dangerous perprocessor
 * hackery here. The idea of how to do this __FILE__ and __LINE__
 * workaround is taken from Paul "Midnight" Nettle's memory
 * manager. As he states,
 * <quote>
 *         Macros -- "Kids, please don't try this at home.
 *         We're trained professionals here." :)
 * </quote>
 *
 * :))
 */

#ifndef __SYS_ERROR_H_RTFB__
#define __SYS_ERROR_H_RTFB__


inline void the_void ( const char*, ... ){ return; }

#define SYS_Error (_SetFileAndLine ( __FILE__, __LINE__ ),false)?(void)the_void:_SYS_Error
#define SYS_Warning (_SetFileAndLine ( __FILE__, __LINE__ ),false)?(void)the_void:_SYS_Warning

//#if !defined(DAVE_RELEASE)
#define SYS_Debug (_SetFileAndLine ( __FILE__, __LINE__ ),false)?(void)the_void:_SYS_Debug
//#else
//#define SYS_Debug (void)the_void
//#endif



void _SetFileAndLine ( const char *filename, const int linenum );
void _SYS_Error ( const char *fmt, ... );
void _SYS_Warning ( const char *fmt, ... );
void _SYS_Debug ( const char *fmt, ... );


#endif			// #ifndef __SYS_ERROR_H_RTFB__

