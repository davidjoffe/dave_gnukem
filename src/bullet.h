/*!
\file    bullets.h
\brief   Bullet class
\author  David Joffe

Copyright (C) 2002-2018 David Joffe

License: GNU GPL Version 2
*/
#ifndef _BULLET_H_
#define _BULLET_H_

#define BULLET_WIDTH 16
#define BULLET_HEIGHT 8

class CBullet
{
public:
	enum EType
	{
		BULLET_HERO,
		BULLET_MONSTER
	};
	CBullet();

	void Tick();
	void Draw();

	int x; // World (pixel) X coordinate
	int y; // World (pixel) Y coordinate
	int dx;
	int dy;
	bool bDrawnOnce;
	int nAnim;
	EType eType;
};

#endif
