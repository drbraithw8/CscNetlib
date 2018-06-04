
// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#ifndef csc_FILEPROP_H
#define csc_FILEPROP_H 1

#include <inttypes.h>
#include "std.h"

typedef struct csc_fileProp_t csc_fileProp_t;

// Constructor.  Returns a new file properties object
csc_fileProp_t *csc_fileProp_new(const char *path);


// Destructor.
void csc_fileProp_free(csc_fileProp_t *fprop);


// Returns csc_TRUE on successfully obtaining information about the file.
// Returns csc_FALSE otherwise, e.g. the file does not exist, or some other
// error obtaining information about the file.
int csc_fileProp_isOK(csc_fileProp_t *fprop);


// Returns csc_TRUE if file exists, returns csc_FALSE otherwise.
int csc_fileProp_isExist(csc_fileProp_t *fprop);


// Returns csc_TRUE only if the file exists, is a regular file and no errors.
int csc_fileProp_isRegFile(csc_fileProp_t *fprop);


// Returns csc_TRUE only if the file exists, is a directory and no errors.
int csc_fileProp_isDir(csc_fileProp_t *fprop);


// Returns the file size.
int64_t csc_fileProp_fileSize(csc_fileProp_t *fprop);


// Returns a string representation of details of a previous error.  The
// string returned is valid until the next non const method call.
const char *csc_fileProp_getErrMsg(const csc_fileProp_t *fprop);


#endif
