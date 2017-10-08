#ifndef csc_CSTR_H
#define csc_CSTR_H 1

#include <stdio.h>
#include "std.h"

typedef enum csc_httpErr_e
{   csc_httpErr_Ok = 0
,   csc_httpErr_BadRequest
,   csc_httpErr_BadFilePath
,   csc_httpErr_InvalidDomain
,   csc_httpErr_NoRequest
,   csc_httpErr_NoResponse
} csc_httpErr_t;



typedef struct csc_http_t csc_http_t;

// Create new HTTP request object.
// Server may or may not honour close or keep alive request.
// You need to consult response from server.
csc_http_t *csc_http_newReq(  const char *request  // For now, only "GET" is valid.
							, const char *filePath // An absolute path.
							, const char *host     // HTTP1.1 requires the host name.
							, csc_bool_t isClose   // Request server close the connection?
							);

// Add headers to the request.
void csc_http_addReqHeader(csc_http_t *http, const char *name, const char *value);

// Perform the IO for HTTP.
csc_bool_t csc_http_doReq(csc_http_t *http, FILE *fin, FILE *fout);

// What was the response code?
int csc_http_respCode(csc_http_t *http);

// What was the response message?
const char *csc_http_respMsg(csc_http_t *http);

// Returns FALSE if the response had a "Connection" of "Keep-Alive",
// otherwise returns TRUE.
csc_bool_t csc_http_respIsClose(csc_http_t *http);

// Returns the value of one of the headers with the name 'name' if at least
// one exists, otherwise returns NULL.
const char *csc_http_getRespHdr(csc_http_t *http, const char *name);

// Free a HTTP object.
void csc_http_free(csc_http_t *http);

#endif
