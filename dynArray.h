
// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#ifndef csc_DYNARRAY_H
#define csc_DYNARRAY_H 1

#include <stdlib.h>
#include "alloc.h"


#define csc_dynArray_headers(parent, child)                                    \
                                                                               \
typedef struct                                                                 \
{	child ## _t **els;                                                         \
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
{	parent ## _t *El = csc_allocOne(parent ## _t);                             \
	El->els = NULL;                                                            \
	El->nEls = 0;                                                              \
	El->mEls = 0;                                                              \
}                                                                              \
                                                                               \
void  parent ## _add(parent ## _t *El, child ## _t *el)                        \
{	if (El->nEls == El->mEls)                                                  \
	{	El->mEls = El->mEls * 2 + 10;                                          \
		El->els = csc_ck_ralloc(El->els, El->mEls*sizeof(child ## _t*));       \
	}                                                                          \
	El->els[El->nEls++] = el;                                                  \
}                                                                              \
                                                                               \
void parent ## _free(parent ## _t *El)                                         \
{	for (int i=0; i < El->nEls; i++)                                           \
		child ## _free(El->els[i]);                                            \
	free(El->els);                                                             \
	free(El);                                                                  \
}                                                                              \


#define csc_dynArray_s_headers(parent, child)                                  \
                                                                               \
typedef struct                                                                 \
{	child *els;                                                                \
	int nEls;                                                                  \
	int mEls;                                                                  \
} parent ## _t;                                                                \
                                                                               \
parent ## _t * parent ## _new();                                               \
void  parent ## _add(parent ## _t *El, child el);                              \
void parent ## _free(parent ## _t *El);                                        \


#define csc_dynArray_s_code(parent, child)                                     \
                                                                               \
parent ## _t * parent ## _new()                                                \
{	parent ## _t *El = csc_allocOne(parent ## _t);                             \
	El->els = NULL;                                                            \
	El->nEls = 0;                                                              \
	El->mEls = 0;                                                              \
}                                                                              \
                                                                               \
void  parent ## _add(parent ## _t *El, child el)                               \
{	if (El->nEls == El->mEls)                                                  \
	{	El->mEls = El->mEls * 2 + 10;                                          \
		El->els = csc_ck_ralloc(El->els, El->mEls*sizeof(child));              \
	}                                                                          \
	El->els[El->nEls++] = el;                                                  \
}                                                                              \
                                                                               \
void parent ## _free(parent ## _t *El)                                         \
{	free(El->els);                                                             \
	free(El);                                                                  \
}                                                                              \


#endif
