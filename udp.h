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

// Create a blank new UDP address.
csc_udpAddr_t *csc_udpAddr_new();

// Set the UDP address.
csc_bool_t csc_udpAddr_setAddr(csc_udpAddr_t *addr, const char *ipStr, int portNum);

// Get the IP from an UDP address.
char *csc_udpAddr_getAllocIpStr(csc_udpAddr_t *addr);

// Get the port number from an UDP address.
int csc_udpAddr_getPortNum(csc_udpAddr_t *addr);

// Free an UDP address.
void csc_udpAddr_free(csc_udpAddr_t *addr);

//-- Errors in UDP address. --
const char *csc_udpAddr_getErrMsg(const csc_udpAddr_t *this);
const char *csc_udpAddr_resetErrMsg(csc_udpAddr_t *this);



// --------------------------------------------
// ------------------ UDP ---------------------

typedef struct csc_udp_t csc_udp_t;

// Creates new udp object.  You will need to then call csc_udp_setCli() or
// csc_udp_serSrv() before you can receive or send packets.
csc_udp_t *csc_udp_new();

// Releases udp object.
void csc_udp_free(csc_udp_t *udp);


// Tells the udp object to behave like a client.
// On success returns number of bytes read.  On error returns -1.
csc_bool_t csc_udp_setCli( csc_udp_t *udp       // UDP object.
						 , int ipType   // AF_INET or AF_INET6
						 );
 
// Tells the udp object to behave like a server.  Tells it what port and
// what interfaces to bind to and listen on.  On success returns number of
// bytes read.  On error returns -1.
csc_bool_t csc_udp_setSrv( csc_udp_t *udp       // UDP object.
						 , const char *addr    // NULL or IP of interface.
						 , int portNo         // Port number to serve on.
						 );
 
// csc_udp_rcv() will block indefinitely if the a packet fails to arrive.
// Calling this will set a timeout so that csc_udp_rcv() will return after
// the specified time, so that the call cannot block indefinitely.  On
// success returns number of bytes read.  On error returns -1.
csc_bool_t csc_udp_setRcvTimeout(csc_udp_t *udp, int millisecs);


// To be called after a call to csc_udp_setCli(), and before the first
// attempt to send a packet.  If this HAS NOT been called, then
// connect() will be called prior to the the first attempt to send a
// packet, and then 'udp' can only receive packets from that address.  This is
// normally what you want, because you may no longer have to check the
// source of the packet for csc_udp_rcv(), and NULL may be passed for
// 'addr'.  If this HAS been called, then 'udp' will still be able receive
// packets from other addresses.
void csc_udp_cliNoConnect(csc_udp_t *udp);


// Receive a packet.  On success returns number of bytes read.
// Returns >=0:nBytes read, -2:timeout, -1:error - use csc_udp_getErrMsg().
// If addr is not NULL, then *addr should be NULL.  In the latter case,
// *addr will be assigned a new object.  The caller owns the new address
// object and is responsible for csc_udpAddr_free()ing it.  
int csc_udp_rcv( csc_udp_t *udp          // UDP object.
			   , char *buf, int bufSiz  // Buffer to read into.
			   , csc_udpAddr_t **addr  // Address to assign allocated or NULL.
			   );


// Send a packet.  Returns TRUE on only on success.
csc_bool_t csc_udp_snd( csc_udp_t *udp          // UDP object.
					  , char *buf, int msgLen   // Data and length of data to be sent.
					  , csc_udpAddr_t *addr  // Address to send to.
					  );

//-- Errors --
const char *csc_udp_getErrMsg(const csc_udp_t *this);

#endif
