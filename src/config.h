// Copyright (C) 1998-2020 David Joffe
/*--------------------------------------------------------------------------*/
#ifndef _CONFIG_H_
#define _CONFIG_H_
/*--------------------------------------------------------------------------*/

#ifdef WIN32
//! [dj2018-04-10] See comments at this stuff. If this works nicely and is stable, can probably get rid of the djWINXP_SUPPORT stuff ultimately ???
//! NB if this causes crashiness or other issues, then maybe go back to static binding?
#define djDYNAMICALLY_BIND_SETPROCESSDPIAWARE
//! [dj2018-03] Enable this if you want the application to still support Windows XP. Off by default. See comments at the fix for https://github.com/davidjoffe/dave_gnukem/issues/98 - we end up with an overly large game window on if DPI scaling is set, but the SetProcessDPIAware() APIs to fix this are only supported on later versions of Windows.
//! Not sure if this still makes sense, unless djDYNAMICALLY_BIND_SETPROCESSDPIAWARE is not defined
//#define djWINXP_SUPPORT
#endif

// These should be dynamic, to allow any size level. I initially made
// restrictions because of the memory limitations in the DOS world.
#ifndef LEVEL_WIDTH
#define LEVEL_WIDTH  (128)
#endif
#ifndef LEVEL_HEIGHT
#define LEVEL_HEIGHT (100)
#endif
#define LEVEL_BYTESPERBLOCK (4)
//! Size in bytes of the in-memory level 'data block'. See comments at \ref SLevelBlock below for where this 4 comes from (in short: foreground block (a,b) + background block(a,b), where a=spritesetindex and b=offsetintospriteset)
#define LEVEL_SIZE   (LEVEL_BYTESPERBLOCK * LEVEL_WIDTH * LEVEL_HEIGHT)
// Convenience helper //dj2019-07
#define LEVEL_BYTESPERROW (LEVEL_WIDTH * LEVEL_BYTESPERBLOCK)

#ifndef BLOCKW
//! Width of a basic game 'block'/'sprite' in pixels
#define BLOCKW (16)
#endif
#ifndef BLOCKH
//! Height of a basic game 'block'/'sprite' in pixels
#define BLOCKH (16)
#endif



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

extern bool g_bBigViewportMode;//dj2019-06. //Can't have both bigviewport and largeviewport at same time. maybe make enum later? or something else OOP-y.
#endif

//dj2018-03 Start making a (forward-looking) #define if we want to genericize this and make other games off it, then wrap 'very DN1-specific stuff' in a #define - for Dave Gnukem 1 it's always on
#define tBUILD_DAVEGNUKEM1

#define FILE_SAVEGAME "savegame.dat"

//---------------------------------------------------------------------------
// [dj2019-06 start trying to make the 320x200 less hardcoded so the engine can handle other resolution games.]
// This stuff probably needs a diagram or something to be more immediately clearer, as it may be confusing .. DG1 effectively uses a triple-buffering scheme. (dj2019-06)
// For triple-buffering, the application's desired 'pixel resolution' eg 320x200 for Dave Gnukem 1 as it should retro-mimic DN1
// NB: The actual third buffer may well be larger, eg in Windows 3rd buffer may be eg 1600x1000 on a 1920x1080 display, but the DG1 game only renders to a tiny 320x200 corner of it in top-left (then that is scale-blitted up to end up in the window as 1600x1000 or whatever).
// (Note this is not applicable when looking at eg DG1 level editor, which 'becomes' high-resolution to whatever size screen available, not 320x200)

//#define CFG_APPLICATION_RENDER_RES_W (1920)
//#define CFG_APPLICATION_RENDER_RES_H (1080)
//#define CFG_APPLICATION_RENDER_RES_W (960)
//#define CFG_APPLICATION_RENDER_RES_H (540)
//#define CFG_APPLICATION_RENDER_RES_W (1280)
//#define CFG_APPLICATION_RENDER_RES_H (720)

// The point of these settings is so if you want to make your own game with this 'engine' you define your sort of 'actual desired game resolution' here.
// We use #ifndef here so you can pass these in as a compiler or Makefile setting too, in theory, though for DG1 they 'must' be 320x200 for the retro-DN1-style-game to make sense, but other games using this 'engine' could be any resolution you want [dj2019-07]

// These could in theory even be read out of a data/config file, though the compiler can optimize better if it's a constant like this, compared to a variable ... pros/cons either way, and might be game-dependent so let's try keep things flexible. Let's keep it simple for now [dj2019-06]:
#ifndef CFG_APPLICATION_RENDER_RES_W
//! Size in pixels of application's desired offscreen render buffer width [pixels], eg DG1 this would always be 320 no matter what.
#define CFG_APPLICATION_RENDER_RES_W (320)
#endif

#ifndef CFG_APPLICATION_RENDER_RES_H
//! Size in pixels of application's desired offscreen render buffer height [pixels], eg DG1 this would always be 200 no matter what.
#define CFG_APPLICATION_RENDER_RES_H (200)
#endif
//---------------------------------------------------------------------------

//dj2019-07 Convenience helpers, not really too important (in theory could avoid a bitshift for speed here, but optimizing compiler SHOULD do that already as long as BLOCKW is a constant.)
#define HALFBLOCKW ((BLOCKW)>>1)
#define HALFBLOCKH ((BLOCKH)>>1)
//dj2019-07 Top-left world pixel coordinate of hero - more convenience helpers to help clean up and genericize the code and improve readability a bit but these should probably be temporary, don't like them, should probably change:
#define HERO_PIXELX (g_Player.x*BLOCKW + g_Player.x_small * HALFBLOCKW)
#define HERO_PIXELY (g_Player.y*BLOCKH + g_Player.y_offset - BLOCKH)

//dj2019-07 This stuff should probably change. Currently for DG1 the hero (for collision detection purposes etc.) is 1x2 'game blocks'
#define HEROW_COLLISION (BLOCKW)
#define HEROH_COLLISION (BLOCKH*2)

/*--------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------*/
