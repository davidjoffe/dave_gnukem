/*
djtypes.cpp

Copyright (C) 2002-2018 David Joffe

*/

#include "config.h"//hmm should the 'gamelib' parts have a 'dependency' up to the main "config.h"? not sure? or their own config.h maybe.
#include "djtypes.h"
#include <string.h>

void djStripCRLF(char *buf)
{
	// While last character in string is either CR or LF, remove.
	while (strlen(buf)>=1 && ((buf[strlen(buf)-1]=='\r') || (buf[strlen(buf)-1]=='\n')))
		buf[strlen(buf)-1] = '\0';
}
