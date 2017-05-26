#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <CscNetLib/std.h>
#include <CscNetLib/alloc.h>
#include <CscNetLib/isvalid.h>


typedef struct
{	FILE *fout;
	FILE *ffail;
	csc_bool_t (*validFn)(const char *str);
	char *funcName;
} testReport_t;


testReport_t *testReport_new(csc_bool_t (*valid)(const char*), char *func)
{	testReport_t *tr = csc_allocOne(testReport_t);
	tr->fout = fopen("csc_testOut.txt", "w"); assert(tr->fout);
	tr->ffail = fopen("csc_testFails.txt", "w"); assert(tr->ffail);
	tr->validFn = valid;
	tr->funcName = func;
	return tr;
}


void testReport_free(testReport_t *tr)
{	fclose(tr->fout);
	fclose(tr->ffail);
	free(tr);
}


void testReport_out(testReport_t *tr, char *testName, csc_bool_t isOk)
{	if (isOk)
		fprintf(tr->fout, "PASSED %s(%s)\n", tr->funcName, testName);
	else
		fprintf(tr->fout, "FAILED %s(%s)\n", tr->funcName, testName);
}


void testReport_fail(testReport_t *tr, char *testName)
{	fprintf(tr->ffail, "FAILED %s(%s)\n", tr->funcName, testName);
}


void testReport_test( testReport_t *tr
					, char *testName
					, char *str
					, csc_bool_t goodAnswer
					)
{	csc_bool_t result = (*tr->validFn)(str);
	csc_bool_t isOk = (result == goodAnswer);
	testReport_out(tr, testName, isOk);
	if (!isOk)
		testReport_fail(tr, testName);
}


void main(int argc, char **argv)
{	
	testReport_t *tr = testReport_new(csc_isValid_hex, "csc_isValid_hex");
	testReport_test(tr, "emptyString", "", csc_FALSE);
	testReport_test(tr, "single", "3", csc_FALSE);
	testReport_test(tr, "pairBad", "3s", csc_FALSE);
	testReport_test(tr, "pairGood1", "3f", csc_TRUE);
	testReport_test(tr, "pairGood2", "3F", csc_TRUE);
	testReport_test(tr, "pairGood3", "33", csc_TRUE);
	testReport_test(tr, "pairGood4", "03", csc_TRUE);
	testReport_test(tr, "pairGood5", "30", csc_TRUE);
	testReport_test(tr, "longGood5", "30Af", csc_TRUE);
	testReport_test(tr, "longGood6", "30aF", csc_TRUE);
	testReport_test(tr, "longGood7", "aF30", csc_TRUE);
	testReport_test(tr, "longBad1", "aFg9", csc_FALSE);
	testReport_test(tr, "longBad2", "sF20", csc_FALSE);
	testReport_test(tr, "longBad3", "sF2T", csc_FALSE);
	testReport_free(tr);
}
