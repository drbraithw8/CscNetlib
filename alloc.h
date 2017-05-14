// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#ifndef csc_ALLOC_H
#define csc_ALLOC_H 1

#include "std.h"

#ifndef csc_mck_IS_ON 

// Allocate a objects of type t.
#define csc_allocOne(t)  ((t*)csc_ck_malloc((size_t)sizeof(t)))
#define csc_allocMany(t,n)  ((t*)csc_ck_malloc((size_t)((n)*sizeof(t))))


// Like malloc() and calloc() and realloc(), but gives error message and
// quits if it could not obtain the memory.
void *csc_ck_malloc(size_t size);
void *csc_ck_calloc(size_t size);
void *csc_ck_ralloc(void *rem, size_t size);


// Alloc a single string on the heap.  The caller must use free() to
// dispose of the allocated string after use.
char *csc_alloc_str(const char *str);


// Allocate a single string consisting of its arguments concatentated.  The
// caller must use free() to dispose of the allocated string after use.
// The NULL pointer is treated as an empty string, so pass NULL for the
// third string if you wish to concatenate only two strings.
char *csc_alloc_str3(const char *str1, const char *str2, const char *str3);


// Allocate a single string consisting of its arguments concatentated.  The
// caller must use free() to dispose of the allocated string after use.
// The NULL pointer is treated as an empty string, so pass NULL for the
// strings if you have less than seven strings to concatenate.
char *csc_alloc_str7(const char *str1, const char *str2, const char *str3,
                  const char *str4, const char *str5, const char *str6,
                  const char *str7);

#else

#define csc_allocOne(t)  ((t*)csc_ck_malloc_debug((size_t)sizeof(t), __LINE__, __FILE__))
#define csc_allocMany(t,n)  ((t*)csc_ck_malloc_debug((size_t)((n)*sizeof(t)), __LINE__, __FILE__))

#define csc_ck_malloc(size) csc_ck_malloc_debug(size, __LINE__, __FILE__)
#define csc_ck_calloc(size) csc_ck_calloc_debug(size, __LINE__, __FILE__)
#define csc_alloc_str(str) csc_alloc_str_debug(str, __LINE__, __FILE__)
#define csc_alloc_str3(str1, str2, str3) csc_alloc_str3_debug(str1, str2, str3, __LINE__, __FILE__)
#define csc_alloc_str7(str1, str2, str3, str4, str5, str6, str7)   csc_alloc_str7_debug(str1, str2, str3, str4, str5, str6, str7, __LINE__, __FILE__)
#define csc_ck_ralloc(rem, size) csc_ck_ralloc_debug(rem, size, __LINE__, __FILE__)


void *csc_ck_calloc_debug(size_t size, int line, char *fname);
void *csc_ck_malloc_debug(size_t size, int line, char *fname);
char *csc_alloc_str_debug(const char *str, int line, char *fname);
char *csc_alloc_str3_debug(const char *str1, const char *str2, const char *str3,
										int line, char *fname);
char *csc_alloc_str7_debug(const char *str1, const char *str2, const char *str3,
                  const char *str4, const char *str5, const char *str6,
                  const char *str7, int line, char *fname);
void *csc_ck_ralloc_debug(void *rem, size_t size, int line, char *fname);

#endif

#endif
