// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#ifndef __SERVBASE_H__
#define __SERVBASE_H__ 1

// ======= srvBase ===============================
// Logging, Configuration and Forking for TCP 
// server.  Uses netSrv.
// ===============================================


#include <stdlib.h>
#include "std.h"
#include "iniFile.h"
#include "logger.h"


// This function is a base for servers.  Many of its parameters are are not
// passed to it programmatically, but read from its configuration which
// takes the form of an ini file.  You should call this function almost
// immediately from your main routine.
// 
// Listening for connections are terminated if it receives a SIGTERM or
// SIGINT signal, and this routine will return 1 in this case.  Otherwise
// it will have returned due to error, and it will return in this case, and
// the nature of the error will be logged.
// 
// It will call the routine do_init(), if present, after the configuration
// and logging have been estabilished, but before any connections are
// established.  
// 
// It will call the routine doConn() for every connection received.
// Connections are logged at the NOTICE level.
// 
// This routine takes the following arguments:-
// 
// 1)   servModel -  Either "OneByOne" or "Forking".  ("ThreadPool" coming soon).
// 
// 2)   logPath - The path to the file for logging.
// 
// 3)   configPath - The path to the configuration file.  The configuration
//  file is read into an object of type iniFile_t, which is passed on to
//  doConn().  The server object uses the following from the 'ServerBase'
//  section:-
//  *   PortNum -    (required) The port number to listen on.
//  *   LogLevel     (optional. Dflt=2 (i.e. NOTICE)).  Logging level.
//  *   IP -         (optional. Dflt=all interfaces) the IP number to listen on.
//  *   MaxThreads - (optional. Dflt=10) Maximum simultaneous connections.
//  *   Backlog -    (optional. Dflt=10) Max size of connection queue.
// 
// 4)  doConn() is called for each connection.  doConn() returns 0 on
//  success, negative on error.  doConn() must close the file descriptor
//  'fd' before returning.  Its args are:-
//     1:  fd - the file descriptor for the connection.
//     2:  clientIP - a string containing the IP of the connected client.
//     3:  conf - The configuration object (as passed to srvBase_setup()).
//     4:  log - The logging object (as passed to srvBase_setup()).
//     5:  local - The pointer to your stuff (as passed to srvBase_setup()).
// 
// 5)  doInit() is called before any connections are accepted.  If you
//  need something to be done after the configuration and logging have
//  been initialised, but before any connections are accepted, then pass
//  a function to do this, otherwise pass NULL.
// 
// 6)  local - A pointer to whatever your want (usually a structure).  This
//  merely passed onto doInit() and also to doConn().  Pass NULL if you
//  have nothing to pass to doInit() and doConn().
int csc_servBase_server( char *srvModelStr
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
                       );


#endif

