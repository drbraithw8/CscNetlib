#include <CscNetLib/netSrv.h>
#include <CscNetLib/std.h>

#define MaxLineLen 255
int main(int argc, char **argv)
{   csc_srv_t *ntp = NULL;
    int fd = -1;
    FILE *fin = NULL;
    char line[MaxLineLen+1];
 
// Create netSrv object.
    ntp = csc_srv_new();
    csc_srv_setAddr(ntp, "TCP", "127.0.0.1", 9991, -1);

// For each successful connection.
    while ((fd = csc_srv_accept(ntp)) >= 0)
    {   fprintf(stdout, "Connection from %s\n", csc_srv_acceptAddr(ntp));
        fin = fdopen(fd, "r");
        csc_fgetline(fin,line,MaxLineLen);
        fprintf(stdout, "Got line: \"%s\"\n", line);
        fclose(fin);
    }
 
    csc_srv_free(ntp);
	exit(0);
}

 
        


