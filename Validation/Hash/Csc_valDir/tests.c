#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <CscNetLib/std.h>
#include <CscNetLib/alloc.h>
#include <CscNetLib/hash.h>



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

void testReport_iVal(const char *testName, int required, int got)
{	if (got == required)	
		fprintf(stdout, "pass (%s)\n", testName);
	else
		fprintf(stdout, "FAIL (%s)\n", testName);
}


void main(int argc, char **argv)
{	csc_bool_t ret;
	csc_mapStrStr_t *map = csc_mapStrStr_new();
 
	testReport_sVal("mapss_empty", NULL, csc_mapStrStr_get(map, "fred"));
 
	ret = csc_mapStrStr_addex(map, "fred", "bloggs");
	testReport_iVal("mapss_fred_add1", csc_TRUE, ret);
	testReport_sVal("mapss_fred_add2", "bloggs", csc_mapStrStr_get(map, "fred"));
 
	ret = csc_mapStrStr_addex(map, "fred", "blogs");
	testReport_iVal("mapss_fred_addex1", csc_FALSE, ret);
	testReport_sVal("mapss_fred_addex1a", "bloggs", csc_mapStrStr_get(map, "fred"));
 
	ret = csc_mapStrStr_addex(map, "john", "smith");
	testReport_sVal("mapss_empty", "smith", csc_mapStrStr_get(map, "john"));
 
	testReport_sVal("mapss_fred_out1a", "bloggs", csc_mapStrStr_out(map, "fred"));
	testReport_sVal("mapss_fred_out1a", NULL, csc_mapStrStr_get(map, "fred"));
	
	csc_mapStrStr_free(map);
	if (csc_mck_nchunks() == 0)
		fprintf(stdout, "pass (%s)\n", "mapss_memory");
	else
		fprintf(stdout, "FAIL (%s)\n", "mapss_memory");
	csc_mck_print(stdout);
}
