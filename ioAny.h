// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#ifndef csc_IOANY_H
#define csc_IOANY_H 1

#include "std.h"
#include "cstr.h"


//===================================================
// Functions prototypes.
//===================================================

// A function to read a single character from --something--.
// Returns charater.  Returns -1 on EOF.
typedef int (*csc_ioAny_readFunc_t)(void *context);

// A function to write a string to --something--.
typedef void (*csc_ioAny_writeFunc_t)(void *context, const char *str);



//===================================================
// Class that can read single chars from whatever.
//===================================================

typedef struct csc_ioAnyRead_s csc_ioAnyRead_t;

// Constructor.
csc_ioAnyRead_t *csc_ioAnyRead_new(csc_ioAny_readFunc_t readChar, void *context);

// Destructor.
void csc_ioAnyRead_free(csc_ioAnyRead_t *rca);

// Get one character.
int csc_ioAnyRead_getc(csc_ioAnyRead_t *rca);



//===================================================
// Class that can write strings to whatever.
//===================================================

typedef struct csc_ioAnyWrite_s csc_ioAnyWrite_t;

// Constructor.
csc_ioAnyWrite_t *csc_ioAnyWrite_new(csc_ioAny_writeFunc_t writeStr, void *context);

// Destructor.
void csc_ioAnyWrite_free(csc_ioAnyWrite_t *rca);

// Write a string.
void csc_ioAnyWrite_puts(csc_ioAnyWrite_t *rca, char *str);



//======================================================
// Class that can read single chars from a char* string.
//======================================================

typedef struct csc_ioAny_readChStr_s csc_ioAny_readChStr_t;

// Constructor.
// This function only borrows 'str', and does not make a copy. 
// Do not overwrite or dispose of 'str' until after destructor is called.
csc_ioAny_readChStr_t *csc_ioAny_readChStr_new(const char *str);

// Destructor.
void csc_ioAny_readChStr_free(csc_ioAny_readChStr_t *rcs);

// Get one character.
int csc_ioAny_readChStr_getc(csc_ioAny_readChStr_t *rcs);



//======================================================
// Concrete IO 
//======================================================

// Write char* string to a FILE*.
void csc_ioAny_writeFILE(void *context, const char *str);

// Read char from a FILE*.
int csc_ioAny_readCharFILE(void *context);

// Write char* string to a Cstr.
void csc_ioAny_writeCstr(void *context, const char *str);

// Reading char from a char* string.
int csc_ioAny_readCharStr(void *context);


#endif
