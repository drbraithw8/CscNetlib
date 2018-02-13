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

#include <CscNetLib/std.h>
#include <CscNetLib/alloc.h>
#include <CscNetLib/isvalid.h>
#include <CscNetLib/udp.h>



#define MaxBufLen 100
int main(int argc, char **argv)
{	int numRead;
	char buf[MaxBufLen+1];
	struct sockaddr_storage callerAddr;
	char s[INET6_ADDRSTRLEN];
	const char *errMsg;
	
// Get the socket.
	int sockfd = csc_udpRcv_sock( "127.0.0.1", 9992, &errMsg);
	if (sockfd == -1)
	{	fprintf(stderr, "Error: %s\n", errMsg);
		exit(1);
	}
 
// Get a packet.
	socklen_t addrLen = sizeof(callerAddr);
	numRead = recvfrom(sockfd, buf, MaxBufLen, 0, (struct sockaddr *)&callerAddr, &addrLen);
	if (numRead == -1)
	{	fprintf(stderr, "Error: %s\n", "Failed recieve");
		close(sockfd);
		exit(1);
	}
 
// Report what we got, and where from.
    printf("Got packet from %s\n",
        inet_ntop(callerAddr.ss_family,
            get_in_addr((struct sockaddr *)&callerAddr),
            s, sizeof s));
    printf("Packet is %d bytes long\n", numRead);
    buf[numRead] = '\0';
    printf("Packet contains \"%s\"\n", buf);
}
	
