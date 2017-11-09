#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <CscNetLib/std.h>
#include <CscNetLib/alloc.h>
#include <CscNetLib/hash.h>


void testReport_bVal(const char *testName, csc_bool_t required, csc_bool_t got)
{
// 	printf("req=\"%d\"  got=\"%d\"\n", required, got);
	if (got == required)	
		fprintf(stdout, "pass (%s)\n", testName);
	else
		fprintf(stdout, "FAIL (%s)\n", testName);
}


void testReport_iVal(const char *testName, int required, int got)
{
// 	printf("req=\"%d\"  got=\"%d\"\n", required, got);
	if (got == required)	
		fprintf(stdout, "pass (%s)\n", testName);
	else
		fprintf(stdout, "FAIL (%s)\n", testName);
}


void testReport_fVal(const char *testName, double required, double got)
{
// 	printf("req=\"%f\"  got=\"%f\"\n", required, got);
	if (got == required)	
		fprintf(stdout, "pass (%s)\n", testName);
	else
		fprintf(stdout, "FAIL (%s)\n", testName);
}



void testReport_sVal(const char *testName, const char *required, const char *got)
{
// 	printf("req=\"%s\"  got=\"%s\"\n", required, got);
	if (required==NULL || got==NULL)
	{	if (required == got)
			fprintf(stdout, "pass (%s)\n", testName);
		else
			fprintf(stdout, "FAIL (%s)\n", testName);
	}
	else
	{	if (csc_streq(got,required))	
			fprintf(stdout, "pass (%s)\n", testName);
		else
			fprintf(stdout, "FAIL (%s)\n", testName);
	}
}


void hashTest1()
{	csc_bool_t ret;
	csc_mapSS_t *map = csc_mapSS_new();
 
	testReport_sVal("mapss_empty", NULL, csc_mapSS_get(map, "fred"));
 
	ret = csc_mapSS_addex(map, "fred", "bloggs");
	testReport_bVal("mapss_fred_add1", csc_TRUE, ret);
	testReport_sVal("mapss_fred_add2", "bloggs", csc_mapSS_get(map, "fred"));
 
	ret = csc_mapSS_addex(map, "fred", "blogs");
	testReport_bVal("mapss_fred_addex1", csc_FALSE, ret);
	testReport_sVal("mapss_fred_addex1a", "bloggs", csc_mapSS_get(map, "fred"));
 
	ret = csc_mapSS_addex(map, "john", "smith");
	testReport_sVal("mapss_empty", "smith", csc_mapSS_get(map, "john"));
 
	testReport_bVal("mapss_fred_out1a", csc_TRUE, csc_mapSS_out(map, "fred"));
	testReport_sVal("mapss_fred_out1a", NULL, csc_mapSS_get(map, "fred"));

	csc_mapSS_free(map);
}


void hashTest2()
{
	typedef struct
	{	csc_bool_t isHit;
		char name[12];
		char val[12];
	} nvhit_t;

	const int mHits = 20;
	nvhit_t hits[mHits];
	csc_bool_t isOK;
	csc_mapSS_t *map = csc_mapSS_new();

// Insert Initialise array and insert into map.
	for (int i=0; i<mHits; i++)
	{	nvhit_t *hit = &hits[i];
		hit->isHit = csc_FALSE;
		sprintf(hit->name, "%d", i);
		sprintf(hit->val, "val_%d", i);
		csc_mapSS_addex(map, hit->name, hit->val);
	}

// Check all entries in map.
	isOK = csc_TRUE;
	for (int i=0; i<mHits; i++)
	{	nvhit_t *hit = &hits[i];
		if (strcmp(hit->val, csc_mapSS_get(map, hit->name)))
		{	isOK = csc_FALSE;
		}
	}
	testReport_bVal("mapss_2", csc_TRUE, isOK);
		
// Iterate through map, and check.
	csc_mapSS_iter_t *iter = csc_mapSS_iter_new(map);
	int count = 0;
	isOK = csc_TRUE;
	csc_nameVal_t *nv;
	while (nv = (csc_nameVal_t*)csc_mapSS_iter_next(iter))
	{	count++;
		int ndx = atoi(nv->name); assert(ndx>=0 && ndx<mHits);
		nvhit_t *hit = &hits[ndx];
		if (hit->isHit || strcmp(hit->name,nv->name) || strcmp(hit->val,nv->val))
			isOK = csc_FALSE;
		else
			hit->isHit = csc_TRUE;
	}
	testReport_iVal("mapss_3a", mHits, count);
	testReport_bVal("mapss_3b", csc_TRUE, isOK);
		
// Cleanup.
	csc_mapSS_iter_free(iter);
	csc_mapSS_free(map);
}


void main(int argc, char **argv)
{	hashTest1();
	hashTest2();

	if (csc_mck_nchunks() == 0)
		fprintf(stdout, "pass (%s)\n", "mapss_memory");
	else
		fprintf(stdout, "FAIL (%s)\n", "mapss_memory");
	csc_mck_print(stdout);
}
