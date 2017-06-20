
#ifndef __MACROS_H__
#define __MACROS_H__

#include <vector>
#include <string.h>

// Macro stuff
struct SMacro
{
	SMacro():szName(NULL){};
	char		*szName;
	std::vector<int> m_aiBlocks[4];
};

extern int g_iAssignedMacros[9]; // Keys 1 to 9 to place assigned macros
extern std::vector<SMacro*> g_apMacros;
extern bool		blevelfore;		// in ed.cpp

//void level_set( int x, int y, int a, int b, bool bforeground );
//void DrawString( int x, int y, char *szStr );		// in ed.cpp
//void DrawStringClear( int x, int y, char *szStr );	// in ed.cpp

bool LoadMacros();
bool DeleteMacros();
void PlaceMacro(int x, int y, int iMacroIndex);
void ShowMacros();

#define MACROS_X 270


#endif		// #ifndef __MACROS_H__

