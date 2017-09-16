
#include <stdlib.h>
#include <stdio.h>

#include <CscNetLib/std.h>
#include <CscNetLib/iniFile.h>

#define MaxLineLen 255
#define MaxLineWords 2


void usage(char *progname)
{   
    fprintf
    (   stderr
    ,   "Usage: %s iniFile\n"
        "Where: \"iniFile\" is the path of the ini file.\n"
        "\n"
    ,   progname
    );

    exit(1);
}


int main(int argc, char **argv)
{   int retVal;

// Check the number of arguments.
    if (argc != 2)
        usage(argv[0]);

// Create the iniFile object.
    csc_ini_t *ini = csc_ini_new();

// Read in the iniFile.
    retVal = csc_ini_read(ini, argv[1]);
    if (retVal < 0)
    {   fprintf(  stderr
               , "Error: Failed to open configuration file \"%s\"!\n"
               , argv[1]
               );
        exit(1);
    }
    else if (retVal > 0)
    {   fprintf(  stderr
               , "Error reading line %d of configuration file \"%s\"!\n"
               , retVal
               , argv[1]
               );
        exit(1);
    }   

// Get property requests from the command line.
    char line[MaxLineLen+1];
    char *words[MaxLineWords+1];
    while (csc_fgetline(stdin, line, MaxLineLen) != -1)
    {   
        int nWords = csc_param_quote(words, line, MaxLineWords+1);
        if (nWords != 2)
        {   fprintf(stdout, "Usage: sectName keyName\n");
        }
        else
        {   const char *value = csc_ini_getStr(ini, words[0], words[1]);
            if (value == NULL)
            {   fprintf( stdout
                       , "There is no value for key \"%s\" in section \"%s\".\n"
                       , words[1], words[0]
                       );
            }
            else
            {   fprintf( stdout
                       , "The key for \"%s\" in section \"%s\" is \"%s\".\n"
                       , words[1], words[0], value
                       );
            }
        }
    }

    csc_ini_free(ini);
    exit(0);
}


