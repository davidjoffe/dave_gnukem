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



//dj2022-11 [low] some of these aren't used anymore, and I think 'core' headers (i.e. included indirectly by many other things, e.g. logging code) should only include the *minimum* necessary for compile speed reasons
// Also where reasonably possibly should ojnly include in the .cpp code where's it's necessary (rather than the .h) e.g. getcwd used only in a couple of places
// Also not sure if we should even use 'malloc' anymore, personally I don't use it and rather just use 'new' operator where possible (except certain specific APIs may need malloc in the old days)
// 'clean up clean out' this below list a bit?


//dj2022-11 tentatively commenting out various includes in this file and trying to refactor to clean up and simplify and reduce this list a bit .. see above comments
//extern "C"
//{
//#include <stdio.h>
//#include <stdlib.h>
//#include <stdarg.h>//va_start etc.
//#include <stddef.h>
//#include <string.h>
/*
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__)
#include <stdlib.h>//Fixing malloc.h 'not found' error compiling on Mac [dj2016-10]
#else
#include <malloc.h>
#endif
*/
//#include <errno.h>
//}

//#include <math.h>
//#include <float.h>


// fixme high these are too low - but many probably aren't used anymore? clean up .. [dj2022-11 at least making 4096 which is more reasonable for 2022 FOR NOW but this is not the right way to do things ..]
#define SYS_MAX_FULL_PATH	4096	// Don't forget these are with
#define SYS_MAX_PATH		4096	// terminating zeroes. So the real
#define SYS_MAX_FILE		4096	// length is (SYS_MAX_* -1)
#define SYS_MAX_EXT		64


#endif   // #ifndef SYS_DEFS_H_KRANKLYS__
