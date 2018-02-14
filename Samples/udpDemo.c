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
{	char buf[MaxBufLen+1];
	int ret, nRead;

// Resources.
	csc_udp_t *udp = NULL;
	csc_udpAddr_t *addr = NULL;
	char *ipStr = NULL;

// Receive the packet.
	udp = csc_udp_new();  assert(udp);
	ret = csc_udp_setRcvAddr(udp, "127.0.0.1", 9992, 0);
	if (!ret)
	{	fprintf(stderr, "Error: %s.\n", csc_udp_getErrMsg(udp));
		perror(NULL);
		goto freeAll;
	}
	nRead = csc_udp_rcv(udp, buf, MaxBufLen, &addr);  assert(ret); assert(addr);

// Print out.
	buf[nRead] = '\0';
	ipStr = csc_udpAddr_getAllocIpStr(addr);
	int portNum = csc_udpAddr_getPortNum(addr);
	printf("Got from %s %d \"%s\"\n", ipStr, portNum, buf);

// Free resources.
freeAll:
	free(ipStr);
	csc_udpAddr_free(addr);
	csc_udp_free(udp);
	exit(0);
}

 
	
