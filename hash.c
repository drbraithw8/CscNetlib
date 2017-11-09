// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.


#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "std.h"
#include "alloc.h"
#include "hash.h"

#define MUL 293
#define ADD 1

// Old way of iterating through a list.
// It still works, but you can have only one.
#define for_hash_key(h,i,key,dat,dat_type) \
    for (csc_hash_key_init(h,&i,key); ((dat)=(dat_type)csc_hash_key_next(h,&i))!=NULL; )

#define for_hash_all(h,i,dat,dat_type) \
    for (csc_hash_all_init(h,&i); ((dat)=(dat_type)csc_hash_all_next(h,&i))!=NULL; )


typedef struct csc_hash_node_t
{   struct csc_hash_node_t *next;
    void *data;
    unsigned long hash_val;
} csc_hash_node_t;


typedef struct csc_hash_t
{   csc_hash_node_t **table;
    unsigned long tblsize;
    unsigned long count;
    unsigned long maxcount;
    int offset;
    int (*cmp)(void*,void*);
    unsigned long (*hval)(void*);
    void (*free_rec)(void*);
} csc_hash_t;    


csc_hash_t *csc_hash_new(int offset, int (*cmp)(void*,void*),
                        csc_ulong (*hval)(void*), void (*free_rec)(void*) )
/*  This function allocates and initializes a hash table.  
 * Resolution is by chaining.  The table will have 'tblsize' elements.  
 * The key field of the record is at position 'offset' from the 
 * beginning of the record.  The function 'cmp'() must be able to 
 * compare two keys and return zero if they compare equal, non zero 
 * otherwise.  The function 'hval'() will generate a hash value from a 
 * key.  The value returned from 'hval'() will be subsequently 
 * subjected to MOD 'tblsize'.  The function 'free_rec'() is able to 
 * dispose of a record.
 */ 
{   csc_hash_t *h;
    h = csc_allocOne(csc_hash_t);
    h->tblsize = 40;
    h->count = 0;
    h->maxcount = h->tblsize * 5;
    h->table = (csc_hash_node_t**)csc_ck_calloc(h->tblsize * sizeof(csc_hash_node_t*));
    h->offset = offset;
    assert(cmp!=NULL);
    h->cmp = cmp;
    assert(hval!=NULL);
    h->hval = hval;
    assert(free_rec!=NULL);
    h->free_rec = free_rec;
    return h;
}


static void csc_hash_resize(csc_hash_t *h)
{   csc_hash_node_t *pt, *next;
    csc_ulong i_table;
    csc_hash_node_t **new_table;
    csc_ulong ndx, new_tblsize, old_tblsize;
 
/* Allocate new table (already zeroed) - on failure give up. */
    old_tblsize = h->tblsize;
    new_tblsize = h->maxcount * 2;
    new_table = (csc_hash_node_t**) csc_ck_calloc(new_tblsize * sizeof(csc_hash_node_t*));
    if (new_table == 0)
        return;
 
/* Transfer all nodes from old table to new. */
    for (i_table=0; i_table<old_tblsize; i_table++)
    {   pt = h->table[i_table];
        while (pt != 0)
        {   next = pt->next;
            ndx = pt->hash_val % new_tblsize;
            pt->next = new_table[ndx];
            new_table[ndx] = pt;
            pt = next;
        }
    }
 
/* Free old, Assign new. */
    free(h->table);
    h->table = new_table;
    h->tblsize = new_tblsize;
    h->maxcount = 5 * h->tblsize;
}


void csc_hash_add(csc_hash_t *h, void *rec)
/*  This function will add 'rec' to 'h' regardless of whether 
 * there are matching keys.
 */ 
{   csc_ulong ndx;
    csc_ulong hash_val;
    csc_hash_node_t *pt;
    
    hash_val = h->hval((void*)((char*)rec+h->offset));
    ndx = hash_val % h->tblsize;
    pt = csc_allocOne(csc_hash_node_t);
    pt->next = h->table[ndx];
    pt->data = rec;
    pt->hash_val = hash_val;
    h->table[ndx] = pt;
 
    if (h->count++ == h->maxcount)
        csc_hash_resize(h);
}


csc_bool_t csc_hash_addex(csc_hash_t *h, void *rec)
/*  If a key matching 'rec' already exists in 'h' then this 
 * function will return csc_FALSE.  Otherwise it will add 'rec' to 'hash' 
 * and return csc_TRUE.
 */
{   csc_ulong ndx;
    csc_ulong hash_val;
    csc_hash_node_t *pt, **headp;
    int (*cmp)(void*,void*) = h->cmp;
    int offset = h->offset;
    void *key = (char*)rec + offset;
 
    hash_val = h->hval(key);
    ndx = hash_val % h->tblsize;
    headp = &h->table[ndx];
    for (pt=*headp; pt!=NULL; pt=pt->next)
    {   if (pt->hash_val==hash_val && cmp(((char*)pt->data)+offset, key)==0)
            break;
    }
    if (pt == NULL)
    {   pt = csc_allocOne(csc_hash_node_t);
        pt->next = h->table[ndx];
        pt->data = rec;
        pt->hash_val = hash_val;
        h->table[ndx] = pt;
 
        if (h->count++ == h->maxcount)
            csc_hash_resize(h);
        return csc_TRUE;
    }
    else
        return csc_FALSE;
}


void *csc_hash_get(csc_hash_t *h, void *key)
/*  If no record with a key of 'key' exists in 'h', this function will
 * return NULL.  Otherwise it will return a pointer to any record
 * with a key of 'key' (LIFO).
 */ 
{   csc_ulong ndx;
    csc_ulong hash_val;
    csc_hash_node_t *pt;
    int offset = h->offset;
    int (*cmp)(void*,void*) = h->cmp;
 
    hash_val = h->hval(key);
    ndx = hash_val % h->tblsize;
    for (pt=h->table[ndx]; pt!=NULL; pt=pt->next)
    {   if (pt->hash_val==hash_val && cmp(((char*)pt->data)+offset, key)==0)
            break;
    }
    if (pt == NULL)
        return NULL;
    else
        return pt->data;
}


void *csc_hash_out(csc_hash_t *h, void *key)
/*  If no record with a key of 'key' exists in 'h', this function will
 * return NULL.  Otherwise it will return a pointer to any record
 * with a key of 'key' (LIFO).  This function removes the record from 'h'
 * but it does not free it.
 */   
{   csc_hash_node_t *pt, **ppt;
    csc_ulong ndx;
    csc_ulong hash_val;
    void *rec;
    int offset = h->offset;
    int (*cmp)(void*,void*) = h->cmp;
 
    hash_val = h->hval(key);
    ndx = hash_val % h->tblsize;
    ppt = &h->table[ndx];
    for (pt=*ppt; pt!=NULL; pt=pt->next)
    {   if (pt->hash_val==hash_val && cmp(((char*)pt->data)+offset, key)==0)
            break;
        ppt = &pt->next;
    }
    if (pt == NULL)
        return NULL;
    else
    {   rec = pt->data;
        *ppt = pt->next;
        free(pt);
        h->count--;
        return rec;
    }
}


csc_bool_t csc_hash_del(csc_hash_t *h, void *key)
/*  If no record with a key of 'key' exists in 'h', this function 
 * will return csc_FALSE.  Otherwise it will remove any record 
 * with a key of 'key', free it and return csc_TRUE.
 */   
{   void *p;
    p = csc_hash_out(h, key);
    if (p == NULL)
        return csc_FALSE;
    else
    {   h->free_rec(p);
        h->count--;
        return csc_TRUE;
    }
}


void csc_hash_free(csc_hash_t *h)
/*  This function will free any records remaining in 'h' and free the 
 * space associated with the table.
 */ 
{   csc_ulong i;
    csc_hash_node_t *pt, *next;
    csc_ulong size = h->tblsize;
    csc_hash_node_t **table = h->table;
    void (*free_rec)(void*) = h->free_rec;
 
    for (i=0; i<size; i++)
    {   pt = table[i];
        while (pt != 0)
        {   next = pt->next;
            free_rec(pt->data);
            free(pt);
            pt = next;
        }
        table[i] = 0;
    }
    free(table);
    free(h);
}


csc_ulong csc_hash_str(void *arg)
/*  Creates a hash index from a null terminated string.  (case sensitive).
 */
{   
    char *str = arg;
    csc_ulong sum;
    int ch;
 
    sum=1;
    while ((ch=*str++) != '\0')
        sum = (sum+ch+ADD)*MUL;
    return sum;
}


// --------------------------------------------------
// --------- Miscellaneous useful -------------------
// --------------------------------------------------

csc_ulong csc_hash_StrPt(void *pt)
{   return csc_hash_str(*(char**)pt);
}


int csc_hash_StrPtCmpr(void *pt1, void *pt2)
{   return strcmp(*(char**)pt1, *(char**)pt2);
}


csc_ulong csc_hash_ptr(void *pt)
/*  Creates a hash index from a pointer.
 */
{   csc_ulong pt_val = (csc_ulong)pt;
    int n_bytes = sizeof(csc_ulong);
    csc_ulong hind = 0;
    int i;
 
    for (i=0; i<n_bytes; i++)
    {   hind = hind * MUL + ADD + ((csc_ulong)255 & pt_val);
        pt_val >>= 8;  /* Shift right 1 byte. */
    }
 
    return hind;
}


int csc_hash_PtrCmpr(void *pt1, void *pt2)
/* Useful if the pointers are hashd.
 */
{   return pt1 != pt2;
}


#pragma argsused
void csc_hash_FreeNothing(void *blk)
{}


void csc_hash_FreeBlk(void *blk)
{   free(blk);
}


// --------------------------------------------------
// --------- Iterator -------------------------------
// --------------------------------------------------

typedef struct csc_hash_iter_t
{	csc_hash_t *hash;
    csc_hash_node_t *pt;
    void *key;
    unsigned long ilst;
    unsigned long hash_val;
} csc_hash_iter_t;


static void *csc_hash_key_next(csc_hash_iter_t *iter)
{	csc_hash_t *hash = iter->hash;
    int (*cmp)(void*,void*) = hash->cmp;
    int offset = hash->offset;
    void *key = iter->key;
    unsigned long hash_val = iter->hash_val;
	csc_hash_node_t *pt;
 
    for (pt=iter->pt; pt!=NULL; pt=pt->next)
    {   if (pt->hash_val==hash_val && cmp(((char*)pt->data)+offset, key)==0)
            break;
    }
    if (pt == NULL)
    {   iter->pt = NULL;
        return NULL;
    }
    else
    {   iter->pt = pt->next;
        return pt->data;
    }
}


static void *csc_hash_all_next(csc_hash_iter_t *iter)
{	int ilst;
    int nlst;
    csc_hash_node_t **table;
    csc_hash_node_t *pt = iter->pt;
 
    if (pt == NULL)
	{	csc_hash_t *hash = iter->hash;
        ilst = iter->ilst;
        nlst = hash->tblsize;
        table = hash->table;
        while (ilst<nlst && table[ilst]==NULL)
            ilst++;
        iter->ilst = ilst;
        if (ilst == nlst)
            return NULL;
        else
            pt = table[ilst];
    }
 
    iter->pt = pt->next;
    if (iter->pt == NULL)
        iter->ilst++;
    return pt->data;
}


csc_hash_iter_t *csc_hash_iter_new(csc_hash_t *hash, void *key)
{	csc_hash_iter_t *iter = csc_allocOne(csc_hash_iter_t);
	iter->hash = hash;
	iter->key = key;
	if (key == NULL)
	{	iter->ilst = 0;
		iter->pt = NULL;
	}
	else
	{	iter->hash_val = hash->hval(key);
    	iter->pt = hash->table[iter->hash_val % hash->tblsize];
	}
	return iter;
}


void csc_hash_iter_free(csc_hash_iter_t *iter)
{	free(iter);
}


void *csc_hash_iter_next(csc_hash_iter_t *iter)
{	if (iter->key == NULL)
		return csc_hash_all_next(iter);
	else
		return csc_hash_key_next(iter);
}


// --------------------------------------------------
// --------- nameValue class
// --------------------------------------------------

csc_nameVal_t *csc_nameVal_new(const char *name, const char *val)
{	csc_nameVal_t *nv = csc_allocOne(csc_nameVal_t);
	nv->name = csc_alloc_str(name);
	nv->val = csc_alloc_str(val);
	return nv;
}

void csc_nameVal_free(csc_nameVal_t *nv)
{	free((void*)nv->name);
	free((void*)nv->val);
	free(nv);
}

static void csc_nameVal_vfree(void *nv)
{	csc_nameVal_free((csc_nameVal_t*)nv);
}


// --------------------------------------------------
// --------- mapSS class: Map nameVal pairs.
// --------------------------------------------------

typedef struct csc_mapSS_t
{	csc_hash_t *hash;
} csc_mapSS_t;


csc_mapSS_t *csc_mapSS_new()
{	csc_mapSS_t *hss = csc_allocOne(csc_mapSS_t);
	hss->hash = csc_hash_new(offsetof(csc_nameVal_t,name),
	csc_hash_StrPtCmpr, csc_hash_StrPt, csc_nameVal_vfree);
	return hss;
}

void csc_mapSS_free(csc_mapSS_t *hss)
{	csc_hash_free(hss->hash);
	free(hss);
}

// void csc_mapSS_add(csc_mapSS_t *hss, const char *name, const char *val)
// {	csc_nameVal_t *nv = csc_nameVal_new(name, val);
// 	csc_hash_add(hss->hash, &nv);
// }

csc_bool_t csc_mapSS_addex(csc_mapSS_t *hss, const char *name, const char *val)
{	csc_nameVal_t *nv = csc_nameVal_new(name, val);
	csc_bool_t ret = csc_hash_addex(hss->hash, nv);
	if (!ret)
		csc_nameVal_vfree(nv);
	return ret;
}

const char *csc_mapSS_get(csc_mapSS_t *hss, const char *name)
{	csc_nameVal_t key;
	key.name = name;
	csc_nameVal_t *nv = csc_hash_get(hss->hash, &key);
	if (nv == NULL)
		return NULL;
	else
		return nv->val;
}

csc_bool_t csc_mapSS_out(csc_mapSS_t *hss, const char *name)
{	csc_nameVal_t *nv = csc_hash_out(hss->hash, &name);
	if (nv == NULL)
		return csc_FALSE;
	else
	{	csc_nameVal_free(nv);
		return csc_TRUE;
	}
}


// --------------------------------------------------
// --------- Iterator for mapSS class.
// --------------------------------------------------

typedef struct csc_mapSS_iter_t
{	csc_hash_iter_t *iter;
} csc_mapSS_iter_t;


// Constructor
csc_mapSS_iter_t *csc_mapSS_iter_new(csc_mapSS_t *hss)
{	csc_mapSS_iter_t *iter = csc_allocOne(csc_mapSS_iter_t);
	iter->iter = csc_hash_iter_new(hss->hash, NULL);
	return iter;
}

// Destructor
void csc_mapSS_iter_free(csc_mapSS_iter_t *iter)
{	csc_hash_iter_free(iter->iter);
	free(iter);
}

// Get next.
const csc_nameVal_t *csc_mapSS_iter_next(csc_mapSS_iter_t *iter)
{	return (csc_nameVal_t*)csc_hash_iter_next(iter->iter);
}





