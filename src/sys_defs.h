/*
 * File:    sys_defs.h
 * Created: 2002-06-29 (Saturday), 11:15
 * Author: Vytautas Shaltenis, a.k.a. rtfb
 *
 * Project: Dave Gnukem
 *
 * Description: Main system definitions.
 *
 */

#ifndef SYS_DEFS_H_KRANKLYS__
#define SYS_DEFS_H_KRANKLYS__



extern "C"
{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__)
#include <stdlib.h>//Fixing malloc.h 'not found' error compiling on Mac [dj2016-10]
#else
#include <malloc.h>
#endif
#include <errno.h>
#ifdef WIN32
#include <direct.h> // getcwd etc
#else
#include <unistd.h>
#endif
}

#include <math.h>
#include <float.h>


// fixme high these are too low - but many probably aren't used anymore? clean up .. [dj2022-11 at least making 4096 which is more reasonable for 2022 FOR NOW but this is not the right way to do things ..]
#define SYS_MAX_FULL_PATH	4096	// Don't forget these are with
#define SYS_MAX_PATH		4096	// terminating zeroes. So the real
#define SYS_MAX_FILE		4096	// length is (SYS_MAX_* -1)
#define SYS_MAX_EXT		64


#endif   // #ifndef SYS_DEFS_H_KRANKLYS__
