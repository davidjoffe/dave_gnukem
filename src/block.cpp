/*********
block.cpp

Copyright (C) 2000-2024 David Joffe
*********/
#include "block.h"

const char * block_type_names[TYPE_LASTONE+1] =
{
    "Nothing",
    "Generic sprite/block",
    "(2)",
    "SoftSolid",
    "(4)",
    "Water",
    "Dust",
    "Lift",
    "Teleporter",
    "Rocket",
    "(10)",
    "Firepower",
    "Crumbling Floor",
    "Conveyor",
    "Letter",
    "Box",
    "(16)",
    "Pickup",
    "(18)",
    "PowerBoots",
    "Soda Can",
    "Full Health [Molecule]",
    "(22)",
    "Dynamite",
    "FlameThrow",
    "Key",
    "Door",
    "DoorActivator",
    "Fan",
    "AccessCard",
    "Antivirus",
    "Exit",
    "HeroStart",
    "Main Computer",
    "(34)",
    "Acme",
    "Balloon",
    "Spike",
    "CrawlerMonster",
    "Banana",
    "Camera",
    "SoftBlock",
    "(42)",
    "SpikeBall",
    "Robot",
    "Flying Robot",
    "Rabbit(ish)",
    "HighVoltage barrier",
    "Cannon-ish Thing",
    "Jumping Monster",
    "Dr Proton",
    "LastOne(UnusedNextID)"//51
};

std::string GetBlockTypeName(EBlockType eType)
{
	if ((int)eType < 0) return "";
	if ((int)eType >= TYPE_LASTONE+1) return "";

	return block_type_names[ eType ];
}
