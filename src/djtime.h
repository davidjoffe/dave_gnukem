/*!
\file    djtime.h
\brief   Some timing functions
\author  David Joffe

Copyright (C) 1999-2018 David Joffe
*/
#ifndef _DJTIME_H_
#define _DJTIME_H_

#include <cstdint>//uint64_t

//! Initialize the time system
extern void  djTimeInit();

//! Shut down the time system
extern void  djTimeDone();

//! return time of day in seconds
//! return time since djTime started in seconds
extern float djTimeGetTime();

//! dj2022-11 new helper return ticks in milliseconds since start of app (depending on SDL version, if at least 2.0.18, should solve 49-day wraparound on Windows)
extern uint64_t djTimeGetTicks64();

#endif
