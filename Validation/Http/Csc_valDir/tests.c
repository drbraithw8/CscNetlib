#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <CscNetLib/std.h>
#include <CscNetLib/alloc.h>
#include <CscNetLib/http.h>


void testReport_iVal(FILE *fout, const char *testName, int required, int got)
{	if (got == required)	
		fprintf(fout, "pass (%s)\n", testName);
	else
		fprintf(fout, "FAIL (%s)\n", testName);
}


void testReport_sVal(FILE *fout, const char *testName, const char *required, const char *got)
{
// 	printf("req=\"%s\"  got=\"%s\"\n", required, got);
	if (required==NULL || got==NULL)
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


void testCrst( const char *testName
			 , const char *testStr
			 , const char *proto
			 , const char *statCode
			 , const char *reason
			 )
{
// Resources
	csc_httpMsg_t *msg = NULL;
 
// Read from the text.
	msg = csc_httpMsg_new();
	csc_httpErr_t errCode = csc_httpMsg_rcvCliStr(msg, testStr);
	testReport_iVal(stdout, "http_cRcv1_retVal", csc_httpErr_Ok, errCode);
 
// Check 
	testReport_sVal(stdout, testName, proto,
					csc_httpMsg_getHdr(msg, csc_http_protocol));
	testReport_sVal(stdout, testName, statCode,
					csc_httpMsg_getHdr(msg, csc_http_statCode));
	testReport_sVal(stdout, testName, reason,
					csc_httpMsg_getHdr(msg, csc_http_reason));
 
// Free resources.
	if (msg)
		csc_httpMsg_free(msg);
}


void testCliRcv1()
{	testCrst( "http_cRcvStr1"
			, "HTTP/1.1 200 OK\nServer: webfs/1.21\n"
			, "HTTP/1.1", "200", "OK");
	testCrst( "http_cRcvStr2"
			, "HTTP/1.0 404 Not Found\nServer: webfs/1.21\n"
			, "HTTP/1.0", "404", "Not Found");
}



void testSrst( const char *testName
			 , const char *testStr
			 , const char *method
			 , const char *reqUri
			 , const char *proto
			 )
{
// Resources
	csc_httpMsg_t *msg = NULL;
 
// Read from the text.
	msg = csc_httpMsg_new();
	csc_httpErr_t errCode = csc_httpMsg_rcvSrvStr(msg, testStr);
	testReport_iVal(stdout, "http_sRcv1_retVal", csc_httpErr_Ok, errCode);
	if (errCode != csc_httpErr_Ok)
		fprintf(stdout, "\terrStr=%s\n", csc_httpMsg_getErrMsg(msg));
 
// Check 
	testReport_sVal(stdout, testName, method,
					csc_httpMsg_getHdr(msg, csc_http_method));
	testReport_sVal(stdout, testName, reqUri,
					csc_httpMsg_getHdr(msg, csc_http_reqUri));
	testReport_sVal(stdout, testName, proto,
					csc_httpMsg_getHdr(msg, csc_http_protocol));
 
// Free resources.
	if (msg)
		csc_httpMsg_free(msg);
}

void testSrvRcv1()
{	testSrst( "http_sRcvStr1"
			, "GET / HTTP/1.1\nHost: bology.com.au\n"
			, "GET", "/", "HTTP/1.1");
	testSrst( "http_sRcvStr2"
			, "POST /doing/char.php HTTP/1.0\nHost: bology.com.au\n"
			, "POST", "/doing/char.php", "HTTP/1.0");
}


void testCliRcv2()
{	
// Resources.
	FILE *fin = fopen("testCliRcv2.txt", "r"); assert(fin);
	csc_httpMsg_t *msg = csc_httpMsg_new();

// Read all in.
	csc_httpErr_t errVal = csc_httpMsg_rcvCliFILE(msg, fin);
	testReport_iVal(stdout, "http_cRcv2_retVal", csc_httpErr_Ok, errVal);

// Check all the headers.
	testReport_sVal(stdout, "http_cRcv2_proto", "HTTP/1.1",
					csc_httpMsg_getHdr(msg, csc_http_protocol));
	testReport_sVal(stdout, "http_cRcv2_statCode", "200",
					csc_httpMsg_getHdr(msg, csc_http_statCode));
	testReport_sVal(stdout, "http_cRcv2_reason", "OK",
					csc_httpMsg_getHdr(msg, csc_http_reason));
	testReport_sVal(stdout, "http_cRcv2_server", "webfs/1.21",
					csc_httpMsg_getHdr(msg, "Server"));
	testReport_sVal(stdout, "http_cRcv2_connection", "Close",
					csc_httpMsg_getHdr(msg, "Connection"));
	testReport_sVal(stdout, "http_cRcv2_accRanges", "bytes",
					csc_httpMsg_getHdr(msg, "Accept-Ranges"));
	testReport_sVal(stdout, "http_cRcv2_contType", "text/html",
					csc_httpMsg_getHdr(msg, "Content-Type"));
	testReport_sVal(stdout, "http_cRcv2_contLen", "157",
					csc_httpMsg_getHdr(msg, "Content-Length"));
	testReport_sVal( stdout
				   , "http_cRcv2_lastMod"
				   , "Thu, 06 Jul 2017 05:33:59 GMT"
				   , csc_httpMsg_getHdr(msg, "Last-Modified")
				   );
	testReport_sVal( stdout
				   , "http_cRcv2_date"
				   , "Thu, 06 Jul 2017 05:56:58 GMT"
				   , csc_httpMsg_getHdr(msg, "Date")
				   );
	testReport_sVal(stdout, "http_cRcv2_notThere", NULL,
					csc_httpMsg_getHdr(msg, "notThere"));
	
// Free resources.
	csc_httpMsg_free(msg);
	fclose(fin);
}


void testSrvRcv2()
{	
// Resources.
	FILE *fin = fopen("testSrvRcv2.txt", "r"); assert(fin);
	csc_httpMsg_t *msg = csc_httpMsg_new();

// Read all in.
	csc_httpErr_t errVal = csc_httpMsg_rcvSrvFILE(msg, fin);
	testReport_iVal(stdout, "http_sRcv2_retVal", csc_httpErr_Ok, errVal);

// Check all the headers.
	testReport_sVal(stdout, "http_sRcv2_proto", "HTTP/1.1",
					csc_httpMsg_getHdr(msg, csc_http_protocol));
	testReport_sVal(stdout, "http_sRcv2_method", "GET",
					csc_httpMsg_getHdr(msg, csc_http_method));
	testReport_sVal(stdout, "http_sRcv2_reqUri", "/",
					csc_httpMsg_getHdr(msg, csc_http_reqUri));
	testReport_sVal(stdout, "http_sRcv2_accept", "*/*",
					csc_httpMsg_getHdr(msg, "Accept"));
	testReport_sVal(stdout, "http_sRcv2_accLang", "en-us",
					csc_httpMsg_getHdr(msg, "Accept-Language"));
	testReport_sVal(stdout, "http_sRcv2_accEnc", "gzip, deflate",
					csc_httpMsg_getHdr(msg, "Accept-Encoding"));
	testReport_sVal(stdout, "http_sRcv2_userAgent", "Mozilla/4.0",
					csc_httpMsg_getHdr(msg, "User-Agent"));
	testReport_sVal(stdout, "http_sRcv2_host", "www.ft.com",
					csc_httpMsg_getHdr(msg, "Host"));
	testReport_sVal(stdout, "http_sRcv2_connection", "Keep-Alive",
					csc_httpMsg_getHdr(msg, "Connection"));
	testReport_sVal(stdout, "http_sRcv2_notThere", NULL,
					csc_httpMsg_getHdr(msg, "notThere"));
	
// Free resources.
	csc_httpMsg_free(msg);
	fclose(fin);
}


int main(int argc, char **argv)
{	testAddGet(stdout);
	testCliRcv1();
	testCliRcv2();
	testSrvRcv1();
	testSrvRcv2();
	if (csc_mck_nchunks() == 0)
		fprintf(stdout, "pass (%s)\n", "http_memory");
	else
		fprintf(stdout, "FAIL (%s)\n", "http_memory");
	csc_mck_print(stdout);
}
