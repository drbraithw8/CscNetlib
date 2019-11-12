#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <CscNetLib/std.h>
#include <CscNetLib/alloc.h>
#include <CscNetLib/hash.h>


void testReport_bVal(const char *testName, csc_bool_t required, csc_bool_t got)
{
//  printf("req=\"%d\"  got=\"%d\"\n", required, got);
    if (got == required)    
        fprintf(stdout, "pass (%s)\n", testName);
    else
        fprintf(stdout, "FAIL (%s)\n", testName);
}


void testReport_iVal(const char *testName, int required, int got)
{
//  printf("req=\"%d\"  got=\"%d\"\n", required, got);
    if (got == required)    
        fprintf(stdout, "pass (%s)\n", testName);
    else
        fprintf(stdout, "FAIL (%s)\n", testName);
}


void testReport_fVal(const char *testName, double required, double got)
{
//  printf("req=\"%f\"  got=\"%f\"\n", required, got);
    if (got == required)    
        fprintf(stdout, "pass (%s)\n", testName);
    else
        fprintf(stdout, "FAIL (%s)\n", testName);
}



void testReport_sVal(const char *testName, const char *required, const char *got)
{
//  printf("req=\"%s\"  got=\"%s\"\n", required, got);
    if (required==NULL || got==NULL)
    {   if (required == got)
            fprintf(stdout, "pass (%s)\n", testName);
        else
            fprintf(stdout, "FAIL (%s)\n", testName);
    }
    else
    {   if (csc_streq(got,required))    
            fprintf(stdout, "pass (%s)\n", testName);
        else
            fprintf(stdout, "FAIL (%s)\n", testName);
    }
}


void hashTest1()
{   csc_bool_t ret;
    csc_mapSS_t *map = csc_mapSS_new();
 
    testReport_sVal("mapss_empty", NULL, (const char*)csc_mapSS_get(map, "fred"));
 
    ret = csc_mapSS_addex(map, "fred", "bloggs");
    testReport_bVal("mapss_fred_add1", csc_TRUE, ret);
    testReport_sVal("mapss_fred_add2", "bloggs", csc_mapSS_get(map,"fred")->val);
 
    ret = csc_mapSS_addex(map, "fred", "blogs");
    testReport_bVal("mapss_fred_addex1", csc_FALSE, ret);
    testReport_sVal("mapss_fred_addex1a", "bloggs", csc_mapSS_get(map,"fred")->val);
 
    ret = csc_mapSS_addex(map, "john", "smith");
    testReport_sVal("mapss_empty", "smith", csc_mapSS_get(map,"john")->val);
 
    testReport_bVal("mapss_fred_out1a", csc_TRUE, csc_mapSS_out(map, "fred"));
    testReport_sVal("mapss_fred_out1a", NULL, (const char*)csc_mapSS_get(map,"fred"));
 
    csc_mapSS_free(map);
}


void hashTest2(int mHits)
{	char string[30];
 
    typedef struct
    {   csc_bool_t isHit;
        char name[12];
        char val[12];
    } nvhit_t;
 
    nvhit_t *hits = NULL;
	hits = csc_allocMany(nvhit_t, mHits);
 
    csc_bool_t isOK;
    csc_mapSS_t *map = csc_mapSS_new();
 
// Insert Initialise array and insert into map.
    for (int i=0; i<mHits; i++)
    {   nvhit_t *hit = &hits[i];
        hit->isHit = csc_FALSE;
        sprintf(hit->name, "%d", i);
        sprintf(hit->val, "val_%d", i);
        csc_mapSS_addex(map, hit->name, hit->val);
    }
 
// Check all entries in map.
    isOK = csc_TRUE;
    for (int i=0; i<mHits; i++)
    {   nvhit_t *hit = &hits[i];
        if (strcmp(hit->val, csc_mapSS_get(map,hit->name)->val))
        {   isOK = csc_FALSE;
        }
    }
	sprintf(string, "mapss_2_%d", mHits);
    testReport_bVal(string, csc_TRUE, isOK);
        
// Iterate through map, and check.
    csc_mapSS_iter_t *iter = csc_mapSS_iter_new(map);
    int count = 0;
    isOK = csc_TRUE;
    csc_nameVal_t *nv;
    while (nv = (csc_nameVal_t*)csc_mapSS_iter_next(iter))
    {   count++;
        int ndx = atoi(nv->name); assert(ndx>=0 && ndx<mHits);
        nvhit_t *hit = &hits[ndx];
        if (hit->isHit || strcmp(hit->name,nv->name) || strcmp(hit->val,nv->val))
            isOK = csc_FALSE;
        else
            hit->isHit = csc_TRUE;
    }
	sprintf(string, "mapss_3a_%d", mHits);
    testReport_iVal(string, mHits, count);
	sprintf(string, "mapss_3b_%d", mHits);
    testReport_bVal(string, csc_TRUE, isOK);
        
// Cleanup.
    csc_mapSS_iter_free(iter);
    csc_mapSS_free(map);
	free(hits);
}


void hashTest3(int maxWords)
{	char buf[256];
	char *str;
	char *errStr = NULL;
 
// Create the hash object.
	csc_hash_t *hash = csc_hash_new( 0   // We are passing a simple string.
						   , (int (*)(void*,void*))strcmp
						   , csc_hash_str
						   , csc_hash_FreeBlk
						   );
 
// Add number strings into the hash table.
	for (int i=0; i<maxWords; i++)
	{	sprintf(buf, "%d", i);
		str = csc_alloc_str(buf);
		if (!csc_hash_addex(hash, str))
		{	errStr = "addex() failed";
			break;
		}
		if (csc_hash_get(hash, buf) != str)
		{	errStr = "removed string still present";
			break;
		}
	}
 
	if (errStr)
	{	fprintf(stdout, "FAIL (%s_%d): %s\n", "hash3_adding", maxWords, errStr);
		return;
	}
	else
	{	fprintf(stdout, "pass (%s_%d)\n", "hash3_adding", maxWords);
	}
 
// Pull out each one.
	for (int i=0; i<maxWords; i++)
	{	sprintf(buf, "%d", i);
 
	// Pull out.
		str = csc_hash_out(hash, buf);
		if (str == NULL)
		{	errStr = "hashOut() failed1";
			break;
		}
		free(str);
 
	// Second attempt to pull out should fail.
		str = csc_hash_out(hash, buf);
		if (str != NULL)
		{	errStr = "hashOut() stringRemains";
			break;
		}
	}
 
	if (errStr)
	{	fprintf(stdout, "FAIL (%s_%d): %s\n", "hash3_pullout", maxWords, errStr);
		return;
	}
	else
	{	fprintf(stdout, "pass (%s_%d)\n", "hash3_pullout", maxWords);
	}
 
// Bye. Check for memory leaks.
	csc_hash_free(hash);
  	csc_mck_print(stderr);
}


void hashTest4(int maxWords)
{   char buf[256];
    char *str;
	char *errStr = NULL;
 
// Create the table.
    int *entryTable = csc_ck_calloc(maxWords * sizeof(int));
 
// Create the hash object.
    csc_hash_t *hash = csc_hash_new( 0   // We are passing a simple string.
                             , (int (*)(void*,void*))strcmp
                             , csc_hash_str
                             , csc_hash_FreeBlk
                           );
 
// Add number strings into the hash table.
    for (int i=0; i<maxWords; i++)
    {   sprintf(buf, "%d", i);
        str = csc_alloc_str(buf);
        if (!csc_hash_addex(hash, str))
		{	errStr = "string failed to add";
			break;
		}
        if (csc_hash_get(hash, buf) != str)
		{	errStr = "string failed to get";
			break;
		}
    }
 
	if (errStr)
	{	fprintf(stdout, "FAIL (%s_%d): %s\n", "iter_addex", maxWords, errStr);
		return;
	}
	else
	{	fprintf(stdout, "pass (%s_%d)\n", "iter_addex", maxWords);
	}
 
 
// Loop each string.
    csc_hash_iter_t *it = csc_hash_iter_new(hash);
    while ((str = (char*)csc_hash_iter_next(it)) != NULL)
    {   int ndx = atoi(str);
        if (ndx<0 || ndx>=maxWords)
		{	errStr = "string out of bounds";
			break;
		}
        else
            entryTable[ndx]++;
    }
	if (errStr)
	{	fprintf(stdout, "FAIL (%s_%d): %s\n", "iter_looping", maxWords, errStr);
		return;
	}
	else
	{	fprintf(stdout, "pass (%s_%d)\n", "iter_looping", maxWords);
	}
 
 
// Check that all have value 1.
    for (int i=0; i<maxWords; i++)
    {   if (entryTable[i] != 1)
		{	errStr = "entry count not 1";
			break;
		}
    }
	if (errStr)
	{	fprintf(stdout, "FAIL (%s_%d): %s\n", "iter_accounting", maxWords, errStr);
		return;
	}
	else
	{	fprintf(stdout, "pass (%s_%d)\n", "iter_accounting", maxWords);
	}
 
// Bye. Check for memory leaks.
    csc_hash_iter_free(it);
    csc_hash_free(hash);
    free(entryTable);
    csc_mck_print(stderr);
}


void main(int argc, char **argv)
{   hashTest1();
    hashTest2(10);
    hashTest2(100);
    hashTest2(1000);
    hashTest2(10000);
    hashTest2(100000);
    hashTest3(10);
    hashTest3(100);
    hashTest3(1000);
    hashTest3(10000);
    hashTest3(100000);
    hashTest4(10);
    hashTest4(100);
    hashTest4(1000);
    hashTest4(10000);
    hashTest4(100000);
    hashTest4(1000000);

    if (csc_mck_nchunks() == 0)
        fprintf(stdout, "pass (%s)\n", "hash_memory");
    else
        fprintf(stdout, "FAIL (%s)\n", "hash_memory");
    csc_mck_print(stdout);
}
