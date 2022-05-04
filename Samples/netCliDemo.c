#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <CscNetLib/netCli.h>
#include <CscNetLib/std.h>

#define LINE_MAX 99



void work(const char *ipStr, int portNum)
{   char line[LINE_MAX+1];
    int isFin;
    int lineNo;
 
// Make the connection.
    csc_cli_t *ntp = csc_cli_new();
    int fdes0 = csc_cli_mkConn(ntp, ipStr, portNum); assert(fdes0!=-1);
    csc_cli_free(ntp);
 
// Convert file descriptor to input and output streams.
    int fdes1 = dup(fdes0);         assert(fdes1!=-1);
    FILE *fin = fdopen(fdes0,"r");  assert(fin!=NULL);
    FILE *fout = fdopen(fdes1,"w"); assert(fout!=NULL);
 
// Data to send.
    const char *sendData[] = 
    { "GET /index.html HTTP/1.1"
    , "Host: example.com" 
    , "Connection: close" 
    , "" 
    };
 
// Send a request.
    lineNo = 0;
    for (int i=0; i<csc_dim(sendData); i++)
    {   fprintf(fout, "%s\n", sendData[i]);
        fprintf(stdout, "Sent %3d \"%s\"\n", ++lineNo, sendData[i]);
    }
    fflush(fout);
 
// Get the response.
    isFin = csc_FALSE;
    lineNo = 0;
    while(!isFin)
    {   int lineLen = csc_fgetline(fin, line, LINE_MAX);
        if (lineLen < 0)
        {   printf("EOF\n");
            isFin = csc_TRUE;
        }
        else
        {   printf("Got %3d \"%s\"\n", ++lineNo, line);
            // if (csc_streq(line,""))
            // {    isFin = csc_TRUE;
            // }
        }
    }
 
// Close input and output streams.
    fclose(fin);
    fclose(fout);
 
// Bye.
    exit(0);
}


int main(int argc, char **argv)
{	assert(argc == 3);
	work(argv[1], atoi(argv[2]));
}
