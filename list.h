// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#ifndef csc_LIST_H
#define csc_LIST_H 1

typedef struct csc_list_t
{   struct csc_list_t *next;
    void *data;
} csc_list_t;


csc_list_t *csc_list_sort(csc_list_t *lst, int cmp(void*,void*));
/*  'lst' points to the head of a linked list.  This function sorts 
 * the linked list and returns a pointer to the new head of the list.
 * 'cmp'(void *a, void *b) is a comparison function that will return
 * +ve if a>b, 0 if a==b and -ve if a<b, where a and b are generic
 * pointers to the data to be sorted.  An example of the use of
 * csc_list_sort() is "l=csc_list_sort(l,cmp);".
 */

void csc_list_free(csc_list_t *lst);
/*      Frees the links of a list whose data has ALREADY been freed.
 */

int csc_list_count(csc_list_t *lst);
/*      Returns the number of items in the list.
 */

void csc_list_freeblk(csc_list_t *lst);
/*      Frees the links and data of a list whose data consist of
 * a single block and requires no further resource freeing.
 */

void csc_list_add(csc_list_t **pt, void *data);
/*  Adds 'data' to the beginning of the linked list *'pt'.
 */

void csc_list_rvrse(csc_list_t **lst);
/* Reverses the order of all elements in the list *'lst'.
 */ 

void *csc_list_nth(csc_list_t *lst, long n);
/* Returns a pointer to the nth element of the list or NULL if the
 * list was too short.  
 * This is inherently slow for a long list.  If this operation HAS to
 * be fast, use some other storage mechanism. 
 */ 

void csc_list_addend(csc_list_t **pt, void *data);
/*  Adds 'data' to the end of the linked list *'pt'.
 * This is inherently slow for a long list.  If this operation HAS to
 * be fast, use some other storage mechanism. 
 */

int csc_list_rm_item(csc_list_t **lst, void *item);
/* Removes item 'item' from list 'lst' and returns TRUE, on success. 
 * Returns FALSE on failure to find 'item'.  Does not free 'item'.
 * This is inherently slow for a long list.  If this operation HAS to
 * be fast, use some other storage mechanism. 
 */


/* The following are declarations for routines which you probably wont
 * need to use.  They are documented in the code if it so happens that you do.
 */ 
void *csc_list_pop(csc_list_t **pt);
void *csc_list_top(csc_list_t *pt);
csc_list_t *csc_list_merge(csc_list_t *left, csc_list_t *right, int cmp(void*,void*));
void csc_list_AddFromPool(csc_list_t **pt, void *data, csc_list_t **pool);
void *csc_list_PopToPool(csc_list_t **lst, csc_list_t **pool);
int csc_list_XferOne(csc_list_t **LstFrom, csc_list_t **LstTo);
void csc_list_XferAll(csc_list_t **LstFrom, csc_list_t **LstTo);


#define csc_list_push(l,d) csc_list_add(l,d)

#endif
