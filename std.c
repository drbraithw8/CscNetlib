// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "std.h"


FILE *csc_errOut = NULL;

void csc_setErrOut(const char *pathErrOut)
{   csc_errOut = fopen(pathErrOut, "a"); csc_assert(csc_errOut);
    int ret = setvbuf(csc_errOut, NULL, _IONBF, 0); csc_assert(ret==0);
}

void csc_assertFail(const char *fname, int lineNo, const char *expr)
{   fprintf(csc_stderr
           , "csc_assert failure (%s) in file \"%s\" at line %d\n" 
           , expr, fname, lineNo
           );
    exit(1);
}


int csc_fgetwd(FILE *fp, char *wd, int wdmax)
{   int len=0;
    int ch = getc(fp);
 
/* Skip whitespace */
    while(isspace(ch))
        ch = getc(fp);
 
/* Deal with a possible EOF */
    if(ch==EOF)
        return -1;
 
/* read in the word */
    while(!isspace(ch) && ch!=EOF && len<wdmax)
    {   wd[len++] = ch;
        ch = getc(fp);
    }
    wd[len]='\0';
 
/* Skip remainder of the word */
    while(!isspace(ch) && ch != EOF)
    {   len++;
        ch = getc(fp);
    }
    if (ch != EOF)
        ungetc(ch, fp);
 
/* Bye. */
    return len;
}


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


int64_t csc_xferBytes(FILE *fin, FILE *fout)
{   int ch;
    int64_t iByte = 0;
    ch = getc(fin);
    while (ch != EOF)  // If we should still xfer bytes.
    {   ch = putc(ch,fout);  // Write out the byte.
        if (ch != EOF)
        {   iByte++;  // Keep count of bytes successfully transferred.
            ch = getc(fin);
        }
    }
    return iByte;  // Return the numbers of bytes transferred. 
}


int64_t csc_xferBytesN(FILE *fin, FILE *fout, int64_t nBytes)
{   int ch;
    int64_t iByte = 0;
    while (iByte < nBytes)  // If we should still xfer bytes.
    {   ch = getc(fin);    // Get one char.
        if (ch == EOF)    // If we found the end of file, then
            nBytes = -1; // Terminate the loop.
        else
        {   ch = putc(ch,fout);  // Write out the byte.
            if (ch == EOF)
                nBytes = -1;   // Terminate the loop.
            else
                iByte++;     // Keep count of bytes successfully transferred.
        }
    }
    return iByte;  // Return the numbers of bytes transferred. 
}


void csc_dateTimeStr(char str[csc_timeStrSize+1])
{   time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
    strftime(str, csc_timeStrSize+1, "%Y%m%d.%H%M%S", now);
}


uint64_t csc_cs4(char *str)
{	const int MUL=293;
	const int ADD=1;
 
	register uint8_t *pnt = (uint8_t*)str;
	register uint8_t *end = pnt + strlen(str);
	register uint64_t sum = 1;
	while (pnt<end)
	{	sum = (sum+(*pnt++)+ADD)*MUL;
	}
 
	return sum;
}


void csc_strncpy(char *dest, const char *source, int n)
{	while (n-- && (*dest++ = *source++))
		;
	if (n < 0)
		*dest = '\0';
}


static void csc_strncat(char *dest, char *source, int n)
{   int len;
 
	len = strlen(dest);
	if (n > len)
		csc_strncpy(dest+len, source, n-len);
}
