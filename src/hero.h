/*!
\file    hero.h
\brief   Hero-related stuff
\author  David Joffe

Copyright (C) 2001-2024 David Joffe

I'm not a fan of the various globals here; they're mostly from the very
oldest parts of the codebase, when I was still an inexperienced coder ...
but not necessarily at this stage worth investing time in refactoring,
unless we want to genericize this codebase more/better for other derived
games. [dj2018]
[dj2020-06: Fixing some of the most embarassing globals, eg global x,y and create new CPlayer class for the hero]
*/
#ifndef _HERO_H_
#define _HERO_H_

//! Hero movement modes
enum
{
	MODE_NORMAL = 0,
	MODE_JUMPING
};

class CPlayer
{
public:
	CPlayer();

	//! x,y = Hero's absolute x position in level (in level block coordinates, not pixel units, and excluding the half-block 'x_small' offset that is fairly specific to DN1/DaveGnukem) (0,0 = upper left of level)
	int x;
	//! x,y = Hero's absolute y position in level (in level block coordinates, not pixel units) (0,0 = upper left of level)
	int y;

	//! If x_small is 0 then hero is at 'x', if 1 then hero is at x+8 pixels (or to be more specific, an extra half-block offset in the x direction, whatever the block width in pixels) ... this is slightly quite specific-ish to DN1/DaveGnukem style of movement, in a generic 2D platformer I might not want it - in that case, we might want to derive e.g. CPlayerGnukem from e.g. CPlayer and put it in the derived class as a possible solution to doing things generically. Note that when this code was created, many compilers didn't have 'bool' type in C++, that's why it was represented as an int.
	int x_small;
	//! Pixel offset, e.g. [-15,15] relative to the hero's 'block unit' 'y' position. For smooth vertical movement. [Added dj2017-06]
	int y_offset;

	//! Left/right direction hero is facing, left==0, right==1
	int hero_dir;

	//! Hero movement mode
	int hero_mode;
	//! When hero is hurt, this is set to a positive "framecount" that
	//! counts down. Can't get hurt again until it reaches 0.
	int nHurtCounter;

	// This m_nFalltime thing is to make hero fall initially slower
	// then faster (full block at a time).
	// Apart from looking/feeling slightly more natural, it also 'masks'
	// an issue with smooth vertical movement enabled where you get a jerky
	// effect that looks like hero bouncing jerkily up and down when falling
	// off bottom of view, as the view code scrolls vertically always in increments of 16 pixels,
	// whereas if hero falls at 8 pixels off bottom then relative vertical offset
	// of hero on screen toggles 8 pixels each consecutive frame. With
	// this falltime thing, by the time he is falling off the bottom,
	// he is falling 16 pixels, and the view scrolls 16 pixels too.
	// It's a bit fiddly but anyway, we have to do fine tweaks like this.
	// See also liveedu.tv video 2017-06-24 [dj2017-06-24]
	int m_nFalltime;
};
//! Main hero/player (for now, we only support a single player etc., but in future can make this more generic and support multiple players)
extern CPlayer g_Player;

// This should really just be permanently on I guess, not sure if there's any good reason to turn it off, unless we want to backtrack today's changes. [dj2017-06-24]
extern bool g_bSmoothVerticalMovementEnabled;

// [dj2020-06 - Wrap old globals xo,yo etc. into new class for viewport - note this doesn't belong in 'hero.h/cpp', should maybe have its own h/cpp - low prio]
class CViewport
{
public:
	CViewport();

	//! xo,yo = top-left corner of view for scrolling
	int xo;
	//! xo,yo = top-left corner of view for scrolling
	int yo;
	//! View offset by 8 pixels? (If I remember correctly this is basically either 0 or 1, but we do use it as an int for some calculations, so maybe still best to leave it as an int.) This stuff has to do with the particular way DN1's viewport scrolling worked; either the horizontal game viewport is aligned to the 16-pixel boundaries (in which case this is 0), or it's further offset by half a block i.e. 8 pixels, depending.)
	int xo_small;
};
extern CViewport g_Viewport;

//! hero animation image index offset [dj2020-06 naively one might think this belongs in CPlayer, but really it's more to do with the CPlayer 'visual' - so maybe belongs in a separate class - not important]
extern int hero_picoffs;
extern int nSlowDownHeroWalkAnimationCounter;

//! Immediately after firing weapon, the hero sprite is drawn slightly differently
//! briefly, which gives almost a slight 'recoil/kickback' visual effect, this ugly
//! global variable is to simulate that (this is based on the animation behaviour in
//! DN1, where it does that).
//! The visual behaviour still needs a little tweaking, it's not quite correct - in DN1
//! when 'resting', the hero normally returns to a 'stand-still position', and the
//! on-fired-weapon offset is relative to that ... in ours, we 'incorrectly' don't return
//! to standstill (e.g. may 'rest' with one foot forward as if halfway through walking)
//! so for now we just add a relative offset from whatever the current sprite is [low - dj2018-01]
//! (Design-wise, there should probably be some sort of hero class or something (or two
//! classes, e.g. a hero 'state' and hero 'view', e.g. Model/View/Controller paradigm),
//! and e.g. Nukem/Gnukem-specific stuff like this could be in derived classes from those
//! ... that's extremely low-prio, I'll probably never bother doing that. ~dj2018-01)
extern int g_nHeroJustFiredWeaponCounter;



//! Jump modes
enum EJump
{
	JUMP_NORMAL,
	JUMP_POWERBOOTS
};
//! Set hero's current jump mode (i.e. normal height jump, or higher jump with powerboots)
extern void HeroSetJumpMode(EJump eJump);
//! Get hero's current jump mode (i.e. normal height jump, or higher jump with powerboots)
extern EJump HeroGetJumpMode();
//! Start hero jump
extern void HeroStartJump();
//! Cancel an in-progress jump (e.g. if hero gets hurt)
extern void HeroCancelJump();
//! Update one frame of hero's jump
extern void HeroUpdateJump();
extern void HeroUpdate();


//! "Freeze" hero's movement for next n frames (e.g. if teleporting or exit activated)
extern void HeroFreeze(int n);
//! Unfreeze hero's movement
extern void HeroUnfreeze();
//! Test if hero is currently frozen
extern bool HeroIsFrozen();
//! Reset hero state on start of level
extern void HeroReset();


//! Set the hero's position in level block coordinates (this resets the x_small and yoffset to 0 but does not alter the left/right direction the hero is currently facing)
extern void relocate_hero( int xnew, int ynew );
//! Attempt to move the hero
extern int  move_hero(int xdiff, int ydiff, bool bChangeLookDirection=true);



#endif
