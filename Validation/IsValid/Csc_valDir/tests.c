#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <CscNetLib/std.h>
#include <CscNetLib/alloc.h>
#include <CscNetLib/isvalid.h>


typedef struct
{	FILE *fout;
	csc_bool_t (*validFn)(const char *str);
	char *funcName;  // Not allocated.  Not freed.
} testReport_t;


testReport_t *testReport_new(csc_bool_t (*valid)(const char*), char *func)
{	testReport_t *tr = csc_allocOne(testReport_t);
	tr->fout = fopen("csc_testOut.txt", "a"); assert(tr->fout);
	tr->validFn = valid;
	tr->funcName = csc_alloc_str(func);  // Not allocated.  Not freed.
	return tr;
}


void testReport_free(testReport_t *tr)
{	fclose(tr->fout);
	free(tr->funcName);
	free(tr);
}


void testReport_out(testReport_t *tr, char *testName, csc_bool_t isOk)
{
}


void testReport_test( testReport_t *tr
					, char *testName
					, char *str
					, csc_bool_t goodAnswer
					)
{	csc_bool_t result = (*tr->validFn)(str);
	if (result == goodAnswer)
		fprintf(tr->fout, "pass %s(%s)\n", tr->funcName, testName);
	else
		fprintf(tr->fout, "FAIL %s(%s)\n", tr->funcName, testName);
}


void main(int argc, char **argv)
{	testReport_t *tr;

// csc_isValid_hex().
	tr = testReport_new(csc_isValid_hex, "csc_isValid_hex");
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

// csc_isValid_int().
	tr = testReport_new(csc_isValid_int, "csc_isValid_int");
	testReport_test(tr, "emptyString", "", csc_FALSE);
	testReport_test(tr, "zero", "0", csc_TRUE);
	testReport_test(tr, "float0", "0.3", csc_FALSE);
	testReport_test(tr, "float1", "1.3", csc_FALSE);
	testReport_test(tr, "negativeZero", "-0", csc_TRUE);
	testReport_test(tr, "doubleNegativeZero", "--0", csc_FALSE);
	testReport_test(tr, "signleDigit", "9", csc_TRUE);
	testReport_test(tr, "negativeSignleDigit", "-3", csc_TRUE);
	testReport_test(tr, "doubleNegativeSignleDigit", "--3", csc_FALSE);
	testReport_test(tr, "signleNonDigit", "a", csc_FALSE);
	testReport_test(tr, "trailingMinus", "3-", csc_FALSE);
	testReport_test(tr, "longInt", "123456789", csc_TRUE);
	testReport_test(tr, "badLongInt1", "123456789a", csc_FALSE);
	testReport_test(tr, "badLongInt2", "123456789-", csc_FALSE);
	testReport_free(tr);

// csc_isValid_float().
	tr = testReport_new(csc_isValid_float, "csc_isValid_float");
	testReport_test(tr, "emptyString", "", csc_FALSE);
	testReport_test(tr, "zero", "0", csc_TRUE);
	testReport_test(tr, "negativeZero", "-0", csc_TRUE);
	testReport_test(tr, "doubleNegativeZero", "--0", csc_FALSE);
	testReport_test(tr, "signleDigit", "9", csc_TRUE);
	testReport_test(tr, "negativeSignleDigit", "-3", csc_TRUE);
	testReport_test(tr, "doubleNegativeSignleDigit", "--3", csc_FALSE);
	testReport_test(tr, "signleNonDigit", "a", csc_FALSE);
	testReport_test(tr, "trailingMinus", "3-", csc_FALSE);
	testReport_test(tr, "longInt", "123456789", csc_TRUE);
	testReport_test(tr, "badLongInt1", "123456789a", csc_FALSE);
	testReport_test(tr, "badLongInt2", "123456789-", csc_FALSE);
	testReport_test(tr, "float0", "0.3", csc_TRUE);
	testReport_test(tr, "float1", "1.3", csc_TRUE);
	testReport_test(tr, "float2", ".3", csc_TRUE);
	testReport_test(tr, "float3", "3.", csc_TRUE);
	testReport_test(tr, "float4", "3e4", csc_TRUE);
	testReport_test(tr, "float5", "36e4", csc_TRUE);
	testReport_test(tr, "float6", "0.3e4", csc_TRUE);
	testReport_test(tr, "float7", "-0.3e4", csc_TRUE);
	testReport_test(tr, "badFloat1", "1.3a", csc_FALSE);
	testReport_test(tr, "badFloat2", "1.a", csc_FALSE);
	testReport_test(tr, "badFloat3", "1.1.", csc_FALSE);
	testReport_test(tr, "badFloat4", "1e.1", csc_FALSE);
	testReport_test(tr, "badFloat5", "1e1.1", csc_FALSE);
	testReport_free(tr);

// csc_isValid_IpV4().
	tr = testReport_new(csc_isValid_ipV4, "csc_isValid_ipV4");
	testReport_test(tr, "emptyString", "", csc_FALSE);
	testReport_test(tr, "number", "3", csc_FALSE);
	testReport_test(tr, "ipGood1", "192.168.1.1", csc_TRUE);
	testReport_test(tr, "ipGood2", "255.255.255.255", csc_TRUE);
	testReport_test(tr, "ipGood3", "1.1.1.1", csc_TRUE);
	testReport_test(tr, "ipBad1", "a.1.1.1", csc_FALSE);
	testReport_test(tr, "ipBad2", "1.0.1.A", csc_FALSE);
	testReport_test(tr, "ipBad3", "1.0.11", csc_FALSE);
	testReport_test(tr, "ipBad4", "1.0.11.3.2", csc_FALSE);
	testReport_test(tr, "ipBad5", "1.0.11.256", csc_FALSE);
	testReport_test(tr, "ipBad6", "1.0.11.-25", csc_FALSE);
	testReport_free(tr);

// csc_isValid_IpV6().
	tr = testReport_new(csc_isValid_ipV6, "csc_isValid_ipV6");
	testReport_test(tr, "emptyString", "", csc_FALSE);
	testReport_test(tr, "number", "3", csc_FALSE);
	testReport_test(tr, "ipGood1", "192.168.1.1", csc_FALSE);
	testReport_test(tr, "ipGood2", "::ffff:192.168.1.1", csc_TRUE);
	testReport_test(tr, "ipGood3", "::ffff:255.255.255.255", csc_TRUE);
	testReport_test(tr, "ipGood4", "::ffff:1.1.1.1", csc_TRUE);
	testReport_test(tr, "ipGood5", "2001:0db8::ff00:0042:8329", csc_TRUE);
	testReport_test(tr, "ipGood6", "2001:0gb8::ff00:0042:8329", csc_FALSE);
	testReport_test(tr, "ipGood7", "2001:0db8:0000:0000:0000:FF00:0042:8329", csc_TRUE);
	testReport_test(tr, "ipGood8", "2001:0db8:0000:0000:0000:0000:ff00:0042:8329", csc_FALSE);
	testReport_test(tr, "ipGood9", "2001:0db8:0000:0000:ff00:0042:8329", csc_FALSE);
	testReport_test(tr, "ipGood10", "2001:0DB8:0000:0000:0000:ff00:0042:8329", csc_TRUE);
	testReport_test(tr, "ipGood11", "2001:0db8::", csc_TRUE);
	testReport_test(tr, "ipGood12", "::1", csc_TRUE);
	testReport_test(tr, "ipGood13", "2001:0db8:000:0000:0000:ff00:42:8329", csc_TRUE);
	testReport_test(tr, "ipBad1", "::ffff:a.1.1.1", csc_FALSE);
	testReport_test(tr, "ipBad2", "::ffff:1.0.1.A", csc_FALSE);
	testReport_test(tr, "ipBad3", "::ffff:1.0.11", csc_FALSE);
	testReport_test(tr, "ipBad4", "::ffff:1.0.11.3.2", csc_FALSE);
	testReport_test(tr, "ipBad5", "::ffff:1.0.11.256", csc_FALSE);
	testReport_test(tr, "ipBad6", "::ffff:1.0.11.-25", csc_FALSE);
	testReport_free(tr);

// csc_isValid_domain().
	tr = testReport_new(csc_isValid_domain, "csc_isValid_domain");
	testReport_test(tr, "emptyString", "", csc_FALSE);
	testReport_test(tr, "good1", "google.com", csc_TRUE);
	testReport_test(tr, "good2", "google.COM.au", csc_TRUE);
	testReport_test(tr, "bad1", "google", csc_FALSE);
	testReport_test(tr, "bad2", ".google.com", csc_FALSE);
	testReport_test(tr, "bad3", ".google.com.", csc_FALSE);
	testReport_test(tr, "bad4", "google@com", csc_FALSE);
	testReport_test(tr, "bad5", ".google.c-m.", csc_FALSE);
	testReport_free(tr);

}
