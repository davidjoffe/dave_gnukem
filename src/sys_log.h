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


// The logger itself. Shall be more overloads on demand
void Log ( unsigned long log_mask, const char *fmt, ... );
void Log ( const char *fmt, ... );


// This tells logger whether or not to log to system console
void LogToScreen ( const bool l2s );
// This tells logger whether or not to log to game console
void LogToConsole ( const bool l2c );


#endif      // #ifndef SYS_LOG_H_KRANKLYS__

