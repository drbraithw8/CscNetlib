
// One of the following two lines should be commented out:-
#define netSrvDo(s,l,d,g,m)  csc_srvDo_oneByOne(s,l,d,g)
// #define netSrvDo(s,l,d,g,m)  csc_srvDo_forking(s,l,d,g,m)

#include <CscNetLib/std.h>
#include <CscNetLib/netSrvDo.h>


#define MaxLineLen 255
#define MaxThreads 3

int doConn(int fd, const char *clientIp, void *global)
{   char line[MaxLineLen+1];
    fprintf(stdout, "Servicing connection from \"%s\"\n", clientIp);
    FILE *fp = fdopen(fd, "rb+");
    csc_fgetline(fp,line,MaxLineLen);
    fprintf(stdout, "Got line: \"%s\"\n", line);
    fprintf(fp, "Well Howdy!\n"); fflush(fp);
    fclose(fp);
}


int main(int argc, char **argv)
{   int retVal;
 
// Resources.
    csc_srv_t *srv = NULL;
    csc_log_t *log = NULL;
 
// Create netSrv object.
    srv = csc_srv_new();
    csc_srv_setAddr(srv, "TCP", "127.0.0.1", 9991, -1);
 
// Open the logging channel.
    log = csc_log_new("test.log", csc_log_TRACE);
 
// For each successful connection.
    retVal = netSrvDo(srv, log, doConn, NULL, MaxThreads);
    if (retVal)
        fprintf(stderr, "Server Terminated OK.\n");
    else
        fprintf(stderr, "Server Terminated. Error =\"%s\"\n", csc_srv_getErrMsg(srv));
 
// Free resources.
    if (srv != NULL)
        csc_srv_free(srv);
    if (log != NULL)
        csc_log_free(log);

    exit(0);
}

