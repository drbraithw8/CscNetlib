#include "http.h"
#include "hash.h"
#include "alloc.h"
#include "isvalid.h"


typedef struct hdr_t
{	char *name;
	char *value;
} nameVal_t;


typedef struct csc_httpMsg_t
{	
// Errors.
	csc_httpErr_t errCode;
	char *errMsg;
 
// Headers.
	csc_mapSS_t *headers;
	char *reqUri;
 
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
 
// Headers.
	msg->reqUri = NULL;
	msg->headers = csc_mapSS_new();
 
// Home with the bacon.
	return msg;
}


void csc_httpMsg_free(csc_httpMsg_t *msg)
{	
// Errors.
	if (msg->errMsg)
		free(msg->errMsg);
	
// Pseudo headers.
	if (msg->reqUri)
		free(msg->reqUri);
 
// Http Headers.
	if (msg->headers)
		csc_mapSS_free(msg->headers);
 
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
// printf("addHdr(name=\"%s\" val=\"%s\")\n", name, value);
	if (csc_streq(name, csc_http_reqUri))
	{	return addPseudo( msg
						  , &msg->reqUri
						  , value
						  , "reqUri already set"
						  , csc_httpErr_AlreadyReqUri
						  );
	}
	csc_mapSS_addex(msg->headers, name, value);
	return csc_httpErr_Ok;
}


const char *csc_httpMsg_getHdr(csc_httpMsg_t *msg, const char *name)
{	if (csc_streq(name, csc_http_reqUri))
		return msg->reqUri;
	return csc_mapSS_get(msg->headers, name);
}


csc_httpErr_t csc_httpMsg_getErrCode(csc_httpMsg_t *msg)
{	return msg->errCode;
}


const char *csc_httpMsg_getErrMsg(csc_httpMsg_t *msg)
{	return msg->errMsg;
}


void skipTillBlankLine(csc_ioAnyRead_t *rca)
{	int prevCh = ' ';
	int ch = '\n';
	while (ch!=EOF && (ch!='\n' || prevCh!='\n'))
	{	prevCh = ch;
		ch = csc_ioAnyRead_getc(rca);
		if (ch == '\r')
			ch = csc_ioAnyRead_getc(rca);
	}
}


// ------------------------------------------------
// ---------- Class for input of HTTP -------------
// ------------------------------------------------

typedef enum
{	httpIn_OK = 0,
	httpIn_done,
	httpIn_EOF,
	httpIn_lineTooLong,
	httpIn_error
} httpIn_result_t;


typedef struct httpIn_t
{
// Input stream.
	csc_ioAnyRead_t *rca;
 
// Characters stored reading in.
	int nextCh;
	int lastCh;
	csc_bool_t isNextCh;
} httpIn_t;
 
const int httpIn_bufLen = 255;

httpIn_t *httpIn_new(csc_ioAnyRead_t *rca)
{	httpIn_t *hin = csc_allocOne(httpIn_t);
	hin->rca = rca;
	hin->nextCh = 0;
	hin->lastCh = 0;
	hin->isNextCh = csc_FALSE;
	return hin;
}

void httpIn_free(httpIn_t *hin)
{	free(hin);
}


int httpIn_getc(httpIn_t *hin)
// Intended for http.
// *	Replaces \r\n[\s|\t] by \s.
// *	Replaces \n[\s|\t] by \s.
// *	Replaces \r\n by \n.
// *	Does not ask for the next char if it got
//		the end, cos that would block.
{	int ch = 0;
 
// Substitute saved char, if one is waiting
	if (hin->isNextCh)
	{	hin->isNextCh = csc_FALSE;
		ch = hin->nextCh;
	}
	else
	{	ch = csc_ioAnyRead_getc(hin->rca);
	}
 
	if (ch == '\r')
	{	ch = csc_ioAnyRead_getc(hin->rca);
		if (ch == '\n')
		{	if (hin->lastCh != '\n')
			{	ch = csc_ioAnyRead_getc(hin->rca);
				if (ch==' ' || ch=='\t')
				{	hin->lastCh = ' ';
				}
				else
				{	hin->nextCh = ch;
					hin->isNextCh = csc_TRUE;
					hin->lastCh = '\n';
				}
			}
		}
		else
		{	hin->nextCh = ch;
			hin->isNextCh = csc_TRUE;
			hin->lastCh = '\r';
		}
	}
	else if (ch == '\n')
	{	if (hin->lastCh != '\n')
		{	ch = csc_ioAnyRead_getc(hin->rca);
			if (ch==' ' || ch=='\t')
				hin->lastCh = ' ';
			else
			{	hin->nextCh = ch;
				hin->isNextCh = csc_TRUE;
				hin->lastCh = '\n';
			}
		}
	}
	else if (ch == '\t')
		hin->lastCh = ' ';
	else
		hin->lastCh = ch;
 
// Return char.
	return hin->lastCh;
}


httpIn_result_t httpIn_getLine( httpIn_t *hin
							  , csc_str_t *headName
							  , csc_str_t *content
							  )
{	char buf[httpIn_bufLen+1];
	int bufLen;
	httpIn_result_t result = httpIn_OK;
	int ch;
 
// Skip whitespace.
	ch = httpIn_getc(hin);
	while (ch==' ')
		ch = httpIn_getc(hin);
 
// Read in the head word.
	bufLen = 0;
	while (  ch!=-1
		  && ch!='\n'
		  && ch!=' '
		  && ch!=':'
		  && bufLen<httpIn_bufLen 
		  )
	{	buf[bufLen++] = ch;
		ch = httpIn_getc(hin);
	}
 
// If it is empty, we are done.
	if (bufLen == 0)
		return httpIn_done;  // Finished.
 
// Assign the header name.
	buf[bufLen] = '\0';
    csc_str_assign(headName, buf);
 
// Skip whitespace.
	while (ch == ' ')
		ch = httpIn_getc(hin);
	if (ch == ':')
		ch = httpIn_getc(hin);
	while (ch == ' ')
		ch = httpIn_getc(hin);
 
// Get the remainder of the line.
	bufLen = 0;
	while (ch!=-1 && ch!='\n' && bufLen<httpIn_bufLen)
	{	buf[bufLen++] = ch;
		ch = httpIn_getc(hin);
	}
 
// Was the line longer than our buffer?
	if (ch!=-1 && ch!='\n')
		result = httpIn_lineTooLong; // The line was too long.
	else
		result = httpIn_OK;  // Good. more to come.
 
// Trim space from end of line.
	while (bufLen>0 && buf[bufLen-1]==' ')
		bufLen--;
 
// Assign the line.
	buf[bufLen] = '\0';
    csc_str_assign(content, buf);
 
// Discard the remainder of the line.
	while (ch!=-1 && ch!='\n')
		ch = httpIn_getc(hin);
 
// Bye.
	return result;
}

// ------------------------------------------------


csc_httpErr_t csc_httpMsg_rcv(csc_httpMsg_t *msg, csc_ioAnyRead_t *rca)
{	
// Resources.
	httpIn_t *hin = httpIn_new(rca);
	csc_str_t *headName = csc_str_new(NULL);
	csc_str_t *content = csc_str_new(NULL);
			
// Read in the headers.
	httpIn_result_t getLineResult = httpIn_OK;
	while (getLineResult==httpIn_OK || getLineResult==csc_httpErr_LineTooLong)
	{	getLineResult = httpIn_getLine(hin, headName, content);
 
	// Set error code.
		if (getLineResult == httpIn_error)
			setErr(msg, csc_httpErr_syntaxErr, "Error reading header");
		else if (getLineResult == httpIn_lineTooLong)
			setErr(msg, csc_httpErr_LineTooLong, "Header line too long");
 
	// Add the header.
		if (  getLineResult == httpIn_OK
		   || getLineResult == httpIn_lineTooLong
		   )
		{	csc_httpMsg_addHdr(msg, csc_str_charr(headName), csc_str_charr(content));
		}
	}
			
// Free resouces.
	csc_str_free(headName);
	csc_str_free(content);
	httpIn_free(hin);
 
// Return the restlt.
	return csc_httpMsg_getErrCode(msg);
}


// Receive a HTTP message from whatever as a client.
csc_httpErr_t csc_httpMsg_rcvSrv(csc_httpMsg_t *msg, csc_ioAnyRead_t *rca)
{	int wdLen;
	csc_httpErr_t errCode;
	const char *wd;
 
// Resources.
	csc_str_t *word = NULL;
 
// Get the method.
	word = csc_str_new(NULL);
	wdLen = csc_ioAnyRead_getwd(rca, word);
	if (wdLen < 1)
	{	setErr( msg, csc_httpErr_UnexpectedEOF
			  , "Unexpected EOF reading method of request line");
		goto freeResources;
	}
 
// Add the method.
	wd = csc_str_charr(word);
	errCode = csc_httpMsg_addHdr(msg, csc_http_method, wd);
	if (errCode != csc_httpErr_Ok)
	{	skipTillBlankLine(rca);
		goto freeResources;
	}
 
// Check the method
	if (  strcmp(wd,"GET") && strcmp(wd,"POST")   && strcmp(wd,"HEAD") 
	   && strcmp(wd,"PUT") && strcmp(wd,"DELETE") && strcmp(wd,"TRACE") 
	   && strcmp(wd,"OPTIONS") 
	   )
	{	setErr(msg, csc_httpErr_BadMethod, "Bad method in request line");
		skipTillBlankLine(rca);
		goto freeResources;
	}
 
// Get the resource URI.
	wdLen = csc_ioAnyRead_getwd(rca, word);
	if (wdLen < 1)
	{	setErr( msg, csc_httpErr_UnexpectedEOF
			  , "Unexpected EOF reading resource in request line");
		goto freeResources;
	}
 
// Add the resource URI.
	errCode = csc_httpMsg_addHdr(msg, csc_http_reqUri, csc_str_charr(word));
	if (errCode != csc_httpErr_Ok)
	{	skipTillBlankLine(rca);
		goto freeResources;
	}
 
// Check the resource URI.
	wd = csc_str_charr(word);
	if ( !(  strcmp(wd,"GET")  && strcmp(wd,"POST") && strcmp(wd,"HEAD")
		  && strcmp(wd,"PUT") && strcmp(wd,"DELETE")
		  )
	   && !csc_isValid_decentAbsPath(wd)
	   )
	{	setErr(msg, csc_httpErr_BadReqUri, "Bad requested resource in request line");
		skipTillBlankLine(rca);
		goto freeResources;
	}
 
// Get the protocol.
	wdLen = csc_ioAnyRead_getwd(rca, word);
	if (wdLen < 1)
	{	setErr( msg, csc_httpErr_UnexpectedEOF
			  , "Unexpected EOF reading protocol in request line");
		goto freeResources;
	}
 
// Add the protocol.
	errCode = csc_httpMsg_addHdr(msg, csc_http_protocol, csc_str_charr(word));
	if (errCode != csc_httpErr_Ok)
	{	skipTillBlankLine(rca);
		goto freeResources;
	}
 
// Check the protocol
	if (strcmp(csc_str_charr(word),"HTTP/1.1") && strcmp(csc_str_charr(word),"HTTP/1.0"))
	{	setErr(msg, csc_httpErr_BadProtocol, "Bad protocol in request line");
		skipTillBlankLine(rca);
		goto freeResources;
	}
 
// Read in all the other headers.
	csc_httpMsg_rcv(msg, rca);
 
// Free resources.
freeResources:
	if (word)
		csc_str_free(word);
 
// Bye.
	return csc_httpMsg_getErrCode(msg);
}


// Receive a HTTP message from whatever as a client.
csc_httpErr_t csc_httpMsg_rcvCli(csc_httpMsg_t *msg, csc_ioAnyRead_t *rca)
{	int wdLen;
	csc_httpErr_t errCode;
 
// Resources.
	csc_str_t *word = NULL;
 
// Get the protocol.
	word = csc_str_new(NULL);
	wdLen = csc_ioAnyRead_getwd(rca, word);
	if (wdLen < 1)
	{	setErr(msg, csc_httpErr_UnexpectedEOF, "Unexpected EOF reading status line");
		goto freeResources;
	}
 
// Add the protocol.
	errCode = csc_httpMsg_addHdr(msg, csc_http_protocol, csc_str_charr(word));
	if (errCode != csc_httpErr_Ok)
	{	skipTillBlankLine(rca);
		goto freeResources;
	}
 
// Check the protocol
	if (strcmp(csc_str_charr(word),"HTTP/1.1") && strcmp(csc_str_charr(word),"HTTP/1.0"))
	{	setErr(msg, csc_httpErr_BadProtocol, "Bad protocol in status line");
		skipTillBlankLine(rca);
		goto freeResources;
	}
 
// Get the response code.
	wdLen = csc_ioAnyRead_getwd(rca, word);
	if (wdLen < 1)
	{	setErr(msg, csc_httpErr_UnexpectedEOF, "Unexpected EOF reading status line");
		goto freeResources;
	}
 
// Add the response code.
	errCode = csc_httpMsg_addHdr(msg, csc_http_statCode, csc_str_charr(word));
	if (errCode != csc_httpErr_Ok)
	{	skipTillBlankLine(rca);
		goto freeResources;
	}
 
// Check the response code
	if (!csc_isValidRange_int(csc_str_charr(word), 100, 599, NULL))
	{
		setErr(msg, csc_httpErr_BadStatCode, "Bad status code in status line");
		skipTillBlankLine(rca);
		goto freeResources;
	}
 
// Get the reason phrase.
	wdLen = csc_ioAnyRead_getline(rca, word);
	if (wdLen < 1)
	{	setErr(msg, csc_httpErr_UnexpectedEOF, "Unexpected EOF 3 reading status line");
		goto freeResources;
	}
 
// Add the reason phrase.
	errCode = csc_httpMsg_addHdr(msg, csc_http_reason, csc_str_charr(word));
	if (errCode != csc_httpErr_Ok)
	{	skipTillBlankLine(rca);
		goto freeResources;
	}
 
// Read in all the other headers.
	csc_httpMsg_rcv(msg, rca);
 
// Free resources.
freeResources:
	if (word)
		csc_str_free(word);
 
// Bye.
	return csc_httpMsg_getErrCode(msg);
}


// Receive a HTTP message from an input string as a client.
csc_httpErr_t csc_httpMsg_rcvCliStr(csc_httpMsg_t *msg, const char *str)
{	
// Create the stream.
	csc_ioAny_readChStr_t *inStr = csc_ioAny_readChStr_new(str);
	csc_ioAnyRead_t *rca = csc_ioAnyRead_new(csc_ioAny_readCharStr, inStr);
 
// Read the HTTP.
	csc_httpErr_t errCode = csc_httpMsg_rcvCli(msg, rca);
 
// Free the stream.
	csc_ioAnyRead_free(rca);
	csc_ioAny_readChStr_free(inStr);
 
// Return the error code.
	return errCode;
}

// Receive a HTTP message from an input FILE stream as a client.
csc_httpErr_t csc_httpMsg_rcvCliFILE(csc_httpMsg_t *msg, FILE *fin)
{
// Create the stream.
	csc_ioAnyRead_t *rca = csc_ioAnyRead_new(csc_ioAny_readCharFILE, fin);
 
// Read the HTTP.
	csc_httpErr_t errCode = csc_httpMsg_rcvCli(msg, rca);
 
// Free the stream.
	csc_ioAnyRead_free(rca);
 
// Return the error code.
	return errCode;
}


// Receive a HTTP message from an input string as a client.
csc_httpErr_t csc_httpMsg_rcvSrvStr(csc_httpMsg_t *msg, const char *str)
{	
// Create the stream.
	csc_ioAny_readChStr_t *inStr = csc_ioAny_readChStr_new(str);
	csc_ioAnyRead_t *rca = csc_ioAnyRead_new(csc_ioAny_readCharStr, inStr);
 
// Read the HTTP.
	csc_httpErr_t errCode = csc_httpMsg_rcvSrv(msg, rca);
 
// Free the stream.
	csc_ioAnyRead_free(rca);
	csc_ioAny_readChStr_free(inStr);
 
// Return the error code.
	return errCode;
}

// Receive a HTTP message from an input FILE stream as a client.
csc_httpErr_t csc_httpMsg_rcvSrvFILE(csc_httpMsg_t *msg, FILE *fin)
{
// Create the stream.
	csc_ioAnyRead_t *rca = csc_ioAnyRead_new(csc_ioAny_readCharFILE, fin);
 
// Read the HTTP.
	csc_httpErr_t errCode = csc_httpMsg_rcvSrv(msg, rca);
 
// Free the stream.
	csc_ioAnyRead_free(rca);
 
// Return the error code.
	return errCode;
}



