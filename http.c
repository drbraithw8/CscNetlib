// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#include <ctype.h>

#include "http.h"
#include "hash.h"
#include "alloc.h"
#include "isvalid.h"
#include "dynArray.h"


#define staticKeyword static
csc_dynArray_headers(nameValArr, csc_nameVal)
csc_dynArray_code(nameValArr, csc_nameVal)


typedef struct csc_http_t
{   
// Errors.
    csc_httpErr_t errCode;
    char *errMsg;
 
// Start line.
    char *startFields[csc_httpSF_numSF];
 
// Headers.
    nameValArr_t *headers;
 
// URI args: Name value string string pairs.
    csc_mapSS_t *uriArgs;

// A limit on the number of chars to read in.
    int maxInputChars;
 
} csc_http_t;


csc_http_t *csc_http_new()
{   
// Allocate the structure.
    csc_http_t *msg = csc_allocOne(csc_http_t);
    
// Errors.
    msg->errCode = csc_httpErr_Ok;
    msg->errMsg = NULL;
 
// Start Fields.
    for (int i=0; i<csc_httpSF_numSF; i++)
        msg->startFields[i] = NULL;
 
// Headers.
    msg->headers = nameValArr_new();
 
// URI args.
    msg->uriArgs = csc_mapSS_new();

// A limit on the number of chars to read in.
    msg->maxInputChars = 3000;
 
// Home with the bacon.
    return msg;
}


void csc_http_setMaxInputChars(csc_http_t *msg, int maxChars)
{   msg->maxInputChars = maxChars;
}


void csc_http_free(csc_http_t *msg)
{   
// Errors.
    if (msg->errMsg)
        free(msg->errMsg);
    
// Start Fields.
    for (int i=0; i<csc_httpSF_numSF; i++)
    {   if (msg->startFields[i])
            free(msg->startFields[i]);
    }
 
// Http Headers.
    if (msg->headers)
        nameValArr_free(msg->headers);
 
// URI args.
    csc_mapSS_free(msg->uriArgs);
 
// Free the structure.
    free(msg);
}


static void setErr(csc_http_t *msg, csc_httpErr_t errCode, const char *errMsg)
{   if (msg->errMsg)
        free(msg->errMsg);
    msg->errCode = errCode;
    msg->errMsg = csc_alloc_str(errMsg);
}


csc_httpErr_t csc_http_addSF(csc_http_t *msg, csc_httpSF_t fldNdx, const char *value)
{   csc_httpErr_t errCode;
 
// Check that the field index is within range.
    if (fldNdx<0 || fldNdx>=csc_httpSF_numSF)
    {   errCode = csc_httpErr_BadSF;
        setErr(msg, errCode, "Start line field out of range");
        return errCode;
    }
 
// Check that the field has not already been assigned.
    if (msg->startFields[fldNdx])
    {   errCode = csc_httpErr_AlreadySF;
        setErr(msg, errCode, "Start line field assigned twice");
        return errCode;
    }

// Check the URI.
    if (fldNdx == csc_httpSF_reqUri)
    {   if (!csc_isValid_decentAbsPath(value))
        {   errCode = csc_httpErr_BadReqUri;
            setErr(msg, errCode, "Bad Request URI");
            return errCode;
        }
    }
 
// Assign it.
    msg->startFields[fldNdx] = csc_alloc_str(value);
    return csc_httpErr_Ok;
}


csc_httpErr_t csc_http_addHdr(csc_http_t *msg, const char *name, const char *value)
{   csc_nameVal_t *nv = csc_nameVal_new(name, value);
    nameValArr_add(msg->headers, nv);
    return csc_httpErr_Ok;
}


const char *csc_http_getSF(csc_http_t *msg, csc_httpSF_t fldNdx)
{   if (fldNdx<0 || fldNdx>=csc_httpSF_numSF)
        return NULL;
    return msg->startFields[fldNdx];
}


const char *csc_http_getHdr(csc_http_t *msg, const char *name)
{   const char *val = NULL;
    csc_nameVal_t **els = msg->headers->els;
    int nEls = msg->headers->nEls;
    for (int i=0; i<nEls; i++)
    {   if (csc_streq(els[i]->name, name))
        {   val = els[i]->val;
            break;
        }
    }
    return val;
}


csc_httpErr_t csc_http_getErrCode(csc_http_t *msg)
{   return msg->errCode;
}


const char *csc_http_getErrStr(csc_http_t *msg)
{   return msg->errMsg;
}


// ------------------------------------------------
// ---------- Class for input of HTTP -------------
// ------------------------------------------------

typedef enum
{   httpIn_OK = 0,
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
 
// Count and limit.
    int maxChars;
    int countChars;
} httpIn_t;
 
const int httpIn_bufLen = 255;


static httpIn_t *httpIn_new(csc_ioAnyRead_t *rca, int maxChars)
{
// Allocate the structure.
    httpIn_t *hin = csc_allocOne(httpIn_t);
    hin->rca = rca;
    hin->nextCh = 0;
    hin->lastCh = 0;
    hin->isNextCh = csc_FALSE;
 
// Count and limit.
    hin->maxChars = maxChars;
    hin->countChars = 0;
 
// Bye.
    return hin;
}


static void httpIn_free(httpIn_t *hin)
{   free(hin);
}


static int httpIn_getc(httpIn_t *hin)
{   int ch;
    if (hin->countChars++ >= hin->maxChars)
        ch = EOF;
    else
    {   ch = csc_ioAnyRead_getc(hin->rca);
        if (ch == '\r')
            ch = csc_ioAnyRead_getc(hin->rca);
    }
    return ch;
}

// The previous just ignores a single '\r' anywhere.
// It does not bother handling continuation lines.
// 
// The following tries to only ignore '\r' if it is followed by a '\n'.
// It also tries to join continued lines.
// It turned out to be complicated, and sadly does not work too well.  It blocks.
// To make this work, it has to be careful not to look ahead whenever it
// has had an empty line.

// static int httpIn_getc(httpIn_t *hin)
// // Intended for http.
// // * Replaces \r\n[\s|\t] by \s.
// // * Replaces \n[\s|\t] by \s.
// // * Replaces \r\n by \n.
// // * Does not ask for the next char if it got
// //       the end, cos that would block.
// {    int ch = 0;
//  
// // Substitute saved char, if one is waiting
//  if (hin->isNextCh)
//  {   hin->isNextCh = csc_FALSE;
//      ch = hin->nextCh;
//  }
//  else
//  {   ch = csc_ioAnyRead_getc(hin->rca);
//  }
//  
//  if (ch == '\r')
//  {   ch = csc_ioAnyRead_getc(hin->rca);
//      if (ch == '\n')
//      {   ch = csc_ioAnyRead_getc(hin->rca);
//          if (ch==' ' || ch=='\t')
//          {   hin->lastCh = ' ';
//          }
//          else
//          {   hin->nextCh = ch;
//              hin->isNextCh = csc_TRUE;
//              hin->lastCh = '\n';
//          }
//      }
//      else
//      {   hin->nextCh = ch;
//          hin->isNextCh = csc_TRUE;
//          hin->lastCh = '\r';
//      }
//  }
//  else if (ch == '\n')
//  {   ch = csc_ioAnyRead_getc(hin->rca);
//      if (ch==' ' || ch=='\t')
//          hin->lastCh = ' ';
//      else
//      {   hin->nextCh = ch;
//          hin->isNextCh = csc_TRUE;
//          hin->lastCh = '\n';
//      }
//  }
//  else if (ch == '\t')
//      hin->lastCh = ' ';
//  else if (ch == -1)
//  {   hin->nextCh = ch;
//      hin->isNextCh = csc_TRUE;
//      hin->lastCh = -1;
//  }
//  else
//      hin->lastCh = ch;
//  
// // Return char.
//  return hin->lastCh;
// }


static int httpIn_getline(httpIn_t *hin, csc_str_t *line)
{   int ch;
    csc_str_reset(line);
 
// Skip leading space.
    ch = httpIn_getc(hin);
    while (ch==' ' || ch=='\t')
    ch = httpIn_getc(hin);
 
// Read in line. 
    while (ch!=EOF && ch!='\n')
    {   if (ch != '\r')
            csc_str_append_ch(line, ch);
        ch = httpIn_getc(hin);
    }
 
// Trim space from end of line.
    const char *buf = csc_str_charr(line);
    int bufLen = csc_str_length(line);
    while (bufLen>0 && buf[bufLen-1]==' ')
        bufLen--;
    csc_str_truncate(line, bufLen);
 
// Return result. 
    if (ch == EOF)
        return -1;
    else
        return csc_str_length(line);
}


static void httpIn_skipTillBlankLine(httpIn_t *hin)
{   int prevCh = ' ';
    int ch = '\n';
    while (ch!=EOF && (ch!='\n' || prevCh!='\n'))
    {   prevCh = ch;
        ch = httpIn_getc(hin);
        if (ch == '\r')
            ch = httpIn_getc(hin);
    }
}


static int httpIn_getwd(httpIn_t *hin, csc_str_t *word)
{
    int ch = httpIn_getc(hin);
    csc_str_reset(word);
 
/* Skip whitespace */
    while(isspace(ch))
        ch = httpIn_getc(hin);
 
/* Deal with a possible EOF */
    if(ch==EOF)
        return -1;
 
/* read in the word */
    while(!isspace(ch) && ch!=EOF)
    {   csc_str_append_ch(word, ch);
        ch = httpIn_getc(hin);
    }
 
/* Return the length. */
    return csc_str_length(word);
}



static httpIn_result_t httpIn_getHdr( httpIn_t *hin
                                      , csc_str_t *headName
                                      , csc_str_t *content
                                      )
{   httpIn_result_t result = httpIn_OK;
    int ch;
 
// Resources.
    csc_str_t *buf = csc_str_new("------ make initial size suitable to avoid unnecessary realloc -----");
 
// Skip whitespace.
    ch = httpIn_getc(hin);
    while (ch==' ')
        ch = httpIn_getc(hin);
 
// Read in the head word.
    csc_str_truncate(buf, 0); 
    while (  ch!=-1
          && ch!='\n'
          && ch!=' '
          && ch!=':'
          )
    {   csc_str_append_ch(buf, ch);
        ch = httpIn_getc(hin);
    }
 
// If it is empty, we are done.
    if (csc_str_length(buf) == 0)
    {   result = httpIn_done;
        goto freeResources;
    }
 
// Assign the header name.
    csc_str_assign_str(headName, buf);
 
// Get the remainder of the line.
    httpIn_getline(hin, buf);
    csc_str_assign_str(content, buf);
 
freeResources:
    csc_str_free(buf);
 
// Bye.
    return result;
}

// ------------------------------------------------


static csc_httpErr_t readHeaders(csc_http_t *msg, httpIn_t *hin)
{   
// Resources.
    csc_str_t *headName = csc_str_new(NULL);
    csc_str_t *content = csc_str_new(NULL);
            
// Read in the headers.
    httpIn_result_t getLineResult = httpIn_OK;
    while (getLineResult==httpIn_OK || getLineResult==csc_httpErr_LineTooLong)
    {   getLineResult = httpIn_getHdr(hin, headName, content);
 
    // Set error code.
        if (getLineResult == httpIn_error)
            setErr(msg, csc_httpErr_syntaxErr, "Error reading header");
        else if (getLineResult == httpIn_lineTooLong)
            setErr(msg, csc_httpErr_LineTooLong, "Header line too long");
 
    // Add the header.
        if (  getLineResult == httpIn_OK
           || getLineResult == httpIn_lineTooLong
           )
        {   csc_http_addHdr(msg, csc_str_charr(headName), csc_str_charr(content));
        }
    }
            
// Free resouces.
    csc_str_free(headName);
    csc_str_free(content);
 
// Return the restlt.
    return csc_http_getErrCode(msg);
}


static csc_bool_t isUrlUnres(int ch, csc_bool_t isSlashOk)
{   csc_bool_t result;
    switch(ch)
    {   
        case 'q': case 'w': case 'e': case 'r': case 't': case 'y': case 'u': case 'i':
        case 'o': case 'p': case 'a': case 's': case 'd': case 'f': case 'g': case 'h':
        case 'j': case 'k': case 'l': case 'z': case 'x': case 'c': case 'v': case 'b':
        case 'n': case 'm': case 'Q': case 'W': case 'E': case 'R': case 'T': case 'Y':
        case 'U': case 'I': case 'O': case 'P': case 'A': case 'S': case 'D': case 'F':
        case 'G': case 'H': case 'J': case 'K': case 'L': case 'Z': case 'X': case 'C':
        case 'V': case 'B': case 'N': case 'M': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9': case '0':
        case '-': case '_': case '.': case '~':
            result = csc_TRUE;
            break;
        case '/': case ':':
            result = isSlashOk;
            break;
        default:
            result = csc_FALSE;
    }
    return result;
}


// Returns value of hex digit or -1 if not a hex digit.
static int hexDigToVal(int ch)
{   int result;
    if (ch>='0' && ch<='9')
        result = ch - '0';
    else if (ch>='A' && ch<='F')
        result = ch - ('A'-10);
    else if (ch>='a' && ch<='f')
        result = ch - ('a'-10);
    else
        result = -1;
    return result;
}


// Removes percent encoding from a string.
// Returns allocated string that must be free()d by the caller.
char *csc_http_pcentDec(const char *enc)
{   int encLen = strlen(enc);
    char *dec = malloc(encLen+1);
    const char *pe = enc;
    char *pd = dec;
    int ch = *pe++;
    while (ch != '\0')
    {   if (ch == '%')
        {   ch = *pe++;
            int dig1 = hexDigToVal(ch);
            if (dig1 >= 0)
            {   ch = *pe++;
                int dig2 = hexDigToVal(ch);
                if (dig2 >= 0)
                {   *pd++ = dig1 * 16 + dig2;
                    ch = *pe++;
                }
                else
                    *pd++ = '%';
            }
            else
                *pd++ = '%';
        }
        else
        {   *pd++ = ch;
            ch = *pe++;
        }
    }
    *pd++ = '\0';
    return dec;
}


// Performs percent encoding on a string.
// Returns allocated string that must be csc_str_free()d by the caller.
void csc_http_pcentEnc(const char *dec, csc_str_t *enc, csc_bool_t isSlashOk)
{   const char *hexDigs = "0123456789ABCDEF";
    csc_str_reset(enc);
    const char *pd = dec;
    int ch = *pd++;
    while (ch != '\0')
    {   if (isUrlUnres(ch,isSlashOk))
            csc_str_append_ch(enc, ch);
        else
        {   csc_str_append_ch(enc, '%');
            csc_str_append_ch(enc, hexDigs[ch/16]);
            csc_str_append_ch(enc, hexDigs[ch%16]);
        }
        ch = *pd++;
    }
}


// Add a name/value pair for URL encoding into the requestUrl part of a
// HTTP request line.  If 'val' is NULL, there will be no "=value" part.
// If 'val' is "", then there will be an equals sign, but the value will be
// empty.
csc_httpErr_t csc_http_addUrlVal(csc_http_t *msg, const char *name, const char *val)
{   
    if (!csc_mapSS_addex(msg->uriArgs, name, val))
    {   setErr(msg, csc_httpErr_AlreadyUrlArg, "URL encoded argument already present");
        return csc_httpErr_AlreadyUrlArg;
    }
    else
        return csc_httpErr_Ok;
}


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
// If the name, 'name' is present, and has a value then this function will
// return the value and *'whatsThere' will be set to 4 if 'whatsThere' is
// not null.
const char *csc_http_getUrlVal(csc_http_t *msg, const char *name, int *whatsThere)
{   const csc_nameVal_t *nv = csc_mapSS_get(msg->uriArgs, name);
    const char *result = NULL;
    int what = 0;

// What is there?
    if (nv == NULL)
    {   result = NULL;
        what = 1;
    }
    else if (nv->val == NULL)
    {   result = NULL;
        what = 2;
    }
    else if (csc_streq(nv->val,""))
    {   result = nv->val;
        what = 3;
    }
    else
    {   result = nv->val;
        what = 4;
    }
        
// Bye.
    if (whatsThere != NULL)
        *whatsThere = what;
    return result;
}


// Destructively parse the URI.
static csc_httpErr_t parseUri(csc_http_t *msg, char *uri)
{   char **msgUri = &msg->startFields[csc_httpSF_reqUri];
    csc_httpErr_t result = csc_httpErr_Ok;
    csc_httpErr_t rslt;
    char *p;
    int ch;
 
// Check.
    if (*msgUri != NULL)
    {   setErr(msg, csc_httpErr_AlreadySF, "Request URI already present");
        return csc_httpErr_AlreadySF;
    }
 
// Read in the URI.
    p = uri;
    ch = *p++;
    while (ch!='?' && ch!='\0')
        ch = *p++;
    *(p-1) = '\0';
    
// Decode and assign the URI.
    *msgUri = csc_http_pcentDec(uri);
 
// Decode and assign each argument in turn.
    while (ch != '\0')
    {
    // Name value pair.
        char *argName;
        char *argVal;
 
    // Resources.
        char *decArgName = NULL;
        char *decArgVal = NULL;
 
    // Read the arg name.
        argName = p;
        ch = *p++;
        while (ch!='=' && ch!='&' && ch!='\0')
            ch = *p++;
 
        *(p-1) = '\0';
 
    // Read the arg value if there is one.
        if (ch != '=')
            argVal = NULL;
        else
        {   argVal = p;
            ch = *p++;
            while (ch!='&' && ch!='\0')
                ch = *p++;
            *(p-1) = '\0';
        }
 
    // Decode the arg name and value.
        decArgName = csc_http_pcentDec(argName); 
        if (argVal != NULL)
            decArgVal = csc_http_pcentDec(argVal); 
        else
            decArgVal = NULL; 
 
    // Assign the arg name and value.
        rslt = csc_http_addUrlVal(msg, decArgName, decArgVal);
        if (result == csc_httpErr_Ok)
            result = rslt;
 
    // Free resources.
        if (decArgName)
            free(decArgName);
        if (decArgVal)
            free(decArgVal);
    }
 
// Result.
    return result;
}


// Receive a HTTP message from whatever as a server.
csc_httpErr_t csc_http_rcvSrv(csc_http_t *msg, csc_ioAnyRead_t *rca)
{   int wdLen;
    csc_httpErr_t errCode;
    const char *wd;
 
// Resources.
    csc_str_t *word = NULL;
    char *uri = NULL;
    httpIn_t *hin = NULL;
 
// Input processing.
    hin = httpIn_new(rca, msg->maxInputChars);
 
// Get the method.
    word = csc_str_new(NULL);
    wdLen = httpIn_getwd(hin, word);
    if (wdLen < 1)
    {   setErr( msg, csc_httpErr_UnexpectedEOF
              , "Unexpected EOF reading method of request line");
        goto freeResources;
    }
 
// Add the method.
    wd = csc_str_charr(word);
    errCode = csc_http_addSF(msg, csc_httpSF_method, wd);
    if (errCode != csc_httpErr_Ok)
    {   httpIn_skipTillBlankLine(hin);
        goto freeResources;
    }
 
// Check the method
    if (  strcmp(wd,"GET") && strcmp(wd,"POST")   && strcmp(wd,"HEAD") 
       && strcmp(wd,"PUT") && strcmp(wd,"DELETE") && strcmp(wd,"TRACE") 
       && strcmp(wd,"OPTIONS") 
       )
    {   setErr(msg, csc_httpErr_BadMethod, "Bad method in request line");
        httpIn_skipTillBlankLine(hin);
        goto freeResources;
    }
 
// Get the resource URI.
    wdLen = httpIn_getwd(hin, word);
    if (wdLen < 1)
    {   setErr( msg, csc_httpErr_UnexpectedEOF
              , "Unexpected EOF reading resource in request line");
        goto freeResources;
    }
 
// Add the resource URI.
    uri = csc_str_alloc_charr(word);
    errCode = parseUri(msg, uri);
    if (errCode != csc_httpErr_Ok)
    {   httpIn_skipTillBlankLine(hin);
        goto freeResources;
    }
 
// Check the resource URI.
//  wd = csc_str_charr(word);
//  if (  !csc_isValid_decentAbsPath(wd))
//  {   setErr(msg, csc_httpErr_BadReqUri, "Bad requested resource in request line");
//      httpIn_skipTillBlankLine(hin);
//      goto freeResources;
//  }
 
// Get the protocol.
    wdLen = httpIn_getline(hin, word);
    if (wdLen < 1)
    {   setErr( msg, csc_httpErr_UnexpectedEOF
              , "Unexpected EOF reading protocol in request line");
        goto freeResources;
    }
 
// Add the protocol.
    errCode = csc_http_addSF(msg, csc_httpSF_protocol, csc_str_charr(word));
    if (errCode != csc_httpErr_Ok)
    {   httpIn_skipTillBlankLine(hin);
        goto freeResources;
    }
 
// Check the protocol
    if (strcmp(csc_str_charr(word),"HTTP/1.1") && strcmp(csc_str_charr(word),"HTTP/1.0"))
    {   setErr(msg, csc_httpErr_BadProtocol, "Bad protocol in request line");
        httpIn_skipTillBlankLine(hin);
        goto freeResources;
    }
 
// Read in all the other headers.
    readHeaders(msg, hin);
 
// Free resources.
freeResources:
    if (word)
        csc_str_free(word);
    if (uri)
        free(uri);
    if (hin)
        httpIn_free(hin);
 
// Bye.
    return csc_http_getErrCode(msg);
}


// Receive a HTTP message from whatever as a client.
csc_httpErr_t csc_http_rcvCli(csc_http_t *msg, csc_ioAnyRead_t *rca)
{   int wdLen;
    csc_httpErr_t errCode;
 
// Resources.
    csc_str_t *word = NULL;
    httpIn_t *hin = NULL;
 
// Input processing.
    hin = httpIn_new(rca, msg->maxInputChars);
 
// Get the protocol.
    word = csc_str_new(NULL);
    wdLen = httpIn_getwd(hin, word);
    if (wdLen < 1)
    {   setErr(msg, csc_httpErr_UnexpectedEOF, "Unexpected EOF reading status line");
        goto freeResources;
    }
 
// Add the protocol.
    errCode = csc_http_addSF(msg, csc_httpSF_protocol, csc_str_charr(word));
    if (errCode != csc_httpErr_Ok)
    {   httpIn_skipTillBlankLine(hin);
        goto freeResources;
    }
 
// Check the protocol
    if (strcmp(csc_str_charr(word),"HTTP/1.1") && strcmp(csc_str_charr(word),"HTTP/1.0"))
    {   setErr(msg, csc_httpErr_BadProtocol, "Bad protocol in status line");
        httpIn_skipTillBlankLine(hin);
        goto freeResources;
    }
 
// Get the response code.
    wdLen = httpIn_getwd(hin, word);
    if (wdLen < 1)
    {   setErr(msg, csc_httpErr_UnexpectedEOF, "Unexpected EOF reading status line");
        goto freeResources;
    }
 
// Add the response code.
    errCode = csc_http_addSF(msg, csc_httpSF_statCode, csc_str_charr(word));
    if (errCode != csc_httpErr_Ok)
    {   httpIn_skipTillBlankLine(hin);
        goto freeResources;
    }
 
// Check the response code
    if (!csc_isValidRange_int(csc_str_charr(word), 100, 599, NULL))
    {
        setErr(msg, csc_httpErr_BadStatCode, "Bad status code in status line");
        httpIn_skipTillBlankLine(hin);
        goto freeResources;
    }
 
// Get the reason phrase.
    wdLen = httpIn_getline(hin, word);
    if (wdLen < 1)
    {   setErr(msg, csc_httpErr_UnexpectedEOF, "Unexpected EOF 3 reading status line");
        goto freeResources;
    }
 
// Add the reason phrase.
    errCode = csc_http_addSF(msg, csc_httpSF_reason, csc_str_charr(word));
    if (errCode != csc_httpErr_Ok)
    {   httpIn_skipTillBlankLine(hin);
        goto freeResources;
    }
 
// Read in all the other headers.
    readHeaders(msg, hin);
 
// Free resources.
freeResources:
    if (word)
        csc_str_free(word);
    if (hin)
        httpIn_free(hin);
 
// Bye.
    return csc_http_getErrCode(msg);
}


// Receive a HTTP message from an input string as a client.
csc_httpErr_t csc_http_rcvCliStr(csc_http_t *msg, const char *str)
{   
// Create the stream.
    csc_ioAny_readChStr_t *inStr = csc_ioAny_readChStr_new(str);
    csc_ioAnyRead_t *rca = csc_ioAnyRead_new(csc_ioAny_readCharStr, inStr);
 
// Read the HTTP.
    csc_httpErr_t errCode = csc_http_rcvCli(msg, rca);
 
// Free the stream.
    csc_ioAnyRead_free(rca);
    csc_ioAny_readChStr_free(inStr);
 
// Return the error code.
    return errCode;
}

// Receive a HTTP message from an input FILE stream as a client.
csc_httpErr_t csc_http_rcvCliFILE(csc_http_t *msg, FILE *fin)
{   csc_ioAnyRead_t *rca = csc_ioAnyRead_new(csc_ioAny_readCharFILE, fin);
    csc_httpErr_t errCode = csc_http_rcvCli(msg, rca);
    csc_ioAnyRead_free(rca);
    return errCode;
}


// Receive a HTTP message from an input string as a client.
csc_httpErr_t csc_http_rcvSrvStr(csc_http_t *msg, const char *str)
{   
// Create the stream.
    csc_ioAny_readChStr_t *inStr = csc_ioAny_readChStr_new(str);
    csc_ioAnyRead_t *rca = csc_ioAnyRead_new(csc_ioAny_readCharStr, inStr);
 
// Read the HTTP.
    csc_httpErr_t errCode = csc_http_rcvSrv(msg, rca);
 
// Free the stream.
    csc_ioAnyRead_free(rca);
    csc_ioAny_readChStr_free(inStr);
 
// Return the error code.
    return errCode;
}

// Receive a HTTP message from an input FILE stream as a client.
csc_httpErr_t csc_http_rcvSrvFILE(csc_http_t *msg, FILE *fin)
{   csc_ioAnyRead_t *rca = csc_ioAnyRead_new(csc_ioAny_readCharFILE, fin);
    csc_httpErr_t errCode = csc_http_rcvSrv(msg, rca);
    csc_ioAnyRead_free(rca);
    return errCode;
}


csc_httpErr_t csc_http_sendCli(csc_http_t *msg, csc_ioAnyWrite_t* out)
{   csc_httpErr_t result = csc_httpErr_Ok;
 
// Resources.
    csc_str_t *pcEnc = NULL;
 
// The method.
    char *method = msg->startFields[csc_httpSF_method];
    if (method == NULL) 
    {   setErr(msg, csc_httpErr_MissingMethod, "Missing method for request line");
        result = csc_httpErr_MissingMethod;
        goto freeResources;
    }
 
// The request URI.
    char *reqUri = msg->startFields[csc_httpSF_reqUri];
    if (reqUri == NULL) 
    {   setErr(msg, csc_httpErr_MissingReqUri, "Missing request-URI for request line");
        result = csc_httpErr_MissingReqUri;
        goto freeResources;
    }
 
// The protocol.
    char *protocol = msg->startFields[csc_httpSF_protocol];
    if (protocol == NULL) 
        protocol = "HTTP/1.1";
 
// Send the method.
    csc_ioAnyWrite_puts(out, method);
    csc_ioAnyWrite_puts(out, " ");
 
// Send the request URI.
    pcEnc = csc_str_new(reqUri);
    csc_http_pcentEnc(reqUri, pcEnc, csc_TRUE);
    // csc_ioAnyWrite_puts(out, "/");
    csc_ioAnyWrite_puts(out, csc_str_charr(pcEnc));
 
// Send all the args bundled into the request URI.
    if (csc_mapSS_count(msg->uriArgs) > 0)
    {   const csc_nameVal_t *nv;
        csc_mapSS_iter_t *iter;
        int count = 0;
 
        iter = csc_mapSS_iter_new(msg->uriArgs);
        while ((nv = csc_mapSS_iter_next(iter)) != NULL)
        {   
        // The preceeding char.
            if (count++ == 0)
                csc_ioAnyWrite_puts(out, "?");
            else
                csc_ioAnyWrite_puts(out, "&");
 
        // The name.
            csc_http_pcentEnc(nv->name, pcEnc, csc_FALSE);
            csc_ioAnyWrite_puts(out, csc_str_charr(pcEnc));
 
        // The value, if there is one.
            if (nv->val != NULL)
            {   csc_ioAnyWrite_puts(out, "=");
                csc_http_pcentEnc(nv->val, pcEnc, csc_FALSE);
                csc_ioAnyWrite_puts(out, csc_str_charr(pcEnc));
            }
        }
        csc_mapSS_iter_free(iter);
    }
    csc_ioAnyWrite_puts(out, " ");
 
// Send the protocol.
    csc_ioAnyWrite_puts(out, protocol);
    csc_ioAnyWrite_puts(out, "\r\n");
 
// Send each header.
    const char *val = NULL;
    csc_nameVal_t **els = msg->headers->els;
    int nEls = msg->headers->nEls;
    for (int i=0; i<nEls; i++)
    {   csc_nameVal_t *nv = els[i];
        csc_ioAnyWrite_puts(out, nv->name);
        csc_ioAnyWrite_puts(out, ": ");
        csc_ioAnyWrite_puts(out, nv->val);
        csc_ioAnyWrite_puts(out, "\r\n");
    }
 
// Send a blank line to terminate.
    csc_ioAnyWrite_puts(out, "\r\n");
 
// Free resources.
freeResources:
    if (pcEnc)
        csc_str_free(pcEnc);
 
// We are done.
    return result;
}


csc_httpErr_t csc_http_sendCliFILE(csc_http_t *msg, FILE* fout)
{   csc_ioAnyWrite_t *out = csc_ioAnyWrite_new(csc_ioAny_writeFILE, fout);
    csc_httpErr_t errCode = csc_http_sendCli(msg, out);
    csc_ioAnyWrite_free(out);
    return errCode;
}


csc_httpErr_t csc_http_sendCliStr(csc_http_t *msg, csc_str_t *sout)
{   csc_ioAnyWrite_t *out = csc_ioAnyWrite_new(csc_ioAny_writeCstr, sout);
    csc_httpErr_t errCode = csc_http_sendCli(msg, out);
    csc_ioAnyWrite_free(out);
    return errCode;
}


csc_httpErr_t csc_http_sendSrv(csc_http_t *msg, csc_ioAnyWrite_t *out)
{   
// The protocol.
    char *protocol = msg->startFields[csc_httpSF_protocol];
    if (protocol == NULL) 
        protocol = "HTTP/1.1";
 
// The status code.
    char *statCode = msg->startFields[csc_httpSF_statCode];
    if (statCode == NULL) 
    {   setErr( msg
              , csc_httpErr_MissingStatCode
              , "Missing status code for response line"
              );
        return csc_httpErr_MissingStatCode;
    }
 
// The reason.
    char *reason = msg->startFields[csc_httpSF_reason];
    if (reason == NULL) 
    {   setErr(msg, csc_httpErr_MissingReason, "Missing reason for response line");
        return csc_httpErr_MissingReason;
    }
 
// Send the protocol.
    csc_ioAnyWrite_puts(out, protocol);
    csc_ioAnyWrite_puts(out, " ");
 
// Send the status code.
    csc_ioAnyWrite_puts(out, statCode);
    csc_ioAnyWrite_puts(out, " ");
 
// Send the reason.
    csc_ioAnyWrite_puts(out, reason);
    csc_ioAnyWrite_puts(out, "\r\n");
 
// Send each header.
    const char *val = NULL;
    csc_nameVal_t **els = msg->headers->els;
    int nEls = msg->headers->nEls;
    for (int i=0; i<nEls; i++)
    {   csc_nameVal_t *nv = els[i];
        csc_ioAnyWrite_puts(out, nv->name);
        csc_ioAnyWrite_puts(out, ": ");
        csc_ioAnyWrite_puts(out, nv->val);
        csc_ioAnyWrite_puts(out, "\r\n");
    }
 
// Send a blank line to terminate.
    csc_ioAnyWrite_puts(out, "\r\n");
 
// Bye.
    return csc_httpErr_Ok;
}


csc_httpErr_t csc_http_sendSrvFILE(csc_http_t *msg, FILE* fout)
{   csc_ioAnyWrite_t *out = csc_ioAnyWrite_new(csc_ioAny_writeFILE, fout);
    csc_httpErr_t errCode = csc_http_sendSrv(msg, out);
    csc_ioAnyWrite_free(out);
    return errCode;
}


csc_httpErr_t csc_http_sendSrvStr(csc_http_t *msg, csc_str_t *sout)
{   csc_ioAnyWrite_t *out = csc_ioAnyWrite_new(csc_ioAny_writeCstr, sout);
    csc_httpErr_t errCode = csc_http_sendSrv(msg, out);
    csc_ioAnyWrite_free(out);
    return errCode;
}
 
