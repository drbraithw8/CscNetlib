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

static void setErrMsg(const char **errMsgCaller, const char *errMsg)
{	if (errMsgCaller != NULL)
		*errMsgCaller = errMsg;
}


static void *get_in_addr(struct sockaddr *sa)
{	if (sa->sa_family == AF_INET)
	{	return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int csc_udpRcv_sock( const char *addr    // NULL or the IP of an interface.
                   , int portNo         // Port number to serve on.
				   , char const **errCall
                   )   
{   int result;
    char portStr[MaxPortNoStrSize + 1];
    struct addrinfo hints, *servInfo, *addrInfo; 
    int sockfd;
 
// Check the port number. 
    if (portNo<MinPortNo || portNo>MaxPortNo)
    {   setErrMsg(errCall, "netSrvUdp: Invalid port number");
        return -1;
    }
    sprintf(portStr, "%d", portNo);
 
// Check the address.
    if (addr!=NULL && !csc_isValid_ipV4(addr) && !csc_isValid_ipV6(addr))
    {   setErrMsg(errCall, "SrvUdp: Invalid IP address");
        return -1;
    }
 
// Set up the sockHints.
    memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
    hints.ai_socktype = SOCK_DGRAM;
 
// Resolve the address.
    result = getaddrinfo(NULL,portStr,&hints,&servInfo);
    if (result != 0)
    {   setErrMsg(errCall, "SrvUdp: getaddrinfo() failed");
        return -1;
    }
    addrInfo = servInfo; 
 
// Get a socket for the connection.
    sockfd = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
    if (sockfd == -1)
    {   setErrMsg(errCall, "SrvUdp: socket() failed");
        freeaddrinfo(servInfo);
        return -1;
    }
 
// Bind the socket to the address.
    result = bind(sockfd, addrInfo->ai_addr, addrInfo->ai_addrlen);
    if (result != 0)
    {   setErrMsg(errCall, "SrvUdp: bind() failed");
        freeaddrinfo(servInfo);
		close(sockfd);
        return -1;
    }
 
	freeaddrinfo(servInfo);
    return sockfd;
}

