/*!
\file    hero.h
\brief   Hero-related stuff
\author  David Joffe

Copyright (C) 2001 David Joffe

License: GNU GPL Version 2 (*not* "later versions")
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
//! xo,yo = top-left corner of view for scrolling
extern int xo;

//! hero animation image index offset
extern int hero_picoffs;
//! hero direction, left==0, right==1
extern int hero_dir;




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
