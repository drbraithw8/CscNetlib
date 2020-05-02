// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXTRA_SIZE (sizeof(memchk_type) + sizeof(csc_ulong))
#define CKVAL (1431655765)
#define BNDY_MSK  7
#define align_err(p)    ((unsigned long)(p) & BNDY_MSK)

#define csc_TRUE 1
#define csc_FALSE 0

extern FILE *csc_errOut;
void csc_assertFail(const char *fname, int lineNo, const char *expr);
#define csc_stderr (csc_errOut?csc_errOut:stderr)
#define csc_assert(a)  ( !(a) ? ( \
   csc_assertFail(__FILE__, __LINE__, #a) , 0) : 0)  


typedef unsigned int csc_uint;
typedef unsigned long csc_ulong;

typedef struct memchk       /* size must be integral no *sizeof(double) */
{   struct memchk *next;
    struct memchk *prev;
    char *end;
    char *fname;
    long line_no;
    csc_ulong mark;  // value that helps with debugging.
    csc_ulong ckval;
} memchk_type;


long mck_maxchunks = ((unsigned long)-1 >> 1);

static long nmlc=0;
static memchk_type anchor = { &anchor, &anchor, (char*)NULL, 0 };
static memchk_type *lo_adr = (memchk_type*)NULL;
static memchk_type *hi_adr = (memchk_type*)NULL;

static void freecheck(memchk_type *header, int line, char *file);
static void msg_quit(char *msg, char *file, int line);
int csc_mck_checkmem(int flag, int line, char *file);
void csc_mck_exit(int status, int line, char *file);
char *csc_mck_realloc(char *block, csc_uint size, int line, char *file);
void csc_mck_free(char *block, int line, char *file);
char *csc_mck_strdup(char *str, int line, char *file);
char *csc_mck_calloc(csc_uint nelem, csc_uint elsize, int line, char *file);
char *csc_mck_malloc(csc_uint size, int line, char *file);
long csc_mck_nchunks();


long csc_mck_nchunks()
{   return nmlc;
}


char *csc_mck_malloc(csc_uint size, int line, char *file)
{   char *block;
    memchk_type *header;
    memchk_type *hi;
 
/* Get the memory. */
    if (nmlc >= mck_maxchunks)
        return NULL;
    if ((header=(memchk_type*)malloc((csc_uint)(size+EXTRA_SIZE))) == NULL)
        return NULL;
    csc_assert(!align_err(header));
 
/* Set upper and lower boundaries. */
    hi = (memchk_type*)((char*)header + size + EXTRA_SIZE);
    if (lo_adr == NULL)     /* 1st time called for prog. */
    {   lo_adr = header;
        hi_adr = hi;
        csc_assert(sizeof(memchk_type) % sizeof(double) == 0);
        /* If this turns out to be false, the padding in memchk_type
         *  must be adjusted.  The size affects alignment of allocated chunks.
         */
        csc_assert(BNDY_MSK+1 == sizeof(double));
        /* If this turns out to be false, the align_err() macro may
         * need to be re written.
         */
    }
    else
    {   if (hi > hi_adr)
            hi_adr = hi;
        else if (header < lo_adr)
            lo_adr = header;
    }
 
/* Set up 'block' and 'end'. */
    block = (char*)header + sizeof(memchk_type);
    header->ckval = CKVAL;
    header->end = block+size;
    memcpy(header->end, (char*)(&header->ckval), sizeof(csc_ulong));
    header->fname = file;
    header->line_no = line;
 
/* Link the header block into the doubly linked list. */
    header->prev = &anchor;
    header->next = anchor.next;
    anchor.next = header;
    if ((header->next)->prev != &anchor)
        msg_quit("Non allocated memory overwritten", file, line);
    (header->next)->prev = header;

/* Initialise the mark to zero. */
    header->mark = 0;
 
/* OK. */
    nmlc++;
    return block;
}


char *csc_mck_calloc(csc_uint nelem, csc_uint elsize, int line, char *file)
{   char *block;
    csc_uint size;
    
    size = nelem*elsize;
    if ((block=csc_mck_malloc(size, line, file)) == NULL)
        return NULL;
    memset(block,0,size);
    return block;
}


char *csc_mck_strdup(char *str, int line, char *file)
{   char *block;
    
    if ((block=csc_mck_malloc(strlen(str)+1, line, file)) == NULL)
        return NULL;
    strcpy(block,str);
    return block;
}


void csc_mck_free(char *block, int line, char *file)
{   memchk_type *header;
 
    header = (memchk_type*)(block - sizeof(memchk_type));
    freecheck(header, line,file);
    (header->next)->prev = header->prev;
    (header->prev)->next = header->next;
    /* header->prev = NULL; */
    /* header->next = NULL; */
    free((char*)header);
    nmlc--;
}


char *csc_mck_realloc(char *block, csc_uint size, int line, char *file)
{   memchk_type *header;
    memchk_type *hi;
    long mark;

/* Is this a disguised call to malloc() or free(). */
    if (block == NULL)
    {   return csc_mck_malloc(size, line, file);
    }
    else if (size == 0)
    {   csc_mck_free(block, line, file);
        return NULL;
    }

/* Check the old memory. */
    header = (memchk_type*)(block - sizeof(memchk_type));
    freecheck(header, line,file);
 
/* The mark. */
    mark = header->mark;
 
/* Get the memory. */
    header = (memchk_type*)realloc((char*)header, (csc_uint)(size+EXTRA_SIZE));
    if (header == NULL)
        return NULL;
 
/* Set upper and lower boundaries. */
    hi = (memchk_type*)((char*)header + size + EXTRA_SIZE);
    if (hi > hi_adr)
        hi_adr = hi;
    else if (header < lo_adr)
        lo_adr = header;
    
/* Set up 'block' and 'end'. */
    block = (char*)header + sizeof(memchk_type);
    header->ckval = CKVAL;
    header->end = block+size;
    memcpy(header->end, (char*)(&header->ckval), sizeof(csc_ulong));
 
/* Link the header block into the doubly linked list. */
    (header->next)->prev = header;
    (header->prev)->next = header;

/* The mark. */
    header->mark = mark;
 
 
/* OK. */
    return block;
}


void csc_mck_sexit(int status, int line, char *file)
{   
// 	if (nmlc != 0)
//     {   fprintf(csc_stderr, "memcheck: line %d file \"%s\" :-\n", line, file);
//         fprintf(csc_stderr, "\t%ld memory chunks not released.\n", nmlc);
//     }
    exit(status);
}


void csc_mck_exit(int status, int line, char *file)
{   if (nmlc != 0)
    {   fprintf(csc_stderr, "memcheck: line %d file \"%s\" :-\n", line, file);
        fprintf(csc_stderr, "\t%ld memory chunks not released.\n", nmlc);
    }
    else
        fprintf(csc_stderr, "memcheck: All allocated memory chunks released.\n");
    exit(status);
}


int csc_mck_checkmem(int flag, int line, char *file)
{   memchk_type *pt;
    int err=csc_FALSE;
    if ((anchor.next)->prev != &anchor)
        err = csc_TRUE;
    for (pt=anchor.next; pt!=&anchor && !err; pt=pt->next)
    {   if (  ((pt->next<lo_adr || pt->next>hi_adr) && pt->next!=&anchor)
         ||  align_err(pt->next) && pt->next!=&anchor
         || (pt->next)->prev!=pt 
         ||  (memchk_type*)(pt->end)<pt || (memchk_type*)(pt->end)>hi_adr
         ||  pt->ckval != CKVAL
         ||  memcmp(pt->end, (char*)(&pt->ckval), sizeof(csc_ulong))  )
            err = csc_TRUE;
    }
    if (err)
    {   if (flag)
            msg_quit("Non allocated memory overwritten", file, line);
        else
            return csc_FALSE;
    }
    return csc_TRUE;
}


static void freecheck(memchk_type *header, int line, char *file)
{   memchk_type *pt;
    if (align_err(header) || header<lo_adr || header>hi_adr)
        msg_quit("free'd memory was not allocated", file, line);
    if ( ((header->next<lo_adr || header->next>hi_adr) && header->next!=&anchor)
     ||  ((header->prev<lo_adr || header->prev>hi_adr) && header->prev!=&anchor)
     ||  align_err(header->next) && header->next!=&anchor 
     || (header->next)->prev!=header
     ||  align_err(header->prev) && header->prev!=&anchor
     || (header->prev)->next!=header )
    {   fprintf(csc_stderr, "memcheck: Non allocated memory overwritten");
        fprintf(csc_stderr, "  or  free'd memory not allocated\n\n");
    }
    else if (  (memchk_type*)(header->end) < header
          ||   (memchk_type*)(header->end) > hi_adr 
          ||    header->ckval != CKVAL 
          ||    memcmp(header->end, (char*)(&header->ckval), sizeof(csc_ulong)) )
        msg_quit("Non allocated memory overwritten", file, line);
    else
        return;
 
/* Diagnostics. */
    fprintf(csc_stderr, "Performing diagnostic to determine which one ...\n\n");
    for (pt=anchor.next; pt!=&anchor; pt=pt->next)
    {   if (  ((pt->next<lo_adr || pt->next>hi_adr) && pt->next!=&anchor)
                || align_err(pt->next) && pt->next!=&anchor
                || (pt->next)->prev!=pt )
            msg_quit("Non allocated memory overwritten", file, line);
    }
    msg_quit("free'd memory was not allocated", file, line);
}


static void msg_quit(char *msg, char *file, int line)
{   fprintf(csc_stderr, "memcheck: line %d file \"%s\" :-\n\t%s!\n",line,file,msg);
    exit(1);
}


void csc_mck_print(FILE *fout)
{   memchk_type *pt;
    for (pt=anchor.next; pt!=&anchor; pt=pt->next)
    {   fprintf(fout, "%ld %s\n", pt->line_no, pt->fname);
    }
}


void csc_mck_printMarkEq(FILE *fout, long markVal)
{   memchk_type *pt;
    for (pt=anchor.next; pt!=&anchor; pt=pt->next)
    {   if (pt->mark == markVal)
        {   fprintf(fout, "%ld %s\n", pt->line_no, pt->fname);
        }
    }
}


void csc_mck_setMark(long newMarkVal)
{   memchk_type *pt;
    for (pt=anchor.next; pt!=&anchor; pt=pt->next)
    {   pt->mark = newMarkVal;
    }
}


void csc_mck_changeMark(long oldMarkVal, long newMarkVal)
{   memchk_type *pt;
    for (pt=anchor.next; pt!=&anchor; pt=pt->next)
    {   if (pt->mark == oldMarkVal)
        {   pt->mark = newMarkVal;
        }
    }
}



