// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "std.h"


int csc_fgetline(FILE *fp, char *line, int max)
{   register int ch, i;
 
/*  Look at first char for EOF. */
    ch = getc(fp);
    if (ch == EOF)
        return -1;
 
/* Read in line */
    i=0;
    while (ch!='\n' && ch!=EOF && i<max)
    {   if (ch != '\r')
            line[i++] = ch;
        ch = getc(fp);
    }
    line[i] = '\0';
 
/* Skip any remainder of line */
    while (ch!='\n' && ch!=EOF)
    {   ch = getc(fp);
        i++;
    }
    return(i);
}


static char *param_skip_white(char *p)
/*  Skips over ' ', '\t'. 
 */
{   char ch = *p;
    while (ch==' ' || ch=='\t')
        ch = *(++p);
    return(p);
}


static char *param_skip_word(char *p)
/*  Skips over all chars except ' ', '\t', '\0'.
 */
{   char ch = *p;
    while (ch!=' ' && ch!='\0' && ch!='\t')
        ch = *(++p);
    return(p);
}


int csc_param(char *argv[], char *line, int n)
{   int argc = 0;
    char ch;
    char *p = param_skip_white(line);
    while(*p!='\0' && argc<n)
    {   argv[argc++] = p;
        line = param_skip_word(p);
        p = param_skip_white(line);
        *line = '\0';
    }
    return(argc);
}


int csc_param_quote(char *argv[], char *line, int n)
{   int argc = 0;
    char ch;
    char *p = param_skip_white(line);
    while(*p!='\0' && argc<n)
    {   if (*p == '\"')
        {   argv[argc++] = line = ++p;
            ch = *p;
            while (ch!='\"' && ch!='\0')
            {   if (ch=='\\' && *(p+1)!='\0')
                    ch = *++p;
                *line++ = ch;
                ch = *++p;
            }
            *line = '\0';
            if (ch == '\"')
                p = param_skip_white(p+1);
        }
        else
        {   argv[argc++] = p;
            line = param_skip_word(p);
            p = param_skip_white(line);
            *line = '\0';
        }
    }
    return(argc);
}


int64_t csc_xferBytesN(FILE *fin, FILE *fout, int64_t nBytes)
{   int ch;
    int64_t iByte = 0;
    while (iByte < nBytes)  // If we should still xfer bytes.
    {   ch = getc(fin);    // Get one char.
        if (ch == EOF)    // If we found the end of file, then
            nBytes = -1; // Terminate the loop.
        else
        {   putc(ch,fout);  // Write out the byte.
            iByte++;  // Keep count of bytes transferred.
        }
    }
    return iByte;  // Return the numbers of bytes transferred. 
}


void csc_dateTimeStr(char str[csc_timeStrSize+1])
{   time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
    strftime(str, csc_timeStrSize+1, "%Y%m%d.%H%M%S", now);
}

