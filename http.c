#include "http.h"
#include "list.h"
#include "alloc.h"


typedef struct hdr_t
{	char *name;
	char *value;
} nameVal_t;


typedef struct csc_httpMsg_t
{	
// Errors.
	csc_httpErr_t errCode;
	char *errMsg;
 
// Pseudo headers.
	char *protocol;
	char *reqUri;
	char *method;
	char *statCode;
	char *reason;
	char *host;
 
// Http Headers.
	csc_list_t *headers;
 
} csc_httpMsg_t;


// Psuedo headers.  These represent the fields in the start line of a HTTP message.
const char *csc_http_protocol = "csc_http_protocol"; // protocol, e.g. "HTTP/1.1".
const char *csc_http_reqUri = "csc_http_reqUri"; // absolute path naming resource, e.g. "/index.html".
const char *csc_http_method = "csc_http_method"; // Method, e.g. "GET".
const char *csc_http_statCode = "csc_http_statCode"; // status code, e.g. "200".
const char *csc_http_reason = "csc_http_reason"; // phrase associated with status code, e.g. "OK".
const char *csc_http_host = "csc_http_host"; // phrase associated with status code, e.g. "OK".


csc_httpMsg_t *csc_httpMsg_new()
{	
// Allocate the structure.
	csc_httpMsg_t *msg = csc_allocOne(csc_httpMsg_t);
	
// Errors.
	msg->errCode = csc_httpErr_Ok;
	msg->errMsg = NULL;
 
// Pseudo headers.
	msg->protocol = NULL;
	msg->reqUri = NULL;
	msg->method = NULL;
	msg->statCode = NULL;
	msg->reason = NULL;
	msg->host = NULL;
 
// Http Headers.
	msg->headers = NULL;
 
// Home with the bacon.
	return msg;
}


// Returns the value associated with 'name'.
// Returns NULL if the head is not present in the list.
static const char *getVal(csc_list_t *nameVals, const char *name)
{	const char *value = NULL;
	for (csc_list_t *lpt=nameVals; lpt!=NULL; lpt=lpt->next)
	{	nameVal_t *namVal = lpt->data;
		if (csc_streq(namVal->name, name))
		{	value = namVal->value;
			break;
		}
	}
	return value;
}
 

// Adds a name/value pair to the nameVals list.
static void addNameVal(csc_list_t **nameVals, const char *name, const char *value)
{	nameVal_t *hdr = csc_allocOne(nameVal_t);
	hdr->name = csc_alloc_str(name);
	hdr->value = csc_alloc_str(value);
	csc_list_add(nameVals, hdr);
}


// Frees the list of name/value pairs.
static void freeNameVals(csc_list_t *nameVals)
{	for (csc_list_t *lpt=nameVals; lpt!=NULL; lpt=lpt->next)
	{	nameVal_t *hdr = lpt->data;
		free(hdr->name);
		free(hdr->value);
		free(hdr);
	}
	csc_list_free(nameVals);
}


void csc_httpMsg_free(csc_httpMsg_t *msg)
{	
// Errors.
	if (msg->errMsg)
		free(msg->errMsg);
	
// Pseudo headers.
	if (msg->protocol)
		free(msg->protocol);
	if (msg->reqUri)
		free(msg->reqUri);
	if (msg->method)
		free(msg->method);
	if (msg->statCode)
		free(msg->statCode);
	if (msg->reason)
		free(msg->reason);
	if (msg->host)
		free(msg->host);
 
// Http Headers.
	if (msg->headers)
		freeNameVals(msg->headers);
 
// Free the structure.
	free(msg);
}


static void setErr(csc_httpMsg_t *msg, csc_httpErr_t errCode, const char *errMsg)
{	if (msg->errMsg)
		free(msg->errMsg);
	msg->errCode = errCode;
	msg->errMsg = csc_alloc_str(errMsg);
}


static csc_httpErr_t addPseudo( csc_httpMsg_t *msg
							  , char **pseudo
							  , const char *value
							  , const char *errMsg
							  , csc_httpErr_t errCode
							  )
{	if (*pseudo != NULL)
	{	setErr(msg, errCode, errMsg);
		return errCode;
	}
	else
	{	*pseudo = csc_alloc_str(value);
		return csc_httpErr_Ok;
	}
}


csc_httpErr_t csc_httpMsg_addHdr(csc_httpMsg_t *msg, const char *name, const char *value)
{
	if (csc_streq(name, csc_http_protocol))
	{	return addPseudo( msg
						  , &msg->protocol
						  , value
						  , "protocol already set"
						  , csc_httpErr_AlreadyProtocol
						  );
	}
	else if (csc_streq(name, csc_http_reqUri))
	{	return addPseudo( msg
						  , &msg->reqUri
						  , value
						  , "reqUri already set"
						  , csc_httpErr_AlreadyReqUri
						  );
	}
	else if (csc_streq(name, csc_http_method))
	{	return addPseudo( msg
						  , &msg->method
						  , value
						  , "method already set"
						  , csc_httpErr_AlreadyMethod
						  );
	}
	else if (csc_streq(name, csc_http_statCode))
	{	return addPseudo( msg
						  , &msg->statCode
						  , value
						  , "statCode already set"
						  , csc_httpErr_AlreadyStatCode
						  );
	}
	else if (csc_streq(name, csc_http_reason))
	{	return addPseudo( msg
						  , &msg->reason
						  , value
						  , "reason already set"
						  , csc_httpErr_AlreadyReason
						  );
	}
	else if (csc_streq(name, csc_http_host))
	{	return addPseudo( msg
						  , &msg->host
						  , value
						  , "host already set"
						  , csc_httpErr_AlreadyHost
						  );
	}
	addNameVal(&msg->headers, name, value);
	return csc_httpErr_Ok;
}


const char *csc_httpMsg_getHdr(csc_httpMsg_t *msg, const char *name)
{	if (csc_streq(name, csc_http_protocol))
		return msg->protocol;
	else if (csc_streq(name, csc_http_reqUri))
		return msg->reqUri;
	else if (csc_streq(name, csc_http_method))
		return msg->method;
	else if (csc_streq(name, csc_http_statCode))
		return msg->statCode;
	else if (csc_streq(name, csc_http_reason))
		return msg->reason;
	else if (csc_streq(name, csc_http_host))
		return msg->host;
	return getVal(msg->headers, name);
}


csc_httpErr_t csc_httpMsg_getErrCode(csc_httpMsg_t *msg)
{	return msg->errCode;
}


const char *csc_httpMsg_getErrMsg(csc_httpMsg_t *msg)
{	return msg->errMsg;
}



