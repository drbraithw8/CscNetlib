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


void testAddSF(FILE *fout, char *testName, csc_http_t *hMsg, csc_httpSF_t fldNdx, const char *hdrVal)
{	csc_httpErr_t errCode = csc_http_addSF(hMsg, fldNdx, hdrVal);
	testReport_iVal(fout, testName, csc_httpErr_Ok, errCode);
}

void testAddHdr(FILE *fout, char *testName, csc_http_t *hMsg, const char *hdrName, const char *hdrVal)
{	csc_httpErr_t errCode = csc_http_addHdr(hMsg, hdrName, hdrVal);
	testReport_iVal(fout, testName, csc_httpErr_Ok, errCode);
}

void testGetHdr(FILE *fout, char *testName, csc_http_t *hMsg, const char *hdrName, const char *hdrVal)
{	const char *val = csc_http_getHdr(hMsg, hdrName);
	testReport_sVal(fout, testName, hdrVal, val);
}

void testGetSF(FILE *fout, char *testName, csc_http_t *hMsg, csc_httpSF_t fldNdx, const char *hdrVal)
{	const char *val = csc_http_getSF(hMsg, fldNdx);
	testReport_sVal(fout, testName, hdrVal, val);
}


void testAddGet(FILE *fout)
{
	csc_httpErr_t errCode;
	csc_http_t *hMsg = NULL;
	char *str = NULL;

// Create the message.
	hMsg = csc_http_new();

// Test the empty ones.
	testGetSF(fout, "http_getNull_proto", hMsg, csc_httpSF_protocol, NULL);
	testGetSF(fout, "http_getNull_reqUri", hMsg, csc_httpSF_reqUri, NULL);
	testGetSF(fout, "http_getNull_method", hMsg, csc_httpSF_method, NULL);
	testGetSF(fout, "http_getNull_statCode", hMsg, csc_httpSF_statCode, NULL);
	testGetSF(fout, "http_getNull_reason", hMsg, csc_httpSF_reason, NULL);
	testGetHdr(fout, "http_getHdr_notEntered", hMsg, "invalid", NULL);

// Add start line fields and headers.
	const char *tstHdrVal_proto = "HTTP/1.1";
	testAddSF(fout, "http_addSF_proto", hMsg, csc_httpSF_protocol, tstHdrVal_proto);

	const char *tstHdrVal_reqUri = "/index.html";
	testAddSF(fout, "http_addSF_reqUri", hMsg, csc_httpSF_reqUri, tstHdrVal_reqUri);

	const char *tstHdrVal_method = "GET";
	testAddSF(fout, "http_addSF_method", hMsg, csc_httpSF_method, tstHdrVal_method);

	const char *tstHdrVal_host = "bology.com.au";
	const char *tstHdrName_host = "Host";
	testAddHdr(fout, "http_addHdr_host", hMsg, tstHdrVal_host, tstHdrVal_host);

	const char *tstHdrVal_conn = "close";
	const char *tstHdrName_conn = "Connection";
	testAddHdr(fout, "http_addHdr_conn", hMsg, tstHdrName_conn, tstHdrVal_conn);

	const char *tstHdrVal_any = "something";
	const char *tstHdrName_any = "Anything";
	testAddHdr(fout, "http_addHdr_any", hMsg, tstHdrName_any, tstHdrVal_any);

// Get real and pseudo headers.
	testGetSF(fout, "http_getSF_proto", hMsg, csc_httpSF_protocol, tstHdrVal_proto);
	testGetSF(fout, "http_getSF_reqUri", hMsg, csc_httpSF_reqUri, tstHdrVal_reqUri);
	testGetSF(fout, "http_getSF_method", hMsg, csc_httpSF_method, tstHdrVal_method);
	testGetHdr(fout, "http_getHdr_host", hMsg, tstHdrVal_host, tstHdrVal_host);
	testGetHdr(fout, "http_getHdr_conn", hMsg, tstHdrName_conn, tstHdrVal_conn);
	testGetHdr(fout, "http_getHdr_any", hMsg, tstHdrName_any, tstHdrVal_any);

// Request one that was not entered.
	testGetHdr(fout, "http_getHdr_notEntered", hMsg, "invalid", NULL);
	testGetSF(fout, "http_getSF_notEntered", hMsg, csc_httpSF_reason, NULL);

// Free resources.
	csc_http_free(hMsg );
}


void testCrst( const char *testName
			 , const char *testStr
			 , const char *proto
			 , const char *statCode
			 , const char *reason
			 )
{
// Resources
	csc_http_t *msg = NULL;
 
// Read from the text.
	msg = csc_http_new();
	csc_httpErr_t errCode = csc_http_rcvCliStr(msg, testStr);
	testReport_iVal(stdout, "http_cRcv1_retVal", csc_httpErr_Ok, errCode);
 
// Check 
	testReport_sVal(stdout, testName, proto,
					csc_http_getSF(msg, csc_httpSF_protocol));
	testReport_sVal(stdout, testName, statCode,
					csc_http_getSF(msg, csc_httpSF_statCode));
	testReport_sVal(stdout, testName, reason,
					csc_http_getSF(msg, csc_httpSF_reason));
 
// Free resources.
	if (msg)
		csc_http_free(msg);
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
	csc_http_t *msg = NULL;
 
// Read from the text.
	msg = csc_http_new();
	csc_httpErr_t errCode = csc_http_rcvSrvStr(msg, testStr);
	testReport_iVal(stdout, "http_sRcv1_retVal", csc_httpErr_Ok, errCode);
	if (errCode != csc_httpErr_Ok)
		fprintf(stdout, "\terrStr=%s\n", csc_http_getErrStr(msg));
 
// Check 
	testReport_sVal(stdout, testName, method,
					csc_http_getSF(msg, csc_httpSF_method));
	testReport_sVal(stdout, testName, reqUri,
					csc_http_getSF(msg, csc_httpSF_reqUri));
	testReport_sVal(stdout, testName, proto,
					csc_http_getSF(msg, csc_httpSF_protocol));
 
// Free resources.
	if (msg)
		csc_http_free(msg);
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
	csc_http_t *msg = csc_http_new();
 
// Read all in.
	csc_httpErr_t errVal = csc_http_rcvCliFILE(msg, fin);
	testReport_iVal(stdout, "http_cRcv2_retVal", csc_httpErr_Ok, errVal);
 
// Check all the headers.
	testReport_sVal(stdout, "http_cRcv2_proto", "HTTP/1.1",
					csc_http_getSF(msg, csc_httpSF_protocol));
	testReport_sVal(stdout, "http_cRcv2_statCode", "200",
					csc_http_getSF(msg, csc_httpSF_statCode));
	testReport_sVal(stdout, "http_cRcv2_reason", "OK",
					csc_http_getSF(msg, csc_httpSF_reason));
	testReport_sVal(stdout, "http_cRcv2_server", "webfs/1.21",
					csc_http_getHdr(msg, "Server"));
	testReport_sVal(stdout, "http_cRcv2_connection", "Close",
					csc_http_getHdr(msg, "Connection"));
	testReport_sVal(stdout, "http_cRcv2_accRanges", "bytes",
					csc_http_getHdr(msg, "Accept-Ranges"));
	testReport_sVal(stdout, "http_cRcv2_contType", "text/html",
					csc_http_getHdr(msg, "Content-Type"));
	testReport_sVal(stdout, "http_cRcv2_contLen", "157",
					csc_http_getHdr(msg, "Content-Length"));
	testReport_sVal( stdout
				   , "http_cRcv2_lastMod"
				   , "Thu, 06 Jul 2017 05:33:59 GMT"
				   , csc_http_getHdr(msg, "Last-Modified")
				   );
	testReport_sVal( stdout
				   , "http_cRcv2_date"
				   , "Thu, 06 Jul 2017 05:56:58 GMT"
				   , csc_http_getHdr(msg, "Date")
				   );
	testReport_sVal(stdout, "http_cRcv2_notThere", NULL,
					csc_http_getHdr(msg, "notThere"));
	
// Free resources.
	csc_http_free(msg);
	fclose(fin);
}


void testSrvRcv2()
{	
// Resources.
	FILE *fin = fopen("testSrvRcv2.txt", "r"); assert(fin);
	csc_http_t *msg = csc_http_new();
 
// Read all in.
	csc_httpErr_t errVal = csc_http_rcvSrvFILE(msg, fin);
	testReport_iVal(stdout, "http_sRcv2_retVal", csc_httpErr_Ok, errVal);
 
// Check all the headers.
	testReport_sVal(stdout, "http_sRcv2_proto", "HTTP/1.1",
					csc_http_getSF(msg, csc_httpSF_protocol));
	testReport_sVal(stdout, "http_sRcv2_method", "GET",
					csc_http_getSF(msg, csc_httpSF_method));
	testReport_sVal(stdout, "http_sRcv2_reqUri", "/",
					csc_http_getSF(msg, csc_httpSF_reqUri));
	testReport_sVal(stdout, "http_sRcv2_accept", "*/*",
					csc_http_getHdr(msg, "Accept"));
	testReport_sVal(stdout, "http_sRcv2_accLang", "en-us",
					csc_http_getHdr(msg, "Accept-Language"));
	testReport_sVal(stdout, "http_sRcv2_accEnc", "gzip, deflate",
					csc_http_getHdr(msg, "Accept-Encoding"));
	testReport_sVal(stdout, "http_sRcv2_userAgent", "Mozilla/4.0",
					csc_http_getHdr(msg, "User-Agent"));
	testReport_sVal(stdout, "http_sRcv2_host", "www.ft.com",
					csc_http_getHdr(msg, "Host"));
	testReport_sVal(stdout, "http_sRcv2_connection", "Keep-Alive",
					csc_http_getHdr(msg, "Connection"));
	testReport_sVal(stdout, "http_sRcv2_notThere", NULL,
					csc_http_getHdr(msg, "notThere"));
	
// Free resources.
	csc_http_free(msg);
	fclose(fin);
}


void testSrvRcv4()
{	const csc_nameVal_t *nv;
// Resources.
	FILE *fin = fopen("testSrvRcv4.txt", "r"); assert(fin);
	csc_http_t *msg = csc_http_new();
 
// Read all in.
	csc_httpErr_t errVal = csc_http_rcvSrvFILE(msg, fin);
	testReport_iVal(stdout, "http_sRcv4_retVal", csc_httpErr_Ok, errVal);
 
// Check all the headers.
	testReport_sVal(stdout, "http_sRcv4_proto", "HTTP/1.1",
					csc_http_getSF(msg, csc_httpSF_protocol));
	testReport_sVal(stdout, "http_sRcv4_method", "GET",
					csc_http_getSF(msg, csc_httpSF_method));
	testReport_sVal(stdout, "http_sRcv4_reqUri", "/hello world.php",
					csc_http_getSF(msg, csc_httpSF_reqUri));
 
// Check URL encoded values.
	nv = csc_http_getUrlVal(msg,"name"); assert(nv);
	testReport_sVal(stdout, "http_sRcv4_reqValName", "Fred Bloggs", nv->val);
 
	nv = csc_http_getUrlVal(msg,"rate"); assert(nv);
	testReport_sVal(stdout, "http_sRcv4_reqValRate", "12%+1", nv->val);
 
// Free resources.
	csc_http_free(msg);
	fclose(fin);
}


void testSrvRcv3()
{	const csc_nameVal_t *nv;
// Resources.
	FILE *fin = fopen("testSrvRcv3.txt", "r"); assert(fin);
	csc_http_t *msg = csc_http_new();
 
// Read all in.
	csc_httpErr_t errVal = csc_http_rcvSrvFILE(msg, fin);
	testReport_iVal(stdout, "http_sRcv3_retVal", csc_httpErr_Ok, errVal);
 
// Check all the headers.
	testReport_sVal(stdout, "http_sRcv3_proto", "HTTP/1.1",
					csc_http_getSF(msg, csc_httpSF_protocol));
	testReport_sVal(stdout, "http_sRcv3_method", "GET",
					csc_http_getSF(msg, csc_httpSF_method));
	testReport_sVal(stdout, "http_sRcv3_reqUri", "/helloWorld.php",
					csc_http_getSF(msg, csc_httpSF_reqUri));
 
// Check URL encoded values.
	nv = csc_http_getUrlVal(msg,"name"); assert(nv);
	testReport_sVal(stdout, "http_sRcv3_reqValName", "fred", nv->val);
 
	nv = csc_http_getUrlVal(msg,"middle"); assert(nv);
	testReport_sVal(stdout, "http_sRcv3_reqEmptyVal", "", nv->val);
 
	nv = csc_http_getUrlVal(msg,"male"); assert(nv);
	testReport_sVal(stdout, "http_sRcv3_reqNoVal", NULL, nv->val);
 
	nv = csc_http_getUrlVal(msg,"weight"); assert(nv);
	testReport_sVal(stdout, "http_sRcv3_reqValWeight", "25", nv->val);
 
	nv = csc_http_getUrlVal(msg,"notThere");
	testReport_sVal(stdout, "http_sRcv3_reqValNotThere", NULL, (const char*)nv);
	
// Free resources.
	csc_http_free(msg);
	fclose(fin);
}


void testEncDec(const char *testName, const char *dec, csc_bool_t isSlashOk)
{	csc_str_t *enc = csc_http_pcentEnc(dec, isSlashOk);
	char *ddec = csc_http_pcentDec(csc_str_charr(enc));
	// printf("enc=\"%s\" ddec=\"%s\"\n", csc_str_charr(enc), ddec);
	testReport_sVal(stdout, testName, dec, ddec);
	free(ddec);
	csc_str_free(enc);
}


void testPcent()
{	testEncDec("http_encDec1", "/cafe/main.php", csc_TRUE);
	testEncDec("http_encDec2", "/ca*fe/ma in.p\nhp", csc_TRUE);
	testEncDec("http_encDec3", "ca%fe/ma in.p\"\'hp", csc_FALSE);
}


int main(int argc, char **argv)
{	testAddGet(stdout);
	testCliRcv1();
	testCliRcv2();
	testSrvRcv1();
	testSrvRcv2();
	testSrvRcv3();
	testSrvRcv4();
	testPcent();
	if (csc_mck_nchunks() == 0)
		fprintf(stdout, "pass (%s)\n", "http_memory");
	else
		fprintf(stdout, "FAIL (%s)\n", "http_memory");
	csc_mck_print(stdout);
}

