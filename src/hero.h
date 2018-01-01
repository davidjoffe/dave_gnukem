/*!
\file    hero.h
\brief   Hero-related stuff
\author  David Joffe

Copyright (C) 2001-2018 David Joffe

I'm not a fan of the various globals here; they're mostly from the very
oldest parts of the codebase, when I was still an inexperienced coder ...
but not necessarily at this stage worth investing time in refactoring,
unless we want to genericize this codebase more/better for other derived
games. [dj2018]

License: GNU GPL Version 2
*/
#ifndef _HERO_H_
#define _HERO_H_

//! Hero movement modes
enum
{
	MODE_NORMAL = 0,
	MODE_JUMPING
};
//! Hero movement mode
extern int hero_mode;
//! When hero is hurt, this is set to a positive "framecount" that
//! counts down. Can't get hurt again until it reaches 0.
extern int nHurtCounter;


// Fixme, these names have GOT to change! [should be in a class too - not globals - dj2016-10]
extern int x;			//!< Hero's absolute x position, in level block coordinates
extern int y;			//!< Hero's absolute y position, in level block coordinates
extern int x_small;		//!< x_small == 0 ? hero at x : hero at x + 8 pixels

// This should really just be permanently on I guess, not sure if there's any good reason to turn it off, unless we want to backtrack today's changes. [dj2017-06-24]
extern bool g_bSmoothVerticalMovementEnabled;
extern int y_offset;	//!< Pixel offset (e.g. [-15,15] relative to the hero's 'block unit' 'y' position. For smooth vertical movement. [Added dj2017-06]
// This g_nFalltime thing is to make hero fall initially slower
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
extern int g_nFalltime;

//! xo,yo = top-left corner of view for scrolling
extern int xo;

//! hero animation image index offset
extern int hero_picoffs;
//! hero direction, left==0, right==1
extern int hero_dir;

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


//! xo,yo = top-left corner of view for scrolling
extern int yo;
//! View offset by 8 pixels?
extern int xo_small;

extern int nSlowDownHeroWalkAnimationCounter;


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


//! Set the hero's position in level block coordinates
extern void relocate_hero( int xnew, int ynew );
//! Attempt to move the hero
extern int  move_hero(int xdiff, int ydiff, bool bChangeLookDirection=true);



#endif
