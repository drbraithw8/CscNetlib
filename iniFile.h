// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#ifndef csc_INI_H
#define csc_INI_H 1

typedef struct csc_ini_t csc_ini_t;

// Constructor.  Returns a new iniFile object with no configuration
// information in it.
csc_ini_t *csc_ini_new(void);

// Destructor.
void csc_ini_close(csc_ini_t *ini);


// Read the ini file, whose path is given by 'iniFilePath' into the ini
// file object.  Expects only ASCII characters in ini file, not unicode
// characters. 
// 
// On success, will assimilate all the ini information from the ini file
// and return 0.  If this routine cannot read the ini file, this function
// will return -1.  If the ini file contains errors, this function will
// return the line number of the error, and it will have assimilated some,
// but not all of the ini file.
// 
// Records with the same section name and different keys will be merged.
// If there are records with the same section and key names, then the
// earlier one will be retained and the later value lost.  This function
// may be called multiple times to that more than one inifile may be
// assimilated into this.
int csc_ini_read(csc_ini_t *ini, const char *iniFilePath);


// If the key 'ident' exists in the ini file section 'section' then this
// returns a constant string containing the associated value.  The string
// is owned by the 'ini' object.  You must not write onto or otherwise
// alter this string, and calls to methods that alter 'ini' may alter this
// string.
// 
// Returns NULL if the key 'ident' does not exist in the section 'section'.
const char *csc_ini_getStr(csc_ini_t *ini, const char *section, const char *ident);


// If the key 'ident' exists in the ini file section 'section' then this
// returns an allocated string containing the associated value.  The caller
// now owns and must free the allocated string.  
// 
// Returns NULL if the key 'ident' does not exist in the section 'section'.
char *csc_ini_getAllocStr(csc_ini_t *ini, const char *section, const char *ident);


#endif
