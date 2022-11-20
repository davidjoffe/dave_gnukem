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
 *
 * Update: It's toned down now - we couldn't handle the danger.
 * 
 * dj2022-11 note that comment about using "quite an intensive and dangerous perprocessor hackery" was not mine and I agree all this code must be simpler/safter more robust and easier to maintain.
 */

#ifndef __SYS_ERROR_H_RTFB__
#define __SYS_ERROR_H_RTFB__


inline void the_void ( const char*, ... ){ return; }

#define SYS_Error(...) (_SYS_Error(__FILE__, __LINE__, __VA_ARGS__))
#define SYS_Warning(...) (_SYS_Warning(__FILE__, __LINE__, __VA_ARGS__))
#define SYS_Debug(...) (_SYS_Debug(__FILE__, __LINE__, __VA_ARGS__))


void _SYS_Error ( const char *file, int line, const char *fmt, ... );
void _SYS_Warning ( const char *file, int line, const char *fmt, ... );
void _SYS_Debug ( const char *file, int line, const char *fmt, ... );


#endif			// #ifndef __SYS_ERROR_H_RTFB__

