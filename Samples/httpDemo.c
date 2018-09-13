#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <CscNetLib/http.h>



int main(int argc, char **argv)
{   csc_httpErr_t errCode;

// Resources.
	csc_http_t *sendMsg = NULL;
	csc_http_t *rcvMsg = NULL;

// Create HTTP message.
	sendMsg = csc_http_new();			assert(sendMsg!=NULL);
	errCode = csc_http_addSF(sendMsg, csc_httpSF_method, "GET");           assert(errCode==csc_httpErr_Ok);
	errCode = csc_http_addSF(sendMsg, csc_httpSF_reqUri, "/index.html");   assert(errCode==csc_httpErr_Ok);
	errCode = csc_http_addSF(sendMsg, csc_httpSF_protocol, "HTTP/1.1");    assert(errCode==csc_httpErr_Ok);
	errCode = csc_http_addHdr(sendMsg, "Host", "example.com");             assert(errCode==csc_httpErr_Ok);
	errCode = csc_http_addHdr(sendMsg, "Connection", "close");             assert(errCode==csc_httpErr_Ok);

// Add GET arguments.
	errCode = csc_http_addUrlVal(sendMsg, "name", "Fred Blogs");           assert(errCode==csc_httpErr_Ok);
	errCode = csc_http_addUrlVal(sendMsg, "age", "23");                    assert(errCode==csc_httpErr_Ok);

// Send HTTP message.
	printf("%s", "------------- Begin send message ----------------\n");
	errCode = csc_http_sendCliFILE(sendMsg, stdout);  assert(errCode==csc_httpErr_Ok);
	printf("%s", "------------- End send message ----------------\n");

// Create and receive the HTTP response.
	rcvMsg = csc_http_new();							  assert(rcvMsg!=NULL);
    const char *responseStr = 
		"HTTP/1.1 200 OK\n"
		"Accept-Ranges: bytes\n"
		"Content-Type: text/html\n"
		"Date: Fri, 01 Jun 2018 03:36:55 GMT\n"
		"Last-Modified: Fri, 01 Jun 2018 03:29:13 GMT\n"
		"Server: ECS (oxr/83C0)\n"
		"Content-Length: 94\n"
		"Connection: close\n"
		"\n" ;
	errCode = csc_http_rcvCliStr(rcvMsg, responseStr);                     assert(errCode==csc_httpErr_Ok);
	printf("Protocol: %s\n", csc_http_getSF(rcvMsg, csc_httpSF_protocol));
	const char *statCode = csc_http_getSF(rcvMsg, csc_httpSF_statCode);
	printf("StatCode: %s\n", statCode);
	if (csc_streq(statCode,"200"))
		printf("Content-Length: %s\n", csc_http_getHdr(rcvMsg, "Content-Length"));

// Free resources.
	if (sendMsg)
		csc_http_free(sendMsg);
	if (rcvMsg)
		csc_http_free(rcvMsg);

// Bye.
    exit(0);
}

