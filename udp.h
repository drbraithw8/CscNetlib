// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

// ======= netSrvUdp ================================
// Set up socket for UDP server.
// ===============================================

#ifndef csc_UDP_H
#define csc_UDP_H 1

#include "std.h"

// --------------------------------------------
// --------- UDP address to send --------------

typedef struct csc_udpAddr_t csc_udpAddr_t;
csc_udpAddr_t *csc_udpAddr_new();
csc_bool_t csc_udpAddr_setAddr(csc_udpAddr_t *addr, const char *ipStr, int portNum);
char *csc_udpAddr_getAllocIpStr(csc_udpAddr_t *addr);
int csc_udpAddr_getPortNum(csc_udpAddr_t *addr);
void csc_udpAddr_free(csc_udpAddr_t *addr);

//-- Errors --
const char *csc_udpAddr_getErrMsg(const csc_udpAddr_t *this);
const char *csc_udpAddr_resetErrMsg(csc_udpAddr_t *this);



// --------------------------------------------
// ------------------ UDP ---------------------

typedef struct csc_udp_t csc_udp_t;
csc_udp_t *csc_udp_new();
void csc_udp_free(csc_udp_t *udp);

// On success returns number of bytes read.  On error returns -1.
csc_bool_t csc_udp_setCli( csc_udp_t *udp       // UDP object.
						 , int ipType   // AF_INET or AF_INET6
						 );
 
// On success returns number of bytes read.  On error returns -1.
csc_bool_t csc_udp_setSrv( csc_udp_t *udp       // UDP object.
						 , const char *addr    // NULL or IP of interface.
						 , int portNo         // Port number to serve on.
						 );
 
// On success returns number of bytes read.  On error returns -1.
csc_bool_t csc_udp_setRcvTimeout(csc_udp_t *udp, int millisecs);

// On success returns number of bytes read.
// Returns >=0:nBytes read, -2:timeout, -1:error - use csc_udp_getErrMsg().
// If addr is not NULL, then *addr should be NULL.  In the latter case,
// *addr will be assigned a new object.  The caller owns the new address
// object and is responsible for csc_udpAddr_free()ing it.  
int csc_udp_rcv( csc_udp_t *udp          // UDP object.
			   , char *buf, int bufSiz  // Buffer to read into.
			   , csc_udpAddr_t **addr  // Address to assign allocated or NULL.
			   );

csc_bool_t csc_udp_snd( csc_udp_t *udp          // UDP object.
					  , char *buf, int msgLen   // Buffer to read into.
					  , csc_udpAddr_t *addr  // Address.
					  );

//-- Errors --
const char *csc_udp_getErrMsg(const csc_udp_t *this);

#endif
