// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "std.h"
#include "alloc.h"
#include "list.h"



void *csc_list_top(csc_list_t *pt)
/* Returns a pointer to the data element at the top of the list.
 */
{   if (pt == NULL)
        return NULL;
    else
        return pt->data;
}


void *csc_list_pop(csc_list_t **pt)
/* Pops the head off the list *'lst', and returns a pointer to the data.
 * Returns NULL if the list is empty.
 */ 
{   void *data;
    csc_list_t *head;
 
    head = *pt;
    if (head == NULL)
        return NULL;
    data = head->data;
    *pt = head->next;
    free(head);
    return data;
}


void csc_list_add(csc_list_t **pt, void *data)
/*  Adds 'data' to the beginning of the linked list *'pt'.
 */
{   csc_list_t *head = (csc_list_t*)csc_ck_malloc((csc_uint)sizeof(csc_list_t));
    head->data = data;
    head->next = *pt;
    *pt = head;
}


void csc_list_freeblk(csc_list_t *lst)
/*      Frees the links and data of a list whose data consist of
 * a single block and requires no further resource freeing.
 */
{   csc_list_t *pt;
    for (pt=lst; pt!=NULL; pt=pt->next)
        free(pt->data);
    csc_list_free(lst);
}


void csc_list_free(csc_list_t *lst)
/*      Frees the links of a list whose data has ALREADY been freed.
 */
{   csc_list_t *prev;
    while (lst != NULL)
    {   prev = lst;
        lst = lst->next;
        free(prev);
    }
}


int csc_list_count(csc_list_t *lst)
/*      Returns the number of items in the list.
 */
{	int count = 0;
	while (lst != NULL)
	{	count++;
		lst = lst->next;
	}
	return count;
}


csc_list_t *csc_list_merge(csc_list_t *left, csc_list_t *right, int cmp(void*,void*))
{   csc_list_t *head, **cur;
 
    if (left == NULL)
        return right;
    if (right == NULL)
        return left;
 
    cur = &head;
    for (;;)
    {   if (cmp(left->data, right->data) < 0)
        {   *cur = left;
            cur = &left->next;
            left = left->next;
            if (left == NULL)
            {   *cur = right;
                return head;
            }
        }
        else
        {   *cur = right;
            cur = &right->next;
            right = right->next;
            if (right == NULL)
            {   *cur = left;
                return head;
            }
        }
    }
}


csc_list_t *csc_list_sort(csc_list_t *lst, int cmp(void*,void*))
/*  'lst' points to the head of a linked list.  This function sorts 
 * the linked list and returns a pointer to the new head of the list.
 * 'cmp'(void *a, void *b) is a comparison function that will return
 * +ve if a>b, 0 if a==b and -ve if a<b, where a and b are generic
 * pointers to the data to be sorted.  An example of the use of
 * csc_list_sort() is "l=csc_list_sort(l,cmp);".
 * BUGS: This can sort as most 4 000 000 000 list elements, but this number
 * can be easily increased.
 */
{   csc_list_t *next, *frame[32];
    int i;
 
    for (i=0; i<32; i++)
        frame[i] = NULL;
 
    while (lst != NULL)
    {   next = lst->next;
        lst->next = NULL;
        for (i=0; frame[i]!=NULL; i++)
        {   lst = csc_list_merge(lst, frame[i], cmp);
            frame[i] = NULL;
        }
        frame[i] = lst;
        lst = next;
    }
 
    for (i=0; i<32; i++)
        lst = csc_list_merge(lst, frame[i], cmp);
 
    return lst;
}


void csc_list_rvrse(csc_list_t **lst)
/* Reverses the order of all elements in the list *'lst'.
 */ 
{   csc_list_t *newlist = NULL;
    csc_list_t *oldlist = *lst;
    csc_list_t *thisnode;
 
    while (oldlist != NULL)
    {   thisnode = oldlist;
        oldlist = thisnode->next;
        thisnode->next = newlist;
        newlist = thisnode;
    }
 
    *lst = newlist;
}


void csc_list_XferAll(csc_list_t **LstFrom, csc_list_t **LstTo)
/* Transfers all elements of the list 'LstFrom' to the list 'LstTo'.
 */ 
{   csc_list_t *last, *pt, *head;
    
/* Get ahead. */
    head = *LstFrom;
    if (head == NULL)
        return;
 
/* Find last element of 'LstFrom'. */
    for (pt=head; pt!= NULL; pt=pt->next)
        last = pt;
 
/* Move whole of *'LstFrom' onto beginning of *'LstTo'. */
    last->next = *LstTo;
    *LstTo = head;
    *LstFrom = NULL;
}


int csc_list_XferOne(csc_list_t **LstFrom, csc_list_t **LstTo)
/* Transfers the first element of the list *'LstFrom' to the head of list
 * *'LstTo'.  Returns csc_TRUE on success, or csc_FALSE if *'LstFrom' was empty.
 */ 
{   csc_list_t *last, *pt, *head;
    
/* Get ahead. */
    head = *LstFrom;
    if (head == NULL)
        return csc_FALSE;
 
/* Move 'head' onto beginning of *'LstTo'. */
    *LstFrom = head->next;
    head->next = *LstTo;
    *LstTo = head;
    return csc_TRUE;
}


int csc_list_rm_item(csc_list_t **lst, void *item)
/* Removes item 'item' from list 'lst' and returns csc_TRUE, on success. 
 * Returns csc_FALSE on failure to find 'item'.  
 * Does not free 'item'.
 * This is inherently slow for a long list.  If this operation HAS to
 * be fast, use some other storage mechanism. 
 */
{  csc_list_t *prev, *this;
   
   this = *lst;
 
/* Null list special case. */
   if (this == NULL)
      return csc_FALSE;
 
/* First item special case. */
   if (this->data == item)
   {  *lst = this->next;
      free(this);
      return csc_TRUE;
   }
 
/* Every other case. */
   for (;;)
   {  prev = this;
      this = this->next;
 
      if (this == NULL)
         return csc_FALSE;
 
      if (this->data == item)
      {  prev->next = this->next;
         free(this);
         return csc_TRUE;
      }
   }
}
        
    
void *csc_list_nth(csc_list_t *lst, long n)
/* Returns a pointer to the nth element of the list or NULL if the
 * list was too short.  
 * This is inherently slow for a long list.  If this operation HAS to
 * be fast, use some other storage mechanism. 
 */ 
{   long i=0;
    while (lst != NULL)
    {   if (i++ == n)
            return lst->data;
        else
            lst = lst->next;
    }
    return NULL;
}


void csc_list_addend(csc_list_t **pt, void *data)
/*  Adds 'data' to the end of the linked list *'pt'.
 * This is inherently slow for a long list.  If this operation HAS to
 * be fast, use some other storage mechanism. 
 */
{   while (*pt != NULL)
        pt = &((*pt)->next);
    *pt = (csc_list_t*)csc_ck_malloc((csc_uint)sizeof(csc_list_t));
    (*pt)->next = NULL;
    (*pt)->data = data;
}


/* ----------------------- link pooling extensions ------------------ 
 * These link pooling extensions re_use old links.  Instead of free()ing a
 * link, these routines place the link in a pool for later use.  The hope
 * is that greater efficiency can be achieved by avoiding the use of memory
 * allocation where possible.  Obviously the following routines should work
 * together.  The pool should eventually be freed using csc_list_free().
 */ 


void *csc_list_PopToPool(csc_list_t **lst, csc_list_t **pool)
/* Pops the head off the list *'lst', and returns a pointer to the data.
 * Returns NULL if the list is empty.
 * Discards the csc_list_t by adding to *'pool', instead of freeing it.
 */ 
{   void *data;
    csc_list_t *head;
 
/* Get ahead. */
    head = *lst;
    if (head == NULL)
        return NULL;
 
/* Get data. */
    data = head->data;
    *lst = head->next;
 
/* Return head to pool. */
    head->next = *pool;
    *pool = head;
 
    return data;
}


void csc_list_AddFromPool(csc_list_t **pt, void *data, csc_list_t **pool)
/*  Adds 'data' to the beginning of the linked list *'pt'.  Tries
 * not to malloc(), but gets new links from 'pool' if there are any.
 */
{   csc_list_t *head;
 
/* Get ahead. */
   head = *pool;
   if (head != NULL)
      *pool = head->next;
   else
      head = (csc_list_t*)csc_ck_malloc((csc_uint)sizeof(csc_list_t));
 
/* Hook head into list. */
    head->data = data;
    head->next = *pt;
    *pt = head;
}


/* ------------------------- Testing -------------------------------- */
#if 0

void main(int argc, char **argv)
{   char line[201];
    char *args[3], *s;
    int nargs;
    csc_list_t *pool = NULL;    /* List of nothing - just pool of links. */
    csc_list_t *wlist = NULL;  /* List of char*. */
    csc_list_t *save = NULL;  /* List of char*. */
    csc_list_t *pt;
 
    while (csc_fgetline(stdin,line,200) != -1)
    {  
        nargs = csc_param_quote(args, line, 3);
        if (csc_streq(args[0],"a"))
        { 
            csc_list_AddFromPool(&wlist, csc_alloc_str(args[1]), &pool);
        }
        else if (csc_streq(args[0],"s"))
        {   if (!csc_list_XferOne(&wlist, &save))
            {   printf("None\n");
            }
        }
        else if (csc_streq(args[0],"r"))
        {   if (!csc_list_XferOne(&save, &wlist))
            {   printf("None\n");
            }
        }
        else if (csc_streq(args[0],"D"))
        {   for (pt=wlist; pt!=NULL; pt=pt->next)
                free(pt->data);
            csc_list_XferAll(&wlist, &pool);
        }
        else if (csc_streq(args[0],"P"))
        {   s = csc_list_PopToPool(&wlist, &pool);
            if (s)
                free(s);
        }
        else if (csc_streq(args[0],"p"))
        {   for (pt=wlist; pt!=NULL; pt=pt->next)
                printf("%s\n", pt->data);
        }
        else if (csc_streq(args[0],"q"))
        {   break;
        }
        else
        {  printf("BAD\n");
        }
    }
    csc_list_freeblk(wlist);
    csc_list_free(pool);
    exit(0);
}


#if 0
void main(int argc, char **argv)
{   int iarg;
    char line[101];
    csc_list_t *lst=NULL;
    csc_list_t *lpt;
    char *pt;
 
    for (iarg=1; iarg<argc; iarg++)
        csc_list_add(&lst, argv[iarg]);
 
    csc_list_rvrse(&lst);
 
    for (lpt=lst; lpt!=NULL; lpt=lpt->next)
        printf("%s ", (char*)lpt->data);
    printf("\n");
 
    printf("Enter word ?");  fflush(stdout);
    csc_fgetline(stdin, line,100);
    pt = (void*)23;
    for (lpt=lst; lpt!=NULL; lpt=lpt->next)
    {  if (strcmp(lpt->data,line)==0)
         pt = lpt->data;
    }
    if (csc_list_rm_item(&lst, pt))
    {  printf("word \"%s\" removed.\n", pt);
    }
    else
    {  printf("word \"%s\" not found.\n", line);
    }
 
    csc_list_free(lst);
    exit(0);
}
#endif


static char *skip_white(char *p);
static char *skip_word(char *p);

static char *skip_white(char *p)
/*  Skips over ' ', '\t'. 
 */
{   char ch = *p;
    while (ch==' ' || ch=='\t')
        ch = *(++p);
    return(p);
}

static char *skip_word(char *p)
/*  Skips over all chars except ' ', '\t', '\0'.
 */
{   char ch = *p;
    while (ch!=' ' && ch!='\0' && ch!='\t')
        ch = *(++p);
    return(p);
}

#endif




