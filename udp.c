// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

// ======= netSrvUdp ================================
// Set up socket for UDP server.
// ===============================================


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
#include "udp.h"

// --------------------------------------------
// --------- UDP address to send --------------

typedef struct csc_udpAddr_t
{	int flags;
	char *errMsg;
	int portDest;
	char *addrDest;
} csc_udpAddr_t;  // Address to send to.


static void udpAddr_setErrMsg(csc_udpAddr_t *this, char *newErrMsg)
{   if (this->errMsg != NULL)
        free(this->errMsg);
    this->errMsg = newErrMsg;
}


const char *csc_udpAddr_getErrMsg(const csc_udpAddr_t *this)
{   return this->errMsg;
}


csc_udpAddr_t *csc_udpAddr_new()
{	csc_udpAddr_t *addr = allocOne(csc_udpAddr_t);  assert(addr);
	addr->flags = 0;
	addr->portDest = -1;
	addr->addrDest = NULL;
	addr->errMsg = NULL;
}


csc_bool_t csc_udpAddr_setAddr( csc_udpAddr_t *addr
							  , char *addrDest
							  , int portDest
							  , int flags    
							  )
{	addr->portDest = portDest;
	addr->addrDest = addrDest;
	addr->flags = flags;
	addr->errMsg = NULL;
}


csc_bool_t csc_udpAddr_setFlags(csc_udpAddr_t *addr , int flags)
{	addr->flags = flags;
}


void csc_udpAddr_free(csc_udpAddr_t *addr)
{	if (addr->addrDest)
		free(addr->addrDest);
	if (addr->errMsg)
		free(addr->errMsg);
	free(addr);
}


// --------------------------------------------
// ------------------ UDP ---------------------


static void *get_in_addr(struct sockaddr *sa)
{	if (sa->sa_family == AF_INET)
	{	return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


typedef struct csc_udp_t
{	int flags;
	int portSrc;
	char *addrSrc;
	int sockFd;
	char *errMsg;
} csc_udp_t;


static void udp_setErrMsg(csc_udp_t *this, char *newErrMsg)
{   if (this->errMsg != NULL)
        free(this->errMsg);
    this->errMsg = newErrMsg;
}


const char *csc_udp_getErrMsg(const csc_udp_t *this)
{   return this->errMsg;
}


csc_udp_t *csc_udp_new()
{	csc_udp_t *udp = allocOne(csc_udp_t);  assert(udp);
	udp->portSrc = -1;
	udp->addrSrc = NULL;
	udp->errMsg = NULL;
	udp->flags = 0;
	udp->sockFd = -1;
}


void csc_udp_free(csc_udp_t *udp)
{	if (udp->errMsg)
		free(udp->errMsg);
	if (udp->addrSrc)
		free(udp->addrSrc);
	free(udp);
}


csc_bool_t csc_udp_setRcvAddr( csc_udp_t *udp       // UDP object.
							 , const char *addr    // NULL or IP of interface.
							 , int portNo         // Port number to serve on.
							 , int flags         // flags to recv sys call.
							 )
{   int result;
    char portStr[MaxPortNoStrSize + 1];
    struct addrinfo hints, *servInfo, *addrInfo; 

// Misc.
	udp->sockFd = -1;
	udp->flags = flags;

// The port number. 
    if (portNo<MinPortNo || portNo>MaxPortNo)
	{	udp_setErrMsg(udp, "udpSetRcvAddr: Invalid port number");
        return csc_FALSE;
    }
	udp->portSrc = portNo;

// The address.
    if (addr!=NULL && !csc_isValid_ipV4(addr) && !csc_isValid_ipV6(addr))
	{	udp_setErrMsg(udp, "udpSetRcvAddr: Invalid IP address");
        return csc_FALSE;
    }
	if (udp->addrSrc)
		free(udp->addrSrc);
	udp->addrSrc = csc_allocStr(addr);

// Set up the sockHints.
    memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
    hints.ai_socktype = SOCK_DGRAM;
 
// Resolve the address.
    result = getaddrinfo(NULL,portStr,&hints,&servInfo);
    if (result != 0)
    {   udp_setErrMsg(udp, "udpSetRcvAddr: getaddrinfo() failed");
        return csc_FALSE;
    }
    addrInfo = servInfo; 
 
// Get a socket for the connection.
    udp->sockfd = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
    if (sockfd == -1)
    {   udp_setErrMsg(udp, "udpSetRcvAddr: socket() failed");
        freeaddrinfo(servInfo);
        return csc_FALSE;
    }
 
// Bind the socket to the address.
    result = bind(sockfd, addrInfo->ai_addr, addrInfo->ai_addrlen);
    if (result != 0)
    {   setErrMsg(errCall, "udpSetRcvAddr: bind() failed");
        freeaddrinfo(servInfo);
		close(udp->sockfd);
		udp->sockfd = -1;
        return csc_FALSE;
    }
 
	freeaddrinfo(servInfo);
    return csc_TRUE;
}

