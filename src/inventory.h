/*!
\file    inventory.h
\brief   The hero's inventory
\author  David Joffe

Copyright (C) 2001 David Joffe

License: GNU GPL Version 2 (*not* "later versions")
*/

#include <stdio.h>

///\name Inventory functions
//@{

class CThing;

#define INV_MAXSIZE 10

//! Clear the inventory out at beginning of level. Some objects
//! may remain in inventory, such as power boots.
extern void InvClear();
//! When we advance to next level, all stored objects become "persisent",
//! i.e. we can't lose them just by dying.
extern void InvMakeAllPersistent();
//! Empty the inventory out completely, deleting all objects.
extern void InvEmpty();
//! Add an item to the inventory. Return false if failed (e.g. if inventory full)
extern bool InvAdd(CThing *pThing);
//! Draw the inventory
extern void InvDraw();
//! Get number of items in inventory
extern int InvGetSize();
//! Get item at index n
extern CThing *InvGetItem(int n);
//! Remove (and delete) an item from the inventory
extern void InvRemove(CThing *pThing);

//! Save inventory to file for save-game (only saves items marked as persistent)
extern void InvSave(FILE *pOut);
//! Load game: load inventory
extern void InvLoad(FILE *pIn);


//@}
