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


void testCliRcv1()
{	const csc_nameVal_t *nv;
	const char *tempFname = "csc_temp_CliRcv1.txt";
	csc_httpErr_t errCode;
	csc_http_t *msg;
	FILE *fout, *fin;
 
	const char *testRcv1 =
		"HTTP/1.1 404 Not Found\n"
		"\n";
 
// Read from string to message.
	msg = csc_http_new();
	errCode = csc_http_rcvCliStr(msg, testRcv1);
	testReport_iVal(stdout, "http_cRcv1_retVal1", csc_httpErr_Ok, errCode);
 
// Write the message out to a temporary file.
	fout = fopen(tempFname, "w"); assert(fout);
	errCode = csc_http_sendSrvFILE(msg, fout);
	testReport_iVal(stdout, "http_cRcv1_retVal2", csc_httpErr_Ok, errCode);
	fclose(fout);
	csc_http_free(msg);
 
// Read all in.
	msg = csc_http_new();
	fin = fopen(tempFname, "r"); assert(fin);
	errCode = csc_http_rcvCliFILE(msg, fin);
	fclose(fin);
	testReport_iVal(stdout, "http_cRcv1_retVal3", csc_httpErr_Ok, errCode);
 
// Check all the headers.
	testReport_sVal(stdout, "http_cRcv1_proto", "HTTP/1.1",
					csc_http_getSF(msg, csc_httpSF_protocol));
	testReport_sVal(stdout, "http_cRcv1_statCode", "404",
					csc_http_getSF(msg, csc_httpSF_statCode));
	testReport_sVal(stdout, "http_cRcv1_reason", "Not Found",
					csc_http_getSF(msg, csc_httpSF_reason));
	testReport_sVal(stdout, "http_cRcv1_notThere", NULL,
					csc_http_getHdr(msg, "notThere"));
	
// Free resources.
	csc_http_free(msg);
}


void testCliRcv2()
{	const csc_nameVal_t *nv;
	const char *tempFname = "csc_temp_CliRcv2.txt";
	csc_httpErr_t errCode;
	csc_http_t *msg;
	FILE *fout, *fin;
 
	const char *testRcv2 =
		"HTTP/1.1 200 OK\n"
		"Server: webfs/1.21\n"
		"Connection: Close \n"
		"Accept-Ranges: bytes\n"
		"Content-Type: text/html   \n"
		"Content-Length: 157\n"
		"Last-Modified: Thu, 06 Jul 2017 05:33:59 GMT\n"
		"Date: Thu, 06 Jul 2017 05:56:58 GMT\n"
		"\n";
 
// Read from string to message.
	msg = csc_http_new();
	errCode = csc_http_rcvCliStr(msg, testRcv2);
	testReport_iVal(stdout, "http_cRcv2_retVal1", csc_httpErr_Ok, errCode);
 
// Write the message out to a temporary file.
	fout = fopen(tempFname, "w"); assert(fout);
	errCode = csc_http_sendSrvFILE(msg, fout);
	testReport_iVal(stdout, "http_cRcv2_retVal2", csc_httpErr_Ok, errCode);
	fclose(fout);
	csc_http_free(msg);
 
// Read all in.
	msg = csc_http_new();
	fin = fopen(tempFname, "r"); assert(fin);
	errCode = csc_http_rcvCliFILE(msg, fin);
	fclose(fin);
	testReport_iVal(stdout, "http_cRcv2_retVal", csc_httpErr_Ok, errCode);
 
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
}


void testSrvRcv4()
{	const csc_nameVal_t *nv;
	const char *tempFname = "csc_temp_SrvRcv4.txt";
	csc_httpErr_t errCode;
	csc_http_t *msg;
	FILE *fout, *fin;
 
	const char *testRcv4 =
		"GET /hello%20world.php?name=Fred%20Bloggs&rate=12%25%2B1 HTTP/1.1\n"
		"Accept: */*\n"
		"\n" ;
 
// Read from string to message.
	msg = csc_http_new();
	errCode = csc_http_rcvSrvStr(msg, testRcv4);
	testReport_iVal(stdout, "http_sRcv4_retVal1", csc_httpErr_Ok, errCode);
 
// Write the message out to a temporary file.
	fout = fopen(tempFname, "w"); assert(fout);
	errCode = csc_http_sendCliFILE(msg, fout);
	testReport_iVal(stdout, "http_sRcv4_retVal2", csc_httpErr_Ok, errCode);
	fclose(fout);
	csc_http_free(msg);
 
// Read all in.
	msg = csc_http_new();
	fin = fopen(tempFname, "r"); assert(fin);
	errCode = csc_http_rcvSrvFILE(msg, fin);
	fclose(fin);
	testReport_iVal(stdout, "http_sRcv4_retVal3", csc_httpErr_Ok, errCode);
 
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
	testReport_sVal(stdout, "http_sRcv4_reqValWeight", "12%+1", nv->val);
 
	nv = csc_http_getUrlVal(msg,"notThere");
	testReport_sVal(stdout, "http_sRcv4_reqValNotThere", NULL, (const char*)nv);
 
// Check the headers.
	testReport_sVal(stdout, "http_sRcv4_accept", "*/*",
					csc_http_getHdr(msg, "Accept"));
	testReport_sVal(stdout, "http_sRcv4_notThere", NULL,
					csc_http_getHdr(msg, "notThere"));
	
// Free resources.
	csc_http_free(msg);
}


void testSrvRcv3()
{	const csc_nameVal_t *nv;
	const char *tempFname = "csc_temp_SrvRcv3.txt";
	csc_httpErr_t errCode;
	csc_http_t *msg;
	FILE *fout, *fin;
 
	const char *testRcv3 =
		"GET /helloWorld.php?name=fred&male&middle=&weight=25 HTTP/1.1\n"
		"Accept: */*\n"
		"Accept-Language: en-us\n"
		"Accept-Encoding: gzip, deflate\n"
		"User-Agent: Mozilla/4.0\n"
		"Host: www.ft.com\n"
		"Connection: Keep-Alive\n"
		"\n" ;
 
// Read from string to message.
	msg = csc_http_new();
	errCode = csc_http_rcvSrvStr(msg, testRcv3);
	testReport_iVal(stdout, "http_sRcv3_retVal1", csc_httpErr_Ok, errCode);
 
// Write the message out to a temporary file.
	fout = fopen(tempFname, "w"); assert(fout);
	errCode = csc_http_sendCliFILE(msg, fout);
	testReport_iVal(stdout, "http_sRcv3_retVal2", csc_httpErr_Ok, errCode);
	fclose(fout);
	csc_http_free(msg);
 
// Read all in.
	msg = csc_http_new();
	fin = fopen(tempFname, "r"); assert(fin);
	errCode = csc_http_rcvSrvFILE(msg, fin);
	fclose(fin);
	testReport_iVal(stdout, "http_sRcv3_retVal3", csc_httpErr_Ok, errCode);
 
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
 
// Check the headers.
	testReport_sVal(stdout, "http_sRcv3_accept", "*/*",
					csc_http_getHdr(msg, "Accept"));
	testReport_sVal(stdout, "http_sRcv3_accLang", "en-us",
					csc_http_getHdr(msg, "Accept-Language"));
	testReport_sVal(stdout, "http_sRcv3_accEnc", "gzip, deflate",
					csc_http_getHdr(msg, "Accept-Encoding"));
	testReport_sVal(stdout, "http_sRcv3_userAgent", "Mozilla/4.0",
					csc_http_getHdr(msg, "User-Agent"));
	testReport_sVal(stdout, "http_sRcv3_host", "www.ft.com",
					csc_http_getHdr(msg, "Host"));
	testReport_sVal(stdout, "http_sRcv3_connection", "Keep-Alive",
					csc_http_getHdr(msg, "Connection"));
	testReport_sVal(stdout, "http_sRcv3_notThere", NULL,
					csc_http_getHdr(msg, "notThere"));
	
// Free resources.
	csc_http_free(msg);
}


void testEncDec(const char *testName, const char *dec, csc_bool_t isSlashOk)
{	csc_str_t *enc = csc_str_new(NULL);
	csc_http_pcentEnc(dec, enc, isSlashOk);
	char *ddec = csc_http_pcentDec(csc_str_charr(enc));
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
	testPcent();
	testCliRcv1();
	testCliRcv2();
	testSrvRcv3();
	testSrvRcv4();
	if (csc_mck_nchunks() == 0)
		fprintf(stdout, "pass (%s)\n", "http_memory");
	else
		fprintf(stdout, "FAIL (%s)\n", "http_memory");
	csc_mck_print(stdout);
}

