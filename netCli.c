// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#ifndef csc_CLI_H
#define csc_CLI_H 1

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include "std.h"
#include "alloc.h"
#include "isvalid.h"
#include "netCli.h"

#define MinPortNo 1
#define MaxPortNo 65535
#define MaxPortNoStrSize 5


typedef struct csc_cli_t
{   int conType;
    char *errMsg;
    int portNo;
    struct addrinfo *servAddresses; 
    struct addrinfo sockHints;
} csc_cli_t ;



csc_cli_t *csc_cli_new()
{ 
// Allocate the structure and fill in the data.
    csc_cli_t *this = csc_allocOne(csc_cli_t);
    this->errMsg = NULL;
    this->servAddresses = NULL; 
 
// Return the goods.
    return this;
}


static void setErrMsg(csc_cli_t *this, char *newErrMsg)
{   if (this->errMsg != NULL)
        free(this->errMsg);
    this->errMsg = newErrMsg;
}


int csc_cli_setServAddr(csc_cli_t *this, const char *conType, const char *addr, int portNo)
{   int result;
    char portStr[MaxPortNoStrSize + 1];
 
// Check the connection type.
    if (csc_streq(conType,"UDP"))
        this->conType = SOCK_DGRAM; // UDP sockets
    else if (csc_streq(conType,"TCP"))
        this->conType = SOCK_STREAM; // TCP stream sockets
    else 
    {   setErrMsg(this, csc_alloc_str("csc_cli_setServAddr(): Invalid connection type"));
        return 0;
    }
 
// Set up the sockHints.
    memset(&this->sockHints, 0, sizeof(this->sockHints)); // Make sure the struct is empty.
    this->sockHints.ai_family = AF_UNSPEC;     // Don't care IPv4 or IPv6..
    this->sockHints.ai_flags = AI_PASSIVE;     // Fill in my IP for me.
    this->sockHints.ai_socktype = this->conType; // TCP stream sockets.
 
// Check the port number. 
    if (portNo<MinPortNo || portNo>MaxPortNo)
    {   setErrMsg(this, csc_alloc_str("csc_cli_setServAddr(): Invalid port number"));
        return 0;
    }
    sprintf(portStr, "%d", portNo);
 
// Check the address.
    if ( addr == NULL
        || (   !csc_isValid_ipV4(addr)
            && !csc_isValid_ipV6(addr)
            && !csc_isValid_domain(addr)
            )
        )
    {   setErrMsg(this, csc_alloc_str("netcli_setServAddr(): Invalid domain or IP address"));
        return 0;
    }
 
// Free old address resolution results
    if (this->servAddresses != NULL)
        freeaddrinfo(this->servAddresses);
 
// Resolve the address.
    result = getaddrinfo(addr,portStr,&this->sockHints,&this->servAddresses);
    if (result != 0)
    {   setErrMsg(this, csc_alloc_str3("netcli_setServAddr(): getaddrinfo():"
                                  , gai_strerror(result), NULL));
        return 0;
    }
 
// Make sure we have a server address to connect to.
    if (this->servAddresses == NULL)
    {   setErrMsg(this, csc_alloc_str("netcli_setServAddr(): "
                                  "No server addresses to connect to"));
        return 0;
    }
 
    return 1;
}


int csc_cli_connect(csc_cli_t *this)
{   struct addrinfo *res = this->servAddresses; 
    int sockfd;
    int result;
 
// Get a socket for the connection.
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1)
    {   setErrMsg(this, csc_alloc_str3("netcli_connect(): obtaining socket: ", strerror(errno), NULL));
        return -1;
    }
 
// Make the connection.
    result = connect(sockfd, res->ai_addr, res->ai_addrlen);
    if (result == -1)
    {   setErrMsg(this, csc_alloc_str3("netcli_connect(): ", strerror(errno), NULL));
        return -1;
    }
 
    return sockfd;
}


void csc_cli_free(csc_cli_t *this)
{   
// Free any error message.
    if (this->errMsg != NULL)
        free(this->errMsg);
    
// Free the address resolution results
    if (this->servAddresses != NULL)
        freeaddrinfo(this->servAddresses);
 
// Free the parent structure.
    free(this);
}


const char *csc_cli_getErrMsg(const csc_cli_t *this)
{   return this->errMsg;
}


#endif
