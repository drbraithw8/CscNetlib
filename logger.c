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
#include <assert.h>

#include "std.h"
#include "alloc.h"
#include "logger.h"



typedef struct csc_log_t
{   char *path;
	char *idStr;
    csc_log_level_t level;
    csc_bool_t isShowProcessId;
	sem_t sem;
} csc_log_t;    


csc_log_t *csc_log_new(const char *path, csc_log_level_t logLevel)
{   csc_log_t *lgr;
	int retVal;
 
// Check the log level.
    if (logLevel<csc_log_TRACE || logLevel>csc_log_FATAL)
        return NULL;
 
// Create the logger.
    lgr = csc_allocOne(csc_log_t);
    lgr->path = csc_alloc_str(path);
    lgr->level = logLevel;
	lgr->idStr = NULL;
	retVal = sem_init(&lgr->sem, 1, 1); assert(retVal==0);
 
// Test the logger with an initial entry.
    lgr->isShowProcessId = TRUE;
    if (!csc_log_str(lgr, logLevel, "Logging commenced"))
    {   fprintf( stderr
			   , "Error: Logger failed to write intial entry to log file:"
			           "\n\"%s\"!\n" , path);
        csc_log_free(lgr);
        return NULL;
    }
    lgr->isShowProcessId = FALSE;
 
    return lgr;
}


void csc_log_setIsShowPid(csc_log_t *logger, csc_bool_t isShow)
{   logger->isShowProcessId = isShow;
}


void csc_log_setIdStr(csc_log_t *logger, const char *str)
{	if (logger->idStr)
		free(logger->idStr);
	logger->idStr = csc_alloc_str(str);
}


csc_bool_t csc_log_setLogLevel(csc_log_t *logger, csc_log_level_t logLevel)
{   if (logLevel<csc_log_TRACE || logLevel>csc_log_FATAL)
        return FALSE;
    else
    {   logger->level = logLevel;
        return TRUE;
    }
}


void csc_log_free(csc_log_t *logger)
{   int retVal;
	free(logger->path);
	if (logger->idStr != NULL)
		free(logger->idStr);
	retVal = sem_destroy(&logger->sem); assert(retVal==0);
    free(logger);
}


static FILE *logGuts(csc_log_t *logger, csc_log_level_t logLevel, int *retVal)
{   char timeStr[csc_timeStrSize+1];
 
// Only log if logLevel is greater or equal to the threshold.
    if (logLevel < logger->level)
    {   *retVal = TRUE;
        return NULL;
    }

// Prevent concurrent write to the logfile.
    sem_wait(&logger->sem);
 
// Open the file for append.
    FILE *fp = fopen(logger->path, "a");
    if (fp == NULL)
    {   *retVal = FALSE;
        return NULL;
    }
 
// Create and format the time.
	csc_dateTimeStr(timeStr);
 
// Make the entry.
    fprintf(fp, "%d[%s]", (int)logLevel, timeStr);
    if (logger->idStr)
        fprintf(fp, "%s ", logger->idStr);
    if (logger->isShowProcessId)
        fprintf(fp, "%d ", (int)getpid());
    
// Return stuff.
    return fp;
}


int csc_log_str(csc_log_t *logger, csc_log_level_t logLevel, const char *msg)
{   int retVal;
 
// Create initial part of entry, if the logging level is suitable.
    FILE *fp = logGuts(logger, logLevel, &retVal);
    if (fp == NULL)
        return retVal;
 
// Print the message.
    fprintf(fp, "%s\n", msg);
 
// Close the file.
    fclose(fp);

// Allow other threads to write to the logfile.
    sem_post(&logger->sem);
 
    return TRUE;
}


int csc_log_printf( csc_log_t *logger
                 , csc_log_level_t logLevel
                 , const char *format
                 , ...
                 )
{   int retVal;
    va_list args;
 
// Create initial par of entry, if the logging level is suitable.
    FILE *fp = logGuts(logger, logLevel, &retVal);
    if (fp == NULL)
        return retVal;
 
// Print the message.
   va_start(args, format);
   vfprintf(fp, format, args);
   va_end(args);
   fprintf(fp, "\n");
 
// Close the file.
    fclose(fp);

// Allow other threads to write to the logfile.
    sem_post(&logger->sem);

    return TRUE;
}

