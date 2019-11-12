#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <time.h>
#include "std.h"
#include "alloc.h"
#include "list.h"
#include "hash.h"
#include "blacklist.h"


typedef struct csc_blacklist_t
{   csc_hash_t *hash;
    int expireTime;
    int accessCount;
    csc_bool_t isTimeFaked;
    time_t fakeNow;
} csc_blacklist_t;


typedef struct
{   time_t lastEval;
    long blackness;
    char *idStr;
} blEntry_t;



static blEntry_t *blEntry_new(const char *idStr)
{   blEntry_t *be = csc_allocOne(blEntry_t);  assert(be);
    be->idStr = csc_allocStr(idStr);  assert(be->idStr);
    return be;
}


static void blEntry_free(void *bee)
{   blEntry_t *be = bee;
    free(be->idStr);
    free(be);
}


csc_blacklist_t *csc_blacklist_new(int expireTime)
{   csc_blacklist_t *bl = csc_allocOne(csc_blacklist_t);  assert(bl);
    bl->hash = csc_hash_new( offsetof(blEntry_t, idStr)
                           , csc_hash_StrPtCmpr
                           , csc_hash_StrPt
                           , blEntry_free
                           );
    bl->expireTime = expireTime;
    bl->accessCount = 0;
    bl->isTimeFaked = csc_FALSE;
    bl->fakeNow = 0;
    return bl;
}


void csc_blacklist_free(csc_blacklist_t *bl)
{   csc_hash_free(bl->hash);
    free(bl);
}


int csc_blacklist_blackness(csc_blacklist_t *bl, const char *idStr)
{   int ret;
    blEntry_t *be;
    long blackness;
    time_t now = 0 ;

// Get the current time.
    if (bl->isTimeFaked)
        now = bl->fakeNow;
    else
        now = time(NULL);
 
    bl->accessCount++;
    be = csc_hash_get(bl->hash, &idStr);
    if (be == NULL)
    {
// fprintf(stdout, "new %s\n", idStr);
        be = blEntry_new(idStr);
        blackness = 1;
        ret = csc_hash_addex(bl->hash, be); assert(ret);
    }
    else
    {
// fprintf(stdout, "%ld %ld %ld\n", be->blackness, now, be->lastEval);
        blackness = be->blackness - (now - be->lastEval)/bl->expireTime;
        if (blackness <= 0)
            blackness = 1;
    }
    be->lastEval = now;
    be->blackness = blackness + 1;
    return blackness;
}


void csc_blacklist_clean(csc_blacklist_t *bl)
{   int ret;
    blEntry_t *be;
    csc_list_t *lstDelIds = NULL;
    time_t now = 0 ;
    long blackness;

// Get the current time.
    if (bl->isTimeFaked)
        now = bl->fakeNow;
    else
        now = time(NULL);
 
// Gather the ID strings of all with negative blackness.
    csc_hash_iter_t *iter = csc_hash_iter_new(bl->hash);
    while ((be = csc_hash_iter_next(iter)) != NULL)
    {
        blackness = be->blackness - (now - be->lastEval)/bl->expireTime;
        if (blackness < 0)
        {   csc_list_add(&lstDelIds, be->idStr);
        }
        else
        {   be->lastEval = now;
            be->blackness = blackness;
        }
    }
    csc_hash_iter_free(iter);
 
// Remove items with negative blackness from blacklist.
    for (csc_list_t *lp=lstDelIds; lp!=NULL; lp=lp->next)
    {   char *idStr = lp->data;
        ret = csc_hash_del(bl->hash, &idStr);
        assert(ret);
    }
    csc_list_free(lstDelIds);
 
// Reset the access count.
    bl->accessCount = 0;
}


int csc_blacklist_accessCount(csc_blacklist_t *bl)
{   return bl->accessCount;
}


void csc_blacklist_setTimeFaked(csc_blacklist_t *bl, csc_bool_t isFaked)
{   bl->isTimeFaked = isFaked;
}


void csc_blacklist_setFakeTime(csc_blacklist_t *bl, time_t fakeNow)
{   bl->fakeNow = fakeNow;
}

