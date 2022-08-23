#include <stdlib.h>
#include <stdio.h>
#include <CscNetLib/std.h>
#include <CscNetLib/isvalid.h>
#include <CscNetLib/logger.h>



void usage(char *progname)
{   
    fprintf( stderr
           , "Usage %s logPath logLevel\n\n"
             "   where logPath is the path of the log file\n"
             "     and logLevel is an integer from %d to %d\n\n"
           , progname, csc_log_TRACE, csc_log_FATAL
           );
    exit(1);
}


int main(int argc, char **argv)
{   int logLevel;
 
// Check the number of command line arguments.
    if (argc != 3)
        usage(argv[0]);
 
// Check the log level.
    if (!csc_isValidRange_int(argv[2], csc_log_TRACE, csc_log_FATAL, &logLevel))
        usage(argv[0]);
 
// Create the logging object.
    csc_log_t *log = csc_log_new(argv[1], "logDemo", logLevel);
    if (log == NULL)
        return csc_FALSE;
 
// Make a log at each level.
    for (int iLvl=csc_log_TRACE; iLvl<=csc_log_FATAL; iLvl++)
        csc_log_printf(log, iLvl, "entry at level %d", iLvl); 
 
// Release resources.
    csc_log_free(log);
 
    exit(0);
}

