/*
 * File:    sys_log.h
 * Created: 2002-06-29 (Saturday), 11:40
 * Modified: 2002-07-22 (Monday), 04:13
 * Author: Vytautas Shaltenis, a.k.a. rtfb
 *
 * Project: Dave Gnukem
 *
 * Description: System logger
 */

#ifndef SYS_LOG_H_KRANKLYS__
#define SYS_LOG_H_KRANKLYS__


// Init/Kill
void InitLog ();
void KillLog ();



// creates log file and returns log id
// Filename parameter may be NULL. Then defaults to `game.log'
unsigned long CreateLog ( const char *filename, const char *descr );
void DisposeLog ( unsigned long lg_id );



unsigned long SysLog ();


//dj2022 these are 'risky' names for global namespace :/ .. fix all that
class djLog
{
public:
	// The logger itself. Shall be more overloads on demand
	// dj2022 making these names longer LogFormatStr() to make it semantically CLEAR you are calling a printf-style formatting thing (which is thus riskier i.e. extra risk of introducing bugs of mismatched printf strings and parameters) (rather use LogStr when don't need formatting)
	static void LogFormatStr(const char* fmt, ...);

	//dj2022-11 RENAMING THIS as using function overloading creates risk of ambiguously calling wrong version of this function .. should we phase out below also, not sure
	//static void LogFormatStr2(unsigned long log_mask, const char* fmt, ...);

	// Log plain string (no printf formatting) - safer than printf so lean towards using this when you don't need printf style formatting [dj2022-11]
	static void LogStr(const char* szStr);
};
//dj2022-11 convenience helper for new log of plain string (with no printf style formatting)
#define djLOGSTR(sz) djLog::LogStr(sz)

// This tells logger whether or not to log to system console
void LogToScreen ( const bool l2s );
// This tells logger whether or not to log to game console
void LogToConsole ( const bool l2c );


#endif      // #ifndef SYS_LOG_H_KRANKLYS__

