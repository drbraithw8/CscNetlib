#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <CscNetLib/netCli.h>
#include <CscNetLib/std.h>

#define LINE_MAX 99


int main(int argc, char **argv)
{   char line[LINE_MAX+1];
    int fdes;
	int isFin;
    FILE *fp;
	int lineNo;
    csc_cli_t *ntp;

// Make the connection.
    ntp = csc_cli_new();
    csc_cli_setServAddr(ntp, "TCP", "example.com", 80);
    fdes = csc_cli_connect(ntp);
    csc_cli_free(ntp);


// Make FILE* of file descriptor.
    fp = fdopen(fdes, "r+");

// Data to send.
	const char *sendData[] = 
	{ "GET /index.html HTTP/1.1"
	, "Host: localhost" 
	, "Connection: close" 
	, "" 
	};

// Send a request.
	lineNo = 0;
	for (int i=0; i<csc_dim(sendData); i++)
	{	fprintf(fp, "%s\n", sendData[i]);
		fprintf(stdout, "Sent %3d \"%s\"\n", ++lineNo, sendData[i]);
	}
    fflush(fp);

// Get the response.
	isFin = csc_FALSE;
	lineNo = 0;
	while(!isFin)
	{	int lineLen = csc_fgetline(fp, line, LINE_MAX);
		if (lineLen < 0)
		{	printf("EOF\n");
			isFin = csc_TRUE;
		}
		else
		{	printf("Got %3d \"%s\"\n", ++lineNo, line);
			// if (csc_streq(line,""))
			// {	isFin = csc_TRUE;
			// }
		}
	}
    fclose(fp);

// Bye.
    exit(0);
}

