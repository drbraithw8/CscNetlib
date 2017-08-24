// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#ifndef csc_LOG_H
#define csc_LOG_H 1

#include "std.h"

typedef struct csc_log_t csc_log_t;

typedef enum
{   csc_log_TRACE = 1,   // For debugging your server.
    csc_log_NOTICE,     // Important events such as server starts or stops.
    csc_log_WARN,      // Something might be wrong.
    csc_log_ERROR,    // An error condition, but the server will continue.
    csc_log_FATAL    // An error condition, and the server cannot continue.
} csc_log_level_t;

// Constructor.  Returns a new logger object on success.  'path' is the file path of
// the log file.  'logLevel' is the logging threshold.  Logging entries in
// the log file are only logged if their logLevels are greater or equal to
// 'logLevel'.  Returns NULL if logging cannot be initiated.
csc_log_t *csc_log_new(const char *path, csc_log_level_t logLevel);


// Change the logging level to 'logLevel'
// Returns csc_TRUE if logLevel has acceptable value, csc_FALSE otherwise.
csc_bool_t csc_log_setLogLevel(csc_log_t *logger, csc_log_level_t logLevel);


// Set a string to show in each log entry.
void csc_log_setIdStr(csc_log_t *logger, const char *str);


// Set whether logger should show the process ID
void csc_log_setIsShowPid(csc_log_t *logger, csc_bool_t isShow);


// Destructor.
void csc_log_free(csc_log_t *logger);


// Create an entry in the log file with the message 'msg' if 'logLevel' is
// greater or equal to the logging objects logging threshold.
int csc_log_str(csc_log_t *logger, csc_log_level_t logLevel, const char *msg);


// Creates an entry in the log file using printf() semantics if 'logLevel' is
// greater or equal to the logging objects logging threshold.  'fmt' and
// the variablt argument list that follows has the same semantics as the
// function printf(). e.g. The following make a log entry of "value=5" :-
// 
//      csc_log_printf(log, csc_log_NOTICE, "value=%d", 5);
// 
// Dont put any newlines unless you actually want an extra line.
int csc_log_printf( csc_log_t *logger
                 , csc_log_level_t logLevel
                 , const char *fmt
                 , ...
                 );


// If we set the logging level to TRACE then this puts an entry into the
// log file with the file name and line number.
// 
// This would be useful for debugging.  Say, for example, your program
// stops and crashes at some point.  Paste the following line of code ...
// 
//       csc_log_trace(log);
// 
// ... between blocks of code in the main line, and the re-run the program.
// (Note: In the above example, 'log' is of type 'csc_log_t' and has been
// initialised.) If you look in the log file, you will fill find the point
// where the trace ends.  That is where your program execution stopped.
// 
// Setting the logging level back to NOTICE (e.g. in your configuration
// file) would turn these trace logging statements off.


#define csc_log_trace(log)  csc_log_printf(log, csc_log_TRACE, \
                        "Got to line %d in file %s", __LINE__, __FILE__)


// Logger version of assert.
#define csc_log_assert(log, a)  ( !(a) ? ( \
   csc_log_assertFail(log, __FILE__, __LINE__, __func__, #a) , 0) : 0)  
void csc_log_assertFail( csc_log_t *log
                       , const char *fname
                       , int lineNo
                       , const char *expr
                       );


#endif
