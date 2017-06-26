
#include "djstring.h"
#include "ed_macros.h"
#include "ed_common.h"
#include "ed_lvled.h"
#include "sys_error.h"
#include "graph.h"//pVisMain
#include <vector>
using namespace std;
#include <stdio.h>


// defined in ed_lvled.cpp
void SetLevel( int x, int y, int a, int b, bool bforeground );



int g_iAssignedMacros[9]; // Keys 1 to 9 to place assigned macros
vector<SMacro*> g_apMacros;



// Macro stuff
bool LoadMacros()
{
	FILE *fin=NULL;
	char buf[1024]={0};
	if (NULL == (fin = fopen( "data/editor/macros.txt", "r" )))
		return false;

	SMacro* pMacro=NULL;
	fgets( buf, sizeof(buf), fin );
	buf[strlen(buf)-1] = 0; // strip trailing newline
	while (strcmp(buf, "$") && !feof(fin))
	{
		pMacro = new SMacro;

		pMacro->szName = djStrDeepCopy( buf );
		if (pMacro->szName[strlen(pMacro->szName)-1] == '\r')
			pMacro->szName[strlen(pMacro->szName)-1] = 0;

		fgets( buf, sizeof(buf), fin );
		buf[strlen(buf)-1] = 0; // strip trailing newline
		while (strcmp(buf, "~") && strcmp(buf, "~\r") && !feof(fin))
		{
			int ix, iy, a, b;
			sscanf( buf, "%d %d %d %d", &ix, &iy, &a, &b );
			pMacro->m_aiBlocks[0].push_back(ix);
			pMacro->m_aiBlocks[1].push_back(iy);
			pMacro->m_aiBlocks[2].push_back(a);
			pMacro->m_aiBlocks[3].push_back(b);

			fgets( buf, sizeof(buf), fin );
			buf[strlen(buf)-1] = 0; // strip trailing newline
		}

		g_apMacros.push_back(pMacro);

		fgets( buf, sizeof(buf), fin );
		buf[strlen(buf)-1] = 0; // strip trailing newline
	}

	int i;
	for ( i=0; i<9; i++ )
	{
		g_iAssignedMacros[i] = (i<(int)g_apMacros.size() ? i : -1);
	}

	for ( i=0; i<(int)g_apMacros.size(); i++ )
	{
		SMacro *pMac = g_apMacros[i];
		printf( "%s\n", pMac->szName==NULL ? "" : pMac->szName );
		for ( int j=0; j<(int)pMac->m_aiBlocks[0].size(); j++ )
		{
			printf( "%d %d %d %d\n",
				pMac->m_aiBlocks[0][j],
				pMac->m_aiBlocks[1][j],
				pMac->m_aiBlocks[2][j],
				pMac->m_aiBlocks[3][j] );
		}
	}

	fclose(fin);

	return true;
}


bool DeleteMacros()
{
	vector<SMacro*>::iterator	i;

	for ( i=g_apMacros.begin();
		i != g_apMacros.end(); )
	{
		delete *i;
		i = g_apMacros.erase ( i );
	}

	return true;
}

void PlaceMacro(int x, int y, int iMacroIndex)
{
	x = x;				// shut up
	y = y;				// shut up
	iMacroIndex = iMacroIndex;	// shut up

	if ( (x < 0) || (y < 0) || (x >= 128) || (y >= 100) )
		return;

	int iMacro = g_iAssignedMacros[iMacroIndex];
	if (iMacro<0) return;
	if (iMacro>=(int)g_apMacros.size()) return;

	SMacro *pMacro = g_apMacros[iMacro];
	if (pMacro==NULL)
		return;

	for ( int i=0; i<(int)pMacro->m_aiBlocks[0].size(); i++ )
	{
		SetLevel(
			x+pMacro->m_aiBlocks[0][i],
			y+pMacro->m_aiBlocks[1][i],
			pMacro->m_aiBlocks[2][i],
			pMacro->m_aiBlocks[3][i],
			LVLED_GetLevelFore () );
		SetDocumentDirty();
	}
}

void ShowMacros()
{
	// [dj2016-10] Changed this so that macros list is displayed relative to bottom of screen
	unsigned int y = pVisMain->height - 8*10;
	for ( int i=0; i<9; i++ )
	{
		int iMacro = g_iAssignedMacros[i];
		char buf[1024]={0};
		sprintf( buf, "%d.", i+1 );
		ED_DrawStringClear( MACROS_X, y, "Macros:" );
		ED_DrawString( MACROS_X, y, "Macros:" );

		ED_DrawStringClear( MACROS_X, y+i*8+8, buf );
		ED_DrawString( MACROS_X, y+i*8+8, buf );
		if (iMacro==-1)
		{
			ED_DrawStringClear( MACROS_X+16, y+i*8+8, "none" );
			ED_DrawString( MACROS_X+16, y+i*8+8, "none" );
		}
		else
		{
			SMacro *pMac = g_apMacros[iMacro];
			ED_DrawStringClear( MACROS_X+16, y+i*8+8, pMac->szName );
			ED_DrawString( MACROS_X+16, y+i*8+8, pMac->szName  );
		}
	}
}

