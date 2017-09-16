// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "std.h"


static void csc_ck_defaultErrorHandler(void *context)
{   fprintf(stderr, "Error: Out of Memory!\n");
    exit(1);
}

static void (*errHndl)(void*) = csc_ck_defaultErrorHandler;
static void *errHndlContext = NULL;

void csc_ck_setErrHandler(void (*errHandle)(void*), void *errHandleContext)
{   errHndl = errHandle;
    errHndlContext = errHandleContext;
}


#ifndef csc_mck_IS_ON 

void *csc_ck_ralloc(void *rem, size_t size)
/*  Changes the size of the block pointed to by 'rem' to 'size' 
 * bytes using realloc().  If 'size' bytes could not be allocated, 
 * this function gives an error message and then exits the program.  
 * Otherwise it returns a pointer to the reallocated bytes.
 */ 
{   void *p;
    if((p=realloc(rem, size)) == NULL)
        errHndl(errHndlContext);
    return(p);
}


void *csc_ck_calloc(size_t size)
/* Allocates 'size' bytes, using malloc().  If 'size' bytes could
 * not be allocated, this function gives an error message and then
 * exits the program.  Otherwise it returns a pointer to the allocated
 * bytes.
 */
{   void *p;
    if((p=calloc((size_t)1, size)) == NULL)
        errHndl(errHndlContext);
    return(p);
}


void *csc_ck_malloc(size_t size)
/* Allocates 'size' bytes, using malloc().  If 'size' bytes could
 * not be allocated, this function gives an error message and then
 * exits the program.  Otherwise it returns a pointer to the allocated
 * bytes.
 */
{   void *p;
    if((p=malloc(size)) == NULL)
        errHndl(errHndlContext);
    return(p);
}


char *csc_alloc_str(const char *str)
/* This function allocates sufficient space to hold the string 'str'.
 * 'str' is copied into that space.  A pointer to that space is returned.
 */
{   char *s;
    s = (char*)csc_ck_malloc((size_t)strlen(str)+1);
    strcpy(s,str);
    return(s);
}


char *csc_alloc_str3(const char *str1, const char *str2, const char *str3)
/* This function allocates sufficient space to hold the string 'str1',
 * 'str2' and 'str3' concatonated.  These are then copied into that space
 * in order.  A pointer to that space is returned.  Any of 'str1', 'str2'
 * and 'str3' may be null strings or indeed NULL pointers.
 */
{   char *s, *spt;
    int len1, len2, len3;
 
    len1 = str1 ? strlen(str1) : 0;
    len2 = str2 ? strlen(str2) : 0;
    len3 = str3 ? strlen(str3) : 0;
    spt = s = (char*)csc_ck_malloc(len1+len2+len3+1);
 
    if (len1)
    {   memcpy(spt,str1,len1);
        spt += len1;
    }
    if (len2)
    {   memcpy(spt,str2,len2);
        spt += len2;
    }
    if (len3)
    {   memcpy(spt,str3,len3);
        spt += len3;
    }
    *spt = '\0';
    return(s);
}


char *csc_alloc_str7(const char *str1, const char *str2, const char *str3,
                  const char *str4, const char *str5, const char *str6,
                  const char *str7)
/* This function allocates sufficient space to hold the string 'str1',
 * 'str2', 'str3', 'str4', 'str5', 'str6' and 'str7' concatonated. 
 * These are then copied into that space in order.  A pointer to that
 * space is returned.  Any of 'str1', 'str2', 'str3', 'str4', 'str5',
 * 'str6' and 'str7' may be null strings or indeed NULL pointers.  
 */
{   char *s, *spt;
    int len1, len2, len3, len4, len5, len6, len7;
 
    len1 = str1 ? strlen(str1) : 0;
    len2 = str2 ? strlen(str2) : 0;
    len3 = str3 ? strlen(str3) : 0;
    len4 = str4 ? strlen(str4) : 0;
    len5 = str5 ? strlen(str5) : 0;
    len6 = str6 ? strlen(str6) : 0;
    len7 = str7 ? strlen(str7) : 0;
    spt = s = (char*)csc_ck_malloc(len1+len2+len3+len4+len5+len6+len7+1);
 
    if (len1)
    {   memcpy(spt,str1,len1);
        spt += len1;
    }
    if (len2)
    {   memcpy(spt,str2,len2);
        spt += len2;
    }
    if (len3)
    {   memcpy(spt,str3,len3);
        spt += len3;
    }
    if (len4)
    {   memcpy(spt,str4,len4);
        spt += len4;
    }
    if (len5)
    {   memcpy(spt,str5,len5);
        spt += len5;
    }
    if (len6)
    {   memcpy(spt,str6,len6);
        spt += len6;
    }
    if (len7)
    {   memcpy(spt,str7,len7);
        spt += len7;
    }
    *spt = '\0';
    return(s);
}

#else


void *csc_ck_ralloc_debug(void *rem, size_t size, int line, char *file)
/*  Changes the size of the block pointed to by 'rem' to 'size' 
 * bytes using realloc().  If 'size' bytes could not be allocated, 
 * this function gives an error message and then exits the program.  
 * Otherwise it returns a pointer to the reallocated bytes.
 */ 
{   void *p;
    if((p=csc_mck_realloc(rem, size, line, file)) == NULL)
        errHndl(errHndlContext);
    return(p);
}


void *csc_ck_calloc_debug(size_t size, int line, char *file)
/* Allocates 'size' bytes, using malloc().  If 'size' bytes could
 * not be allocated, this function gives an error message and then
 * exits the program.  Otherwise it returns a pointer to the allocated
 * bytes.
 */
{   void *p;
    if((p=csc_mck_calloc((size_t)1, size, line, file)) == NULL)
        errHndl(errHndlContext);
    return(p);
}


void *csc_ck_malloc_debug(size_t size, int line, char *file)
/* Allocates 'size' bytes, using malloc().  If 'size' bytes could
 * not be allocated, this function gives an error message and then
 * exits the program.  Otherwise it returns a pointer to the allocated
 * bytes.
 */
{   void *p;
    if((p=csc_mck_malloc(size, line, file)) == NULL)
        errHndl(errHndlContext);
    return(p);
}


char *csc_alloc_str_debug(const char *str, int line, char *file)
/* This function allocates sufficient space to hold the string 'str'.
 * 'str' is copied into that space.  A pointer to that space is returned.
 */
{   char *s;
    s = (char*)csc_ck_malloc_debug((size_t)strlen(str)+1, line, file);
    strcpy(s,str);
    return(s);
}


char *csc_alloc_str3_debug(const char *str1, const char *str2, const char *str3, int line, char *file)
/* This function allocates sufficient space to hold the string 'str1',
 * 'str2' and 'str3' concatonated.  These are then copied into that space
 * in order.  A pointer to that space is returned.  Any of 'str1', 'str2'
 * and 'str3' may be null strings or indeed NULL pointers.
 */
{   char *s, *spt;
    int len1, len2, len3;
 
    len1 = str1 ? strlen(str1) : 0;
    len2 = str2 ? strlen(str2) : 0;
    len3 = str3 ? strlen(str3) : 0;
    spt = s = (char*)csc_ck_malloc_debug(len1+len2+len3+1, line, file);
 
    if (len1)
    {   memcpy(spt,str1,len1);
        spt += len1;
    }
    if (len2)
    {   memcpy(spt,str2,len2);
        spt += len2;
    }
    if (len3)
    {   memcpy(spt,str3,len3);
        spt += len3;
    }
    *spt = '\0';
    return(s);
}


char *csc_alloc_str7_debug(const char *str1, const char *str2, const char *str3,
                  const char *str4, const char *str5, const char *str6,
                  const char *str7, int line, char *file)
/* This function allocates sufficient space to hold the string 'str1',
 * 'str2', 'str3', 'str4', 'str5', 'str6' and 'str7' concatonated. 
 * These are then copied into that space in order.  A pointer to that
 * space is returned.  Any of 'str1', 'str2', 'str3', 'str4', 'str5',
 * 'str6' and 'str7' may be null strings or indeed NULL pointers.  
 */
{   char *s, *spt;
    int len1, len2, len3, len4, len5, len6, len7;
 
    len1 = str1 ? strlen(str1) : 0;
    len2 = str2 ? strlen(str2) : 0;
    len3 = str3 ? strlen(str3) : 0;
    len4 = str4 ? strlen(str4) : 0;
    len5 = str5 ? strlen(str5) : 0;
    len6 = str6 ? strlen(str6) : 0;
    len7 = str7 ? strlen(str7) : 0;
    spt = s = (char*)csc_ck_malloc_debug(len1+len2+len3+len4+len5+len6+len7+1, line, file);
 
    if (len1)
    {   memcpy(spt,str1,len1);
        spt += len1;
    }
    if (len2)
    {   memcpy(spt,str2,len2);
        spt += len2;
    }
    if (len3)
    {   memcpy(spt,str3,len3);
        spt += len3;
    }
    if (len4)
    {   memcpy(spt,str4,len4);
        spt += len4;
    }
    if (len5)
    {   memcpy(spt,str5,len5);
        spt += len5;
    }
    if (len6)
    {   memcpy(spt,str6,len6);
        spt += len6;
    }
    if (len7)
    {   memcpy(spt,str7,len7);
        spt += len7;
    }
    *spt = '\0';
    return(s);
}



// char *nStrdup(char *s1, ...)
// /* Returns a pointer to newly allocated copy of the argument strings
//  * concatenated together.  The list of strings to be concatenated is
//  * terminated with a NULL pointer, i.e. the last argument to this function
//  * should always be NULL.  There may be at most 100 argument strings - for
//  * efficiency sake, and args after this are ignored.
//  */  
// {    enum {MaxStr=100};
//  
//  char *buf, *pos;
//  int nStr, iStr;
//  int len[MaxStr];
//  char *str[MaxStr];
//  char *CurStr;
//  int CurLen;
//  int TotalLen;
//  va_list ap;
//  
// /* Count the strings and their lengths. */
//  nStr = 0;
//  TotalLen = 0;
//  {
//      CurStr = s1;
//      str[nStr] = CurStr;
//      CurLen = strlen(CurStr);
//      len[nStr] = CurLen;
//      TotalLen += CurLen;
//      nStr++;
//  }
//  va_start(ap, s1);
//  while (1)
//  {   if (nStr == MaxStr)
//          break;
//      CurStr = va_arg(ap, char*);
//      if (CurStr == NULL)
//          break;
//      str[nStr] = CurStr;
//      CurLen = strlen(CurStr);
//      len[nStr] = CurLen;
//      TotalLen += CurLen;
//      nStr++;
//  }
//  va_end(ap);
//  
// /* Allocate buf for result string. */
//  buf = malloc(TotalLen+1);
//  if (buf == NULL)
//      return NULL;
//  
// /* Copy in strings. */
//  pos = buf;
//  for (iStr=0; iStr<nStr; iStr++)
//  {   CurLen = len[iStr];
//      memcpy(pos, str[iStr], CurLen);
//      pos += CurLen;
//  }
//  *pos = '\0';
//  
//  return buf;
// }

#endif
