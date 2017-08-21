
// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#ifndef csc_DYNARRAY_H
#define csc_DYNARRAY_H 1

#include <stdlib.h>
#include "std.h"
#include "alloc.h"

// -------------------------------------------------------------------
// The following is for a dynamic array of pointers to the child type.
// -------------------------------------------------------------------

#define csc_dynArray_headers(parent, child)                                    \
                                                                               \
typedef struct                                                                 \
{   child ## _t **els;                                                         \
    int nEls;                                                                  \
    int mEls;                                                                  \
} parent ## _t;                                                                \
                                                                               \
parent ## _t * parent ## _new();                                               \
void  parent ## _add(parent ## _t *El, child ## _t *el);                       \
void parent ## _free(parent ## _t *El);                                        \



#define csc_dynArray_code(parent, child)                                       \
                                                                               \
parent ## _t * parent ## _new()                                                \
{   parent ## _t *El = csc_allocOne(parent ## _t);                             \
    El->els = NULL;                                                            \
    El->nEls = 0;                                                              \
    El->mEls = 0;                                                              \
    return El;                                                                 \
}                                                                              \
                                                                               \
void  parent ## _add(parent ## _t *El, child ## _t *el)                        \
{   if (El->nEls == El->mEls)                                                  \
    {   El->mEls = El->mEls * 2 + 10;                                          \
        El->els = csc_ck_ralloc(El->els, El->mEls*sizeof(child ## _t*));       \
    }                                                                          \
    El->els[El->nEls++] = el;                                                  \
}                                                                              \
                                                                               \
void parent ## _free(parent ## _t *El)                                         \
{   for (int i=0; i < El->nEls; i++)                                           \
        child ## _free(El->els[i]);                                            \
    free(El->els);                                                             \
    free(El);                                                                  \
}                                                                              \


#if 0 // This code is untested.

// -------------------------------------------------------------------
// The following is for a dynamic array of elements of the child type.
// It is assumed that there is no further freeing of these types.
// -------------------------------------------------------------------

#define csc_dynArrayE_headers(parent, child)                                   \
                                                                               \
typedef struct                                                                 \
{   child ## _t *els;                                                          \
    int nEls;                                                                  \
    int mEls;                                                                  \
} parent ## _t;                                                                \
                                                                               \
parent ## _t * parent ## _new();                                               \
void  parent ## _add(parent ## _t *El, child ## _t *el);                       \
void parent ## _free(parent ## _t *El);                                        \



#define csc_dynArrayE_code(parent, child)                                      \
                                                                               \
parent ## _t * parent ## _new()                                                \
{   parent ## _t *El = csc_allocOne(parent ## _t);                             \
    El->els = NULL;                                                            \
    El->nEls = 0;                                                              \
    El->mEls = 0;                                                              \
    return El;                                                                 \
}                                                                              \
                                                                               \
void  parent ## _add(parent ## _t *El, child ## _t *el)                        \
{   if (El->nEls == El->mEls)                                                  \
    {   El->mEls = El->mEls * 2 + 10;                                          \
        El->els = csc_ck_ralloc(El->els, El->mEls*sizeof(child ## _t));        \
    }                                                                          \
    El->els[El->nEls++] = *el;                                                 \
}                                                                              \
                                                                               \
void parent ## _free(parent ## _t *El)                                         \
{   free(El->els);                                                             \
    free(El);                                                                  \
}                                                                              \


#endif


#endif
