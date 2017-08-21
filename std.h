// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#ifndef csc_STD_H
#define csc_STD_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define csc_versionStr "1.7.2"


#define csc_streq(a,b)  (!strcmp((a),(b)))
#define csc_strieq(a,b)  (!stricmp((a),(b)))

#define csc_isalu(ch) (isalpha(ch) || (ch)=='_')
#define csc_ungetc(ch,fp)  ((ch)==EOF?(ch):ungetc(ch,fp))

typedef enum csc_bool_e { csc_FALSE=0, csc_TRUE=1 } csc_bool_t;

#define csc_CKCK fprintf(stderr, \
                "Got to line %d in file %s !\n", __LINE__, __FILE__)

typedef unsigned int csc_uint;
typedef unsigned long csc_ulong;
typedef unsigned char csc_uchar;
typedef unsigned short csc_ushort;


#define csc_dim(array)      (sizeof(array) / sizeof(array[0]))
#define csc_fdim(typ,arr) (sizeof(((typ*)NULL)->arr)/sizeof(((typ*)NULL)->arr[0]))
#define csc_fsizeof(type,field)     (sizeof(((type *)NULL)->field))


// Reads a line from the stream 'fp' into the array 'line'.  The
// terminating newline is not included into 'line', but it is consumed.
// 
// If the line is longer than 'max' characters, then only the
// first 'max' chars of the line will be placed into 'line'.  The
// remainder of the line will be skipped.
// 
// This function will append a '\0' to the characters read into
// 'line'.  Hence 'line' should have room for 'max'+1 characters.
// 
// If no characters were read in due to end of file, -1 will
// be returned.  Otherwise the original length of the line in the
// stream 'fp' (not including the newline) will be returned.
int csc_fgetline(FILE *fp, char *line, int max);


// This routine takes a string 'line' as input, splits up the words
// in the string, and assigns them in order to the array of character
// strings 'argv'.   The number of words assigned in argv is returned.
// 
// On entry to this function, arguments in 'line' are separated by
// whitespace.  'line' will be overwritten, and the pointers in 'argv'
// will point into 'line'.
// 
// Argv can have at most 'n' strings.
int csc_param(char *argv[], char *line, int n);


// csc_param_quote() takes the same arguments as csc_param(), and the only
// difference is that strings enclosed in quotes (e.g. "one word") are
// treated as words.  In this case the character after the opening quote
// will appear as the first character of the word and the terminating quote
// will be replaced by a null byte.  
int csc_param_quote(char *argv[], char *line, int n);


// Fills 'str' with a a null terminated string containing the date and time
// in the format "YYYYMMDD.hhmmss".
#define csc_timeStrSize 15
void csc_dateTimeStr(char str[csc_timeStrSize+1]);


// Transfers up to 'N' bytes from the stream 'fin' to the stream 'fout'.
// May transfer less than 'N' if the end of file is discovered, or on
// error.  Returns the number of bytes actually transferred.
int64_t csc_xferBytesN(FILE *fin, FILE *fout, int64_t nBytes);


#define csc_mck_IS_ON 1
#ifdef csc_mck_IS_ON 

#define malloc(size)            csc_mck_malloc(size,__LINE__,__FILE__)
#define calloc(nelem,elsize)    csc_mck_calloc(nelem,elsize,__LINE__,__FILE__)
#define free(block)             csc_mck_free(block,__LINE__,__FILE__)
#define realloc(block,size)     csc_mck_realloc(block,size,__LINE__,__FILE__)
#define strdup(str)             csc_mck_strdup(str,__LINE__,__FILE__)

#define csc_mck_check(flag)     csc_mck_checkmem(flag,__LINE__,__FILE__)

#ifdef MEMCHECK_SILENT
#define exit(status)            csc_mck_sexit(status,__LINE__,__FILE__)
#else
#define exit(status)            csc_mck_exit(status,__LINE__,__FILE__)
#endif

// extern long mck_maxchunks;

#ifdef __STDC__
void *csc_mck_malloc(unsigned int size, int line, char *file);
void *csc_mck_calloc(unsigned int nelem, unsigned int elsize, int line, char *file);
char *csc_mck_strdup(char *str, int line, char *file);
void csc_mck_free(void *block, int line, char *file);
void *csc_mck_realloc(void *block, unsigned int size, int line, char *file);
void csc_mck_sexit(int status, int line, char *file);
void csc_mck_exit(int status, int line, char *file);
long csc_mck_nchunks(void);
int csc_mck_checkmem(int flag, int line, char *file);
void csc_mck_print(FILE *fout);
#else
char *csc_mck_malloc();
char *csc_mck_calloc();
char *csc_mck_strdup();
void csc_mck_free();
char *csc_mck_realloc();
void csc_mck_exit();
void csc_mck_sexit();
long csc_mck_nchunks();
int csc_mck_check();
void csc_mck_print();
#endif

#else
#define csc_mck_check(flag) ((void)0)
#define csc_mck_print(fout) ((void)0)
#endif

#endif


