#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>

#include <CscNetLib/std.h>
#include <CscNetLib/alloc.h>
#include <CscNetLib/isvalid.h>
#include <CscNetLib/udp.h>


void usage(char *progName)
{	fprintf(stderr, "Usage: %s ipAddr port msg\n\n", progName);
	exit(1);
}


#define MaxBufLen 100
int main(int argc, char **argv)
{	int ret, nRead;
	char buf[MaxBufLen+1];

// Resources.
	csc_udp_t *udp = NULL;
	csc_udpAddr_t *addr = NULL;
	char *ipStr = NULL;

// Check the args.  TODO: really check the args.
	if (argc != 4)
		usage(argv[0]);

// Port Num.
	if (!csc_isValid_int(argv[2]))
		usage(argv[0]);
	int portNum = atoi(argv[2]);

// Make the address.
	addr = csc_udpAddr_new();
	ret = csc_udpAddr_setAddr(addr, argv[1], portNum);
	if (!ret)
	{	fprintf(stderr, "Error: %s.\n", csc_udpAddr_getErrMsg(addr));
		goto freeAll;
	}

// Set up the client.
	udp = csc_udp_new();  assert(udp);
	ret = csc_udp_setCli(udp, AF_INET);
	if (!ret)
	{	fprintf(stderr, "Error: %s.\n", csc_udp_getErrMsg(udp));
		goto freeAll;
	}

// Set the timeout for receiving packets.
	ret = csc_udp_setRcvTimeout(udp, 3);
	if (!ret)
	{	fprintf(stderr, "Error: %s.\n", csc_udp_getErrMsg(udp));
		goto freeAll;
	}

// Send the packet.
	ret = csc_udp_snd(udp, argv[3], strlen(argv[3]), addr);
	if (!ret)
	{	fprintf(stderr, "Error: %s.\n", csc_udp_getErrMsg(udp));
		goto freeAll;
	}

// Receive the response.
	nRead = csc_udp_rcv(udp, buf, MaxBufLen, NULL);
	if (nRead == -2)
	{	fprintf(stderr, "Receive packet timed out.\n");
		goto freeAll;
	}
	else if (nRead == -1)
	{	fprintf(stderr, "Error: %s.\n", csc_udp_getErrMsg(udp));
		goto freeAll;
	}

// Print the result.
	buf[nRead] = '\0';
	printf("Got: %s\n", buf);

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

 
	
