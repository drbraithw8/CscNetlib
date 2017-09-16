// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

// ======= netSrv ================================
// Listen for and accept connections from clients.
// ===============================================


#ifndef csc_SRV_H
#define csc_SRV_H 1

#include "std.h"

typedef struct csc_srv_t csc_srv_t ;


// Constructor.  Create new netSrv object.  Returns pointer to object on
// success.  Returns NULL on failure.
csc_srv_t *csc_srv_new();


// Tell the netSrv object what we will listen on.  Clearly we can only
// listen on IP numbers on interfaces belonging to the current computer.
// It is expected that this is called once after calling csc_srv_new(), and
// before calling csc_srv_accept().
// 
// 'conType' must be either "TCP" or "UDP".
// 
// The address, 'addr', may be in the form of  may be in IPV4 format, e.g.
// "192.168.0.3".  Or it may be in IPV6 format, e.g.
// "2001:0db8:c9d2:0012:0000:0000:0000:0051" or "2001:db8:c9d2:12::51".
// Pass NULL for 'addr' for the server to accept connections on any
// interface connected to the computer.  
// 
// The port number, 'portNo', is the port number that the server will
// listen on.  
// 
// The listening socket maintains a queue of length 'backlog' of attempted
// connections so that if the server is too busy to accept a connection
// immediately, the connection attempt will queue.  If you dont know what
// to use here, then then specify -1 to let this routine pick a sensible
// default.
// 
// Returns 1 on success, and 0 on failure.  Use csc_srv_getErrMsg() 
// to get details of failure.
int csc_srv_setAddr( csc_srv_t *srv
                  , const char *conType  // Either "TCP" or "UDP"
                  , const char *addr    // NULL or the IP number of the interface.
                  , int portNo         // Port number to serve on.
                  , int backlog);     // -1, or how many connections to queue.


// Accept a connection.  On success, returns a file descriptor associated
// with a connection.  On failure returns a negative value.  -2 indicates
// interrupt due to signal.  -1 indicates other errors.
int csc_srv_accept(csc_srv_t *srv);


// Returns address of client just connected to.  Returns NULL on failure.
const char *csc_srv_acceptAddr(csc_srv_t *srv);


// Free up resources.
void csc_srv_free(csc_srv_t *srv);


// Returns a string representation of details of a previous error.  The
// string returned is valid until the next non const method call.
const char *csc_srv_getErrMsg(const csc_srv_t *srv);


#endif
