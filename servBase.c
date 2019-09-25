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
#include "blacklist.h"
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


typedef struct
{   int readTimeoutSecs, writeTimeoutSecs;
    int blacklistMax, blacklistExpire;
    int backlog, maxThreads;
    int portNum;
    const char *ipStr;
} config_t;

static void sigHandler(int sigNum, void *context)
{   servSig_t *servSig = context;
    servSig->isQuit = csc_TRUE;
    csc_log_printf(servSig->log, csc_log_NOTICE,
                "Received SIGNAL %d", sigNum);
}


static int serv_OneByOne( csc_log_t *log
                        , csc_ini_t *ini
                        , csc_srv_t *srv
                        , config_t *conf
                        , int (*doConn)( int fd            // client file descriptor
                                       , const char *clientIp   // IP of client, or NULL
                                       , csc_ini_t *ini // Configuration object.
                                       , csc_log_t *log  // Logging object.
                                       , void *local
                                       )
                        , void *local
                        )
{   int rwSock = -1;
    const char *cliAddr = NULL;
    int retVal = -2;
 
// Resources.
    csc_blacklist_t *blacklist = NULL;
    
// Set up blacklisting.
    if (conf->blacklistMax > 0)
        blacklist = csc_blacklist_new(conf->blacklistExpire);
 
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
        {   retVal = 1;
            csc_log_str(log, csc_log_NOTICE
                        , "Server terminating due to caught signal");
        }
        else if (rwSock < 0)   // Some sort of error.
        {   csc_log_str(log, csc_log_FATAL, csc_srv_getErrMsg(srv)); 
            servSig.isQuit = csc_TRUE;
            retVal = 0;
        }
        else  // The socket is OK.
        {
        // Accept the connection.
            cliAddr = csc_srv_acceptAddr(srv);
 
        // Blacklisting.
            if (blacklist && csc_blacklist_blackness(blacklist,cliAddr) > conf->blacklistMax)
            { // That IP has been blacklisted. Reject the connection.
                close(rwSock);
                csc_log_printf(log, csc_log_NOTICE, "Connection blacklisted %s", cliAddr);
            }
            else
            {
            // Clean blacklist.
                if (blacklist && csc_blacklist_accessCount(blacklist) > 200)
                    csc_blacklist_clean(blacklist);
 
            // Logging.
                csc_log_printf(log, csc_log_NOTICE,
                            "Accepted connection from %s", cliAddr);
 
            // Impose read/write timeouts.
                csc_sock_setTimeout(rwSock, "r", conf->readTimeoutSecs);
                csc_sock_setTimeout(rwSock, "w", conf->writeTimeoutSecs);
     
            // Handle the connection.
                doConn(rwSock, cliAddr, ini, log, local);
            }
        }
    }
 
// We are finished here, so remove the signal handling.
    csc_signal_delHndl(SIGINT, &servSig);
    csc_signal_delHndl(SIGTERM, &servSig);
 
// Free resources.
    if (blacklist)
        csc_blacklist_free(blacklist);
 
    return retVal;
}


static int serv_Forking( csc_log_t *log
                       , csc_ini_t *ini
                       , csc_srv_t *srv
                       , config_t *conf
                       , int (*doConn)( int fd            // client file descriptor
                                      , const char *clientIp   // IP of client, or NULL
                                      , csc_ini_t *ini // Configuration object.
                                      , csc_log_t *log  // Logging object.
                                      , void *local
                                      )
                       , void *local
                       )
{   int rwSock = -1;
    const char *cliAddr = NULL;
    int retVal = -2;
    int numThreads = 0;
    int isMoreDeadChildren = csc_FALSE;
    pid_t newChildProcId = 0;
    pid_t deadChildProcId = 0;
    
// Resources.
    csc_blacklist_t *blacklist = NULL;
    
// Set up blacklisting.
    if (conf->blacklistMax > 0)
        blacklist = csc_blacklist_new(conf->blacklistExpire);
 
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
        {   retVal = 1;
            csc_log_str(log, csc_log_NOTICE
                        , "Server terminating due to caught signal");
        }
        else if (rwSock < 0)  // Some sort of error.
        {   csc_log_str(log, csc_log_FATAL, csc_srv_getErrMsg(srv)); 
            servSig.isQuit = csc_TRUE;
            retVal = 0;
        }
        else
        { // Successful accept.
 
        // Log the start of the processing.
            cliAddr = csc_srv_acceptAddr(srv);
 
        // Blacklisting.
            if (blacklist && csc_blacklist_blackness(blacklist, cliAddr) > conf->blacklistMax)
            { // That IP has been blacklisted. Reject the connection.
                close(rwSock);
                csc_log_printf(log, csc_log_NOTICE, "Connection blacklisted %s", cliAddr);
            }
            else
            {
            // Clean blacklist.
                if (blacklist && csc_blacklist_accessCount(blacklist) > 200)
                    csc_blacklist_clean(blacklist);
 
            // Logging.
                csc_log_printf(log, csc_log_NOTICE,
                        "Accepted connection from %s", cliAddr);
 
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
     
                // Impose read/write timeouts.
                    csc_sock_setTimeout(rwSock, "r", conf->readTimeoutSecs);
                    csc_sock_setTimeout(rwSock, "w", conf->writeTimeoutSecs);
     
                // Handle the connection.
                    doConn(rwSock, cliAddr, ini, log, local);
     
                // Child finished therefore child dies.
                    exit(0);
                }
                else  // This is the parent process.
                {   numThreads++;  // The parent has created another thread.
                    close(rwSock);  // Must close or else have socket for every child started.
     
                // If we are up to the maximum number of threads, then block until a child dies.
                    if (numThreads == conf->maxThreads)
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
            } // Not blacklisted.
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
 
// Free resources.
    if (blacklist)
        csc_blacklist_free(blacklist);
 
    return retVal;
}


csc_bool_t readConfig(csc_log_t *log, csc_ini_t **ini, config_t *conf, char *configPath)
{   int iniFileLineNum;
    const char *str;

// Initialise each element of the configuration to invalid values.
    conf->portNum = 0;
    conf->ipStr = NULL;
    conf->backlog = -1;
    conf->maxThreads = -1;
    conf->readTimeoutSecs = -1;
    conf->writeTimeoutSecs = -1;
    conf->blacklistMax = -1;
    conf->blacklistExpire = -1;
 
// Get configuration object.
    *ini = csc_ini_new();
    if (*ini == NULL)
    {   csc_log_str(log, csc_log_FATAL
                    , "Failed to create iniFile object");
        return csc_FALSE;
    }
 
// Read configuration.
    iniFileLineNum = csc_ini_read(*ini, configPath);
    if (iniFileLineNum > 0)
    {   csc_log_printf(log, csc_log_FATAL
                    , "Error reading ini file \"%s\" on line number %d"
                    , configPath, iniFileLineNum);
        return csc_FALSE;
    }
    else if (iniFileLineNum < 0)
    {   csc_log_printf(log, csc_log_FATAL 
                    , "Error reading ini file \"%s\".  Could not open."
                    , configPath);
        return csc_FALSE;
    }
 
// Get and set logging level
    str = csc_ini_getStr(*ini, ConfSection, configId_LogLevel);
    if (str != NULL)
    {   if (!csc_isValid_int(str) || !csc_log_setLogLevel(log, atoi(str)))
        {   csc_log_printf( log
                         , csc_log_FATAL
                         , "Invalid \"%s\" in section \"%s\" configuration file \"%s\""
                         , configId_LogLevel
                         , ConfSection
                         , configPath
                         );
            return csc_FALSE;
        }
    }
 
// Set the error output.
    const char *stderrPath = csc_ini_getStr(*ini, ConfSection, configId_errPath);
    if (stderrPath != NULL)
    {   if (!csc_isValid_decentAbsPath(stderrPath))
        {   csc_log_printf( log
                         , csc_log_FATAL
                         , "Invalid \"%s\" in section \"%s\" configuration file \"%s\""
                         , configId_errPath
                         , ConfSection
                         , configPath
                         );
            return csc_FALSE;
        }
        csc_setErrOut(stderrPath);
    }
 
// Get the port number.
// Some error handling for this is performed already in csc_srv_setAddr().
    str = csc_ini_getStr(*ini, ConfSection, configId_Port);
    if (str==NULL || !csc_isValid_int(str))
    {   csc_log_printf( log
                     , csc_log_FATAL
                     , "Invalid or missing \"%s\" in section \"%s\" configuration file \"%s\""
                     , configId_Port
                     , ConfSection
                     , configPath
                     );
        return csc_FALSE;
    }
    conf->portNum = atoi(str);
 
// Get the IP.
// Error handling for this is performed already in csc_srv_setAddr().
    conf->ipStr = csc_ini_getStr(*ini, ConfSection, configId_Ip);
 
// Get the backlog.
    str = csc_ini_getStr(*ini, ConfSection, configId_Backlog);
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
        return csc_FALSE;
    }
    conf->backlog = atoi(str);
 
// Get the max number of connections.
    str = csc_ini_getStr(*ini, ConfSection, configId_MaxThreads);
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
        return csc_FALSE;
    }
    conf->maxThreads = atoi(str);
 
// Get the read timeout value.
    str = csc_ini_getStr(*ini, ConfSection, configId_ReadTimeout);
    if (str == NULL)
        str = "20";
    if (!csc_isValidRange_int(str, 0, 1000, &conf->readTimeoutSecs))
    {   csc_log_printf( log
                     , csc_log_FATAL
                     , "Invalid \"%s\" in section \"%s\" configuration file \"%s\""
                     , configId_ReadTimeout
                     , ConfSection
                     , configPath
                     );
        return csc_FALSE;
    }
 
// Get the write timeout value.
    str = csc_ini_getStr(*ini, ConfSection, configId_WriteTimeout);
    if (str == NULL)
        str = "20";
    if (!csc_isValidRange_int(str, 0, 1000, &conf->writeTimeoutSecs))
    {   csc_log_printf( log
                     , csc_log_FATAL
                     , "Invalid \"%s\" in section \"%s\" configuration file \"%s\""
                     , configId_WriteTimeout
                     , ConfSection
                     , configPath
                     );
        return csc_FALSE;
    }
 
// Get blacklistMax.
    str = csc_ini_getStr(*ini, ConfSection, configId_BlacklistMax);
    if (str == NULL)
        str = "0";
    if (!csc_isValidRange_int(str, 0, 1000, &conf->blacklistMax))
    {   csc_log_printf( log
                     , csc_log_FATAL
                     , "Invalid \"%s\" in section \"%s\" configuration file \"%s\""
                     , configId_BlacklistMax
                     , ConfSection
                     , configPath
                     );
        return csc_FALSE;
    }
 
// Get blacklistExpire.
    str = csc_ini_getStr(*ini, ConfSection, configId_BlacklistExpire);
    if (str == NULL)
        str = "20";
    if (!csc_isValidRange_int(str, 1, 1000, &conf->blacklistExpire))
    {   csc_log_printf( log
                     , csc_log_FATAL
                     , "Invalid \"%s\" in section \"%s\" configuration file \"%s\""
                     , configId_BlacklistExpire
                     , ConfSection
                     , configPath
                     );
        return csc_FALSE;
    }
 
// If we got to here, its all good.
    return csc_TRUE;
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
    int srvModel, result;
    config_t config;
 
// Resources.
    csc_ini_t *ini = NULL;
    csc_log_t *log = NULL;
    csc_srv_t *srv = NULL;
 
// Initialise the logging.
    log = csc_log_new(logPath, initialLogLevel);
    if (log == NULL)
    {   fprintf(csc_stderr, "Failed to initialise the logging.\n");
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
 
// Read in the configuration.
    retVal = readConfig(log, &ini, &config, configPath);
    if (!retVal)
        goto cleanup;
 
// Create netSrv object.
    srv = csc_srv_new();
    if (srv == NULL)
    {   csc_log_str(log, csc_log_FATAL, "Failed to open netSrv object");
        goto cleanup;
    }
 
// Set up the server object.
    result = csc_srv_setAddr(srv, config.ipStr, config.portNum, config.backlog);
    if (!result)
    {   csc_log_str(log , csc_log_FATAL, csc_srv_getErrMsg(srv));
        retVal = csc_FALSE; 
        goto cleanup;
    }
 
// Perform initialisations.  doInit() is passed from the caller.
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
                  , config.portNum
                  );
    if (csc_errOut)
    {   fprintf( csc_errOut
               , "AllGood.  %s server now serving on port %d\n"
               , srvModelStr
               , config.portNum
               );
    }
 
// Do each successful connection.
    if (srvModel == srvModel_OneByOne)
        serv_OneByOne(log, ini, srv, &config, doConn, local);
    else
        serv_Forking(log, ini, srv, &config, doConn, local);
 
cleanup:  // Free resources.
    if (ini != NULL)
        csc_ini_free(ini);
    if (log != NULL)
        csc_log_free(log);
    if (srv != NULL)
        csc_srv_free(srv);
 
    return 0;
}


