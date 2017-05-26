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
#define ConfIdentIp "IP"
#define ConfIdentPort "PortNum"
#define ConfIdentMaxThreads "MaxThreads"
#define ConfIdentBacklog "Backlog"
#define ConfIdentLogLevel "LogLevel"

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
                        , csc_ini_t *conf
                        , csc_log_t *log
                        , void *local
                        , int (*doConn)( int fd            // client file descriptor
                                       , const char *clientIp   // IP of client, or NULL
                                       , csc_ini_t *conf // Configuration object.
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
        if (rwSock==-2 && servSig.isQuit)
        {   retVal = 1;
            csc_log_str(log, csc_log_NOTICE
                        , "Server terminating due to caught signal");
        }
        else if (rwSock < 0)
        {   csc_log_str(log, csc_log_FATAL, csc_srv_getErrMsg(srv)); 
            servSig.isQuit = csc_TRUE;
            retVal = 0;
        }
        else
        {   cliAddr = csc_srv_acceptAddr(srv);
            csc_log_printf(log, csc_log_NOTICE,
                        "Accepted connection from %s", cliAddr);
            doConn(rwSock, cliAddr, conf, log, local);
        }
    }
 
// We are finished here, so remove the signal handling.
    csc_signal_delHndl(SIGINT, &servSig);
    csc_signal_delHndl(SIGTERM, &servSig);
 
    return retVal;
}


static int serv_Forking( csc_srv_t *srv
                       , int maxThreads
                       , csc_ini_t *conf
                       , csc_log_t *log
                       , void *local
                       , int (*doConn)( int fd            // client file descriptor
                                      , const char *clientIp   // IP of client, or NULL
                                      , csc_ini_t *conf // Configuration object.
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
        if (rwSock==-2 && servSig.isQuit)
        {   retVal = 1;
            csc_log_str(log, csc_log_NOTICE
                        , "Server terminating due to caught signal");
        }
        else if (rwSock < 0)
        {   csc_log_str(log, csc_log_FATAL, csc_srv_getErrMsg(srv)); 
            servSig.isQuit = csc_TRUE;
            retVal = 0;
        }
        else
        { // Successful accept.
        // Forking.
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
 
            // Handle the connection.
                doConn(rwSock, cliAddr, conf, log, local);
 
            // Child finished therefore child dies.
                exit(0);
            }
            else  // This is the parent process.
            {   numThreads++;  // The parent has created another thread.
                close(rwSock);  // Must close or else have socket for every child started.
 
            // If we are up to the maximum number of threads, then block until a child dies.
                if (numThreads == maxThreads)
                {   deadChildProcId = wait(NULL);
                    // fprintf(stderr, "full up child buried\n");
                    numThreads--;  // One thread died.
                }
 
            // Collect all available dead children without blocking.
                isMoreDeadChildren = csc_TRUE;
                while (isMoreDeadChildren && numThreads>0)
                {   deadChildProcId = waitpid(-1,NULL,WNOHANG);
                    if (deadChildProcId == 0)
                    {   isMoreDeadChildren = csc_FALSE;  // Failed.  No more dead.  Terminate loop.
                        // fprintf(stderr, "no more dead children\n");
                    }
                    else
                    {   numThreads --;  // One child died.
                        // fprintf(stderr, "child buried\n");
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
            // fprintf(stderr, "no more dead children\n");
        }
        else
        {   numThreads --;  // One child died.
            // fprintf(stderr, "child buried\n");
        }
    }
 
// Restore the signal handling.
    csc_signal_delHndl(SIGINT, &servSig);
    csc_signal_delHndl(SIGTERM, &servSig);
 
    return retVal;
}


int csc_servBase_server( char *connType
					   , char *srvModelStr
					   , char *logPath
					   , char *configPath
					   , int (*doConn)( int fd            // client file descriptor
									  , const char *clientIp   // IP of client, or NULL
									  , csc_ini_t *conf // Configuration object.
									  , csc_log_t *log  // Logging object.
									  , void *local
									  )
					   , int (*doInit)( csc_ini_t *conf // Configuration object.
							 , csc_log_t *log  // Logging object.
							 , void *local
							 )
					   , void *local      // Values to pass to doConn() and to doInit().
					   )
{   int retVal = csc_TRUE;
    const char *logLevelStr, *portNumStr, *backlogStr, *ipStr, *maxThreadsStr;
    int iniFileLineNum, portNum, srvModel, backlog, maxThreads, result;
 
// Resources to free (should match Free resources in cleanup).
    csc_log_t *log = NULL;
    csc_ini_t *ini = NULL;
    csc_srv_t *srv = NULL;
 
// Initialise the logging.
    log = csc_log_new(logPath, initialLogLevel);
    if (log == NULL)
    {   fprintf(stderr, "Failed to initialise logging!\n");
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
    logLevelStr = csc_ini_getStr(ini, ConfSection, ConfIdentLogLevel);
    if (logLevelStr != NULL)
    {   if (!csc_isValid_int(logLevelStr) || !csc_log_setLogLevel(log, atoi(logLevelStr)))
        {   csc_log_printf( log
                         , csc_log_FATAL
                         , "Invalid \"%s\" in section \"%s\" configuration file \"%s\""
                         , ConfIdentLogLevel
                         , ConfSection
                         , configPath
                         );
            retVal = csc_FALSE; 
            goto cleanup;
        }
    }
 
// Get the port number.
    portNumStr = csc_ini_getAllocStr(ini, ConfSection, ConfIdentPort);
    if (portNumStr==NULL || !csc_isValid_int(portNumStr))
    {   csc_log_printf( log
                     , csc_log_FATAL
                     , "Invalid or missing \"%s\" in section \"%s\" configuration file \"%s\""
                     , ConfIdentPort
                     , ConfSection
                     , configPath
                     );
        retVal = csc_FALSE; 
        goto cleanup;
    }
    portNum = atoi(portNumStr);;
 
// Get the IP.
    ipStr = csc_ini_getAllocStr(ini, ConfSection, ConfIdentIp);
    // Error handling for this is performed already in csc_srv_setAddr().
 
// Get the backlog.
    backlogStr = csc_ini_getAllocStr(ini, ConfSection, ConfIdentBacklog);
    if (backlogStr == NULL)
        backlogStr = "10";
    if (!csc_isValid_int(backlogStr))
    {   csc_log_printf( log
                     , csc_log_FATAL
                     , "Invalid \"%s\" in section \"%s\" configuration file \"%s\""
                     , ConfIdentBacklog
                     , ConfSection
                     , configPath
                     );
        retVal = csc_FALSE; 
        goto cleanup;
    }
    backlog = atoi(backlogStr);
 
// Get the max number of connections.
    maxThreadsStr = csc_ini_getAllocStr(ini, ConfSection, ConfIdentMaxThreads);
    if (maxThreadsStr == NULL)
        maxThreadsStr = "4";
    else if (!csc_isValid_int(maxThreadsStr))
    {   csc_log_printf( log
                     , csc_log_FATAL
                     , "Invalid \"%s\" in section \"%s\" configuration file \"%s\""
                     , ConfIdentMaxThreads
                     , ConfSection
                     , configPath
                     );
        retVal = csc_FALSE; 
        goto cleanup;
    }
    maxThreads = atoi(maxThreadsStr);
 
// Create netSrv object.
    srv = csc_srv_new();
    if (srv == NULL)
    {   csc_log_str(log, csc_log_FATAL, "Failed to open netSrv object");
        goto cleanup;
    }
 
// Set up the server object.
    result = csc_srv_setAddr(srv, connType, ipStr, portNum, backlog);
    if (!result)
    {   csc_log_str(log , csc_log_FATAL, csc_srv_getErrMsg(srv));
        retVal = csc_FALSE; 
        goto cleanup;
    }
 
// Perform initialisations.
    if (doInit != NULL)
    {	if (!doInit(ini, log, local))
		{	retVal = csc_FALSE; 
			goto cleanup;
		}
    }
 
// Log success so far.
    csc_log_printf( log
                 , csc_log_NOTICE
                 , "%s server accepting %s connections on port %d"  
                 , srvModelStr
                 , connType
                 , portNum
                 );
 
// Do each successful connection.
    if (srvModel == srvModel_OneByOne)
        serv_OneByOne(srv, ini, log, local, doConn);
    else
        serv_Forking(srv, maxThreads, ini, log, local, doConn);
 
cleanup:  // Free resources.
    if (ini != NULL)
        csc_ini_close(ini);
    if (log != NULL)
        csc_log_close(log);
    if (srv != NULL)
        csc_srv_close(srv);
 
    return 0;
}


void csc_srvBase_daemonise(csc_log_t *log)
{   pid_t pid, sid;
 
// Fork the Parent Process
    pid = fork();
    if (pid < 0)
    {   csc_log_str(log, csc_log_ERROR,
                    "srvBase_daemonise() Forking failed");
        exit(1);
    }
 
// Close the Parent Process
    if (pid > 0)
    {   exit(1);
    }
 
// Change File Mask
    umask(0);
 
// Create a new signature id.
    sid = setsid();
    if (sid < 0)
    {   csc_log_str(log, csc_log_ERROR,
                    "srvBase_daemonise() Create signature id failed");
        exit(1);
    }
 
// Change Directory
    if ((chdir("/")) < 0)
    {   csc_log_str(log, csc_log_ERROR,
                    "srvBase_daemonise() Change directory failed");
        exit(1);
    }
 
// Close Standard File Descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}
    
    

