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
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "std.h"
#include "alloc.h"
#include "isvalid.h"

#include "udp.h"

// --------------------------------------------
// --------- UDP address to send --------------

typedef struct csc_udpAddr_t
{	int flags;
	char *errMsg;
	struct sockaddr_storage sockAddr;
	int isSet;
} csc_udpAddr_t;  // Address to send to.


static void udpAddr_setErrMsg(csc_udpAddr_t *this, char *errMsg, char *otherErrMsg)
{
// Free the old err msg.
	if (this->errMsg != NULL)
        free(this->errMsg);
 
// Allocate the new error message.
	if (otherErrMsg == NULL)
		this->errMsg = csc_alloc_str(errMsg);
	else
		this->errMsg = csc_alloc_str3(errMsg, ": ", otherErrMsg);
}


const char *csc_udpAddr_getErrMsg(const csc_udpAddr_t *this)
{   return this->errMsg;
}


const char *csc_udpAddr_resetErrMsg(csc_udpAddr_t *this)
{   if (this->errMsg != NULL)
    {   free(this->errMsg);
		this->errMsg = NULL;
	}
}


csc_udpAddr_t *csc_udpAddr_new()
{	csc_udpAddr_t *addr = csc_allocOne(csc_udpAddr_t);  assert(addr);
	addr->errMsg = NULL;
	addr->flags = 0;
	addr->isSet = csc_FALSE;
}


static csc_bool_t csc_udpAddr_setAdd( csc_udpAddr_t *addr
							 , struct sockaddr_storage *sockAddr
							 )
{	memcpy(&addr->sockAddr, sockAddr, sizeof (struct sockaddr_storage));
	addr->isSet = csc_TRUE;
}


csc_bool_t csc_udpAddr_setAddr(csc_udpAddr_t *addr, char *ipStr, int portNum)
{   int result;
    char portStr[MaxPortNoStrSize + 1];
    struct addrinfo hints, *servInfo=NULL, *addrInfo; 

// Clear out.
	addr->isSet = csc_FALSE;
 
// The port number. 
    if (portNum<MinPortNo || portNum>MaxPortNo)
	{	udpAddr_setErrMsg(addr, "csc_udpAddr_setAddr(): Invalid port number", NULL);
        return csc_FALSE;
    }
	sprintf(portStr, "%d", portNum);
 
// The IP address.
    if (  !csc_isValid_domain(ipStr)
	   && !csc_isValid_ipV4(ipStr)
	   && !csc_isValid_ipV6(ipStr)
	   )
	{	udpAddr_setErrMsg(addr, "csc_udpAddr_setAddr(): Invalid IP address", NULL);
        return csc_FALSE;
    }
 
// Set up the sockHints.
    memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
    hints.ai_socktype = SOCK_DGRAM;
 
// Resolve the address.
    result = getaddrinfo(ipStr,portStr,&hints,&servInfo);
    if (result != 0)
    {   udpAddr_setErrMsg(addr, "csc_udpAddr_setAddr(): getaddrinfo() failed", strerror(result));
        return csc_FALSE;
    }
    addrInfo = servInfo; 
 
// Copy in the address.
	memcpy(&addr->sockAddr, addrInfo->ai_addr, addrInfo->ai_addrlen);
	addr->isSet = csc_TRUE;
	return csc_TRUE;
}


char *csc_udpAddr_getAllocIpStr(csc_udpAddr_t *addr)
{	char ipStr[INET6_ADDRSTRLEN+1];
 
	if (!addr->isSet)
	{	return NULL;
	}
	else if (addr->sockAddr.ss_family == AF_INET)
	{	inet_ntop( addr->sockAddr.ss_family
				 , (const void*)&((struct sockaddr_in*)(&addr->sockAddr))->sin_addr
				 , ipStr
				 , INET6_ADDRSTRLEN
				 );
		ipStr[15] = '\0';
	}
	else if (addr->sockAddr.ss_family == AF_INET6)
	{	inet_ntop( addr->sockAddr.ss_family
				 , (const void*)&((struct sockaddr_in6*)(&addr->sockAddr))->sin6_addr 
				 , ipStr
				 , INET6_ADDRSTRLEN
				 );
		ipStr[INET6_ADDRSTRLEN] = '\0';
	}
	else
		assert(csc_FALSE);
 
	return csc_allocStr(ipStr);
}


int csc_udpAddr_getPortNum(csc_udpAddr_t *addr)
{	int portNum;
 
	if (!addr->isSet)
		portNum = -1;
	else if (addr->sockAddr.ss_family == AF_INET)
		portNum = ((struct sockaddr_in*)(&addr->sockAddr))->sin_port;
	else if (addr->sockAddr.ss_family == AF_INET6)
		portNum = ((struct sockaddr_in6*)(&addr->sockAddr))->sin6_port;
 
	return portNum;
}

	
void csc_udpAddr_free(csc_udpAddr_t *addr)
{	if (addr->errMsg)
		free(addr->errMsg);
	free(addr);
}


// --------------------------------------------
// ------------------ UDP ---------------------


typedef struct csc_udp_t
{	int flags;
	int portRcv;
	char *ipRcv;
	int sockfd;
	char *errMsg;
} csc_udp_t;


static void udp_setErrMsg(csc_udp_t *this, char *errMsg, char *otherErrMsg)
{
// Free the old err msg.
	if (this->errMsg != NULL)
        free(this->errMsg);
 
// Allocate the new error message.
	if (otherErrMsg == NULL)
		this->errMsg = csc_alloc_str(errMsg);
	else
		this->errMsg = csc_alloc_str3(errMsg, ": ", otherErrMsg);
}


const char *csc_udp_getErrMsg(const csc_udp_t *this)
{   return this->errMsg;
}


csc_udp_t *csc_udp_new()
{	csc_udp_t *udp = csc_allocOne(csc_udp_t);  assert(udp);
	udp->portRcv = -1;
	udp->ipRcv = NULL;
	udp->errMsg = NULL;
	udp->flags = 0;
	udp->sockfd = -1;
}


void csc_udp_free(csc_udp_t *udp)
{	if (udp->errMsg)
		free(udp->errMsg);
	if (udp->ipRcv)
		free(udp->ipRcv);
	if (udp->sockfd != -1)
		close(udp->sockfd);
	free(udp);
}


csc_bool_t csc_udp_setSrv( csc_udp_t *udp       // UDP object.
						 , const char *ipStr    // NULL or IP of interface.
						 , int portNo         // Port number to serve on.
						 )
{   int result;
    char portStr[MaxPortNoStrSize + 1];
    struct addrinfo hints, *servInfo=NULL, *addrInfo; 
 
// Clear the socket.
	if (udp->sockfd != -1)
		close(udp->sockfd);
	udp->sockfd = -1;
 
// The port number. 
    if (portNo<MinPortNo || portNo>MaxPortNo)
	{	udp_setErrMsg(udp, "csc_udp_setSrv(): Invalid port number", NULL);
        return csc_FALSE;
    }
	udp->portRcv = portNo;
	sprintf(portStr, "%d", portNo);
 
// The address.
    if (ipStr!=NULL && !csc_isValid_ipV4(ipStr) && !csc_isValid_ipV6(ipStr))
	{	udp_setErrMsg(udp, "csc_udp_setSrv(): Invalid IP address", NULL);
        return csc_FALSE;
    }
	if (udp->ipRcv)
		free(udp->ipRcv);
	udp->ipRcv = csc_allocStr(ipStr);
 
// Set up the sockHints.
    memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
    hints.ai_socktype = SOCK_DGRAM;
 
// Resolve the address.
    result = getaddrinfo(ipStr,portStr,&hints,&servInfo);
    if (result != 0)
    {   udp_setErrMsg(udp, "csc_udp_setSrv(): getaddrinfo() failed", strerror(result));
        return csc_FALSE;
    }
    addrInfo = servInfo; 
 
// Get a socket for the connection.
    udp->sockfd = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
    if (udp->sockfd == -1)
    {   udp_setErrMsg(udp, "csc_udp_setSrv(): socket() failed", strerror(result));
        freeaddrinfo(servInfo);
        return csc_FALSE;
    }
 
// Bind the socket to the address.
    result = bind(udp->sockfd, addrInfo->ai_addr, addrInfo->ai_addrlen);
    if (result != 0)
    {   udp_setErrMsg(udp, "csc_udp_setSrv(): bind() failed", strerror(result));
        freeaddrinfo(servInfo);
		close(udp->sockfd);
		udp->sockfd = -1;
        return csc_FALSE;
    }
 
	freeaddrinfo(servInfo);
    return csc_TRUE;
}
 


csc_bool_t csc_udp_setCli(csc_udp_t *udp, int ipType)
{   int result;
    char portStr[MaxPortNoStrSize + 1];
    struct addrinfo hints, *servInfo=NULL, *addrInfo; 
 
// Clear the socket.
	if (udp->sockfd != -1)
		close(udp->sockfd);
	udp->sockfd = -1;
 
// Get a socket for the connection.
    udp->sockfd = socket(ipType, SOCK_DGRAM, 0);
    if (udp->sockfd == -1)
    {   udp_setErrMsg(udp, "csc_udp_setCli(): socket() failed", strerror(result));
        return csc_FALSE;
    }
 
    return csc_TRUE;
}


csc_bool_t csc_udp_snd( csc_udp_t *udp          // UDP object.
					  , char *buf, int msgLen   // Buffer to read into.
					  , csc_udpAddr_t *addr  // Address.
					  )
{	socklen_t addrLen = sizeof(addr->sockAddr);

// Check that the address is valid.
	if (!addr->isSet)
    {   udp_setErrMsg(udp, "csc_udp_snd(): address not set", NULL);
		return csc_FALSE;
	}

// Send the packet.
	int nSent = sendto( udp->sockfd
					  , buf, msgLen
					  , addr->flags
					  , (struct sockaddr *)&addr->sockAddr, addrLen
					  );
	if (nSent != msgLen)
    {   udp_setErrMsg(udp, "csc_udp_snd(): sendto() failed", strerror(errno));
		return csc_FALSE;
	}

	return csc_TRUE;
}


// On success returns number of bytes read.  On error returns -1.
int csc_udp_rcv( csc_udp_t *udp          // UDP object.
			   , char *buf, int bufSiz   // Buffer to read into.
			   , csc_udpAddr_t **addr  // Address to assign allocated or NULL.
			   )
{	int numRead;
	struct sockaddr_storage caller;
	
// Receive the packet.
	socklen_t addrLen = sizeof(caller);
	numRead = recvfrom( udp->sockfd
					  , buf, bufSiz
					  , udp->flags
					  , (struct sockaddr *)&caller, &addrLen
					  );
	if (numRead == -1)
    {   udp_setErrMsg(udp, "csc_udp_setRcvAddr(): recvform() failed", NULL);
		if (udp->sockfd)
			close(udp->sockfd);
		return -1;
	}
	buf[numRead] = '\0';
 
// Address packet received from.
	if (addr != NULL)
	{	if (*addr != NULL)
		{	csc_udpAddr_free(*addr);
		}
		*addr = csc_udpAddr_new();
		csc_udpAddr_setAdd(*addr, &caller);
	}

// Bye.
	return numRead;
}

