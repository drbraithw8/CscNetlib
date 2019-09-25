// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

// ----------------------------
// Basis for simple TCP server.
// ----------------------------

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <CscNetLib/std.h>
#include <CscNetLib/logger.h>
#include <CscNetLib/iniFile.h>
#include <CscNetLib/isvalid.h>
#include <CscNetLib/servBase.h>

#define MaxLineLen 255


typedef struct
{   float height;
    float width;
    float depth;
} boxDim_t;


int configGetCheckFloat( csc_log_t *log
                       , csc_ini_t *conf, char *sect, char *ident
                       , float *val, float min, float max)
{   float value;
 
    const char *str = csc_ini_getStr(conf, sect, ident);
    if (str == NULL)
    {   csc_log_printf(log, csc_log_FATAL
            , "Missing floating configuration property \"%s\" in section \"%s\""
            , ident, sect);
        return csc_FALSE;
    }
    else if (!csc_isValid_float(str))
    {   csc_log_printf(log, csc_log_FATAL
            , "Invalid floating configuration property \"%s\" in section \"%s\""
            , ident, sect);
        return csc_FALSE;
    }
    value = atof(str);
    if (value<min || value>max)
    {   csc_log_printf(log, csc_log_FATAL
            , "Floating configuration property \"%s\" in section \"%s\" is out of range"
            , ident, sect);
        return csc_FALSE;
    }
 
    *val = value;
    return csc_TRUE;
}


int doInit(csc_ini_t *conf, csc_log_t *log, void *local)
{   boxDim_t *boxDim = local;
    char *str;
 
// Get height from configuration.
    if (!configGetCheckFloat(log, conf, "Demo", "Height", &boxDim->height, 0, FLT_MAX))
        return csc_FALSE;
 
// Get width from configuration.
    if (!configGetCheckFloat(log, conf, "Demo", "Width", &boxDim->width, 0, FLT_MAX))
        return csc_FALSE;
 
// Get depth from configuration.
    if (!configGetCheckFloat(log, conf, "Demo", "Depth", &boxDim->depth, 0, FLT_MAX))
        return csc_FALSE;
 
    return csc_TRUE;
}

 
int doConn( int fd0            // client file descriptor
          , const char *clientIp   // IP of client, NULL if unknown.
          , csc_ini_t *conf // Configuration object.
          , csc_log_t *log  // Logging object.
          , void *local
          )
{   boxDim_t *boxDim = local;
    char line[MaxLineLen+1];
 
// Convert the file descriptor into FILE streams.
    int fd1 = dup(fd0);           csc_log_assert(log, fd1!=-1);
    FILE *fin = fdopen(fd0,"r");  csc_log_assert(log, fin!=NULL);
    FILE *fout = fdopen(fd1,"w"); csc_log_assert(log, fout!=NULL);
 
// Read one line.
    csc_fgetline(fin,line,MaxLineLen);
    fprintf(stdout, "Got line: \"%s\"\n", line);
 
// Respond.
    fprintf(fout, "%f %f %f\n", boxDim->height, boxDim->width, boxDim->depth);
    fflush(fout);
 
// Close the TCP streams.
    fclose(fin);
    fclose(fout);

// Bye
    return 0;  // success.
}


int main(int argc, char **argv)
{   boxDim_t boxDims;
 
    csc_servBase_server( "OneByOne"    // Server Model.
                       , "test.log"    // Path to log file.
                       , "test.ini"    // Path to configuration file.
                       , doConn       // Callback called for each connection.
                       , doInit       // Callback called for initialisation.
                       , &boxDims     // Passed to doInit() and to doConn().
                       );
    exit(0);
}

