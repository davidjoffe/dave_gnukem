/*--------------------------------------------------------------------------*/
/* mixins.cpp */
/* David Joffe 1999/02 */
/* "mixins"; not intended to be classes on their own, just contain */
/* standalone characteristics that others can multiple-inherit from to get */
/* that behaviour */
/*
mixins.cpp

Copyright (C) 1999-2018 David Joffe

License: GNU GPL Version 2
*/

/*--------------------------------------------------------------------------*/
#include "mixins.h"
#include "djstring.h"
#include "djtypes.h"
#include <stdlib.h> // For NULL

CNamed::CNamed()
: m_szName(NULL)
{
}

CNamed::~CNamed()
{
	djDELV(m_szName);
}

void CNamed::SetName( const char *szName )
{
	djDELV(m_szName);
	m_szName = djStrDup(szName);
}
