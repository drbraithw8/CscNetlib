#include <stdlib.h>
#include <stdio.h>

#include <CscNetLib/std.h>
#include <CscNetLib/alloc.h>
#include <CscNetLib/http.h>


void testReport_iVal(FILE *fout, char *testName, int required, int got)
{	if (got == required)	
		fprintf(fout, "pass (%s)\n", testName);
	else
		fprintf(fout, "FAIL (%s)\n", testName);
}


void testReport_sVal(FILE *fout, char *testName, const char *required, const char *got)
{	if (required==NULL || got==NULL)
	{	if (required == got)
			fprintf(fout, "pass (%s)\n", testName);
		else
			fprintf(fout, "FAIL (%s)\n", testName);
	}
	else
	{	if (csc_streq(got,required))	
			fprintf(fout, "pass (%s)\n", testName);
		else
			fprintf(fout, "FAIL (%s)\n", testName);
	}
}


void testAddHdr(FILE *fout, char *testName, csc_httpMsg_t *hMsg, const char *hdrName, const char *hdrVal)
{	csc_httpErr_t errCode = csc_httpMsg_addHdr(hMsg, hdrName, hdrVal);
	testReport_iVal(fout, testName, csc_httpErr_Ok, errCode);
}


void testGetHdr(FILE *fout, char *testName, csc_httpMsg_t *hMsg, const char *hdrName, const char *hdrVal)
{	const char *val = csc_httpMsg_getHdr(hMsg, hdrName);
	testReport_sVal(fout, testName, hdrVal, val);
}


void testAddGet(FILE *fout)
{
	csc_httpErr_t errCode;
	csc_httpMsg_t *hMsg = NULL;
	char *str = NULL;

// Create the message.
	hMsg = csc_httpMsg_new();

// Test the empty ones.
	testGetHdr(fout, "http_getNull_proto", hMsg, csc_http_protocol, NULL);
	testGetHdr(fout, "http_getNull_reqUri", hMsg, csc_http_reqUri, NULL);
	testGetHdr(fout, "http_getNull_method", hMsg, csc_http_method, NULL);
	testGetHdr(fout, "http_getNull_statCode", hMsg, csc_http_statCode, NULL);
	testGetHdr(fout, "http_getNull_reason", hMsg, csc_http_reason, NULL);
	testGetHdr(fout, "http_getNull_host", hMsg, csc_http_host, NULL);
	testGetHdr(fout, "http_getHdr_notEntered", hMsg, "invalid", NULL);

// Add real and pseudo headers.
	const char *tstHdrVal_proto = "HTTP/1.1";
	testAddHdr(fout, "http_addHdr_proto", hMsg, csc_http_protocol, tstHdrVal_proto);

	const char *tstHdrVal_reqUri = "/index.html";
	testAddHdr(fout, "http_addHdr_reqUri", hMsg, csc_http_reqUri, tstHdrVal_reqUri);

	const char *tstHdrVal_method = "GET";
	testAddHdr(fout, "http_addHdr_method", hMsg, csc_http_method, tstHdrVal_method);

	const char *tstHdrVal_host = "bology.com.au";
	testAddHdr(fout, "http_addHdr_host", hMsg, csc_http_host, tstHdrVal_host);

	const char *tstHdrVal_statCode = "200";
	testAddHdr(fout, "http_addHdr_statCode", hMsg, csc_http_statCode, tstHdrVal_statCode);

	const char *tstHdrVal_reason = "OK";
	testAddHdr(fout, "http_addHdr_reason", hMsg, csc_http_reason, tstHdrVal_reason);

	const char *tstHdrVal_conn = "close";
	const char *tstHdrName_conn = "Connection";
	testAddHdr(fout, "http_addHdr_conn", hMsg, tstHdrName_conn, tstHdrVal_conn);

	const char *tstHdrVal_any = "something";
	const char *tstHdrName_any = "Anything";
	testAddHdr(fout, "http_addHdr_any", hMsg, tstHdrName_any, tstHdrVal_any);

// Get real and pseudo headers.
	testGetHdr(fout, "http_getHdr_proto", hMsg, csc_http_protocol, tstHdrVal_proto);
	testGetHdr(fout, "http_getHdr_reqUri", hMsg, csc_http_reqUri, tstHdrVal_reqUri);
	testGetHdr(fout, "http_getHdr_method", hMsg, csc_http_method, tstHdrVal_method);
	testGetHdr(fout, "http_getHdr_host", hMsg, csc_http_host, tstHdrVal_host);
	testGetHdr(fout, "http_getHdr_statCode", hMsg, csc_http_statCode, tstHdrVal_statCode);
	testGetHdr(fout, "http_getHdr_reason", hMsg, csc_http_reason, tstHdrVal_reason);
	testGetHdr(fout, "http_getHdr_conn", hMsg, tstHdrName_conn, tstHdrVal_conn);
	testGetHdr(fout, "http_getHdr_any", hMsg, tstHdrName_any, tstHdrVal_any);

// Request one that was not entered.
	testGetHdr(fout, "http_getHdr_notEntered", hMsg, "invalid", NULL);

// Free resources.
	csc_httpMsg_free(hMsg );
}


int main(int argc, char **argv)
{	testAddGet(stdout);
	exit(0);
}
