/*!
\file    datadir.h
\brief   Data directory
\author  David Joffe

Copyright (C) 1999-2022 David Joffe
*/
//dj2022-11 adding datadir.cpp to corresponding to old datadir.h to extend this functionality
//dj2022-11 Adding new helper djDataDir() (and corresponding set) to make this more flexible (e.g. make it easier either for porters to override, or add functionality like checking fallback folders - e.g. check /usr/share but if not found e.g. check relative say to the executable path)

#ifndef _dj_DATADIR_H_
#define _dj_DATADIR_H_

//! Data directory [dj2017-08 should this move to config.h?]
#ifndef DATA_DIR
#define DATA_DIR "data/"
#endif

//[dj2022-11]
extern const char* djDataDir();

//[dj2022-11]
extern void djSetDataDir(const char* szDataDir);

#endif
