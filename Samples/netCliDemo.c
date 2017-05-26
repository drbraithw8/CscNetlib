#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <CscNetLib/std.h>
#include <CscNetLib/netCli.h>

#define LINE_MAX 99


int main(int argc, char **argv)
{   char line[LINE_MAX+1];
    int fdes;
	int isFin = 0;
    FILE *fp;
    csc_cli_t *ntp;

// Make the connection.
    ntp = csc_cli_new();
    csc_cli_setServAddr(ntp, "TCP", "www.usq.edu.au", 80);
    fdes = csc_cli_connect(ntp);
    csc_cli_close(ntp);

// Copy one line.
    fp = fdopen(fdes, "r+");
    fprintf(fp, "GET /index.html HTTP/1.0\n\n");
    fflush(fp);
	while(!isFin)
	{	csc_fgetline(fp, line, LINE_MAX);
		printf("Got: \"%s\"\n", line);
		if (csc_streq(line,""))
			isFin = csc_TRUE;
	}
    fclose(fp);

// Bye.
    exit(0);
}

