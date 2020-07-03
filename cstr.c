// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

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
{   fprintf(csc_stderr, "Error in cstr: %s\n", errmsg);
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
    {   this->mchars = 0;
        this->chars = NULL;
    }
    else
    {   this->mchars = this->nchars + 8;
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


void csc_str_append_str(csc_str_t *this, const csc_str_t *str)
{   int new_len;
    int str_len;
 
// What if NULL?
    if (str == NULL)
        return;
 
// Get new length of the string. 
    str_len = str->nchars;
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
    {   memcpy(&this->chars[this->nchars], str->chars, str_len);
        this->nchars = new_len;
    }
}   


void csc_str_append(csc_str_t *this, const char *str)
{   int new_len;
    int str_len;
 
// What if NULL?
    if (str == NULL)
        return;
 
// Get new length of the string. 
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


void csc_str_assign(csc_str_t *this, const char *str)
{   this->nchars = 0;
    csc_str_append(this, str);
}


void csc_str_assign_str(csc_str_t *this, csc_str_t *str)
{
// TODO: This is the very lazy placeholder version.  A native version would
// be more efficient and properly handle the null character.
    this->nchars = 0;
    csc_str_append(this, csc_str_charr(str));
}


void csc_str_truncate(csc_str_t *this, int len)
{   if (len>=0 && len<this->nchars)
        this->nchars = len;
}


const char *csc_str_charr(const csc_str_t *this)
{   if (this->chars == NULL)
    {   return "";
    }
    else
    {   this->chars[this->nchars] = '\0';
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


int csc_str_getword(csc_str_t *this, FILE *fin)
{	int len=0;
	int ch = getc(fin);
    csc_str_reset(this);
 
/* Skip whitespace */
	while(isspace(ch))
		ch = getc(fin);
 
/* Deal with a possible EOF */
	if (ch == EOF)
		len = -1;
 
/* read in the word */
	while(!isspace(ch) && ch!=EOF)
	{	len++;
		csc_str_append_ch(this, ch);
		ch = getc(fin);
	}
 
// Return result. 
	return len;
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

#define csc_str_BufLen 127

static csc_str_t *readFspec(char *fmt, int *lenP, char *specChar)
{   int len = 1;
    int longness = 0;
    int ch;
    csc_str_t *spec = csc_str_new("%");
    while ((ch = *fmt++)) switch(ch)
    {   case 'd': case 'i': case 'c': case 'o':
        case 'u': case 'x': case 'X':
            csc_str_append_ch(spec, ch);
            *lenP = len;
            if (longness == 0)
                *specChar = 'd';
            else if (longness == 1)
                *specChar = 'l';
            else if (longness == 2)
                *specChar = 'L';
            else
            {   csc_str_assign(spec, "longness in format not supported");
                *specChar = '\0';
            }
            return spec;
        case 's':
            csc_str_append_ch(spec, ch);
            *specChar = 's';
            *lenP = len;
            return spec;
        case 'S':
            csc_str_append_ch(spec, ch);
            *specChar = 'S';
            *lenP = len;
            return spec;
        case 'f': case 'F': case 'e': case 'E': case 'g': case 'G': 
            csc_str_append_ch(spec, ch);
            *specChar = 'f';
            *lenP = len;
            return spec;
        case 'p':
            csc_str_append_ch(spec, ch);
            *specChar = 'p';
            *lenP = len;
            return spec;
        case 'l':
            csc_str_append_ch(spec, ch);
            len++;
            longness++;
            break;
        case '*': case '$': case 'a': case 'C':
        case 'n': case 'm': case '%': 
            csc_str_assign(spec, "format not supported");
            *specChar = '\0';
            *lenP = len;
            return spec;
        default:
            csc_str_append_ch(spec, ch);
            len++;
    }
    csc_str_assign(spec, "Unterminated format specifier");
    *lenP = 0;
    *specChar = '\0';
    return spec;
}


void csc_str_append_f(csc_str_t *this, char *fmt, ... )
{   char ch, specChar;
    csc_str_t *fSpec;
    char buf[csc_str_BufLen];
    char *pf = fmt;
    int len;
    va_list ap;
    va_start(ap, fmt);
    while ((ch = *pf++))
    {   if (ch == '%')
        {   if (*pf == '%')
            {   csc_str_append_ch(this, ch);
                pf++;
            }
            else
            {   fSpec = readFspec(pf, &len, &specChar);
                switch(specChar)
                {   case 's':
                        csc_str_append(this, va_arg(ap,char*));
                        break;
                    case 'S':
                        csc_str_append_str(this, va_arg(ap,csc_str_t*));
                        break;
                    case 'd':
                        sprintf(buf, csc_str_charr(fSpec), va_arg(ap,int));
                        csc_str_append(this, buf);
                        break;
                    case 'f':
                        sprintf(buf, csc_str_charr(fSpec), va_arg(ap,double));
                        csc_str_append(this, buf);
                        break;
                    case 'p':
                        sprintf(buf, csc_str_charr(fSpec), va_arg(ap,void*));
                        csc_str_append(this, buf);
                        break;
                    case 'l':
                        sprintf(buf, csc_str_charr(fSpec), va_arg(ap,long int));
                        csc_str_append(this, buf);
                        break;
                    case 'L':
                        sprintf(buf, csc_str_charr(fSpec), va_arg(ap,long long int));
                        csc_str_append(this, buf);
                        break;
                    case '\0':
                        csc_str_assign_str(this, fSpec);
                        fprintf(stderr, "%s", "Error: ");
                        csc_str_out(this, stderr);
                        fprintf(stderr, "\n");
                        // no break - deliberate fall through.
                    default:
                        assert(csc_FALSE);
                }
                csc_str_free(fSpec);
                pf += len;
            }
        }
        else
            csc_str_append_ch(this, ch);
    }
    va_end(ap);
}


size_t csc_str_out(csc_str_t *this, FILE *fout)
{   return fwrite(this->chars, 1, this->nchars, fout);
}

