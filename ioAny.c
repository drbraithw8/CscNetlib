#include <ctype.h>
#include "ioAny.h"
#include "alloc.h"



//================================================
// Class that can read single chars from whatever.
//================================================

typedef struct csc_ioAnyRead_s 
{   csc_ioAny_readFunc_t readChar;
    void *context;
} csc_ioAnyRead_t;

csc_ioAnyRead_t *csc_ioAnyRead_new(csc_ioAny_readFunc_t readChar, void *context)
{   csc_ioAnyRead_t *rca = csc_allocOne(csc_ioAnyRead_t);
    rca->readChar = readChar;
    rca->context = context;
}

void csc_ioAnyRead_free(csc_ioAnyRead_t *rca)
{   free(rca);
}

int csc_ioAnyRead_getc(csc_ioAnyRead_t *rca)
{   return rca->readChar(rca->context);
}

int csc_ioAnyRead_getline(csc_ioAnyRead_t *ior, csc_str_t *line)
{   int ch;
    csc_str_reset(line);
 
// Read in line. 
	ch = csc_ioAnyRead_getc(ior);
    while (ch!=EOF && ch!='\n')
    {   if (ch != '\r')
            csc_str_append_ch(line, ch);
		ch = csc_ioAnyRead_getc(ior);
    }
 
// Return result. 
    if (ch == EOF)
        return -1;
    else
        return csc_str_length(line);
}


int csc_ioAnyRead_getwd(csc_ioAnyRead_t *ior, csc_str_t *word)
{
	int ch = csc_ioAnyRead_getc(ior);
    csc_str_reset(word);
 
/* Skip whitespace */
	while(isspace(ch))
		ch = csc_ioAnyRead_getc(ior);
 
/* Deal with a possible EOF */
	if(ch==EOF)
		return -1;
 
/* read in the word */
	while(!isspace(ch) && ch!=EOF)
	{	csc_str_append_ch(word, ch);
		ch = csc_ioAnyRead_getc(ior);
	}
 
/* Return the length. */
	return csc_str_length(word);
}


//================================================
// Class that can write strings to whatever.
//================================================

typedef struct csc_ioAnyWrite_s
{   csc_ioAny_writeFunc_t writeStr;
    void *context;
}  csc_ioAnyWrite_t;

csc_ioAnyWrite_t *csc_ioAnyWrite_new(csc_ioAny_writeFunc_t writeStr, void *context)
{   csc_ioAnyWrite_t *rca = csc_allocOne(csc_ioAnyWrite_t);
    rca->writeStr = writeStr;
    rca->context = context;
}

void csc_ioAnyWrite_free(csc_ioAnyWrite_t *rca)
{   free(rca);
}

void csc_ioAnyWrite_puts(csc_ioAnyWrite_t *rca, const char *str)
{   rca->writeStr(rca->context, str);
}



//======================================================
// Class that can read single chars from a char* string.
//======================================================

typedef struct csc_ioAny_readChStr_s
{   const char *str;
    const char *p;
} csc_ioAny_readChStr_t;

csc_ioAny_readChStr_t *csc_ioAny_readChStr_new(const char *str)
{   csc_ioAny_readChStr_t *rcs = csc_allocOne(csc_ioAny_readChStr_t);
    rcs->str = str;
    rcs->p = str;
	return rcs;
}

void csc_ioAny_readChStr_free(csc_ioAny_readChStr_t *rcs)
{   free(rcs);
}

int csc_ioAny_readChStr_getc(csc_ioAny_readChStr_t *rcs)
{  
	int ch = *rcs->p++;
    if (ch == '\0')
    {   rcs->p--;
        return EOF;
    }
    else
	{
        return ch;
	}
}



//======================================================
// Concrete IO 
//======================================================

// Write to a FILE*.
void csc_ioAny_writeFILE(void *context, const char *str)
{   fprintf((FILE*)context, "%s", str);
}

// Reading from a FILE*.
int csc_ioAny_readCharFILE(void *context)
{   return getc((FILE*)context);
}

// Write to a Cstr.
void csc_ioAny_writeCstr(void *context, const char *str)
{   csc_str_append((csc_str_t*)context, str);
}

// Reading from a char* string.
int csc_ioAny_readCharStr(void *context)
{   return csc_ioAny_readChStr_getc((csc_ioAny_readChStr_t*)context);
}

