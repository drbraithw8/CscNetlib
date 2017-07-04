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
    csc_cli_t *ntp;

// Make the connection.
    ntp = csc_cli_new();
    csc_cli_setServAddr(ntp, "TCP", "example.com", 80);
    fdes = csc_cli_connect(ntp);
    csc_cli_free(ntp);

// Send a request.
    fp = fdopen(fdes, "r+");
    fprintf( fp,
			"GET /index.html HTTP/1.0\n\n"
		   );
    fflush(fp);

// Get the response.
	isFin = csc_FALSE;
	while(!isFin)
	{	int lineLen = csc_fgetline(fp, line, LINE_MAX);
		if (lineLen < 0)
		{	printf("EOF\n");
			isFin = csc_TRUE;
		}
		else
		{	printf("Got: \"%s\"\n", line);
			// if (csc_streq(line,""))
			// {	isFin = csc_TRUE;
			// }
		}
	}
    fclose(fp);

// Bye.
    exit(0);
}

