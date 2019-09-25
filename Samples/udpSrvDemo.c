#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <CscNetLib/std.h>
#include <CscNetLib/alloc.h>
#include <CscNetLib/isvalid.h>
#include <CscNetLib/udp.h>


#define MaxBufLen 100
int main(int argc, char **argv)
{   char buf[MaxBufLen+1];
    int ret, nRead;

// Resources.
    csc_udp_t *udp = NULL;
    csc_udpAddr_t *addr = NULL;
    char *ipStr = NULL;

// Set up the server.
    udp = csc_udp_new();  assert(udp);
    ret = csc_udp_setSrv(udp, "127.0.0.1", 9992);
    if (!ret)
    {   fprintf(stderr, "Error: %s.\n", csc_udp_getErrMsg(udp));
        perror(NULL);
        goto freeAll;
    }

// Receive the packet.
    nRead = csc_udp_rcv(udp, buf, MaxBufLen, &addr);
    if (nRead==-1 || addr==NULL)
    {   fprintf(stderr, "Error: %s.\n", csc_udp_getErrMsg(udp));
        goto freeAll;
    }

// Print out.
    buf[nRead] = '\0';
    ipStr = csc_udpAddr_getAllocIpStr(addr);
    int portNum = csc_udpAddr_getPortNum(addr);
    printf("Got from %s %d \"%s\"\n", ipStr, portNum, buf);

// Respond.
    sprintf(buf, "%s %d", ipStr, portNum);
    ret = csc_udp_snd(udp, buf, strlen(buf), addr);
    if (!ret)
    {   fprintf(stderr, "Error: %s.\n", csc_udp_getErrMsg(udp));
        goto freeAll;
    }

// Free resources.
freeAll:
    if (ipStr)
        free(ipStr);
    if (addr)
        csc_udpAddr_free(addr);
    if (udp)
        csc_udp_free(udp);
    exit(0);
}

 
    
