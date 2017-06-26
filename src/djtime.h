/*!
\file    djtime.h
\brief   Some timing functions
\author  David Joffe

Copyright (C) 1999-2017 David Joffe

License: GNU GPL Version 2
*/
#ifndef _DJTIME_H_
#define _DJTIME_H_

//! Initialize the time system
extern void  djTimeInit();

//! Shut down the time system
extern void  djTimeDone();

//! return time of day in seconds
//! return time since djTime started in seconds
extern float djTimeGetTime();

#endif

