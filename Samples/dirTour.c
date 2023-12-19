#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <CscNetLib/dtour.h>


void dir_begin(char *pth, char *name, struct stat *info, void *ctx)
{	printf("cd %s\n", name);
}


void dir_file(char *pth, char *name, struct stat *info, void *ctx)
{	printf("ls -l %s\n", name);
}

void dir_end(char *pth, char *name, struct stat *info, void *ctx) 
{	printf("cd ..\n");
}


char *progname;

void usage(void)
{	fprintf(stderr, "Usage: %s dirPath\n", progname);

	fprintf(stderr, "\n%s\n", 
	"This program will produce a list of commands that will recursively list\n"
	"show the details of files in the directory tree 'dirPath'.\n"
		);
	exit(1);
}



int main(int argc, char **argv)
{	int retval;		
	char *path;
	progname = argv[0];

	if (argc != 2)
		usage();

	path = argv[1];

	retval = csc_dtour(path, 0, dir_begin, dir_file, dir_end, NULL);
	return retval;
}


