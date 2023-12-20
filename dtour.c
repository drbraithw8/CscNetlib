
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "std.h"
#include "list.h"
#include "alloc.h"
#include "dtour.h"

typedef struct
{	char *fname;
	struct stat st;
} sinfo_type;

static int dir_recurs
(   char *path
,	int flags   	// What types of files are to be included.
,   void (*dir_begin)(char *pth, char *name, struct stat *info, void *ctx)
,   void (*dir_file)(char *pth, char *name, struct stat *info, void *ctx)
,   void (*dir_end)(char *pth, char *name, struct stat *info, void *ctx)
,	void *ctx
);
static int ss_cmp(const void *first, const void *second);
static void free_files(csc_list_t *files);


int csc_dtour
(   const char *theirPath
,	int flags   	// What types of files are to be included.
,   void (*dir_begin)(char *pth, char *name, struct stat *info, void *ctx)
,   void (*dir_file)(char *pth, char *name, struct stat *info, void *ctx)
,   void (*dir_end)(char *pth, char *name, struct stat *info, void *ctx)
,	void *ctx
)
{	char path[csc_DT_MAXPATH+1] = "";   
    int rval;
	char *pend;
	struct stat st;
	csc_assert(theirPath != NULL);
 
/* Use our path so that theirs is not changed. */
    if ((int)strlen(theirPath) > csc_DT_MAXPATH)
        return ENAMETOOLONG;
    strcpy(path, theirPath);
 
/* Remove any trailing /'s */
	csc_assert(strcmp(path,""));
	pend = &path[strlen(path)];
	while (pend>path && *(pend-1)=='/')
		pend--;
	*pend = '\0';
 
/* Set pend to point to filename part of path. */
	if (csc_streq(path,""))
	{	strcpy(path,"/");
		pend = path;
	}
	else
	{	while (pend>path && *(pend-1)!='/')
			pend--;
	}
	
/* Go. */
	if (lstat(path,&st) != 0)
		return errno;
	if (!S_ISDIR(st.st_mode))
	{	if (dir_file != NULL)
			dir_file(path, pend, &st, ctx);
		return 0;
	}
	else
	{	if (dir_begin != NULL)
			dir_begin(path, pend, &st, ctx);
		rval =  dir_recurs(path, flags, dir_begin, dir_file, dir_end, ctx);
		if (dir_end != NULL)
			dir_end(path, pend, &st, ctx);
		return rval;
	}
}


static int dir_recurs
(   char *path
,	int flags   	// What types of files are to be included.
,   void (*dir_begin)(char *pth, char *name, struct stat *info, void *ctx)
,   void (*dir_file)(char *pth, char *name, struct stat *info, void *ctx)
,   void (*dir_end)(char *pth, char *name, struct stat *info, void *ctx)
,	void *ctx
)
/* This function recursively tours through directories calling any
 * of the function pointers if they are not NULL.
 */
{   char *pend, *fname;                    /* Points to end of path. */
	csc_list_t *files = NULL;
    sinfo_type *rec = NULL;
	struct stat *st;
	struct dirent *dirent;
	DIR *dir = NULL;
    int plen, rval, retval;
 
	if ((dir=opendir(path)) == NULL)
		return errno;
 
/* Length of the current path. */
    plen = strlen(path);
    pend = &path[plen++];
    strcpy(pend++, "/");
 
/* Read in files. */
	while ((dirent=readdir(dir)) != NULL)
	{	fname = dirent->d_name;
 
/* Filter out silent. */
		if (csc_streq(fname,".") || csc_streq(fname,".."))
			continue;
		if (*fname=='.' && !(flags&csc_DT_NOSILENT))
			continue;
		
/* Make sure there is enough space. */
		if ((int)strlen(fname)+plen+1 > csc_DT_MAXPATH)
		{	retval = ENAMETOOLONG;
			goto error;
		}
 
/* Ready one record. */
		if (rec == NULL)
		{	rec = csc_allocOne(sinfo_type);
			rec->fname = NULL;
		}
		st = &rec->st;
 
/* Do a stat to get details. */
		strcpy(pend, fname);
		if (lstat(path, st) != 0)
		{	fprintf(stderr, "lstat(\"%s\",st) failed!\n", path);
			perror("");
			continue;
		}
 
/* Deal with symbolic links. */
		if (S_ISLNK(st->st_mode))
		{	if (!(flags&csc_DT_LINKS))
				continue;
			else
			{	if (stat(path,st) != 0)
				{	fprintf(stderr, "stat(\"%s\",st) failed!\n", path);
					perror("");
					continue;
				}
			}
		}
 
/* Filter out special files. */
		if (!(flags&csc_DT_SPECIAL) &&
				!S_ISREG(st->st_mode) && !S_ISDIR(st->st_mode))
			continue;
 
/* Place record into file list. */
		rec->fname = csc_alloc_str(fname);
		csc_list_add(&files, rec);
		rec = NULL;
	}
	closedir(dir);
	dir = NULL;
 
/* Do each entry in turn. */
	files = csc_list_sort(files, (int(*)(void*,void*))ss_cmp);
	while (files != NULL) // pt=pt->next)
	{	sinfo_type *inf = files->data;
 
	// Process the record
		st = &inf->st;
		fname = inf->fname;
		strcpy(pend, fname);
		if (S_ISDIR(st->st_mode))
		{	if (dir_begin != NULL)
				dir_begin(path, fname, st, ctx);
			rval = dir_recurs(path, flags, dir_begin, dir_file, dir_end, ctx);
			if (dir_end != NULL)
				dir_end(path, fname, st, ctx);
		}
		else
		{	if (dir_file != NULL)
				dir_file(path, fname, st, ctx);
		}
 
	// Free the record.
		free(inf->fname);
		free(inf);
		csc_list_t *pt=files; 
 
	// Pop the head from the list.
		files = files->next;
		free(pt);
	}
	retval = 0;
 
error:
	*(--pend) = '\0';
	if (rec != NULL)
	{	if (rec->fname != NULL)
			free(rec->fname);
		free(rec);
	}
	if (dir != NULL)
		closedir(dir);
	if (files != NULL)
		free_files(files);
	return retval;
}


static void free_files(csc_list_t *files)
{	for (csc_list_t *pt=files; pt!=NULL; pt=pt->next)
	{	sinfo_type *inf = pt->data;
		free(inf->fname);
		free(inf);
	}
	csc_list_free(files);
}


static int ss_cmp(const void *first, const void *second)
/* Used to compare find_t structures based on ffblk.name in the 
 * structure.  It will return +ve if 'first' > 'second', 0 if they are 
 * equal, and -ve otherwise.  Directories are always greater than files.  
 * If 'first' and 'second' are both files or both directories, then one 
 * that is later in dictionary order is taken to be larger.
 */ 
{   sinfo_type *s1 = (sinfo_type*)first;
    sinfo_type *s2 = (sinfo_type*)second;
 
    if (S_ISDIR(s1->st.st_mode))
    {   if (!S_ISDIR(s2->st.st_mode))
            return 1;
    }
    else
    {   if (S_ISDIR(s2->st.st_mode))
            return -1;
    }
    return strcmp(s1->fname, s2->fname);
}                                  


