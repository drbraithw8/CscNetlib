// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

#include "std.h"
#include "alloc.h"
#include "hash.h"
#include "iniFile.h"

#define MaxLineLen 255


typedef struct csc_ini_t
{   csc_mapStrStr_t *map;
} csc_ini_t;    



csc_ini_t *csc_ini_new(void)
{   csc_ini_t *ini = csc_allocOne(csc_ini_t);
    ini->map = csc_mapStrStr_new();
    return ini;
}


void csc_ini_free(csc_ini_t *ini)
{   csc_mapStrStr_free(ini->map);
    free(ini);
}


const char *csc_ini_getStr(const csc_ini_t *ini, const char *section, const char *ident)
{   char *key;
    const char *value;
 
// Look for the key in the map table.
    key = csc_alloc_str3(section, "%", ident);
    value = csc_mapStrStr_get(ini->map, key);
    free(key);
 
// Return result.
	return value;
}


char *csc_ini_getAllocStr(const csc_ini_t *ini, const char *section, const char *ident)
{   char *key;
    const char *value;
 
// Look for the key in the map table.
    key = csc_alloc_str3(section, "%", ident);
    value = csc_mapStrStr_get(ini->map, key);
    free(key);
 
// Return result.
    if (value == NULL)
        return NULL;
    else
		return csc_alloc_str(value);
}


int csc_ini_read(csc_ini_t *ini, const char *iniFilePath)
{   FILE *fin;
    char line[MaxLineLen+1];
    char section[MaxLineLen+1];
    char *lineP, *keyEndP, *lineEndP, *valueP;
    int lineNo = 0;
 
// Initialise the section name to invalid.
    strcpy(section,"");
 
// Open the ini file.
    fin = fopen(iniFilePath, "r");
    if (fin == NULL)
        return -1;
 
// Read in each line.
    while (csc_fgetline(fin,line,MaxLineLen) != -1)
    {   lineNo++;   
 
    // Remove comments from line.
        lineEndP = strchr(line, '#');
        if (lineEndP != NULL)
            *lineEndP = '\0';
 
    // Get to first nonWhite characater of line.
        lineP = line;
        while (isspace(*lineP))
            lineP++;
 
    // Look at the line.
        if (*lineP == '\0')
        {   // ignore blank line.
        }
        else if (*lineP == '[')
        {   // Have found a section header.
            ++lineP;
 
        // Find the end of the section name.
            lineEndP = lineP;
            while (isalnum(*lineEndP) || *lineEndP=='_')
                lineEndP++;
 
        // The section name MUST terminate with a ']'.
            if (*lineEndP != ']')
                goto errorCleanUp;
            *lineEndP = '\0';
 
        //  Copy the section name.
            strcpy(section, lineP);
        }
        else if (isalnum(*lineP) || *lineP=='_')
        {   // Have found a key.
 
        // It is an error if we have a key with no section.
            if (csc_streq(section,""))
                goto errorCleanUp;
 
        // Find the end of the key.
            keyEndP = lineP;
            while (isalnum(*keyEndP) || *keyEndP=='_')
                keyEndP++;
 
        // We expect an "=" or a space here.
            if (!isspace(*keyEndP) && *keyEndP!='=')
                goto errorCleanUp;
 
        // Skip any space;
            lineEndP = keyEndP;
            while (isspace(*lineEndP))
                lineEndP++;
 
        // We expect an "=" here.
            if (*lineEndP != '=')
                goto errorCleanUp;
 
        // Skip the '='
            lineEndP++;
 
        // terminate the key.
            *keyEndP = '\0';
            
        // Skip any space from beginning of value.
            while (isspace(*lineEndP))
                lineEndP++;
            valueP = lineEndP;
            
        // Remove whitespace from end of value.
            lineEndP = valueP + strlen(valueP);
            while (lineEndP>valueP && isspace(*(lineEndP-1)))
                lineEndP--;
            *lineEndP = '\0';
        
        // We have successfully read in a valid key and value.
        // Create the record and add it into the map table.
            char *key = csc_alloc_str3(section, "%", lineP);
            csc_mapStrStr_addex(ini->map, key, valueP);
			free(key);
 
        }  // if
    } // while
 
// return success.
    fclose(fin);
    return 0;
 
// error handling.
errorCleanUp:
    fclose(fin);
    return lineNo;
}


#if 0
void testLookup(csc_ini_t *ini, char *section, char *key) 
{   char *value = csc_ini_getAllocStr(ini, section, key);
    if (value != NULL)
    {   printf("%s,%s: \"%s\"\n", section, key, value);
        free(value);
    }
    else
    {   printf("%s,%s: (not found)\n", section, key);
    }
}


int main()
{   
    char *iniFilePath;
    csc_ini_t *ini;
    int errLineNo;
    char *section;
 
    iniFilePath = "test.ini";
    ini = csc_ini_new();
    errLineNo = csc_ini_read(ini, iniFilePath);
 
    if (errLineNo == -1)
    {   fprintf(stderr, "Error opening in ini file \"%s\"\n", iniFilePath);
    }
    else if (errLineNo > 0)
    {   fprintf(stderr, "Error at line number %d in ini file \"%s\"\n",
                        errLineNo, iniFilePath);
    }
    else
    {   section = "Person"; 
        testLookup(ini,section,"name");
        testLookup(ini,section,"email");
        testLookup(ini,section,"dob");
 
        section = "Position";   
        testLookup(ini,section,"rank");
        testLookup(ini,section,"salary");
        testLookup(ini,section,"phone");
    }   
        
    csc_ini_free(ini);
    exit(0);
}

#endif
    
