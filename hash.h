// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#ifndef csc_HASH_H
#define csc_HASH_H 1
#include "std.h"

typedef struct csc_hash_t csc_hash_t;


csc_hash_t *csc_hash_new(int offset, int (*cmp)(void*,void*),
                unsigned long (*hval)(void*), void (*free_rec)(void*) );
/*  This function allocates and initializes a hash table.  
 * Resolution is by chaining.  The key field of the record is at
 * position 'offset' from the beginning of the record.  The function
 * 'cmp'() must be able to compare two keys and return zero if they
 * compare equal, non zero otherwise.  The function 'hval'() will
 * generate a hash value from a key.  The value returned from 'hval'()
 * will be subsequently subjected to MOD (whatever is the tablesize). 
 * The function 'free_rec'() is able to dispose of a record.
 */ 

csc_bool_t csc_hash_addex(csc_hash_t *hash, void *rec);
/*  If a key matching 'rec' already exists in 'hash' then this 
 * function will return csc_FALSE.  Otherwise it will add 'rec' to 'hash' 
 * and return csc_TRUE.
 */

// void csc_hash_add(csc_hash_t *hash, void *rec);
/*  This function will add 'rec' to 'hash' regardless of whether 
 * there are matching keys.
 */ 

void *csc_hash_get(csc_hash_t *hash, void *key);
/*  If no record with a key of 'key' exists in 'hash', this function will
 * return NULL.  Otherwise it will return a pointer a record
 * with a key of 'key'.
 */ 

void *csc_hash_out(csc_hash_t *hash, void *key);
/*  If no record with a key of 'key' exists in 'hash', this function will
 * return NULL.  Otherwise it will return a pointer to the last added record
 * with a key of 'key' (LIFO).  This function removes the record from 'hash
 * but it does not free it.
 */   

csc_bool_t csc_hash_del(csc_hash_t *hash, void *key);
/*  If no record with a key of 'key' exists in 'hash', this function 
 * will return csc_FALSE.  Otherwise it will remove the last added record 
 * with a key of 'key', free it and return csc_TRUE.
 */   

void csc_hash_free(csc_hash_t *hash);
/*  This function will free any records remaining in 'hash' and free the 
 * space associated with the table.
 */ 

int csc_hash_count(csc_hash_t *h);
/* Returns the number of elements in the hash table.
 */

// --------------------------------------------------
// --------- Miscellaneous useful -------------------
// --------------------------------------------------

unsigned long csc_hash_str(void *str);
/*  Creates a hash index from a null terminated string.  (case sensitive).
 */

unsigned long csc_hash_ptr(void *pt);
/*  Creates a hash index from a pointer and returns it.
 */

int csc_hash_PtrCmpr(void *pt1, void *pt2);
/* Compares two pointers.
 */

unsigned long csc_hash_StrPt(void *pt);
/*  Creates a hash index from a null terminated string.  (case sensitive).
 * 'pt' points to the char* pointer which gives the string.  Handy if the
 * key feild is a POINTER to str.
 */

int csc_hash_StrCmpr(void *pt1, void *pt2);
// For comparison of string fields.  Both 'pt1' and 'pt2' point
// to the beginning of the string.  Handy if the key field is an
// embedded string or the entire record is just a char*.
// 
// Use this in preference to passing strcmp() becuase strcmp could be a
// macro and because passing
// (int (*)(void*,void*))strcmp(const char*,const char*) might be dangerous.
// The validation suite casts strcmp like that, so it could potentially be
// OK so long as the validation passes.  

int csc_hash_StrPtCmpr(void *pt1, void *pt2);
/* For comparison of string fields.  Both 'pt1' and 'pt2' point
 * to the char* pointers which gives the string.
 */

void csc_hash_FreeNothing(void *blk);
/* Free's nothing at all.
 */

void csc_hash_FreeBlk(void *blk);
/* Free's a block that needs no further freeing.  Useful because free()
 * can be a macro.
 */


// --------------------------------------------------
// --------- Iterator -------------------------------
// --------------------------------------------------

// Current way of iterating through a list.
typedef struct csc_hash_iter_t csc_hash_iter_t;

// Constructor
csc_hash_iter_t *csc_hash_iter_new(csc_hash_t *hash);

// Destructor
void csc_hash_iter_free(csc_hash_iter_t *iter);

// Get each item.
void *csc_hash_iter_next(csc_hash_iter_t *iter);


// --------------------------------------------------
// --------- nameValue class
// --------------------------------------------------

typedef struct csc_nameVal_t
{   const char *val;
    const char *name;
} csc_nameVal_t;

csc_nameVal_t *csc_nameVal_new(const char *name, const char *val);

void csc_nameVal_free(csc_nameVal_t *nv);


// --------------------------------------------------
// --------- mapSS class: Map nameVal pairs.
// --------------------------------------------------

typedef struct csc_mapSS_t csc_mapSS_t;

// Constructor
csc_mapSS_t *csc_mapSS_new();

// Destructor
void csc_mapSS_free(csc_mapSS_t *hss);

// Number of elements.
int csc_mapSS_count(csc_mapSS_t *hss);

// Add a name value pair.  Return csc_TRUE on success.
csc_bool_t csc_mapSS_addex(csc_mapSS_t *hss, const char *name, const char *val);

// Get the name value pair corresponding to a name.
const csc_nameVal_t *csc_mapSS_get(csc_mapSS_t *hss, const char *name);

// Remove a name value pair.
csc_bool_t csc_mapSS_out(csc_mapSS_t *hss, const char *name);


// --------------------------------------------------
// --------- Iterator for mapSS class.
// --------------------------------------------------

typedef struct csc_mapSS_iter_t csc_mapSS_iter_t;

// Constructor
csc_mapSS_iter_t *csc_mapSS_iter_new(csc_mapSS_t *hss);

// Destructor
void csc_mapSS_iter_free(csc_mapSS_iter_t *iter);

// Get the next one (or the first one).
const csc_nameVal_t *csc_mapSS_iter_next(csc_mapSS_iter_t *iter);


#endif
