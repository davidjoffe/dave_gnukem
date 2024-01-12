//Copyright (C) 1995-2024 David Joffe
//

//dj2022-11 adding datadir.cpp to corresponding to old datadir.h to extend this functionality
//dj2022-11 Adding new helper djDataDir() (and corresponding set) to make this more flexible (e.g. make it easier either for porters to override, or add functionality like checking fallback folders - e.g. check /usr/share but if not found e.g. check relative say to the executable path)

#include "config.h"
#include "datadir.h"

#include <string>

// [dj2022-11] Slightly experimental but the idea here is to set the default g_sDataDir based on the DATA_DIR (which e.g. comes from Makefile) but this allows dynamic overriding (or selecting fallbacks)
// NOTE we don't currently 'expose' this g_sDataDir (for now?) as it's sort of the 'backend' - use the get/set functions below
std::string g_sDataDir = DATA_DIR;

const char* djDataDir()
{
	//if (g_sDataDir.empty())
	//	g_sDataDir = DATA_DIR;

	return g_sDataDir.c_str();
}

const std::string& djDataDirS()
{
	return g_sDataDir;
}

void djSetDataDir(const char* szDataDir)
{
	if (szDataDir==nullptr)
		g_sDataDir.clear();
	else
		g_sDataDir = szDataDir;

	if (!g_sDataDir.empty() && g_sDataDir.back() != '/')
		g_sDataDir += '/'; // add trailing slash if not present?
}

void djDATAPATH(std::string& sStr, const char* szPathToAppend)
{
	sStr = g_sDataDir;
	// todo append trailing slash if not present
	if (szPathToAppend != nullptr)
		sStr += szPathToAppend;
}

std::string djDATAPATH(const char* szPathToAppend)
{
	std::string sStr = g_sDataDir;
	// todo append trailing slash if not present
	if (szPathToAppend != nullptr)
		sStr += szPathToAppend;
	return sStr;
}

std::string djDATAPATH(const std::string& sPathToAppend)
{
	return g_sDataDir + sPathToAppend;
}
