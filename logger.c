// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include "std.h"
#include "alloc.h"
#include "logger.h"



const int syslogPriority[csc_log_nLogLevels] = { LOG_DEBUG
											   , LOG_NOTICE
											   , LOG_WARNING
											   , LOG_ERR
											   , LOG_EMERG
											   };

typedef struct csc_log_t
{   char *path;
    char *idStr;
    csc_log_level_t level;
    csc_bool_t isShowProcessId;
    csc_bool_t isUseSyslog;
    sem_t sem;
} csc_log_t;    



csc_log_t *csc_log_new(const char *path, const char *idStr, csc_log_level_t logLevel)
{   csc_log_t *lgr;
    int retVal;
 
// Check the log level.
    if (logLevel<csc_log_TRACE || logLevel>csc_log_FATAL)
        return NULL;
 
// Create the logger.
    lgr = csc_allocOne(csc_log_t);
    lgr->level = logLevel;
    lgr->idStr = csc_alloc_str(idStr);
    lgr->isShowProcessId = csc_TRUE;
 
// What sort of logging?
	if (csc_streq(path, "UseSyslog"))
	{	lgr->path = NULL;
    	lgr->isUseSyslog = csc_TRUE;
		openlog(idStr, LOG_PID|LOG_CONS, LOG_USER);
	}
	else
	{	lgr->path = csc_alloc_str(path);
    	lgr->isUseSyslog = csc_FALSE;
		retVal = sem_init(&lgr->sem, 1, 1); csc_assert(retVal==0);
	}
 
// Test the logger with an initial entry.
    if (!csc_log_str(lgr, csc_log_NOTICE, "Logging commenced"))
    {   fprintf(csc_stderr
               , "Error: Logger failed to write intial entry to log file:"
                       "\n\"%s\"!\n" , path);
        csc_log_free(lgr);
        return NULL;
    }
 
    return lgr;
}


void csc_log_free(csc_log_t *lgr)
{   int retVal;
    csc_log_str(lgr, csc_log_NOTICE, "Logging terminated normally");
    if (lgr->isUseSyslog)
	{	closelog();
	}
	else
	{	retVal = sem_destroy(&lgr->sem); csc_assert(retVal==0);
		free(lgr->path);
	}
	free(lgr->idStr);
    free(lgr);
}


csc_bool_t csc_log_setLogLevel(csc_log_t *lgr, csc_log_level_t logLevel)
{   if (logLevel<csc_log_TRACE || logLevel>csc_log_FATAL)
        return csc_FALSE;
    else
    {   lgr->level = logLevel;
        return csc_TRUE;
    }
}


static FILE *logGuts(csc_log_t *lgr, csc_log_level_t logLevel)
{   char timeStr[csc_timeStrSize+1];
 
// Prevent concurrent write to the logfile.
    sem_wait(&lgr->sem);
 
// Open the file for append.
    FILE *fp = fopen(lgr->path, "a");
    if (fp == NULL)
    {   return NULL;
    }
 
// Create and format the time.
    csc_dateTimeStr(timeStr);
 
// Make the entry.
    fprintf(fp, "%d[%s]", (int)logLevel, timeStr);
    if (lgr->idStr)
        fprintf(fp, "%s ", lgr->idStr);
    if (lgr->isShowProcessId)
        fprintf(fp, "%d ", (int)getpid());
    
// Return stuff.
    return fp;
}


int csc_log_printf( csc_log_t *lgr
                 , csc_log_level_t logLevel
                 , const char *format
                 , ...
                 )
{	va_list args;
 
// Only log if logLevel is greater or equal to the threshold.
    if (logLevel < lgr->level)
    {   return csc_TRUE;
    }
 
// Is using Syslog?
	if (lgr->isUseSyslog)
	{	int priority = syslogPriority[logLevel];
		char fformat[100];  csc_assert(strlen(format)<96);
		sprintf(fformat, "%d ", logLevel);
		strcat(fformat, format);
		va_start(args, format);
		vsyslog(priority, fformat, args);
		va_end(args);
	}
	else
	{
	// Create initial part of entry.
		FILE *fp = logGuts(lgr, logLevel);
		if (fp == NULL)
		{	fprintf( stderr, "%s: Logging to file \"%s\" failed!\n"
				   , lgr->idStr, lgr->path
				   );
			return csc_FALSE;
		}
	 
	// Print the message.
	   va_start(args, format);
	   vfprintf(fp, format, args);
	   va_end(args);
	   fprintf(fp, "\n");
	 
	// Close the file.
		fclose(fp);
	 
	// Allow other threads to write to the logfile.
		sem_post(&lgr->sem);
	}
 
    return csc_TRUE;
}


int csc_log_str(csc_log_t *lgr, csc_log_level_t logLevel, const char *msg)
{   return csc_log_printf(lgr, logLevel, "%s", msg);
}


void csc_log_assertFail( csc_log_t *log
                       , const char *fname
                       , int lineNo
                       , const char *expr
                       )
{   csc_log_printf( log, csc_log_FATAL
                  , "Assertion failure (%s) in file \"%s\" at line %d" 
                  , expr, fname, lineNo
                  );
    exit(1);
}

