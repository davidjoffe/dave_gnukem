/*********
block.cpp

Copyright (C) 2000-2017 David Joffe

License: GNU GPL Version 2
*********/
#include "block.h"

const char * block_type_names[TYPE_LASTONE+1] =
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
   " 9 Rocket",
   "10 ",
   "11 Firepower",
   "12 Crumbling Floor",
   "13 Conveyor",
   "14 Letter",
   "15 Box",
   "16 ",
   "17 Pickup",
   "18 ",
   "19 PowerBoots",
   "20 Soda Can",
   "21 Full Health [Molecule]",
   "22 ",
   "23 Dynamite",
   "24 FlameThrow",
   "25 Key",
   "26 Door",
   "27 DoorActivator",
   "28 Fan",
   "29 AccessCard",
   "30 Antivirus",
   "31 Exit",
   "32 HeroStart",
   "33 Master Computer",
   "34 ",
   "35 Acme",
   "36 Balloon",
   "37 Spike",
   "38 CrawlerMonster",
   "39 Banana",
   "40 Camera",
   "41 SoftBlock",
   "42 ",
   "43 SpikeBall",
   "44 Robot",
   "45 Flying Robot",
   "46 Rabbit(ish)",
   "47 HighVoltage barrier",
   "48 Cannon-ish Thing",
   "49 Jumping Monster",
   "50 LastOne(UnusedNextID)"
};
