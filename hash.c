// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.


#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <CscNetLib/std.h>
#include <CscNetLib/alloc.h>
#include <CscNetLib/list.h>
#include "hash.h"

#define HashBits 8
#define TblSiz (1<<HashBits)
#define MaxLevel 7
#define MaxLevels (MaxLevel+1)
#define MaxLstSiz 16
#define MinLstSiz 4
#define toNdx(hval,level) (((uint8_t*)(&(hval)))[level])
// #define toNdx(hval,level) (((hval) >> (HashBits*level)) & (TblSiz-1))

#define msb64_setBit(a)  { a |= ((uint64_t)1<<63); }
#define msb64_clrBit(a)  { a &= (~((uint64_t)1<<63)); }
#define msb64_isSet(a) (((a) & ((uint64_t)1<<63))!=0)
#define msb64_count(a) ((a) & (~((uint64_t)1<<63)))

#define bm_setbit(a,i)             ((a)[(i)/8] |= 1 << ((i) % 8))
#define bm_clrbit(a,i)             ((a)[(i)/8] &= ~(1 << ((i) % 8)))
#define bm_isset(a,i)              ((a)[(i)/8] & (1 << ((i) % 8)))

typedef struct node_t
{   struct node_t *next;
    void *data;
    uint64_t hval;
} node_t;


typedef struct tbl_t tbl_t;

typedef struct
{   union
    {   node_t *list;
        tbl_t *hTbl;
    };
    uint64_t count;
} tblEnt_t;


typedef struct tbl_t
{   tblEnt_t eTbl[TblSiz];
    union
    {   uint8_t useBytes[TblSiz/8];
        uint32_t useWords[TblSiz/32];
    };
} tbl_t;


typedef struct csc_hash_t
{
// First Level.
    tblEnt_t ent;
 
// All about the record type.
    int offset;
    int (*cmp)(void*,void*);
    uint64_t (*hval)(void*);
    void (*freeRec)(void*);
} csc_hash_t;


typedef struct
{   tblEnt_t *eTbl;
    int tNdx;
} iterStkElem_t;


typedef struct csc_hash_iter_t
{   iterStkElem_t *stk;
    int sNdx;
    node_t *node;
    csc_hash_t *hash;
} csc_hash_iter_t;


static void iter_deepNext(csc_hash_iter_t *it)
{   iterStkElem_t *stk = it->stk;  // The stack with one less pointer.
    int sNdx = it->sNdx;          // The stack with one less pointer.
    int tNdx = stk[sNdx].tNdx;   // The index into the table.
    tblEnt_t *eTbl = stk[sNdx].eTbl;  // The table.
 
    for (;;)
    {   if (++tNdx == TblSiz)   // Next index, then are we finished this table.
        {   if (sNdx == 0)  // If we are finished the root table, then we are done.
            {   it->node = NULL;  // Set result data to indicate we are done.
                break;  // return finished. No need to pack coz we aint returning.
            }
            else  // Not the root table, then pop().
            {   sNdx = --it->sNdx;
                tNdx = stk[sNdx].tNdx;  // Unpack the table index.
                eTbl = stk[sNdx].eTbl;  // Unpack the table.
            }
        }
        else
        {   tblEnt_t *ent = &eTbl[tNdx];  // Short hand for current table entry.
            if (msb64_isSet(ent->count))  // Is the entry a table?, then push().
            {   stk[sNdx].tNdx = tNdx;   // Pack the table index.
                stk[sNdx].eTbl = eTbl;   // Pack the table.
                it->sNdx = ++sNdx;       // New stack element.
                eTbl = ent->hTbl->eTbl;  // New table entry.
                tNdx = -1;    // Position new table index before first element.
            }
            else  // Not a table but a simple list.
            {   it->node = ent->list;  // Set resulting data to the list.
                if (it->node)      // If the list is not empty.
                {   stk[sNdx].tNdx = tNdx;  // Pack the table index.
                    stk[sNdx].eTbl = eTbl;  // Pack the table.
                    break;       // .. coz we are returning with the list.
                }
            }
        }
    }
}


csc_hash_iter_t *csc_hash_iter_new(csc_hash_t *hash)
{   csc_hash_iter_t *it = csc_allocOne(csc_hash_iter_t);
    it->stk = NULL;
    it->sNdx = -1;
    it->node = NULL;
    it->hash = hash;
    if (msb64_isSet(hash->ent.count))  // If 'hash' has a table.
    {   // Allocate the stack.
        iterStkElem_t *stk = csc_allocMany(iterStkElem_t, MaxLevels);
        it->stk = stk;
 
        // Set stack prior to first entry in the root table.
        stk[0].eTbl = hash->ent.hTbl->eTbl;  // Root table.
        stk[0].tNdx = -1;  // Prior to first table entry.
    }
    return it;
}


void *csc_hash_iter_next(csc_hash_iter_t *it)
{
// Set it->node to be the one holding the data.
    if (it->sNdx == -1)  // If first call of next() for 'it'.
    {   it->sNdx = 0;   // Subsequent calls are not first call.
        if (it->stk == NULL)  // if 'it' has no tables.
            it->node = it->hash->ent.list;  // then 'it' has only this list.
        else
            iter_deepNext(it);  // otherwise, find the first list.
    }
    else  // Not first call.
    {   if (it->node)   // If the current list is not finished.
        {   it->node = it->node->next;  // Get the next node.
            if (!it->node && it->stk!=NULL)  // If list exhausted and compound table.
                iter_deepNext(it);  // Find the next list.
        }
    }
 
// Return the data.
    if (it->node)  // it->node has the result.
        return it->node->data;
    else
        return NULL;
}


void csc_hash_iter_free(csc_hash_iter_t *it)
{   if (it->stk)
        free(it->stk);
    free(it);
}


csc_hash_t *csc_hash_new( int offset
                  , int (*cmp)(void*,void*)
                  , ulong (*hval)(void*)
                  , void (*freeRec)(void*)
                )
{   csc_hash_t *h;
    h = csc_allocOne(csc_hash_t);
    h->ent.count = 0;   // Count of zero. And it points to a list.
    h->ent.list = NULL;
    h->offset = offset;
    csc_assert(cmp!=NULL);
    h->cmp = cmp;
    csc_assert(hval!=NULL);
    h->hval = hval;
    csc_assert(freeRec!=NULL);
    h->freeRec = freeRec;
    return h;
}


static void cvtLstToTbl(tblEnt_t *ent, int level)
{   node_t *list = ent->list;
    tbl_t *hTbl = csc_ck_calloc(sizeof(tbl_t));
    ent->hTbl = hTbl;
    tblEnt_t *eTbl = hTbl->eTbl;
    uint8_t *useBytes = hTbl->useBytes;
    msb64_setBit(ent->count);
    while (list != NULL)
    {   int ndx = toNdx(list->hval,level);
        ent = &eTbl[ndx];
        node_t *next = list->next;
        list->next = ent->list;
        ent->list = list;
        if (ent->count == 0)
            bm_setbit(useBytes,ndx);
        ent->count++;
        list = next;
    }
}


// // -----------------------
// // PROBLEM WITH HASH_ADD()
// // -----------------------
// // I convert a list to a table when it reaches a length of MaxLstSiz.  But
// // if they are all duplicates, making another table just wont reduce the
// // size of the linked list.  You Would end up with an 8 tier hash table
// // with long linked list.  hash_add() made sense in the old design, but not
// // in this design.
// //
// // Multiple keys with clashing hashes will degrade performance just a
// // duplicate keys, so a very good hashing function is required.
// //
// // I am also wondering whether it is worth having csc_hash_out(), because this
// // too came with an efficiency and memory overhead.
// // --------------------------------
// void hash_add(csc_hash_t *h, void *data)
// {   int ndx;
// 	int level = 0;
// 	node_t *node;
// 	tbl_t *hTbl = NULL;
// 	int offset = h->offset;
//     void *key = (char*)data + offset;
//     uint64_t hval = h->hval(key);
// 	tblEnt_t *ent = &h->ent;
// 	int (*cmp)(void*,void*) = h->cmp;
//
// // Loop to outermost table.
// 	while (msb64_isSet(ent->count))
// 	{	ent->count++;
// 		hTbl = ent->hTbl;
// 		ndx = toNdx(hval,level);
// 		ent = &(hTbl->eTbl[ndx]);
// 		level++;
// 	}
//
// // Add in the node.
// 	node = csc_allocOne(node_t);
// 	node->next = ent->list;
// 	node->data = data;
// 	node->hval = hval;
// 	ent->list = node;
// 	if (hTbl!=NULL && ent->count == 0)
// 		bm_setbit(hTbl->useBytes,ndx);
// 	ent->count++;
// 	if (ent->count > MaxLstSiz)
// 		cvtLstToTbl(ent, level);
// }


void *csc_hash_get(csc_hash_t *h, void *key)
{   int level = 0;
    node_t *pt;
    int offset = h->offset;
    uint64_t hval = h->hval(key);
    tblEnt_t *ent = &h->ent;
    int (*cmp)(void*,void*) = h->cmp;
 
// Loop to outermost table.
    while (msb64_isSet(ent->count))
    {   int ndx = toNdx(hval,level);
        ent = &(ent->hTbl->eTbl[ndx]);
        level++;
    }
 
// Loop through list elements.
    for (pt=ent->list; pt!=NULL; pt=pt->next)
    {   if (pt->hval==hval && cmp(((char*)pt->data)+offset, key)==0)
            break;
    }
 
// Return the result.
    if (pt == NULL)
        return NULL;
    else
        return pt->data;
}


static csc_bool_t addex( csc_hash_t *h
                         , void *data
                         , tblEnt_t *ent
                         , uint64_t hval
                         , int level
                       )
{   csc_bool_t retVal;
    if (msb64_isSet(ent->count))
    {   int ndx = toNdx(hval,level);
        tblEnt_t *childEnt = &ent->hTbl->eTbl[ndx];
        retVal = addex(h, data, childEnt, hval, level+1);
        if (retVal)
        {   if (msb64_count(childEnt->count) == 1)
                bm_setbit(ent->hTbl->useBytes, ndx);
            ent->count++;
        }
    }
    else
    {   node_t *pt;
        int offset = h->offset;
        int (*cmp)(void*,void*) = h->cmp;
        void *key = (char*)data + offset;
        for (pt=ent->list; pt!=NULL; pt=pt->next)
        {   if (pt->hval==hval && cmp(((char*)pt->data)+offset, key)==0)
                break;
        }
        if (pt == NULL)
        {   retVal = csc_TRUE;
            pt = csc_allocOne(node_t);
            pt->next = ent->list;
            pt->data = data;
            pt->hval = hval;
            ent->list = pt;
            ent->count++;
            if (ent->count > MaxLstSiz && level<MaxLevel)
                cvtLstToTbl(ent, level);
        }
        else
        {   retVal = csc_FALSE;
        }
    }
    return retVal;
}


csc_bool_t csc_hash_addex(csc_hash_t *h, void *data)
{   void *key = (char*)data + h->offset;
    return addex(h, data, &h->ent, h->hval(key), 0);
}


static void cvtTblToLst(tblEnt_t *ent)
{
// For list handling.
    node_t *list = NULL;
    node_t *pt, *oldList;
 
// For loop counting.
    int ndx_end, ndx, ib32, ib8_end, ib8;
 
// For data structure.
    tbl_t *hTbl = ent->hTbl;
    tblEnt_t *eTbl = hTbl->eTbl;
    uint8_t *useBytes = hTbl->useBytes;
    uint32_t *useWords = hTbl->useWords;
 
// Gather the list pointers.
    for (ib32=0; ib32<8; ib32++)
    {   if (useWords[ib32])
        {   ib8 = ib32 * 4;
            ib8_end = ib8 + 4;
            for (; ib8<ib8_end; ib8++)
            {   if (useBytes[ib8])
                {   ndx = ib8 * 8;
                    ndx_end = ndx + 8;
                    for (; ndx<ndx_end; ndx++)
                    {   oldList = eTbl[ndx].list;
                        while (oldList != NULL)
                        {   pt = oldList;
                            oldList = pt->next;
                            pt->next = list;
                            list = pt;
                        }
                    }
                }
            }
        }
    }
 
// 	for (ndx=0; ndx<256; ndx++)
// 	{	oldList = eTbl[ndx].list;
// 		while (oldList != NULL)
// 		{	pt = oldList;
// 			oldList = pt->next;
// 			pt->next = list;
// 			list = pt;
// 		}
// 	}
 
// Finish the conversion.
    free(hTbl);
    ent->list = list;
    msb64_clrBit(ent->count);
}


static void *hOut( csc_hash_t *h
                   , void *key
                   , tblEnt_t *ent
                   , uint64_t hval
                   , int level
                 )
{   void *data = NULL;
    if (msb64_isSet(ent->count))
    {   int ndx = toNdx(hval,level);
        tblEnt_t *childEnt = &ent->hTbl->eTbl[ndx];
        data = hOut(h, key, childEnt, hval, level+1);
        if (data != NULL)
        {   if (childEnt->count == 0)
                bm_clrbit(ent->hTbl->useBytes, ndx);
            ent->count--;
            if (msb64_count(ent->count) < MinLstSiz)
            {   // fprintf(stderr, "cvtTblToLst() key=\"%s\"\n", (char*)key);
                cvtTblToLst(ent);
            }
        }
    }
    else
    {   int offset = h->offset;
        int (*cmp)(void*,void*) = h->cmp;
        node_t **prev = &ent->list;
        node_t *lp = *prev;
        while (lp != NULL)
        {   if (lp->hval==hval && cmp(((char*)lp->data)+offset, key)==0)
                break;
            prev = &lp->next;
            lp = *prev;
        }
        if (lp != NULL)
        {   *prev = lp->next;
            data = lp->data;
            free(lp);
            ent->count--;
        }
    }
    return data;
}


void *csc_hash_out(csc_hash_t *h, void *key)
{   return hOut(h, key, &h->ent, h->hval(key), 0);
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
    {   h->freeRec(p);
        return csc_TRUE;
    }
}


static void hfree(tblEnt_t *ent, void (*freeRec)(void*))
{   if (msb64_isSet(ent->count))
    {   tblEnt_t *eTbl = ent->hTbl->eTbl;
        for (int i=0; i<TblSiz; i++)
            hfree(&(eTbl[i]), freeRec);
        free(ent->hTbl);
    }
    else
    {   node_t *pt=ent->list;
        while (pt != NULL)
        {   node_t *prev = pt;
            freeRec(pt->data);
            pt = pt->next;
            free(prev);
        }
    }
}


void csc_hash_free(csc_hash_t *h)
{   hfree(&h->ent, h->freeRec);
    free(h);
}


int csc_hash_count(csc_hash_t *h)
{   return msb64_count(h->ent.count);
}



uint64_t csc_hash_StrPt(void *pt)
{   return csc_hash_str(*(char**)pt);
}


int csc_hash_StrCmpr(void *pt1, void *pt2)
{   return strcmp((char*)pt1, (char*)pt2);
}


int csc_hash_StrPtCmpr(void *pt1, void *pt2)
{   return strcmp(*(char**)pt1, *(char**)pt2);
}


uint64_t csc_hash_ptr(void *pt)
/*  Creates a hash index from a pointer.
 */
{   uint64_t pt_val = (uint64_t)pt;
    int n_bytes = sizeof(uint64_t);
    uint64_t hind = 0;
    int i;
    const int MUL = 293;

    for (i=0; i<n_bytes; i++)
    {   hind = hind * MUL + ((uint64_t)255 & pt_val);
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
// --------- nameValue class
// --------------------------------------------------

csc_nameVal_t *csc_nameVal_new(const char *name, const char *val)
{   csc_nameVal_t *nv = csc_allocOne(csc_nameVal_t);
    nv->name = csc_alloc_str(name);
    if (val == NULL)
        nv->val = NULL;
    else
        nv->val = csc_alloc_str(val);
    return nv;
}

void csc_nameVal_free(csc_nameVal_t *nv)
{
    free((void*)nv->name);
    if (nv->val)
        free((void*)nv->val);
    free(nv);
}

static void csc_nameVal_vfree(void *nv)
{   csc_nameVal_free((csc_nameVal_t*)nv);
}


// --------------------------------------------------
// --------- mapSS class: Map nameVal pairs.
// --------------------------------------------------

typedef struct csc_mapSS_t
{   csc_hash_t *hash;
} csc_mapSS_t;


csc_mapSS_t *csc_mapSS_new()
{   csc_mapSS_t *hss = csc_allocOne(csc_mapSS_t);
    hss->hash = csc_hash_new( offsetof(csc_nameVal_t,name)
                            , csc_hash_StrPtCmpr
                            , csc_hash_StrPt
                            , csc_nameVal_vfree
                            );
    return hss;
}

void csc_mapSS_free(csc_mapSS_t *hss)
{   csc_hash_free(hss->hash);
    free(hss);
}

csc_bool_t csc_mapSS_addex(csc_mapSS_t *hss, const char *name, const char *val)
{   csc_nameVal_t *nv = csc_nameVal_new(name, val);
    csc_bool_t ret = csc_hash_addex(hss->hash, nv);
    if (!ret)
        csc_nameVal_free(nv);
    return ret;
}

int csc_mapSS_count(csc_mapSS_t *hss)
{   return csc_hash_count(hss->hash);
}

const csc_nameVal_t *csc_mapSS_get(csc_mapSS_t *hss, const char *name)
{   return csc_hash_get(hss->hash, &name);
}

csc_bool_t csc_mapSS_out(csc_mapSS_t *hss, const char *name)
{   csc_nameVal_t *nv = csc_hash_out(hss->hash, &name);
    if (nv == NULL)
        return csc_FALSE;
    else
    {   csc_nameVal_free(nv);
        return csc_TRUE;
    }
}


// --------------------------------------------------
// --------- Iterator for mapSS class.
// --------------------------------------------------

typedef struct csc_mapSS_iter_t
{   csc_hash_iter_t *iter;
} csc_mapSS_iter_t;


// Constructor
csc_mapSS_iter_t *csc_mapSS_iter_new(csc_mapSS_t *hss)
{   csc_mapSS_iter_t *iter = csc_allocOne(csc_mapSS_iter_t);
    iter->iter = csc_hash_iter_new(hss->hash);
    return iter;
}

// Destructor
void csc_mapSS_iter_free(csc_mapSS_iter_t *iter)
{   csc_hash_iter_free(iter->iter);
    free(iter);
}

// Get next.
const csc_nameVal_t *csc_mapSS_iter_next(csc_mapSS_iter_t *iter)
{   return (csc_nameVal_t*)csc_hash_iter_next(iter->iter);
}

