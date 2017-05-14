// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "std.h"
#include "alloc.h"
#include "fileProperties.h"


typedef struct csc_fileProp_t
{   struct stat props;
    int isOK;
    int isExist;
    char *errMsg;
} csc_fileProp_t;    


static void setErrMsg(csc_fileProp_t *this, char *newErrMsg)
{   if (this->errMsg != NULL)
        free(this->errMsg);
    this->errMsg = newErrMsg;
}


const char *csc_fileProp_getErrMsg(const csc_fileProp_t *this)
{   return this->errMsg;
}


csc_fileProp_t *csc_fileProp_new(const char *path)
{   int retval;
 
// Allocate the object.
    csc_fileProp_t *this = csc_allocOne(csc_fileProp_t);
    this->errMsg = NULL;
    this->isOK = TRUE;
    this->isExist = TRUE;
 
// Get the properties.
    retval = stat(path, &this->props);
    if (retval == -1)
    {   setErrMsg(this, csc_alloc_str3("stat: ", strerror(errno), NULL));
        this->isOK = FALSE;
        if (errno == ENOENT)
            this->isExist = FALSE;
    }
    else
        this->isOK = TRUE;
    
// Bye
    return this;
}


void csc_fileProp_close(csc_fileProp_t *this)
{
// Free any error message.
    if (this->errMsg != NULL)
        free(this->errMsg);
    
// Free the parent structure.
    free(this);
}


int csc_fileProp_isOK(csc_fileProp_t *this)
{   return this->isOK;
}


int csc_fileProp_isExist(csc_fileProp_t *this)
{   return this->isExist;
}


int csc_fileProp_isRegFile(csc_fileProp_t *this)
{   if (!this->isOK)
        return FALSE;
    return S_ISREG(this->props.st_mode);
}


int csc_fileProp_isDir(csc_fileProp_t *this)
{   if (!this->isOK)
        return FALSE;
    return S_ISDIR(this->props.st_mode);
}


int64_t csc_fileProp_fileSize(csc_fileProp_t *this)
{   if (!this->isOK)
        return -1;
    else
        return (int64_t)this->props.st_size;
}

