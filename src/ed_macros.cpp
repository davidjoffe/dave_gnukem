// Level editor

#include "config.h"
#include "djfile.h"
#include "djstring.h"
#include "ed_macros.h"
#include "ed_common.h"
#include "ed_lvled.h"
#include "sys_error.h"
#include "graph.h"//pVisMain
#include <vector>

#include <stdio.h>


// defined in ed_lvled.cpp
void SetLevel( int x, int y, int a, int b, bool bforeground );



int g_iAssignedMacros[9]; // Keys 1 to 9 to place assigned macros
std::vector<SMacro*> g_apMacros;



// Macro stuff
bool LoadMacros()
{
	FILE *fin=NULL;
	char buf[2048]={0};
	if (NULL == (fin = djFile::dj_fopen(djDATAPATHc("editor/macros.txt"), "r")))
	{
		printf("Warning: Editor: Failed to load macros (this is not critical but may suggest a datapath config problem)\n");
		return false;
	}

	#define djREADLINE() buf[0]=0; if ((fgets(buf, sizeof(buf), fin) == NULL) && ferror(fin)) goto error; djStripCRLF(buf)

	SMacro* pMacro=NULL;
	//djREADLINE();
	//dj2022-11 hmm can't recall why this "$" strcmp should be there .. maybe it should be removed .. why should a dollar suddenly indicate eof? can't remember
	while (strcmp(buf, "$") && !feof(fin))
	{
		djREADLINE();
		// Skip empty lines (or possible read of last line if empty string)
		if (buf[0] == 0) continue;

		pMacro = new SMacro;

		pMacro->szName = djStrDeepCopy( buf );
		if (pMacro->szName[strlen(pMacro->szName)-1] == '\r')
			pMacro->szName[strlen(pMacro->szName)-1] = 0;

		//djREADLINE();
		while (strcmp(buf, "~") && strcmp(buf, "~\r") && !feof(fin))
		{
			djREADLINE();
			if (buf[0]!='~' && buf[0]!=0)
			{
				int ix, iy, a, b;
				if (dj_sscanf(buf, "%d %d %d %d", &ix, &iy, &a, &b) <= 0)
					goto error;
				pMacro->m_aiBlocks[0].push_back(ix);
				pMacro->m_aiBlocks[1].push_back(iy);
				pMacro->m_aiBlocks[2].push_back(a);
				pMacro->m_aiBlocks[3].push_back(b);

				//djREADLINE();
			}
		}
		g_apMacros.push_back(pMacro);
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

error:
	//djLOGSTR("EDITOR: ERROR loading macros\n");
	fclose(fin);
	return false;
}


bool DeleteMacros()
{
	std::vector<SMacro*>::iterator	i;

	for ( i=g_apMacros.begin();
		i != g_apMacros.end(); )
	{
		delete *i;
		i = g_apMacros.erase ( i );
	}

	return true;
}

//dj2019-08 adding underscore prefix to x,y, the only (bad) reason I'm doing that is due to the ugly global x/y for hero position, so as to avoid ambiguity .. can remove later if that ever gets re-done in a better way.
void PlaceMacro(int _x, int _y, int iMacroIndex)
{
	if ( (_x < 0) || (_y < 0) || (_x >= LEVEL_WIDTH) || (_y >= LEVEL_HEIGHT) )
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
			_x+pMacro->m_aiBlocks[0][i],
			_y+pMacro->m_aiBlocks[1][i],
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
		snprintf( buf, sizeof(buf), "%d.", i+1 );
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

