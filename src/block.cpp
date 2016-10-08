/*********
block.cpp

Copyright (C) 2000-2001 David Joffe

License: GNU GPL Version 2 (*not* "later versions")
*********/
#include "block.h"

char * block_type_names[TYPE_LASTONE+1] =
{
   " 0 Nothing",
   " 1 ",
   " 2 ",
   " 3 Softsolid",
   " 4 ",
   " 5 ",
   " 6 Dust",
   " 7 Lift",
   " 8 Teleporter",
   " 9 ",
   "10 ",
   "11 Firepower",
   "12 ",
   "13 Conveyor",
   "14 Letter",
   "15 Box",
   "16 ",
   "17 Pickup",
   "18 ",
   "19 PowerBoots",
   "20 Coke",
   "21 ",
   "22 ",
   "23 Dynamite",
   "24 FlameThrow",
   "25 Key",
   "26 Door",
   "27 DoorActivator",
   "28 Fan",
   "29 AccessCard",
   "30 ",
   "31 Exit",
   "32 HeroStart",
   "33 ",
   "34 ",
   "35 Acme",
   "36 Balloon",
   "37 Spike",
   "38 CrawlerMonster",
   "39 Banana",
   "40 Camera",
   "41 SoftBlock",
   "42 TestBlock",
   "43 SpikeBall",
   "44 Robot",
   "45 LastOne"
};
