#ifndef csc_HTTP_H
#define csc_HTTP_H 1

#include <stdio.h>
#include "ioAny.h"
#include "cstr.h"
#include "hash.h"
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


// Gets the value of a name/value pair from URL encoded requestUrl part of a HTTP
// request line.  Returns NULL if there is no entry for that name.  The
// value element will be NULL if there is no value associated with the
// entry.  The value element will be "" if the value part is empty.
const csc_nameVal_t *csc_http_getUrlVal(csc_http_t *msg, const char *name);


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

#endif
