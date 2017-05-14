// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.
/* ------------------------------------------------------------------------
 * Copyright 1991 Stephen Braithwaite
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it, subject
 * to The Artistic Licence 1.0
 * http://www.perlfoundation.org/artistic_license_1_0
 * ------------------------------------------------------------------------
 */ 


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "std.h"
#include "alloc.h"

#define MUL 293
#define ADD 1

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
{   
    csc_hash_node_t **table;
    unsigned long tblsize;
    unsigned long count;
    unsigned long maxcount;
    int offset;
    int (*cmp)(void*,void*);
    unsigned long (*hval)(void*);
    void (*free_rec)(void*);
} csc_hash_t;    


typedef struct
{
/* Innards are private! */
    csc_hash_node_t *pt;
    void *key;
    unsigned long ilst;
    unsigned long hash_val;
} csc_hash_iter_t;


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


int csc_hash_addex(csc_hash_t *h, void *rec)
/*  If a key matching 'rec' already exists in 'h' then this 
 * function will return FALSE.  Otherwise it will add 'rec' to 'hash' 
 * and return TRUE.
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
        return TRUE;
    }
    else
        return FALSE;
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


int csc_hash_del(csc_hash_t *h, void *key)
/*  If no record with a key of 'key' exists in 'h', this function 
 * will return FALSE.  Otherwise it will remove any record 
 * with a key of 'key', free it and return TRUE.
 */   
{   void *p;
    p = csc_hash_out(h, key);
    if (p == NULL)
        return FALSE;
    else
    {   h->free_rec(p);
        h->count--;
        return TRUE;
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


void csc_hash_key_init(csc_hash_t *h, csc_hash_iter_t *i, void *key)
{  
    i->hash_val = h->hval(key);
    i->pt = h->table[ i->hash_val % h->tblsize ];
    i->key = key;
}

void *csc_hash_key_next(csc_hash_t *h, csc_hash_iter_t *i)
{   csc_hash_node_t *pt;
    int (*cmp)(void*,void*) = h->cmp;
    int offset = h->offset;
    void *key = i->key;
    unsigned long hash_val = i->hash_val;
 
    for (pt=i->pt; pt!=NULL; pt=pt->next)
    {   if (pt->hash_val==hash_val && cmp(((char*)pt->data)+offset, key)==0)
            break;
    }
    if (pt == NULL)
    {   i->pt = NULL;
        return NULL;
    }
    else
    {   i->pt = pt->next;
        return pt->data;
    }
}

void csc_hash_all_init(csc_hash_t *h, csc_hash_iter_t *i)
{   i->ilst = 0;
    i->pt = NULL;
}

void *csc_hash_all_next(csc_hash_t *h, csc_hash_iter_t *i)
{   int ilst;
    int nlst;
    csc_hash_node_t **table;
    csc_hash_node_t *pt = i->pt;
 
    if (pt == NULL)
    {   ilst = i->ilst;
        nlst = h->tblsize;
        table = h->table;
        while (ilst<nlst && table[ilst]==NULL)
            ilst++;
        i->ilst = ilst;
        if (ilst == nlst)
            return NULL;
        else
            pt = table[ilst];
    }
 
    i->pt = pt->next;
    if (i->pt == NULL)
        i->ilst++;
    return pt->data;
}


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



# if 0  
/* For testing hash.  It helps if you make the size and multplier 3
 * and 2 just for testing and undo later.  Use memcheck.
 */ 

void main(int argc, char **argv)
{  char line[201];
   char *args[3], *s;
   int nargs;
   csc_hash_t *tbl;
   csc_hash_iter_t hi;
 
   tbl = csc_hash_new(0, (int (*)(void*,void*))strcmp, csc_hash_str, csc_hash_FreeBlk);
   assert(tbl);
   while (csc_fgetline(stdin,line,200) != -1)
   {  
      nargs = csc_param_quote(args, line, 3);
      if (nargs != 2)
      {  printf("BAD\n");
      }
      else if (csc_streq(args[0],"a"))
      {  csc_hash_add(tbl, csc_alloc_str(args[1]));
      }
      else if (csc_streq(args[0],"g"))
      {  s = csc_hash_get(tbl, args[1]);
         if (s)
            printf("%d\n", 1);
         else
            printf("%d\n", 0);
      }
      else if (csc_streq(args[0],"ax"))
      {  s = csc_alloc_str(args[1]);
         if (csc_hash_addex(tbl, s))
            printf("%d\n", 1);
         else
         {  free(s);
            printf("%d\n", 0);
         }
      }
      else if (csc_streq(args[0],"o"))
      {  s = csc_hash_out(tbl, args[1]);
         if (s)
         {  printf("%d\n", 1);
            free(s);
         }
         else
            printf("%d\n", 0);
      }
      else if (csc_streq(args[0],"p"))
      {  if (csc_streq(args[1], ""))
         {   for_hash_all(tbl,hi,s,char*)
                printf("%s\n", s);
         }
         else
         {
            for_hash_key(tbl,hi,args[1],s,char*)
                printf("%s\n", s);
         }
      }
      else if (csc_streq(args[0],"d"))
      {  printf("%d\n", csc_hash_del(tbl, args[1]));
      }
      else
      {  printf("BAD\n");
      }
   }
   csc_hash_free(tbl);
   exit(0);
}


#endif


