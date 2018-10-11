// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "std.h"
#include "alloc.h"
#include "isvalid.h"
#include "signal.h"
#include "netSrv.h"
#include "iniFile.h"
#include "logger.h"
#include "servBase.h"

#define ConfSection "ServerBase"
#define configId_Ip "IP"
#define configId_Port "PortNum"
#define configId_MaxThreads "MaxThreads"
#define configId_ReadTimeout "ReadTimeout"
#define configId_WriteTimeout "WriteTimeout"
#define configId_BlacklistMax "BlacklistMax"
#define configId_BlacklistExpire "BlacklistExpire"
#define configId_Backlog "Backlog"
#define configId_LogLevel "LogLevel"
#define configId_errPath "StdErrPath"

#define srvModelStr_OneByOne "OneByOne"
#define srvModel_OneByOne 1
#define srvModelStr_Forking "Forking"
#define srvModel_Forking 2

#define initialLogLevel 2


typedef struct
{   int isQuit;
    csc_log_t *log;
} servSig_t;


static void sigHandler(int sigNum, void *context)
{   servSig_t *servSig = context;
    servSig->isQuit = csc_TRUE;
    csc_log_printf(servSig->log, csc_log_NOTICE,
                "Received SIGNAL %d", sigNum);
}


static int serv_OneByOne( csc_srv_t *srv
                        , int readTimeoutSecs
                        , int writeTimeoutSecs
                        , csc_ini_t *ini
                        , csc_log_t *log
                        , void *local
                        , int (*doConn)( int fd            // client file descriptor
                                       , const char *clientIp   // IP of client, or NULL
                                       , csc_ini_t *ini // Configuration object.
                                       , csc_log_t *log  // Logging object.
                                       , void *local
                                       )
                        )
{   int rwSock = -1;
    const char *cliAddr = NULL;
    int retVal = -2;
    
// Set up the signal handling.
    servSig_t servSig;
    servSig.isQuit = csc_FALSE;
    servSig.log = log;
    csc_signal_addHndl(SIGINT, sigHandler, &servSig);
    csc_signal_addHndl(SIGTERM, sigHandler, &servSig);
 
// Call accept.
    while (!servSig.isQuit)
    {   rwSock = csc_srv_accept(srv);
        if (rwSock==-2 && servSig.isQuit)   // Interrupted.
        {	retVal = 1;
            csc_log_str(log, csc_log_NOTICE
                        , "Server terminating due to caught signal");
        }
        else if (rwSock < 0)   // Some sort of error.
        {	csc_log_str(log, csc_log_FATAL, csc_srv_getErrMsg(srv)); 
            servSig.isQuit = csc_TRUE;
            retVal = 0;
        }
        else  // The socket is OK.
        {
		// Accept the connection.
			cliAddr = csc_srv_acceptAddr(srv);
            csc_log_printf(log, csc_log_NOTICE,
                        "Accepted connection from %s", cliAddr);
 
		// Impose read/write timeouts.
			csc_sock_setTimeout(rwSock, "r", readTimeoutSecs);
			csc_sock_setTimeout(rwSock, "w", writeTimeoutSecs);
 
 		// Handle the connection.
            doConn(rwSock, cliAddr, ini, log, local);
        }
    }
 
// We are finished here, so remove the signal handling.
    csc_signal_delHndl(SIGINT, &servSig);
    csc_signal_delHndl(SIGTERM, &servSig);
 
    return retVal;
}


static int serv_Forking( csc_srv_t *srv
                       , int readTimeoutSecs
                       , int writeTimeoutSecs
                       , int maxThreads
                       , csc_ini_t *ini
                       , csc_log_t *log
                       , void *local
                       , int (*doConn)( int fd            // client file descriptor
                                      , const char *clientIp   // IP of client, or NULL
                                      , csc_ini_t *ini // Configuration object.
                                      , csc_log_t *log  // Logging object.
                                      , void *local
                                      )
                       )
{   int rwSock = -1;
    const char *cliAddr = NULL;
    int retVal = -2;
    int numThreads = 0;
    int isMoreDeadChildren = csc_FALSE;
    pid_t newChildProcId = 0;
    pid_t deadChildProcId = 0;
    
// Set up the signal handling.
    servSig_t servSig;
    servSig.isQuit = csc_FALSE;
    servSig.log = log;
    csc_signal_addHndl(SIGINT, sigHandler, &servSig);
    csc_signal_addHndl(SIGTERM, sigHandler, &servSig);
 
// Call accept.
    while (!servSig.isQuit)
    {   rwSock = csc_srv_accept(srv);
        if (rwSock==-2 && servSig.isQuit) // Interrupted.
        {	retVal = 1;
            csc_log_str(log, csc_log_NOTICE
                        , "Server terminating due to caught signal");
        }
        else if (rwSock < 0)  // Some sort of error.
        {	csc_log_str(log, csc_log_FATAL, csc_srv_getErrMsg(srv)); 
            servSig.isQuit = csc_TRUE;
            retVal = 0;
        }
        else
        { // Successful accept.
        // Do the forking.
            newChildProcId = fork();  // One process splits into two.
            if (newChildProcId < 0)  // Error.  Fork failed.
            {   csc_log_printf(log, csc_log_ERROR,
                                "fork: %s", strerror(errno)); 
                servSig.isQuit = csc_TRUE;
                retVal = 0;
            }
            else if (newChildProcId == 0)  // This is the child process.
            {   
            // Only parent accepts connections.  Remove signal handling for accept.
                csc_signal_delHndl(SIGINT, &servSig);
                csc_signal_delHndl(SIGTERM, &servSig);
 
            // Log the start of the processing.
                cliAddr = csc_srv_acceptAddr(srv);
                csc_log_printf(log, csc_log_NOTICE,
                        "Accepted connection from %s", cliAddr);
 
			// Impose read/write timeouts.
				csc_sock_setTimeout(rwSock, "r", readTimeoutSecs);
				csc_sock_setTimeout(rwSock, "w", writeTimeoutSecs);
 
            // Handle the connection.
                doConn(rwSock, cliAddr, ini, log, local);
 
            // Child finished therefore child dies.
                exit(0);
            }
            else  // This is the parent process.
            {   numThreads++;  // The parent has created another thread.
                close(rwSock);  // Must close or else have socket for every child started.
 
            // If we are up to the maximum number of threads, then block until a child dies.
                if (numThreads == maxThreads)
                {   deadChildProcId = wait(NULL);
					// fprintf(csc_stderr, "Full up child buried.\n");
                    numThreads--;  // One thread died.
                }
 
            // Collect all available dead children without blocking.
                isMoreDeadChildren = csc_TRUE;
                while (isMoreDeadChildren && numThreads>0)
                {   deadChildProcId = waitpid(-1,NULL,WNOHANG);
                    if (deadChildProcId == 0)
                    {   isMoreDeadChildren = csc_FALSE;  // Failed.  No more dead.  Terminate loop.
						// fprintf(csc_stderr, "No more dead children.\n");
                    }
                    else
                    {   numThreads --;  // One child died.
						// fprintf(csc_stderr, "Child buried.\n");
                    }
                }
            } // Parent process.
        } // Successful accept.
    } // While we are not quitting.
 
// Collect all available dead children without blocking.
    isMoreDeadChildren = csc_TRUE;
    while (isMoreDeadChildren && numThreads>0)
    {   deadChildProcId = waitpid(-1,NULL,WNOHANG);
        if (deadChildProcId == 0)
        {   isMoreDeadChildren = csc_FALSE;  // Failed.  No more dead.  Terminate loop.
			// fprintf(csc_stderr, "No more dead children.\n");
        }
        else
        {   numThreads --;  // One child died.
			// fprintf(csc_stderr, "Child buried.\n");
        }
    }
 
// Restore the signal handling.
    csc_signal_delHndl(SIGINT, &servSig);
    csc_signal_delHndl(SIGTERM, &servSig);
 
    return retVal;
}


int csc_servBase_server( char *srvModelStr
                       , char *logPath
                       , char *configPath
                       , int (*doConn)( int fd            // client file descriptor
                                      , const char *clientIp   // IP of client, or NULL
                                      , csc_ini_t *ini // Configuration object.
                                      , csc_log_t *log  // Logging object.
                                      , void *local
                                      )
                       , int (*doInit)( csc_ini_t *ini // Configuration object.
									  , csc_log_t *log  // Logging object.
									  , void *local
									  )
                       , void *local      // Values to pass to doConn() and to doInit().
                       )
{   int retVal = csc_TRUE;
	const char *str;
    const char *ipStr;
    int iniFileLineNum, portNum, srvModel, backlog, maxThreads, result;
	int readTimeoutSecs, writeTimeoutSecs;
	int blacklistMax, blacklistExpire;
 
// Resources to free (should match Free resources in cleanup).
    csc_log_t *log = NULL;
    csc_ini_t *ini = NULL;
    csc_srv_t *srv = NULL;
 
// Initialise the logging.
    log = csc_log_new(logPath, initialLogLevel);
    if (log == NULL)
	{	fprintf(csc_stderr, "Failed to initialise the logging.\n");
        retVal = csc_FALSE; 
        goto cleanup;
    }
 
// Check the server model.
    if (csc_streq(srvModelStr,srvModelStr_OneByOne))
        srvModel = srvModel_OneByOne;
    else if (csc_streq(srvModelStr,srvModelStr_Forking))
    {   srvModel = srvModel_Forking;
        csc_log_setIsShowPid(log, csc_TRUE);
    }
    else
    {   csc_log_printf( log , csc_log_FATAL , "Invalid server model");
        retVal = csc_FALSE; 
        goto cleanup;
    }
 
// Get configuration object.
    ini = csc_ini_new();
    if (ini == NULL)
    {   csc_log_str(log, csc_log_FATAL
                    , "Failed to create iniFile object");
        retVal = csc_FALSE; 
        goto cleanup;
    }
 
// Read configuration.
    iniFileLineNum = csc_ini_read(ini, configPath);
    if (iniFileLineNum > 0)
    {   csc_log_printf(log, csc_log_FATAL
                    , "Error reading ini file \"%s\" on line number %d"
                    , configPath, iniFileLineNum);
        retVal = csc_FALSE; 
        goto cleanup;
    }
    else if (iniFileLineNum < 0)
    {   csc_log_printf(log, csc_log_FATAL 
                    , "Error reading ini file \"%s\".  Could not open."
                    , configPath);
        retVal = csc_FALSE; 
        goto cleanup;
    }
 
// Get and set logging level
    str = csc_ini_getStr(ini, ConfSection, configId_LogLevel);
    if (str != NULL)
    {   if (!csc_isValid_int(str) || !csc_log_setLogLevel(log, atoi(str)))
        {   csc_log_printf( log
                         , csc_log_FATAL
                         , "Invalid \"%s\" in section \"%s\" configuration file \"%s\""
                         , configId_LogLevel
                         , ConfSection
                         , configPath
                         );
            retVal = csc_FALSE; 
            goto cleanup;
        }
    }
 
// Set the error output.
	const char *stderrPath = csc_ini_getStr(ini, ConfSection, configId_errPath);
	if (stderrPath != NULL)
	{	if (!csc_isValid_decentAbsPath(stderrPath))
		{   csc_log_printf( log
						 , csc_log_FATAL
						 , "Invalid \"%s\" in section \"%s\" configuration file \"%s\""
						 , configId_errPath
						 , ConfSection
						 , configPath
						 );
			retVal = csc_FALSE; 
			goto cleanup;
		}
		csc_setErrOut(stderrPath);
	}
 
// Get the port number.
    str = csc_ini_getStr(ini, ConfSection, configId_Port);
    if (str==NULL || !csc_isValid_int(str))
    {   csc_log_printf( log
                     , csc_log_FATAL
                     , "Invalid or missing \"%s\" in section \"%s\" configuration file \"%s\""
                     , configId_Port
                     , ConfSection
                     , configPath
                     );
        retVal = csc_FALSE; 
        goto cleanup;
    }
    portNum = atoi(str);
 
// Get the IP.
    ipStr = csc_ini_getStr(ini, ConfSection, configId_Ip);
    // Error handling for this is performed already in csc_srv_setAddr().
 
// Get the backlog.
    str = csc_ini_getStr(ini, ConfSection, configId_Backlog);
    if (str == NULL)
        str = "10";
    if (!csc_isValid_int(str))
    {   csc_log_printf( log
                     , csc_log_FATAL
                     , "Invalid \"%s\" in section \"%s\" configuration file \"%s\""
                     , configId_Backlog
                     , ConfSection
                     , configPath
                     );
        retVal = csc_FALSE; 
        goto cleanup;
    }
    backlog = atoi(str);
 
// Get the max number of connections.
    str = csc_ini_getStr(ini, ConfSection, configId_MaxThreads);
    if (str == NULL)
        str = "4";
    else if (!csc_isValid_int(str))
    {   csc_log_printf( log
                     , csc_log_FATAL
                     , "Invalid \"%s\" in section \"%s\" configuration file \"%s\""
                     , configId_MaxThreads
                     , ConfSection
                     , configPath
                     );
        retVal = csc_FALSE; 
        goto cleanup;
    }
    maxThreads = atoi(str);
 
// Get the read timeout value.
	str = csc_ini_getStr(ini, ConfSection, configId_ReadTimeout);
    if (str == NULL)
        str = "20";
	if (!csc_isValidRange_int(str, 0, 1000, &readTimeoutSecs))
    {   csc_log_printf( log
                     , csc_log_FATAL
                     , "Invalid \"%s\" in section \"%s\" configuration file \"%s\""
                     , configId_ReadTimeout
                     , ConfSection
                     , configPath
                     );
        retVal = csc_FALSE; 
        goto cleanup;
    }
 
// Get the write timeout value.
	str = csc_ini_getStr(ini, ConfSection, configId_WriteTimeout);
    if (str == NULL)
        str = "20";
	if (!csc_isValidRange_int(str, 0, 1000, &writeTimeoutSecs))
    {   csc_log_printf( log
                     , csc_log_FATAL
                     , "Invalid \"%s\" in section \"%s\" configuration file \"%s\""
                     , configId_WriteTimeout
                     , ConfSection
                     , configPath
                     );
        retVal = csc_FALSE; 
        goto cleanup;
    }
 
// Get blacklistMax.
	str = csc_ini_getStr(ini, ConfSection, configId_BlacklistMax);
    if (str == NULL)
        str = "0";
	if (!csc_isValidRange_int(str, 0, 1000, &blacklistMax))
    {   csc_log_printf( log
                     , csc_log_FATAL
                     , "Invalid \"%s\" in section \"%s\" configuration file \"%s\""
                     , configId_BlacklistMax
                     , ConfSection
                     , configPath
                     );
        retVal = csc_FALSE; 
        goto cleanup;
    }
 
// Get blacklistExpire.
	str = csc_ini_getStr(ini, ConfSection, configId_BlacklistExpire);
    if (str == NULL)
        str = "20";
	if (!csc_isValidRange_int(str, 0, 1000, &blacklistExpire))
    {   csc_log_printf( log
                     , csc_log_FATAL
                     , "Invalid \"%s\" in section \"%s\" configuration file \"%s\""
                     , configId_BlacklistExpire
                     , ConfSection
                     , configPath
                     );
        retVal = csc_FALSE; 
        goto cleanup;
    }
 
// Create netSrv object.
    srv = csc_srv_new();
    if (srv == NULL)
    {   csc_log_str(log, csc_log_FATAL, "Failed to open netSrv object");
        goto cleanup;
    }
 
// Set up the server object.
    result = csc_srv_setAddr(srv, ipStr, portNum, backlog);
    if (!result)
    {   csc_log_str(log , csc_log_FATAL, csc_srv_getErrMsg(srv));
        retVal = csc_FALSE; 
        goto cleanup;
    }
 
// Perform initialisations.
    if (doInit != NULL)
    {   if (!doInit(ini, log, local))
        {   retVal = csc_FALSE; 
            goto cleanup;
        }
    }
 
// Log success so far.
    csc_log_printf( log
                 , csc_log_NOTICE
                 , "%s server accepting connections on port %d"  
                 , srvModelStr
                 , portNum
                 );
	if (stderrPath != NULL)
	{	fprintf( csc_stderr
			   , "AllGood.  %s server now serving on port %d\n"
               , srvModelStr
               , portNum
               );
	}
 
// Do each successful connection.
    if (srvModel == srvModel_OneByOne)
        serv_OneByOne( srv, readTimeoutSecs, writeTimeoutSecs
					 , ini, log, local, doConn);
    else
        serv_Forking( srv, readTimeoutSecs, writeTimeoutSecs
					, maxThreads, ini, log, local, doConn);
 
cleanup:  // Free resources.
    if (ini != NULL)
        csc_ini_free(ini);
    if (log != NULL)
        csc_log_free(log);
    if (srv != NULL)
        csc_srv_free(srv);
 
    return 0;
}


