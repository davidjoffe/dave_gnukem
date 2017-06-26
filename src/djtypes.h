/*!
\file    djtypes.h
\brief   Basic types and macros
\author  David Joffe

Copyright (C) 1998-2017 David Joffe

License: GNU GPL Version 2
*/

#ifndef _DJTYPES_H_
#define _DJTYPES_H_

/*!
\class djColor
\nosubgrouping

Basic color type. Consists of four single-byte members for red, green, blue and
alpha (values ranging each from 0 to 255).
*/
class djColor
{
public:
	djColor() : r(0), g(0), b(0), a(255) { }
	djColor(unsigned char ir, unsigned char ig, unsigned char ib)
		: r(ir), g(ig), b(ib), a(255) { }
	djColor(unsigned char ir, unsigned char ig, unsigned char ib, unsigned char ia)
		: r(ir), g(ig), b(ib), a(ia) { }

	// [dj2016-10] These were 'unsigned short' which is [now] 16-bit, I'm not 100% sure if these were somewhere along the way of the code history 'intended'
	// to be 16-bit, or if maybe when I started back in the 90s a short was 8-bit or something, I don't know (it's possible as early dev target for this game was 16-bit);
	// changing this to 'byte' to get rid of some compiler warnings and to make it consistent with the class documentation (which says 'byte' for RGBA etc.) - have
	// checked through all uses of djColor and didn't immediately see anything that should break.

	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};


//! Make 16 bit integer from low and high 8-bit components
#define MAKEINT16(lo,hi) ((lo) | ((hi)<<8))

/* Deprecating, for sake of consistency, use only dj*
#ifndef DEL
//! Helper for "delete" operator
#define DEL(x) if (x) { delete (x); x = NULL; }
#endif

#ifndef DELV
//! Helper for "delete[]" operator
#define DELV(x) if (x) { delete[] (x); x = NULL; }
#endif
*/

//! Helper for "delete" operator
#define djDEL(x) if (x) { delete (x); x = NULL; }

//! Helper for "delete[]" operator
#define djDELV(x) if (x) { delete[] (x); x = NULL; }

//! Return true if point (x,y) is (inclusively) inside the rectangle (x1, y1, x2, y2)
#define INBOUNDS(x,y,x1,y1,x2,y2) ( (x)>=(x1) && (x)<=(x2) && (y)>=(y1) && (y)<=(y2) )

//! Return true if value x is in the range [x1, x2]
#define INBOUNDS2(x,x1,x2) ( (x)>=(x1) && (x)<=(x2) )

//! Test if rectangle (x0, y0, x1, y1) overlaps rectangle (x2, y2, x3, y3)
#define OVERLAPS(x0,y0,x1,y1,x2,y2,x3,y3) (	(!(   ((x0)<(x2) && (x1)<(x2)) || ((x0)>(x3) && (x1)>(x3)) || ((y0)<(y2) && (y1)<(y2)) || ((y0)>(y3) && (y1)>(y3))   ))	)

#ifndef MIN
//! Return smaller of (a, b)
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
//! Return larger of (a, b)
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

//! Return smaller of (a, b)
#define djMIN(a,b) ((a) < (b) ? (a) : (b))

//! Return larger of (a, b)
#define djMAX(a,b) ((a) > (b) ? (a) : (b))

//! Return val clamped to within the range [a, b], where a <= b
#define djCLAMP(val,a,b) ( (val) < (a) ? (a) : ( (val) > (b) ? (b) : (val) ) )


#ifndef ABS
//! Return absolute value of (a)
#define ABS(a) ((a) < 0 ? -(a) : (a))
#endif

//! Return -1 if (a) is negative or 1 if a is positive
#define SGN(a) ((a) < 0 ? -1 : 1)

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif

//
// Utility functions
//

//! Strip newline character from string (to handle both UNIX and stupid DOS text file formats)
extern void djStripCRLF(char *buf);

#endif

