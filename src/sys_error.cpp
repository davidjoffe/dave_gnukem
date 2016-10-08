/*
 * File:    sys_error.cc
 * Created: 2002-08-30 (Friday), 22:38
 * Modified: 2002-08-30 (Friday), 22:38
 * Author: Vytautas Shaltenis, a.k.a. rtfb
 *
 * Project: Dave Gnukem
 *
 * Description: Error, warning and debug output.
 *
 * NOTE: I use quite an intensive and dangerous perprocessor hackery here.
 * Kids, do not try this at home :)
 */

#include "sys_log.h"
#include "sys_defs.h"


static char	*file = NULL;
static int	line = 0;


void _SetFileAndLine ( const char *filename, const int linenum )
{
	file = (char*) filename;
	line = linenum;
}


void _SYS_Error ( const char *fmt, ... )
{
	char		text[1024]={0};
	char		text2[1024]={0};
	char		*ptr=NULL, *ptr2=NULL;
	va_list		args;

	if ( NULL == fmt )
		return;

	va_start ( args, fmt );
		vsprintf ( text, fmt, args );
	va_end ( args );

	sprintf ( text2, "[Error] %s line %d: ", file, line );
	ptr2 = text2 + strlen ( text2 );

	ptr = text;
	while ( *ptr )
	{
		*ptr2++ = *ptr++;
	}
	*ptr2 = '\0';

	Log ( "%s", text2 );
	//DaveCleanup ();			// main application cleanup
	//exit ( 0 );
}


void _SYS_Warning ( const char *fmt, ... )
{
	char		text[1024]={0};
	char		text2[1024]={0};
	char		*ptr=NULL, *ptr2=NULL;
	va_list		args;

	if ( NULL == fmt )
		return;

	va_start ( args, fmt );
		vsprintf ( text, fmt, args );
	va_end ( args );

	sprintf ( text2, "[Warning] %s line %d: ", file, line );
	ptr2 = text2 + strlen ( text2 );

	ptr = text;
	while ( *ptr )
	{
		*ptr2++ = *ptr++;
	}
	*ptr2 = '\0';

	Log ( "%s", text2 );
}


void _SYS_Debug ( const char *fmt, ... )
{
	char		text[1024]={0};
	char		text2[1024]={0};
	char		*ptr=NULL, *ptr2=NULL;
	va_list		args;

	if ( NULL == fmt )
		return;

	va_start ( args, fmt );
		vsprintf ( text, fmt, args );
	va_end ( args );

	sprintf ( text2, "[Debug] %s line %d: ", file, line );
	ptr2 = text2 + strlen ( text2 );

	ptr = text;
	while ( *ptr )
	{
		*ptr2++ = *ptr++;
	}
	*ptr2 = '\0';

	Log ( "%s", text2 );
}

