#include "http.h"

#define errStr_Ok  NULL
#define errStr_BadRequest  "Bad Request"
#define errStr_BadFilePath  "Bad File Path"
#define errStr_InvalidDomain  "Invalid Domain"
#define errStr_NoRequestMade  "No Request Made"
#define errStr_NoResponse     "No Response"


typedef struct hdr_t
{	char *name;
	char *value;
} nameVal_t;


typedef struct csc_http_t
{	
// Errors.
	csc_httpErr_t errCode;
	const char *errMsg;
 
// The request.
	char *request;
	char *reqFilePath;
	char *reqHost;
	csc_bool_t reqIsClose;
	csc_list_t *reqHeaders;
	csc_list_t *reqArgs;
 
// Has the request been made yet?
	csc_bool_t isRequested;
 
// The response.
	int respCode;
	char *respMsg;
	csc_bool_t respIsClose;
	csc_list_t *respHeaders;
 
} csc_http_t;


static void addNameVal(csc_list_t **nameVals, const char *name, const char *value)
{	nameVal_t *hdr = csc_allocOne(nameVal_t);
	hdr->name = csc_alloc_str(name);
	hdr->value = csc_alloc_str(value);
	csc_list_add(nameVals, hdr);
}


static void freeNameVals(csc_list_t *nameVals)
{	for (csc_list_t *lpt=nameVals; lpt!=NULL; lpt=lpt->next)
	{	headrr_t *hdr = lpt->data;
		free(hdr->name);
		free(hdr->value);
		free(hdr);
	}
	csc_list_free(nameVals);
}


// Create new HTTP request object.
// Use csc_http_getErrCode() to check for errors, including errors creating the object.
csc_http_t *csc_http_new( const char *request  // For now, only "GET" is valid.
						, const char *filePath // An absolute path.
						, const char *host     // HTTP1.1 requires the host name.
						, csc_bool_t isClose   // Request server close the connection?
						)
{	csc_bool_t isOK = csc_TRUE;
 
// Allocate the structure.
	csc_http_t *http = csc_allocOne(csc_http_t);
	if (http == NULL)
		return NULL;
 
// Initialise everything.
	http->errCode = csc_httpErr_Ok;
	http->errMsg = NULL;
	http->request = NULL;
	http->reqFilePath = NULL;
	http->reqHost = NULL;
	http->reqIsClose = csc_TRUE;;
	http->reqHeaders = NULL;
	http->reqArgs = NULL;
	http->isRequested = csc_FALSE;;
	http->respCode = -1;
	http->respMsg = NULL;
	http->respIsClose = csc_FALSE;
	http->respHeaders = NULL;
 
// Assign the request.
	if (isOstrcmp(request,"GET"))
	{	http->errCode = csc_httpErr_BadRequest;
		http->errMsg = errStr_BadRequest; 
		isOK = csc_FALSE;
	}
	if (isOK)
		http->request = csc_alloc_str(request);
 
// Assign the file path.
	if (isOK)
	{	if (filePath == NULL)
		{	// http->reqFilePath = NULL; // Its already NULL.
		}
		else if (csc_isValid_decentAbsPath(filePath))
		{	http->reqFilePath = csc_alloc_str(filePath);
		}
		else
		{	http->errCode = csc_httpErr_BadFilePath;
			http->errMsg = errStr_BadFilePath; 
			isOK = csc_FALSE;
		}
	}
 
// Assign the host name.
	if (isOK && !csc_isValid_domain(host))
	{	http->errCode = csc_httpErr_InvalidDomain;
		http->errMsg = errStr_InvalidDomain; 
		isOK = csc_FALSE;
	}
	if (isOK)
		http->host = csc_alloc_str(host);
 
// Bye
	return http;
}


// Free a HTTP object.
void csc_http_free(csc_http_t *http)
{	if (http->errMsg)
		free(http->errMsg);
	if (http->request)
		free(http->request);
	if (http->reqFilePath)
		free(http->reqFilePath);
	if (http->reqHost)
		free(http->reqHost);
	if (http->respMsg)
		free(http->respMsg);
	if (http->reqHeaders)
		freeNameVals(http->reqHeaders);
	if (http->reqArgs)
		freeNameVals(http->reqArgs);
	if (http->respHeaders)
		freeNameVals(http->respHeaders);
	free(http);
}
 

// Add headers to the request.
void csc_http_addReqHeader(csc_http_t *http, const char *name, const char *value)
{	addNameVal(&http->reqHeaders, name, value);
}

// Add headers to the request.
void csc_http_addReqArg(csc_http_t *http, const char *name, const char *value)
{	addNameVal(&http->reqHeaders, name, value);
}

// Get error status.
csc_httpErr_t csc_http_getErrCode(csc_http_t *http)
{	return http->errCode;
}

// Get error message.
const char *csc_http_getErrStr(csc_http_t *http)
{	return http->errMsg;
}


// Perform the IO for HTTP.
csc_bool_t csc_http_doReq(csc_http_t *http, FILE *fin, FILE *fout);

// What was the response code?
int csc_http_respCode(csc_http_t *http);

// What was the response message?
const char *csc_http_respMsg(csc_http_t *http);

// Server may or may not honour close or keep alive request.
// Returns FALSE if the response had a "Connection" of "Keep-Alive",
// otherwise returns TRUE.
csc_bool_t csc_http_respIsClose(csc_http_t *http);

// Returns the value of one of the headers with the name 'name' if at least
// one exists, otherwise returns NULL.
const char *csc_http_getRespHdr(csc_http_t *http, const char *name);


