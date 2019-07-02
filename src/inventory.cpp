/*
inventory.cpp

Copyright (C) 2001-2019 David Joffe
*/

#include "config.h"
#include "inventory.h"
#include "mission.h"
#include "graph.h"
#include "djlog.h"
#include "thing.h"

struct SInvItem
{
	CThing *pThing;
	// Items picked up during current level must go if we die, so items are
	// marked "non-persisent" until hero completes level
	bool bPersistent;
};

#include <vector>
using namespace std;
vector<SInvItem> g_apInventory;


// X,Y location on screen to draw inventory (in pixels)
#define INVENTORY_X (28*8+8)
#define INVENTORY_Y (17*8)

void InvMakeAllPersistent()
{
	for ( int i=0; i<(int)g_apInventory.size(); i++ )
	{
		g_apInventory[i].bPersistent = true;
	}
}

void InvClear()
{
	for ( int i=0; i<(int)g_apInventory.size(); i++ )
	{
		// If non-persistent (picked up during this level), or if thing wants to
		// always be non-persistent, then delete it.
		if (!g_apInventory[i].bPersistent || g_apInventory[i].pThing->OnInventoryClear())
		{
			djDEL(g_apInventory[i].pThing);
			g_apInventory.erase(g_apInventory.begin() + i);
			i--;
		}
	}
}

/*
rtfb:
FIXME: What the hell does this function do? If it releases
the pThings it was fed, then the other routines, that may try to
release thigns may crash.
*/

void InvEmpty()
{
	for ( int i=0; i<(int)g_apInventory.size(); i++ )
	{
		djDEL(g_apInventory[i].pThing);
	}
	g_apInventory.clear();
}

bool InvAdd(CThing *pThing)
{
	if (g_apInventory.size()>=INV_MAXSIZE)
		return false;
	SInvItem InvItem;
	InvItem.pThing = pThing;
	InvItem.bPersistent = false;
	g_apInventory.push_back(InvItem);
	InvDraw(); // redraw
	return true;
}

void InvDraw()
{
	if (g_bLargeViewport || g_bBigViewportMode)
	{
		for ( unsigned int i=0; i<g_apInventory.size(); ++i)
		{
			// Bottom right viewport overlay
			int nX = (g_nViewOffsetX+(VIEW_WIDTH*BLOCKW)) - (i+1)*BLOCKW;
			int nY = (g_nViewOffsetY+(VIEW_HEIGHT*BLOCKH)) - BLOCKH;
			// Background block
			DRAW_SPRITE16(pVisView, 0, 1, nX, nY);
			CThing *pThing = g_apInventory[i].pThing;
			DRAW_SPRITE16A(pVisView, pThing->m_a, pThing->m_b, nX, nY);
		}
	}
	else
	{
		for ( int i=0; i<5; i++)
		{
			// Background blocks
			DRAW_SPRITE16(pVisBack, 0, 1, INVENTORY_X + i*16, INVENTORY_Y);
			DRAW_SPRITE16(pVisBack, 0, 1, INVENTORY_X + i*16, INVENTORY_Y+16);
			// Inventory items 0-4
			if (i<(int)g_apInventory.size())
			{
				CThing *pThing = g_apInventory[i].pThing;
				DRAW_SPRITE16A(pVisBack, pThing->m_a, pThing->m_b, INVENTORY_X + i*16, INVENTORY_Y);
			}
			// Inventory items 5-9
			if (i+5<(int)g_apInventory.size())
			{
				CThing *pThing = g_apInventory[i+5].pThing;
				DRAW_SPRITE16A(pVisBack, pThing->m_a, pThing->m_b, INVENTORY_X + i*16, INVENTORY_Y+16);
			}
		}
	}
}

int InvGetSize()
{
	return g_apInventory.size();
}

CThing *InvGetItem(int n)
{
	return g_apInventory[n].pThing;
}

void InvRemove(CThing *pThing)
{
	for ( int i=0; i<(int)g_apInventory.size(); ++i )
	{
		if (pThing==g_apInventory[i].pThing)
		{
			g_apInventory.erase(g_apInventory.begin() + i);
			InvDraw();
			return;
		}
	}
}

void InvSave(FILE *pOut)
{
	unsigned int i;
	// Save number of *PERSISTENT* items
	int nNumPersistentItems = 0;
	for ( i=0; i<g_apInventory.size(); i++ )
	{
		if (g_apInventory[i].bPersistent)
			++nNumPersistentItems;
	}
	fprintf(pOut, "%d\n", nNumPersistentItems);
	for ( i=0; i<g_apInventory.size(); i++ )
	{
		// Save only persistent things to file, otherwise we'd be saving things
		// we picked up during the currently being saved level.
		if (g_apInventory[i].bPersistent)
		{
			CThing *pItem = InvGetItem(i);
			fprintf(pOut, "%d %d %d\n", pItem->GetTypeID(), pItem->m_a, pItem->m_b);
		}
	}
}

void InvLoad(FILE *pIn)
{
	int i;
	// Clear existing
	InvEmpty();
	// Read number of items
	int nItems=0;
	fscanf(pIn, "%d\n", &nItems);
	djMSG("LOADGAME: InvLoad: %d items\n", nItems);
	// Use object factory to load items
	for ( i=0; i<nItems; i++ )
	{
		int nTypeID=-1, a=0, b=0;
		fscanf(pIn, "%d %d %d\n", &nTypeID, &a, &b);
		CThing *pThing = g_ThingFactory.Allocate(nTypeID);
		if (pThing==NULL)
		{
			djMSG("ERROR: InvLoad: Unable to allocate object with type %d (%d,%d)\n", nTypeID, a, b);
		}
		else
		{
			pThing->SetSprite(a, b);
			pThing->Initialize(a, b);
			pThing->OnInventoryLoad();
			InvAdd(pThing);
		}
	}
	// Make all loaded items "persistent"
	InvMakeAllPersistent();
}
