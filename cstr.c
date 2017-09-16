// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "std.h"
#include "cstr.h"

typedef struct csc_str_t
{   char *chars; // The characters of the string.  Extra space for null char.
                // The extra space is used for csc_str_to_c_static().
    int mchars;  // The size of the array 'chars'.
    int nchars;  // The length of the string.
} csc_str_t;


static void error_handle(const char *errmsg)
// Default memory allocator for cstr.
{   fprintf(stderr, "Error in cstr: %s\n", errmsg);
    exit(1);
}


static void *alloc(size_t size)
// Private memory allocator for cstr.
{   void *mem = malloc(size);
    if (mem == NULL)
        error_handle("Memory allocation failure");
    return mem;
}


csc_str_t *csc_str_new(const char *str)
{   csc_str_t *this;
 
// Get the structure. 
    this = (csc_str_t*)alloc(sizeof(csc_str_t));
 
// Get length of the string. 
    if (str == NULL)
        this->nchars = 0;
    else
        this->nchars = strlen(str);
 
// Allocate chars for the string. 
	if (this->nchars == 0)
	{	this->mchars = 0;
		this->chars = NULL;
	}
	else
    {	this->mchars = this->nchars + 8;
		this->mchars |= 7;  // Round up - Use that which would be wasted. 
		this->chars = (char*)alloc((this->mchars + 1) * sizeof(char));  // '\0'. 
	}
 
// Copy the string. 
    if (this->nchars > 0)
        memcpy(this->chars, str, this->nchars);
 
// Home with the bacon. 
    return this;
}


void csc_str_free(csc_str_t *this)
{   if (this->chars)
		free(this->chars);
    free(this);
}


void csc_str_assign(csc_str_t *this, const char *str)
{
// Get length of the string. 
    if (str == NULL)
        this->nchars = 0;
    else
        this->nchars = strlen(str);
 
// Allocate chars for the string. 
    if (this->mchars < this->nchars)
    {   this->mchars = this->mchars * 2 + 1;
        if (this->mchars < this->nchars)
        {   this->mchars = this->nchars;
            this->mchars |= 7;  // Round up - Use that which would be wasted. 
        }
		if (this->chars)
			free(this->chars);
        this->chars = (char*)alloc((this->mchars + 1) * sizeof(char));
    }
 
// Copy the string. 
    if (this->nchars > 0)
        memcpy(this->chars, str, this->nchars);
}


void csc_str_append_ch(csc_str_t *this, char ch)
{   int new_len;
 
// Get new length of the string. 
    new_len = this->nchars + 1;
 
// Allocate chars for the string. 
    if (this->mchars < new_len)
    {   this->mchars = this->mchars * 2 + 10;
        this->mchars |= 7;  // Round up - Use that which would be wasted. 
        this->chars = (char*)realloc(this->chars, (this->mchars+1)*sizeof(char));
        if (this->chars == NULL)
        {   error_handle("Memory allocation failure");
            exit(1);
        }
    }
 
// Copy the string. 
    {   this->chars[this->nchars] = ch;
        this->nchars = new_len;
    }
}


void csc_str_append(csc_str_t *this, const char *str)
{   int new_len;
    int str_len;
 
// Get new length of the string. 
    if (str == NULL)
        str_len = 0;
    else
        str_len = strlen(str);
    new_len = this->nchars + str_len;
 
// Allocate chars for the string. 
    if (this->mchars < new_len)
    {   this->mchars = this->mchars * 2 + 10;
        if (this->mchars < new_len)
        {   this->mchars = new_len;
        }
        this->mchars |= 7;  // Round up - Use that which would be wasted. 
        this->chars = (char*)realloc(this->chars, (this->mchars+1)*sizeof(char));
        if (this->chars == NULL)
        {   error_handle("Memory allocation failure");
            exit(1);
        }
    }
 
// Copy the string. 
    if (str_len > 0)
    {   memcpy(&this->chars[this->nchars], str, str_len);
        this->nchars = new_len;
    }
}


const char *csc_str_charr(const csc_str_t *this)
{   if (this->chars == NULL)
	{	return "";
	}
	else
	{	this->chars[this->nchars] = '\0';
		return this->chars;
	}
}


char *csc_str_alloc_charr(const csc_str_t *this)
{   char *str;
 
    str = (char*)alloc((this->nchars+1) * sizeof(char));
    if (this->nchars > 0)
        memcpy(str, this->chars, this->nchars);
    str[this->nchars] = 0;
    return str;
}


int csc_str_length(const csc_str_t *this)
{   return this->nchars;
}


int csc_str_getline(csc_str_t *this, FILE *fin)
{   int ch;
    csc_str_reset(this);
 
// Read in line. 
    ch = getc(fin);
    while (ch!=EOF && ch!='\n')
    {   if (ch != '\r')
            csc_str_append_ch(this, ch);
        ch = getc(fin);
    }
 
// Return result. 
    if (ch == EOF)
        return -1;
    else
        return csc_str_length(this);
}


#include <stdarg.h>
void csc_str_append_many(csc_str_t *this, ... )
{   va_list ap;
    char *str;
 
    va_start(ap, this);
    while (1)
    {   str = va_arg(ap, char*);
        if (str == NULL)
            break;
        csc_str_append(this, str);
    }
    va_end(ap);
}

