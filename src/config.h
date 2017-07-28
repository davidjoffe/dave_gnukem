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

/*--------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------*/
