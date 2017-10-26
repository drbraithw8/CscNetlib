#ifndef csc_HTTP_H
#define csc_HTTP_H 1

#include <stdio.h>
#include "std.h"

typedef enum csc_httpErr_e
{   csc_httpErr_Ok = 0
,   csc_httpErr_MissingProtocol
,   csc_httpErr_AlreadyProtocol
,   csc_httpErr_MissingReqUri
,   csc_httpErr_AlreadyReqUri
,   csc_httpErr_MissingMethod
,   csc_httpErr_AlreadyMethod
,   csc_httpErr_MissingStatCode
,   csc_httpErr_AlreadyStatCode
,   csc_httpErr_MissingReason
,   csc_httpErr_AlreadyReason
} csc_httpErr_t;


// Psuedo headers.  These represent the fields in the start line of a HTTP message.
const char *csc_http_protocol = "csc_http_protocol"; // protocol, e.g. "HTTP/1.1".
const char *csc_http_reqUri = "csc_http_reqUri"; // absolute path naming resource, e.g. "/index.html".
const char *csc_http_method = "csc_http_method"; // Method, e.g. "GET".
const char *csc_http_statCode = "csc_http_statCode"; // status code, e.g. "200".
const char *csc_http_reason = "csc_http_reason"; // phrase associated with status code, e.g. "OK".

typedef struct csc_httpMsg_t csc_httpMsg_t;


// Create a new empty HTTP message.
csc_httpMsg_t *csc_httpMsg_new();


// Release resources associated with a HTTP message.
void csc_httpMsg_free(csc_httpMsg_t *msg);


// Receive a HTTP message from an input stream as a client.
csc_httpErr_t csc_httpMsg_rcvCli(csc_httpMsg_t *msg, FILE *fin);


// Receive a HTTP message by receiving HTTP from an input stream as a server.
csc_httpErr_t csc_httpMsg_rcvSrv(csc_httpMsg_t *msg, FILE *fin);


// Add a header to a HTTP message.  HTTP permits a header to be added more
// than once, although it is not recommended.  Pseudo headers will need
// to be set, and these can definitely only be set once.
csc_httpErr_t csc_httpMsg_addHdr(csc_httpMsg_t *msg, const char *hdrName, const char *hdrValue);


// Gets the value of a HTTP header (in the case that there is more than one
// such header, the first one will be returned.
// Returns fields of the status line or request line as pseudo headers.
// Returns "" if the message is empty.  Returns NULL if there is no such
// header.
const char *csc_httpMsg_getHdr(csc_httpMsg_t *msg, const char *hdrName);


// Sends a HTTP message, as a client, to the output stream 'fout'.  The
// following pseudo headers, that correspond to the request line are
// required to have ALREADY been set:-
// "csc_http_method", "csc_http_reqUri" and "csc_http_protocol".
// Other headers may be required even if this class does not mind, e.g. "Host".  
// Returns error code.  csc_httpErr_Ok indicates success.
csc_httpErr_t csc_httpMsg_SendCli(csc_httpMsg_t *msg, FILE *fout);


// Sends a HTTP message, as a server, to the output stream 'fout'.  The
// following pseudo headers, that correspond to the request line are
// required to have ALREADY been set by csc_httpMsg_addHdr():-
// "csc_http_statCode", "csc_http_reason" and "csc_http_protocol".
// Returns error code.  csc_httpErr_Ok indicates success.
csc_httpErr_t csc_httpMsg_SendSrv(csc_httpMsg_t *msg, FILE *fout);


// Returns a string that describes the error.  Returns NULL if there is no error.
const char *csc_httpMsg_getErrMsg(csc_httpMsg_t *msg);
csc_httpErr_t csc_httpMsg_getErrCode(csc_httpMsg_t *msg);


#endif
