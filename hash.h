// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#ifndef csc_HASH_H
#define csc_HASH_H 1
#include "std.h"

typedef struct csc_hash_t csc_hash_t;
typedef struct csc_hash_node_t csc_hash_node_t;

typedef struct
{
/* Innards are private! */
    csc_hash_node_t *pt;
    void *key;
    unsigned long ilst;
    unsigned long hash_val;
} csc_hash_iter_t;


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

void csc_hash_add(csc_hash_t *hash, void *rec);
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



#define for_hash_key(h,i,key,dat,dat_type) \
    for (csc_hash_key_init(h,&i,key); ((dat)=(dat_type)csc_hash_key_next(h,&i))!=NULL; )

#define for_hash_all(h,i,dat,dat_type) \
    for (csc_hash_all_init(h,&i); ((dat)=(dat_type)csc_hash_all_next(h,&i))!=NULL; )

void csc_hash_key_init(csc_hash_t *h, csc_hash_iter_t *i, void *key);
void *csc_hash_key_next(csc_hash_t *h, csc_hash_iter_t *i);
void csc_hash_all_init(csc_hash_t *h, csc_hash_iter_t *i);
void *csc_hash_all_next(csc_hash_t *h, csc_hash_iter_t *i);


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
// --------- mapStrStr class
// --------------------------------------------------

typedef struct csc_mapStrStr_t csc_mapStrStr_t;

csc_mapStrStr_t *csc_mapStrStr_new();

void csc_mapStrStr_free(csc_mapStrStr_t *hss);

csc_bool_t csc_mapStrStr_addex(csc_mapStrStr_t *hss, const char *name, const char *val);

const char *csc_mapStrStr_get(csc_mapStrStr_t *hss, const char *name);

void *csc_mapStrStr_out(csc_mapStrStr_t *hss, const char *name);


#endif
