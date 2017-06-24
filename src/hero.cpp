/*
hero.cpp

Copyright (C) 2000-2017 David Joffe

License: GNU GPL Version 2
*/

#include "hero.h"
#include "game.h"
#include "thing.h" // For CreateDust only .. might move
#include "djtypes.h"


int hero_mode = MODE_NORMAL;
int x = 64, y = 50;      // x,y = hero's absolute positioning in level
int x_small=0, xo_small=0; // x_small == 0 ? hero at x : hero at x + 8 pixels
int xo = 60, yo = 45;    // xo,yo = top-left corner of view for scrolling
int hero_dir = 1;        // hero direction, left==0, right==1
int hero_picoffs = 0;    // hero animation image index offset

bool g_bSmoothVerticalMovementEnabled=true;
int y_offset = 0;//!< Pixel offset (e.g. [-15,15] relative to the hero's 'block unit' 'y' position. For smooth vertical movement. [Added dj2017-06]
int g_nFalltime=0;
// These at '8' correspond relatively closely to original DN1 behavior. Setting these to e.g. 1 or 2 etc. allow much smoother more refined vertical movement of hero, which might be useful in future. [dj2017-06]
const int nFALL_VERTICAL_PIXELS=8;
const int nJUMP_VERTICAL_PIXELS=8;

int nHurtCounter = 0;

// jumping
struct SJumpInfo
{
	int size;         // number of offsets
	const int * jump_diffs; // offsets (y axis)
};
const int g_aiJumpNormal[] = { -1, -1, -1,  0, 0, 0, 1, 1, 1 };
const int g_aiJumpBoots[]  = { -1, -1, -1, -1, 0, 0, 0, 1, 1, 1, 1 };

// NOTE: I've made these 1 smaller than they actually are, so that the last one is always
// just a "natural" falling .. which allows dust to be kicked up. There is still a bug
// (FIXME) whereby no dust can be created on the very last jump-fall-move - this hack
// just makes it less common, as you have to land one block higher from which you've jumped.
// Hackedy hackedy hack ..
const struct SJumpInfo jumpNormal = {  8, g_aiJumpNormal }; // "Normal" jump offsets
const struct SJumpInfo jumpBoots  = { 10, g_aiJumpBoots  }; // Jump offsets with boots
//struct SJumpInfo jumpNormal = {  9, g_aiJumpNormal }; // "Normal" jump offsets
//struct SJumpInfo jumpBoots  = { 11, g_aiJumpBoots  }; // Jump offsets with boots
const struct SJumpInfo * pJumpInfo; // Points to current jump info, normal or boots
int jump_pos = 0; // Offset into the "jump info" array of y-axis offsets

void HeroSetJumpMode(EJump eJump)
{
	switch (eJump)
	{
	case JUMP_NORMAL:     pJumpInfo = &jumpNormal; break;
	case JUMP_POWERBOOTS: pJumpInfo = &jumpBoots; break;
	}
}

void HeroStartJump()
{
	hero_mode = MODE_JUMPING;
	jump_pos = 0;
	y_offset = 0;
	
	djSoundPlay( g_iSounds[SOUND_JUMP] );
}

void HeroCancelJump()
{
	hero_mode = MODE_NORMAL;
	jump_pos = 0;
}

void HeroUpdateJump()
{
	int n=0;
	
	bool bDo = true;//Do 'full-block' movement, i.e. when smooth movement 'wraps' [dj2017-06-24]
	if (g_bSmoothVerticalMovementEnabled &&
		pJumpInfo->jump_diffs[jump_pos] < 0)
	{
		y_offset -= nJUMP_VERTICAL_PIXELS;
		
		bDo = false;
		if (y_offset<-15)
		{
			y_offset = 0;
			bDo = true;
		}
		//else
		{
			// Check if gonna hit head on solid as result of fine 'pixel' movement?
			bool bSolid = check_solid( x, y - 2 ) | check_solid( x + x_small, y - 2 );
			if (bSolid)
			{
				y_offset += nJUMP_VERTICAL_PIXELS;
				HeroCancelJump();
				hero_picoffs = 1;
				return;
			}
		}
	}
	
	if (bDo)
	{
		y_offset = 0;
		n = move_hero(0,pJumpInfo->jump_diffs[jump_pos]);
		if (n == 1) { //cancel jump, fell on block (x 1 ret?)
			HeroCancelJump();
			hero_picoffs = 1;
			// Kick up some dust ..
			AddThing(CreateDust(x, y));
			djSoundPlay( g_iSounds[SOUND_JUMP_LANDING] );
		}
		jump_pos++;
		if (jump_pos >= pJumpInfo->size)
		{
			hero_mode = MODE_NORMAL;
		}
	}
}

int nFrozenCount = -1;

void HeroUpdate()
{
	if (nFrozenCount>=0)
		nFrozenCount--;
}

void HeroFreeze(int n)
{
	nFrozenCount = n;
}

void HeroUnfreeze()
{
	nFrozenCount = -1;
}

bool HeroIsFrozen()
{
	return (nFrozenCount >= 0);
}

void HeroReset()
{
	HeroUnfreeze();
	hero_mode = MODE_NORMAL; // Standing around
	HeroCancelJump(); // Not currently jumping
	nHurtCounter = 0; // Not currently hurting
}



void relocate_hero( int xnew, int ynew )
{
	x = xnew;
	y = ynew;
	x_small = 0;
	y_offset = 0;
	xo = MAX( x - int(VIEW_WIDTH / 2), 0 );
	yo = MAX( y - 6, 0 );
	xo = MIN( xo, 128 - VIEW_WIDTH );
	yo = MIN( yo, 100 - VIEW_HEIGHT );
}


//returns 0 if could move
//returns 1 if couldnt move (y)
//returns 2 if coundnt move (x)
int move_hero(int xdiff, int ydiff, bool bChangeLookDirection)
{
	int n,ret;
	bool bsolid;
	ret = 0;
	
	// Don't do any moving if you're about to teleport or something
	if (HeroIsFrozen())
		return 1;
	
	if (xdiff)
	{
		//simple direction reverse (unless bChangeLookDirection is false, which means we're probably on a conveyor or something)
		if (bChangeLookDirection && (((hero_dir == 1) & (xdiff == -1)) | ((hero_dir == 0) & (xdiff == 1))))
		{
			hero_picoffs = 0;
			hero_dir = (xdiff+1)/2;  //( -1 --> 0 : 1 --> 1 )
			return 0;
		}
		//facing right, must also have pressed right
		//if (hero_dir)
		if (xdiff>0)
		{
			bsolid = false;
			if ( x_small == 0 )
			{
				bsolid = check_solid( x + 1, y ) | check_solid( x + 1, y - 1 );
				// Prevent being able to walk into floors left/right while falling [dj2017-06]
				if (g_bSmoothVerticalMovementEnabled)
				{
					if (y_offset<0)
						bsolid |= check_solid( x + 1, y-1 ) | check_solid( x + 1, y - 2 );
					else if (y_offset>0)
						bsolid |= check_solid( x + 1, y+1 ) | check_solid( x + 1, y );
				}
			}
			ret = 2;
			if (  (!(bsolid)) && ( (x_small) | (!(bsolid)) )  ) {
				ret = 0;
				x += (xdiff * x_small);
				x_small ^= 1;
				
				if (((x-xo)==(VIEW_WIDTH - 5)) & (x_small)) {
					xo_small = 1;
				}
				if (((x-xo)==(VIEW_WIDTH - 4) )) {
					xo++;
					xo_small = 0;
				}
				if ( (xo + xo_small) > 128 - VIEW_WIDTH )
				{
					xo = 128 - VIEW_WIDTH;
					xo_small = 0;
				}
				
			}
		}
		else//Try move left
		{ //facing left, must have pressed left
			bsolid = false;
			if (!(x_small))
			{
				bsolid = check_solid( x - 1, y ) | check_solid( x - 1, y - 1 );
				// Prevent being able to walk into floors left/right while falling [dj2017-06]
				if (g_bSmoothVerticalMovementEnabled)
				{
					if (y_offset<0)
						bsolid |= check_solid( x - 1, y-1 ) | check_solid( x - 1, y - 2 );
					else if (y_offset>0)
						bsolid |= check_solid( x - 1, y+1 ) | check_solid( x - 1, y );
				}
			}
			ret = 2;
			if ((!(bsolid)) & ((x_small) | (!(bsolid))) ) {
				ret = 0;
				x_small ^= 1;
				x += (xdiff * x_small);
				if (((x-xo)==4) & (!(x_small))) {
					xo_small = 0;
				}
				if (((x-xo)==3) & (x_small)) {
					xo--;
					xo_small = 1;
					if (xo < 0)
					{
						xo = 0;
						xo_small = 0;
					}
				}
			}
			
		}
	}
	
	if (ydiff)
	{
		// Jumping
		if (ydiff == 1)
			n = 1;      // falling, check below us
		else
			n = -2;     // going up, check above hero's head
		
		// also stop hero falling if at bottom of screen
		bsolid = check_solid( x, y + n ) | check_solid( x + x_small, y + n );
		
		ret = 1;
		if (!bsolid) {


			bool bDo = true;//Do 'full-block' movement, i.e. when smooth movement 'wraps' [dj2017-06-24]
			
			if (g_bSmoothVerticalMovementEnabled &&
				ydiff>0)//Falling?
			{
				bDo=false;
				// This g_nFalltime thing is to make hero fall initially slower
				// then faster (full block at a time).
				// Apart from looking/feeling slightly more natural, it also 'masks'
				// a current issue where you get a jerky effect that looks like hero
				// bouncing up and down when falling off bottom of view, as the view
				// code scrolls vertically always in increments of 16 pixels, whereas
				// if hero falls at 8 pixels off bottom then relative vertical offset
				// of hero on screen toggles 8 pixels each consecutive frame. With
				// this falltime thing, by the time he is falling off the bottom,
				// he is falling 16 pixels, and the view scrolls 16 pixels too.
				// It's a bit fiddly but anyway, we have to do fine tweaks like this.
				// See also liveedu.tv video 2017-06-24 [dj2017-06-24]
				y_offset += ( g_nFalltime>=6 ? 16 : nFALL_VERTICAL_PIXELS );
				ret = 0;//Return 'busy falling'
				if (y_offset>=15)
				{
					bDo = true;
					y_offset = 0;
				}
			}

			if (bDo)
			{
				ret = 0;
				y += ydiff;
				if (y-yo<4) yo--;
				if (y-yo>7) yo++;
				if ( yo < 0 )
					yo = 0;
				if ( yo > 100 - 10 )
					yo = 100 - 10;
			}
		}
	}
	if ((hero_mode != MODE_JUMPING) && (ret == 0)) {
		if (nSlowDownHeroWalkAnimationCounter == 0)
			hero_picoffs++;
		if (hero_picoffs>3) hero_picoffs = 0;
	}
	
	return(ret);
}

