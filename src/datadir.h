/*!
\file    datadir.h
\brief   Data directory
\author  David Joffe

Copyright (C) 1999-2022 David Joffe
*/
//dj2022-11 adding datadir.cpp to corresponding to old datadir.h to extend this functionality
//dj2022-11 Adding new helper djDataDir() (and corresponding set) to make this more flexible (e.g. make it easier either for porters to override, or add functionality like checking fallback folders - e.g. check /usr/share but if not found e.g. check relative say to the executable path)

// TODO maybe make this an actual potential list of paths, so on initialization it can e.g. go through several of specified options and select one that exists?
//  e.g. "/usr/local/share/blahblah;data/;gnukem_data/"
// Then one could still pass the above in a Makefile or whatever .. also we could make it passable by commandline maybe e.g. "--datadir=XXXX"
//  also should have some way to maybe say 'check relative to executable path'?
// e.g. "/usr/local/share/blahblah;data/;gnukem_data/;{execpath}data/;"
// ALSO for Mac we need it to be able to specify e.g. things like:
// "/Applications/DaveGnukem.app/Contents/Resources/data" etc.
// but if running dev mode should look locally e.g. "data/" or whatever

// also [low] in future mabye want mlutiple datapaths? eg specify different datapath for eg music or something? not sure. [low prio definitely not needed now]

#include <string>

#ifndef _dj_DATADIR_H_
#define _dj_DATADIR_H_

//! Data directory [dj2017-08 should this move to config.h?]
#ifndef DATA_DIR
#define DATA_DIR "data/"
#endif

//[dj2022-11]
extern const char* djDataDir();
// [I use a different name to above as I we don't want function overloading with the same signature but differing return type especially when the return type is potentially ambiguously matchable]
extern const std::string& djDataDirS();

//[dj2022-11]
extern void djSetDataDir(const char* szDataDir);

//dj2022-11 datadir path append helpers
extern void djDATAPATH(std::string& sStr, const char* szPathToAppend);
extern std::string djDATAPATH(const char* szPathToAppend);
extern std::string djDATAPATH(const std::string& sPathToAppend);

#define djDATAPATHc(sPathToAppend) djDATAPATH(sPathToAppend).c_str()
#define djDATAPATHs(sPathToAppend) djDATAPATH(sPathToAppend)

#endif
