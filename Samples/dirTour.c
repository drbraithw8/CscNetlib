#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <CscNetLib/dtour.h>


void dir_begin(char *pth, char *name, struct stat *info)
{	printf("cd %s\n", name);
}


void dir_file(char *pth, char *name, struct stat *info)
{	printf("ls -l %s\n", name);
}

void dir_end(char *pth, char *name, struct stat *info) 
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

	retval = csc_dtour(path, 0, dir_begin, dir_file, dir_end);
	return retval;
}


# if 0

/*	This parameter may have to be changed.  If so.. recompile. */
#define DT_MAXPATH 4096	/* Maximum path size dir_tour() can cope with. */

/*	Values for 'flags' to be OR'ed. */
#define DT_NOSILENT 1		/* Do we want filenames etc beginning with '.'. */
#define DT_SPECIAL 2	/* Do we want to look at fifos, block special etc. */
#define DT_LINKS 4	/* Symbolic links will be ignored if this bit is not set. */
#define DT_GOOD 0

int dir_tour
(   const char *path,
	int flags,  	/* What types of files are to be included. */
    void (*dir_begin)(char *pth, char *name, struct stat *info),
    void (*dir_file)(char *pth, char *name, struct stat *info),
    void (*dir_end)(char *pth, char *name, struct stat *info) 
);
/*  This function will recursively tour a directory tree whose head 
 * is the string 'path'.  It calls 'dir_begin'() before descending into 
 * a directory.  It calls 'dir_file'() for each file met.  It calls 
 * 'dir_end'() after each directory has been visited.  Any of
 * 'dir_begin'(), 'dir_file'() and 'dir_end'() that is NULL will
 * not be called.
 *   If 'path' is the name of an ordinary file, then 'dir_file'() will 
 * be called for that file (i.e. 'dir_begin'() and 'dir_end'() will not 
 * be called).  Otherwise if 'path' is the name of an ordinary 
 * subdirectory, then 'dir_begin'() will be called before visiting 
 * 'path' and 'dir_end'() will be called after resurfacing.  
 *   In any directory, all the ordinary files will be visited first in 
 * ASCII order.  Then all the directories will be visited recursively 
 * in ASCII order.
 *   The arguments for 'dir_begin'(), 'dir_end'() and 'dir_file'() are 
 * as follows:-  If 'path' is a full path name, then each 'pth' will be 
 * full path names for each file/directory.  If 'path' is a path name 
 * from the current directory, then each 'pth' will be a path name from 
 * the current directory.  'name' is the file name only of the 
 * file/directory.  'info' will point to a block such as provided by 
 * stat() holding various information about the file/directory 
 * including last modification dates, attributes and file sizes.
 * 	dir_tour() returns 0 on success.
 */ 

#endif
