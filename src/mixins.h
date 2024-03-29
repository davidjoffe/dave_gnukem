/*!
\file    mixins.h
\brief   Some simple "mixin" helper classes.
\author  David Joffe

Copyright (C) 2000-2024 David Joffe
*/
/*--------------------------------------------------------------------------*/
#ifndef _MIXINS_H_
#define _MIXINS_H_

class CNamed
{
public:
	CNamed();
	virtual ~CNamed();

	void SetName( const char *szName );
	const char *GetName() const { return m_szName; }

protected:
	char * m_szName;
};

#endif
