// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#ifndef csc_CSTR_H
#define csc_CSTR_H 1

#include <stdio.h>

#include "std.h"

typedef struct csc_str_t csc_str_t;

/* Constructors.  Destructors. */

    // This constructs a new string object initialised with the string
    // 'str' and returns a pointer to it.  If 'str' is the NULL pointer,
    // then the empty string is used as the initial value.
    csc_str_t *csc_str_new(const char *str);

    // Frees all memory associated with the string 'this'.
    void csc_str_free(csc_str_t *this);



// Assign.
#define csc_str_reset(this) csc_str_assign(this, NULL)

    // Set this to 'str'.
    void csc_str_assign(csc_str_t *this, const char *str);
    void csc_str_assign_str(csc_str_t *this, csc_str_t *str);


// Truncate.  Does nothing if string is shorter than 'len'.
    void csc_str_truncate(csc_str_t *this, int len);


/* Append. */

    // Assigns 'ch' onto the end of 'this'.
    void csc_str_append_ch(csc_str_t *this, char ch);

    // Assigns 'str' onto the end of 'this'.
    void csc_str_append(csc_str_t *this, const char *str);
    void csc_str_append_str(csc_str_t *this, const csc_str_t *str);

    // Appends an arbitrary number of C style strings onto the end of 'this'.
    // An argument of NULL indicates no more strings, e.g. The following will
    // result in 'str' getting "Jack and Jill":-
    // csc_str_t *cstr = csc_str_new(NULL);
    // csc_str_append_many(cstr,  "Jack",  " and ",  "Jill",  NULL);
    void csc_str_append_many(csc_str_t *this, ... );

	// sprintf() for csc_str_t.
	// 
	// Conversion specifiers 'd','i','o','u','x','X','f','F','g','G','e','E','p'
	// in combination with field width and precision are supported, providing
	// that the output from a single field does not exceed 127 characters.
	// Conversion specifiers 'a','A','n','m' are not supported.
	// 
	// The conversion specifier 's' is supported, but modifiers, width and
	// precision are ignored (i.e. only the simple "%s").  The conversion
	// specifier 'S' refers to a csc_str_t * and is supported without modifiers
	// width and precision . 
	// 
	// Modifiers  'l' and 'll' are supported for ints.  Modifiers
	// 't','Z','z','j','L','q','h', '*', '$', are not supported.
	void csc_str_append_f(csc_str_t *this, char *fmt, ... );


/* Converters to standard C string. */

    // Returns a pointer to a null terminated READ ONLY copy of
    // 'this' string.  You can use the returned string only up until 'this'
    // is changed, as the static buffer will then have been overwritten.
    const char *csc_str_charr(const csc_str_t *this);

    // Returns a null terminated copy of 'this' string that has been
    // allocated by malloc().  The user must free() the space after
    // she is finished with it.
    char *csc_str_alloc_charr(const csc_str_t *this);

/* Misc. */

    // Returns the length of 'this' string.
    int csc_str_length(const csc_str_t *this);

    // Reads a line from stream 'fin' into 'this'.  The terminating newline
    // is read, but not included into 'this'.  Any terminating '\r' is read
    // past, and ignored.  Returns the length of the resulting string in
    // 'this', which will be zero for empty lines read in.  If there were
    // no lines encountered because an EOF was found, then -1 will be returned.
    int csc_str_getline(csc_str_t *this, FILE *fin);

	// Output.  Can even output strings containing null characters.
	size_t csc_str_out(csc_str_t *this, FILE *fout); 

#endif
