/*--------------------------------------------------------------------------*/
#ifndef _CONFIG_H_
#define _CONFIG_H_
/*--------------------------------------------------------------------------*/

// These should be dynamic, to allow any size level. I initially made
// restrictions because of the memory limitations in the DOS world.
#define LEVEL_WIDTH  128
#define LEVEL_HEIGHT 100
//! Size in bytes of the in-memory level 'data block'. See comments at \ref SLevelBlock below for where this 4 comes from (in short: foreground block (a,b) + background block(a,b), where a=spritesetindex and b=offsetintospriteset)
#define LEVEL_SIZE   (4 * LEVEL_WIDTH * LEVEL_HEIGHT)

//! Width of a basic game 'block'/'sprite' in pixels
#define BLOCKW (16)
//! Height of a basic game 'block'/'sprite' in pixels
#define BLOCKH (16)

//! dj2018-01 Experimental auto drop shadows in sprites
#define EXPERIMENTAL_SPRITE_AUTO_DROPSHADOWS
#define EXPERIMENTAL_MAP_AUTO_DROPSHADOWS

//! [dj2017-08] Experimental
#define LARGE_VIEWPORT_TEST
#ifdef LARGE_VIEWPORT_TEST

// NB keep in mind 'big viewport mode' and 'large viewport' mode are
// two totally different things, not to be confused with one another.
// 'Big viewport mode' runs the game in a large HIGH RESOLUTION
// window, keeping 1:1 pixel ratio for sprites (so sprites draw tiny
// and you see a lot more game area); large/wide viewport mode
// still scales the game viewport so it's effectively 320x200,
// but just uses up more area of the screen (i.e. instead of having
// the big right-bar area for score and health etc. and the border
// around the gameplay viewport, that space is used for gameplay area).

//! 'Large viewport mode' NOT TO BE CONFUSED WITH g_bBigViewportMode! TOTALLY DIFFERENT THINGS.
extern bool g_bLargeViewport;
#endif

/*--------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------*/
