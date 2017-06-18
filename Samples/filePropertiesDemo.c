#include <stdio.h>
#include <stdlib.h>
#include <CscNetLib/isvalid.h>
#include <CscNetLib/fileProperties.h>
#include <CscNetLib/std.h>


void usage(char *progName)
{   fprintf(stderr, "\nUsage: %s <filePath>\n", progName);
    exit(1);
}

 
int main(int argc, char **argv)
{   char *filePath;

// Check that the arguments are reasonable.
    if (argc != 2)
        usage(argv[0]);
    filePath = argv[1];

// Does this even look like a reasonable file name.
    if (!csc_isValid_decentPath(filePath))
    {   fprintf(stderr, "Error: Bad file path\n");
        usage(argv[0]);
    }
 
    csc_fileProp_t *fprops = csc_fileProp_new(filePath);
 

    if (!csc_fileProp_isExist(fprops))
        printf("File \"%s\" does not exist.\n", filePath);
 
    if (!csc_fileProp_isOK(fprops))
    {   printf("Error getting info about file \"%s\": %s\n",
                filePath, csc_fileProp_getErrMsg(fprops));
        csc_fileProp_free(fprops);
        exit(1);
    }
 
    if (csc_fileProp_isRegFile(fprops))
    {   printf("File \"%s\" is a regular file.\n", filePath);
        printf("File \"%s\" has %lld bytes.\n",
            filePath, (long long) csc_fileProp_fileSize(fprops));
    }
    else if (csc_fileProp_isDir(fprops))
        printf("File \"%s\" is a directory.\n", filePath);
    else
        printf("File \"%s\" is something else.\n", filePath);
 
    csc_fileProp_free(fprops);
    exit(0);
}
 
