// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

// ======= netSrvUdp ================================
// Set up socket for UDP server.
// ===============================================

#ifndef csc_UDP_H
#define csc_UDP_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include "std.h"
#include "alloc.h"
#include "isvalid.h"


// --------- UDP address to send --------------

typedef struct csc_udpAddr_t csc_udpAddr_t;  // Address to send to.


csc_udpAddr_t *csc_udpAddr_new();


// csc_TRUE indicates success.  csc_FALSE indicates error.
csc_bool_t csc_udpAddr_set( csc_udpAddr_t *addr
						  , char *dest
						  , int portNum
						  );


void csc_udpAddr_free(csc_udpAddr_t *addr);




// --------- Recieve and send --------------

typedef struct csc_udp_t csc_udp_t;

csc_udp_t *csc_udp_new();
void csc_udp_free(csc_udp_t *udp);


// csc_TRUE indicates success.  csc_FALSE indicates error.
csc_bool_t csc_udp_setRcvAddr( csc_udp_t *udp       // UDP object.
							 , const char *addr    // NULL or IP of interface.
							 , int portNo         // Port number to serve on.
							 );


// On success returns number of bytes read.  On error returns -1.
int csc_udp_rcv( csc_udp_t *udp          // UDP object.
			   , char *buf, int bufSiz   // Buffer to read into.
			   , csc_udpAddr_t *udpAddr  // Address to allocate or NULL.
			   );


// On success returns number of bytes sent.  On error returns -1.
int csc_udp_send( csc_udp_t *udp            // UDP object.
			    , char *buf, int bufSiz     // Buffer to send.
			    , csc_udpAddr_t *udpAddr    // Where to send to.
			    );



// ---------------------  What was the error ------------------

// Returns a string representation of details of a previous error.  The
// string returned is valid until the next non const method call.
const char *csc_udp_getErrMsg(const csc_udp_t *cli);
const char *csc_udpAddr_getErrMsg(const csc_udpAddr_t *addr);


#endif
