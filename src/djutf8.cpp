//Copyright (C) 2022 David Joffe / Dave Gnukem project
//
//dj2022-11-30 new utf8 helper(s) (and possibly later other encoding or string conversion helpers or wrappers if/as needed in future)
//NB Design-wise this is conceptually part of the generic reusable code so shouldn't have dependencies to any Dave Gnukem specific parts of the codebase

/*--------------------------------------------------------------------------*/
#include "config.h"
#include "djutf8.h"


//mildly dissatisfied with the various options for simple utf8 iterator options etc. for various reasons so just making my own simple one for now .. that's also nice as means no need to add another dependency .. but later may revisit if we need to start doing lots of other stuff unicode or string encoding conversion-wise etc.. dj2022

// return 0 if nothing to do (e.g. null or empty string or at end of string)
// return negative on error
// return number of utf8 bytes consumed if successful
int djutf8iterate(const char* sz, const unsigned int uLen, int& c)
{
	if (sz==nullptr || sz[0]==0)
	{
		// null or end of string, return 0
		c = 0;
		return 0;
	}
	const char b1 = *sz;

    // [dj2022-11] The best way to understand what this code is doing is to look
    // the Wikipedia page for utf8, and in particular this table under Encoding: 
	// https://en.wikipedia.org/wiki/UTF-8#Encoding
	// (utf8 characters may either be 1, 2, 3, or 4 bytes, and the high bits are set in certain ways)

	//1-byte codepoint?
	if ((b1 & 0b10000000)==0)//7-bit 'ascii range'
	{
		c = (int)b1;
		return 1;
	}
	//start of 2-byte codepoint?
	else if ((b1 & 0b11000000)==0b11000000)
	{
        if (uLen<2) return -1;//<- need enough characters to consume
		const int v1 = (int)(*sz & 0b00011111);		++sz; if (*sz==0) return -1;
		const int v2 = (int)(*sz & 0b00111111);
		c = (v1 << 5) | v2;
		return 2;
	}
	//start of 3-byte codepoint?
	else if ((b1 & 0b11100000)==0b11100000)
	{
        if (uLen<3) return -1;//<- need enough characters to consume
		const int v1 = (int)(*sz & 0b00001111);		++sz; if (*sz==0) return -1;
		const int v2 = (int)(*sz & 0b00111111);		++sz; if (*sz==0) return -1;
		const int v3 = (int)(*sz & 0b00111111);
		c = (v1 << 12) | (v2 << 6) | v3;
		return 3;
	}
	//start of 4-byte codepoint?
	else if ((b1 & 0b11110000)==0b11110000)
	{
        if (uLen<4) return -1;//<- need enough characters to consume
		const int v1 = *sz==0 ? 0 : (int)(*sz & 0b00000111);		++sz; if (*sz==0) return -1;
		const int v2 = *sz==0 ? 0 : (int)(*sz & 0b00111111);		++sz; if (*sz==0) return -1;
		const int v3 = *sz==0 ? 0 : (int)(*sz & 0b00111111);		++sz; if (*sz==0) return -1;
		const int v4 = *sz==0 ? 0 : (int)(*sz & 0b00111111);
		c = (v1 << 18) | (v2 << 12) | (v3 << 6) | v4;
		return 4;
	}

	// invalid input? set c=0 and return -1 to indicate 'stop processing' (?)
	// ERROR! this implies someone may have passed in pointer at a 2nd or 3rd or 4th byte of a valid stirng OR it's an invalid string
	// Return a zero as the character but what should we do, should we try consume until we find a 'correct' starting byte? probably or we may never return from an iteration [dj2022-11]
	//else if ((b1 & 0b10000000)==0b10000000)
	//printf("error utf8iterate not handled properly");
	c=-1;
	return -1;
}


