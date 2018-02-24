// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

// ======= netSrv ================================
// Listen for and accept connections from clients.
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
#include <sys/wait.h>
#include <sys/stat.h>

#include "std.h"
#include "alloc.h"
#include "isvalid.h"
#include "netSrv.h"

#define MinPortNo 1
#define MaxPortNo 65535
#define MaxPortNoStrSize 5


typedef struct csc_srv_t 
{   char *errMsg;
    int portNo;
    struct addrinfo *servAddresses; 
    struct addrinfo sockHints;
    struct sockaddr cliDetails;
    char cliAddr[INET6_ADDRSTRLEN+1];
    int listenSock;
} csc_srv_t ;


csc_srv_t *csc_srv_new()
{   char portNo[10];
 
// Allocate the structure and fill in the data.
    csc_srv_t *this = csc_allocOne(csc_srv_t);
    this->errMsg = NULL;
    this->servAddresses = NULL; 
 
// Return the goods.
    return this;
}


static void setErrMsg(csc_srv_t *this, char *newErrMsg)
{   if (this->errMsg != NULL)
        free(this->errMsg);
    this->errMsg = newErrMsg;
}


const char *csc_srv_getErrMsg(const csc_srv_t *this)
{   return this->errMsg;
}


int csc_srv_setAddr(csc_srv_t *this, const char *addr, int portNo, int backlog)
{   int result;
    char portStr[MaxPortNoStrSize + 1];
    struct addrinfo *addrInfo; 
    int sockfd;
 
// Check the port number. 
    if (portNo<MinPortNo || portNo>MaxPortNo)
    {   setErrMsg(this, csc_alloc_str("netSrv: Invalid port number"));
        return 0;
    }
    sprintf(portStr, "%d", portNo);
 
// Check the address.
    if (addr!=NULL && !csc_isValid_ipV4(addr) && !csc_isValid_ipV6(addr))
    {   setErrMsg(this, csc_alloc_str("Srvcli: Invalid IP address"));
        return 0;
    }
 
// Free old address resolution results
    if (this->servAddresses != NULL)
        freeaddrinfo(this->servAddresses);
 
// Set up the sockHints.
    memset(&this->sockHints, 0, sizeof(this->sockHints)); // make sure the struct is empty
    this->sockHints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    this->sockHints.ai_flags = AI_PASSIVE;     // fill in my IP for me
    this->sockHints.ai_socktype = SOCK_STREAM;
 
// Resolve the address.
    result = getaddrinfo(addr,portStr,&this->sockHints,&this->servAddresses);
    if (result != 0)
    {   setErrMsg(this, csc_alloc_str3("getaddrinfo:", gai_strerror(result), NULL));
        return 0;
    }
    addrInfo = this->servAddresses; 
 
// Get a socket for the connection.
    sockfd = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
    if (sockfd == -1)
    {   setErrMsg(this, csc_alloc_str3("socket: ", strerror(errno), NULL));
        return 0;
    }
    this->listenSock = sockfd;
 
// Bind the socket to the address.
    result = bind(sockfd, addrInfo->ai_addr, addrInfo->ai_addrlen);
    if (result != 0)
    {   setErrMsg(this, csc_alloc_str3("bind:", strerror(errno), NULL));
        return 0;
    }
 
// Set the socket properties to listen, and set the backlog.
    if (backlog < 1)
        backlog = 10;
    result = listen(sockfd, backlog);
    if (result != 0)
    {   setErrMsg(this, csc_alloc_str3("listen:", strerror(errno), NULL));
        return 0;
    }
 
// Return the result.
    return 1;
}


int csc_srv_accept(csc_srv_t *this)
{   int cliDetailsSize = sizeof(this->cliDetails);
    int rwSock;
 
// Accept the connection.
    rwSock = accept(this->listenSock, &this->cliDetails, &cliDetailsSize);
    if (rwSock == -1)
    {   setErrMsg(this, csc_alloc_str3("accept:", strerror(errno), NULL));
        if (errno == EINTR)
            rwSock = -2;
    }
 
// Return the socket or error indication.
    return rwSock;
}


const char *csc_srv_acceptAddr(csc_srv_t *this)
{
// Get the IP number.
    int result = getnameinfo( &this->cliDetails, sizeof(this->cliDetails),
                              this->cliAddr, sizeof(this->cliAddr),
                              0,0,NI_NUMERICHOST
                            );
    if (result != 0) 
        return NULL;
 
// Terminate the IP number.
    this->cliAddr[INET6_ADDRSTRLEN] = '\0';
    
// return the string.
    return this->cliAddr;
}


void csc_srv_free(csc_srv_t *this)
{   close(this->listenSock);
 
// Free any error message.
    if (this->errMsg != NULL)
        free(this->errMsg);
    
// Free the address resolution results
    if (this->servAddresses != NULL)
        freeaddrinfo(this->servAddresses);
 
// Free the parent structure.
    free(this);
}


void csc_srv_daemonise(csc_log_t *log)
{   pid_t pid, sid;
 
// Fork the Parent Process
    pid = fork();
    if (pid < 0)
    {   csc_log_str(log, csc_log_FATAL,
                    "srvBase_daemonise() Forking failed");
        exit(1);
    }
 
// Close the Parent Process
    if (pid > 0)
    {   exit(1);
    }
 
// Change File Mask
    umask(0);
 
// Create a new session id.
    sid = setsid();
    if (sid < 0)
    {   csc_log_str(log, csc_log_FATAL,
                    "srvBase_daemonise() Create session id failed");
        exit(1);
    }
 
// Change Directory
    if ((chdir("/")) < 0)
    {   csc_log_str(log, csc_log_FATAL,
                    "srvBase_daemonise() Change directory failed");
        exit(1);
    }
 
// Close Standard File Descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

// Announce success.
    csc_log_str(log, csc_log_NOTICE, "Daemonisation successful");
}

