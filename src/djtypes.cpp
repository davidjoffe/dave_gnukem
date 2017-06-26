/*
djtypes.cpp

Copyright (C) 2002-2017 David Joffe

License: GNU GPL Version 2
*/

#include "djtypes.h"
#include <string.h>

void djStripCRLF(char *buf)
{
	// While last character in string is either CR or LF, remove.
	while (strlen(buf)>=1 && ((buf[strlen(buf)-1]=='\r') || (buf[strlen(buf)-1]=='\n')))
		buf[strlen(buf)-1] = '\0';
}
