#ifndef csc_HTTP_H
#define csc_HTTP_H 1

#include <stdio.h>
#include "ioAny.h"
#include "cstr.h"
#include "std.h"


typedef enum csc_httpStartLineFields_e
{	csc_httpSF_method = 0   // First field of a request line.
,	csc_httpSF_reqUri       // 2nd field of a request line.
,	csc_httpSF_protocol     // 3rd field of a request line AND also the 1st field of a status line.
,	csc_httpSF_statCode     // 2nd field of a status line.
,	csc_httpSF_reason       // 3rd field of a status line.
,	csc_httpSF_numSF
} csc_httpSF_t;


typedef enum csc_httpErr_e
{   csc_httpErr_Ok = 0
,   csc_httpErr_BadSF
,   csc_httpErr_AlreadySF
,   csc_httpErr_AlreadyUrlArg
,   csc_httpErr_MissingProtocol
,   csc_httpErr_BadProtocol
,   csc_httpErr_MissingReqUri
,   csc_httpErr_BadReqUri
,   csc_httpErr_MissingMethod
,   csc_httpErr_BadMethod
,   csc_httpErr_MissingStatCode
,   csc_httpErr_BadStatCode
,   csc_httpErr_MissingReason
,   csc_httpErr_UnexpectedEOF
,   csc_httpErr_BadStartLine
,   csc_httpErr_LineTooLong
,   csc_httpErr_syntaxErr
} csc_httpErr_t;


typedef struct csc_http_t csc_http_t;


// Create a new empty HTTP message.
csc_http_t *csc_http_new();

// Release resources associated with a HTTP message.
void csc_http_free(csc_http_t *msg);

// Add a start line field.  See def of csc_httpStartLineFields_e, above.
csc_httpErr_t csc_http_addSF(csc_http_t *msg, csc_httpSF_t field, const char *hdrValue);

// Get a start line field.
const char *csc_http_getSF(csc_http_t *msg, csc_httpSF_t fldNdx);


// Add a header to a HTTP message.  HTTP permits a header to be added more
// than once, although it is not recommended.  
csc_httpErr_t csc_http_addHdr(csc_http_t *msg, const char *hdrName, const char *hdrValue);


// Gets the value of a HTTP header (in the case that there is more than one
// such header, the first one will be returned.  Returns fields of the
// status line or request line.  Returns "" if the message is empty.
// Returns NULL if there is no such header.
const char *csc_http_getHdr(csc_http_t *msg, const char *hdrName);


// Add a name/value pair for URL encoding into the requestUrl part of a
// HTTP request line.  If 'val' is NULL, there will be no "=value" part.
// If 'val' is "", then there will be an equals sign, but the value will be
// empty.
csc_httpErr_t csc_http_addUrlVal(csc_http_t *msg, const char *name, const char *val);


// Gets the value associated with a name, 'name' from URL encoded
// requestUrl part of a HTTP request line.
// 
// If the name, 'name' is not present then this function will return NULL,
// and *'whatsThere' will be set to 1 if 'whatsThere' is not null.
// 
// If the name, 'name' is present, and the value is NULL then this function
// will return NULL, and *'whatsThere' will be set to 2 if 'whatsThere' is
// not null.
// 
// If the name, 'name' is present, and the value is the empty string ""
// then this function will return the empty string "", and *'whatsThere'
// will be set to 3 if 'whatsThere' is not null.
// 
// If the name, 'name' is present, and has a non null non empty value then
// this function will return the value and *'whatsThere' will be set to 4
// if 'whatsThere' is not null.
const char *csc_http_getUrlVal(csc_http_t *msg, const char *name, int *whatsThere);


// Receive a HTTP message from whatever as a client.
csc_httpErr_t csc_http_rcvCliFILE(csc_http_t *msg, FILE *fin);
csc_httpErr_t csc_http_rcvCliStr(csc_http_t *msg, const char *str);
csc_httpErr_t csc_http_rcvCli(csc_http_t *msg, csc_ioAnyRead_t *rca);


// Receive a HTTP message by receiving HTTP from whatever as a server.
csc_httpErr_t csc_http_rcvSrvFILE(csc_http_t *msg, FILE *fin);
csc_httpErr_t csc_http_rcvSrvStr(csc_http_t *msg, const char *str);
csc_httpErr_t csc_http_rcvSrv(csc_http_t *msg, csc_ioAnyRead_t *rca);


// Sends a HTTP message, as a client, to whatever.  The 3 request line
// parameters must have already been set.  The server will requires some
// headers also, e.g. "Host".  Returns error code.  csc_httpErr_Ok
// indicates success.
csc_httpErr_t csc_http_sendCliFILE(csc_http_t *msg, FILE* fout);
csc_httpErr_t csc_http_sendCliStr(csc_http_t *msg, csc_str_t *sout);
csc_httpErr_t csc_http_sendCli(csc_http_t *msg, csc_ioAnyWrite_t *ws);


// Sends a HTTP message, as a server, to whatever.  The 3 response line
// parameters must have already been set.  Returns error code.
// csc_httpErr_Ok indicates success.
csc_httpErr_t csc_http_sendSrvFILE(csc_http_t *msg, FILE *fout);
csc_httpErr_t csc_http_sendSrvStr(csc_http_t *msg, csc_str_t *sout);
csc_httpErr_t csc_http_sendSrv(csc_http_t *msg, csc_ioAnyWrite_t *out);


// Returns a string that describes the error.  Returns NULL if there is no error.
const char *csc_http_getErrStr(csc_http_t *msg);
csc_httpErr_t csc_http_getErrCode(csc_http_t *msg);

// Removes percent encoding from a string.
// Returns allocated string that must be free()d by the caller.
char *csc_http_pcentDec(const char *enc);

// Assigns percent encoded version of 'dec' to 'enc'.
// Encodes slashes and colons only if 'isSlashOk'.
void csc_http_pcentEnc(const char *dec, csc_str_t *enc, csc_bool_t isSlashOk);



// Status codes and reasons.
typedef enum
{       csc_http_statCode_100 = 100
#define csc_http_reason_100 "Continue"
 
,       csc_http_statCode_101 = 101
#define csc_http_reason_101 "Switching Protocols"
 
,       csc_http_statCode_200 = 200
#define csc_http_reason_200 "OK"
 
,       csc_http_statCode_201 = 201
#define csc_http_reason_201 "Created"
 
,       csc_http_statCode_202 = 202
#define csc_http_reason_202 "Accepted"
 
,       csc_http_statCode_203 = 203
#define csc_http_reason_203 "Non-Authoritative Information"
 
,       csc_http_statCode_204 = 204
#define csc_http_reason_204 "No Content"
 
,       csc_http_statCode_205 = 205
#define csc_http_reason_205 "Reset Content"
 
,       csc_http_statCode_206 = 206
#define csc_http_reason_206 "Partial Content"
 
,       csc_http_statCode_300 = 300
#define csc_http_reason_300 "Multiple Choices"
 
,       csc_http_statCode_301 = 301
#define csc_http_reason_301 "Moved Permanently"
 
,       csc_http_statCode_302 = 302
#define csc_http_reason_302 "Found"
 
,       csc_http_statCode_303 = 303
#define csc_http_reason_303 "See Other"
 
,       csc_http_statCode_304 = 304
#define csc_http_reason_304 "Not Modified"
 
,       csc_http_statCode_305 = 305
#define csc_http_reason_305 "Use Proxy"
 
,       csc_http_statCode_306 = 306
#define csc_http_reason_306 "(Unused)"
 
,       csc_http_statCode_307 = 307
#define csc_http_reason_307 "Temporary Redirect"
 
,       csc_http_statCode_400 = 400
#define csc_http_reason_400 "Bad Request"
 
,       csc_http_statCode_401 = 401
#define csc_http_reason_401 "Unauthorized"
 
,       csc_http_statCode_402 = 402
#define csc_http_reason_402 "Payment Required"
 
,       csc_http_statCode_403 = 403
#define csc_http_reason_403 "Forbidden"
 
,       csc_http_statCode_404 = 404
#define csc_http_reason_404 "Not Found"
 
,       csc_http_statCode_405 = 405
#define csc_http_reason_405 "Method Not Allowed"
 
,       csc_http_statCode_406 = 406
#define csc_http_reason_406 "Not Acceptable"
 
,       csc_http_statCode_407 = 407
#define csc_http_reason_407 "Proxy Authentication Required"
 
,       csc_http_statCode_408 = 408
#define csc_http_reason_408 "Request Timeout"
 
,       csc_http_statCode_409 = 409
#define csc_http_reason_409 "Conflict"
 
,       csc_http_statCode_410 = 410
#define csc_http_reason_410 "Gone"
 
,       csc_http_statCode_411 = 411
#define csc_http_reason_411 "Length Required"
 
,       csc_http_statCode_412 = 412
#define csc_http_reason_412 "Precondition Failed"
 
,       csc_http_statCode_413 = 413
#define csc_http_reason_413 "Request Entity Too Large"
 
,       csc_http_statCode_414 = 414
#define csc_http_reason_414 "Request-URI Too Long"
 
,       csc_http_statCode_415 = 415
#define csc_http_reason_415 "Unsupported Media Type"
 
,       csc_http_statCode_416 = 416
#define csc_http_reason_416 "Requested Range Not Satisfiable"
 
,       csc_http_statCode_417 = 417
#define csc_http_reason_417 "Expectation Failed"
 
,       csc_http_statCode_500 = 500
#define csc_http_reason_500 "Internal Server Error"
 
,       csc_http_statCode_501 = 501
#define csc_http_reason_501 "Not Implemented"
 
,       csc_http_statCode_502 = 502
#define csc_http_reason_502 "Bad Gateway"
 
,       csc_http_statCode_503 = 503
#define csc_http_reason_503 "Service Unavailable"
 
,       csc_http_statCode_504 = 504
#define csc_http_reason_504 "Gateway Timeout"
 
,       csc_http_statCode_505 = 505
#define csc_http_reason_505 "HTTP Version Not Supported"
} csc_http_statCode_t;

#endif
