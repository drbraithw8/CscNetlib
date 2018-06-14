#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <CscNetLib/netSrv.h>
#include <CscNetLib/std.h>

#define MaxLineLen 255
int main(int argc, char **argv)
{   int fd0;
    char line[MaxLineLen+1];
 
// Create netSrv object.
    csc_srv_t *ntp = csc_srv_new();    assert(ntp!=NULL);
    int ret = csc_srv_setAddr(ntp, "127.0.0.1", 9991, -1); assert(ret);

// For each successful connection.
    while ((fd0 = csc_srv_accept(ntp)) >= 0)
    {   fprintf(stdout, "Connection from %s\n", csc_srv_acceptAddr(ntp));

	// Convert file descriptor to input and output streams.
		int fd1 = dup(fd0);               assert(fd1!=-1);
        FILE *tcpIn = fdopen(fd0, "r");   assert(tcpIn!=NULL);
        FILE *tcpOut = fdopen(fd1, "w");  assert(tcpOut!=NULL);

	// Do input/output.
        csc_fgetline(tcpIn,line,MaxLineLen);
        fprintf(stdout, "Got line: \"%s\"\n", line);
        fprintf(tcpOut, "You said \"%s\"\n", line);
		fflush(tcpOut);

	// Close the streams.
        fclose(tcpOut);
        fclose(tcpIn);
    }
 
// Free resources.
    csc_srv_free(ntp);
    exit(0);
}

 
        


