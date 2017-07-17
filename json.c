// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#include <stdio.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#include "std.h"
#include "alloc.h"
#include "isvalid.h"
#include "dynArray.h"
#include "json.h"

#define MaxIntLen 22
#define MaxFloatLen 44

typedef union val_u
{	int iVal;
	double fVal;
	csc_bool_t bVal;
	char *sVal;
	csc_json_t *oVal;
	csc_jsonArr_t *aVal;
} val_t;

typedef struct elem_s
{	csc_jsonType_t type;
	char *name;
	val_t val;
} elem_t;

typedef struct csc_json_s
{   elem_t *els;  
    int nEls;
    int mEls;
	csc_jsonErr_t errNum;
	char *errStr;
	int errPos;
	int errLinePos;
} csc_json_t;

typedef void (*writeStr_t)(void *context, const char *str);


static void elem_free(elem_t *el)
{
// Free the value.
	if (el->type == csc_jsonType_String)
	{	free(el->val.sVal);
	}
	else if (el->type == csc_jsonType_Obj) 
	{	csc_json_free(el->val.oVal);
	}	
	else if (el->type == csc_jsonType_Arr) 
	{	csc_jsonArr_free(el->val.aVal);
	}	
 
// Free the name.
	if (el->name != NULL)
		free(el->name);
}


csc_json_t *csc_json_new()
{   csc_json_t *js = csc_allocOne(csc_json_t);
    js->els = NULL;
    js->nEls = 0;
    js->mEls = 0;
	js->errNum = csc_jsonErr_Ok;
	js->errStr = NULL;
	js->errPos = 0;
	js->errLinePos = 0;
	return js;
}
csc_jsonArr_t *csc_jsonArr_new()
{	return (csc_jsonArr_t*)csc_json_new();
}


void csc_json_free(csc_json_t *js)
{	elem_t *els = js->els;
	
// Free the elements.
	if (els != NULL)
	{
	// Free stuff from each element.
		int nEls = js->nEls;
		for (int i=0; i<nEls; i++)
		{	elem_free(&els[i]);
		}
 
	// Free the elements.
		free(els);
	}

// Release error messages.
	if (js->errStr)
		free(js->errStr);
 
// Free the object.
	free(js);
}
void csc_jsonArr_free(csc_jsonArr_t *jas)
{	csc_json_free((csc_json_t*)jas);
}


static void csc_json_setErr(csc_json_t *js, const char *errMsg, int errPos, int errLinePos)
{	if (js->errStr)
		free(js->errStr);
	js->errStr = csc_alloc_str(errMsg);
	js->errNum = csc_jsonErr_BadParse;
	js->errPos = errPos;
	js->errLinePos = errLinePos;
}
int csc_json_getErrPos(csc_json_t *js)
{	return js->errPos;
}
int csc_json_getErrLinePos(csc_json_t *js)
{	return js->errLinePos;
}
const char *csc_json_getErrStr(csc_json_t *js)
{	return js->errStr;
}


static void csc_json_addVal(csc_json_t *js, csc_jsonType_t type, const char *name, val_t val)
{	
// Make the element.
	elem_t *el;
 
// Expand the dynamic array if needed.
	if (js->nEls == js->mEls)
    {   js->mEls = js->mEls * 2 + 10;
        js->els = csc_ck_ralloc(js->els, js->mEls*sizeof(elem_t));
    }
 
// Assign the element.
	el = &js->els[js->nEls++];
	el->type = type;
	el->val = val;
	if (name == NULL)
		el->name = NULL;
	else
		el->name = csc_alloc_str(name);
}


static elem_t *findByName(const csc_json_t *js, char *name)
{	elem_t *els = js->els;
	int nEls = js->nEls;
	int i;
	for (i=0; i<nEls; i++)
	{	char *eName = els[i].name;
		if (eName!=NULL && csc_streq(eName, name))
			break;
	}
	if (i < nEls)
		return &els[i];
	else
		return NULL;
}


static elem_t *findByIndex(const csc_json_t *js, int ndx)
{	if (ndx<0 || ndx>js->nEls)
		return NULL;
	else
		return &js->els[ndx];
}


void csc_json_addNull(csc_json_t *js, char *name)
{	val_t v;
	v.bVal = csc_FALSE;
	csc_json_addVal(js, csc_jsonType_Null, name, v);
}
void csc_jsonArr_apndNull(csc_jsonArr_t *jas)
{	csc_json_addNull((csc_json_t*)jas, NULL);
}


void csc_json_addBool(csc_json_t *js, char *name, csc_bool_t val)
{	val_t v;
	v.bVal = val;
	csc_json_addVal(js, csc_jsonType_Bool, name, v);
}
void csc_jsonArr_apndBool(csc_jsonArr_t *jas, csc_bool_t val)
{	csc_json_addBool((csc_json_t*)jas, NULL, val);
}


void csc_json_addInt(csc_json_t *js, char *name, int val)
{	val_t v;
	v.iVal = val;
	csc_json_addVal(js, csc_jsonType_Int, name, v);
}
void csc_jsonArr_apndInt(csc_jsonArr_t *jas, int val)
{	csc_json_addInt((csc_json_t*)jas, NULL, val);
}


void csc_json_addFloat(csc_json_t *js, char *name, double val)
{	val_t v;
	v.fVal = val;
	csc_json_addVal(js, csc_jsonType_Float, name, v);
}
void csc_jsonArr_apndFloat(csc_jsonArr_t *jas, double val)
{	csc_json_addFloat((csc_json_t*)jas, NULL, val);
}


void csc_json_addStr(csc_json_t *js, char *name, const char *val)
{	val_t v;
	if (val == NULL)
		v.sVal = NULL;
	else
		v.sVal = csc_alloc_str(val);
	csc_json_addVal(js, csc_jsonType_String, name, v);
}
void csc_jsonArr_apndStr(csc_jsonArr_t *jas, const char *val)
{	csc_json_addStr((csc_json_t*)jas, NULL, val);
}


void csc_json_addObj(csc_json_t *js, char *name, csc_json_t *val)
{	val_t v;
	v.oVal = val;
	csc_json_addVal(js, csc_jsonType_Obj, name, v);
}
void csc_jsonArr_apndObj(csc_jsonArr_t *jas, csc_json_t *val)
{	csc_json_addObj((csc_json_t*)jas, NULL, val);
}


void csc_json_addArr(csc_json_t *js, char *name, csc_jsonArr_t *val)
{	val_t v;
	v.aVal = val;
	csc_json_addVal(js, csc_jsonType_Arr, name, v);
}
void csc_jsonArr_apndArr(csc_jsonArr_t *jas, csc_jsonArr_t *val)
{	csc_json_addArr((csc_json_t*)jas, NULL, val);
}


int csc_json_length(const csc_json_t *js)
{	return js->nEls;
}
int csc_jsonArr_length(const csc_jsonArr_t *jas)
{	return csc_json_length((csc_json_t*)jas);
}


csc_jsonType_t csc_json_getType(const csc_json_t *js, char *name)
{	elem_t *el = findByName(js, name);
	if (el == NULL)
		return csc_jsonType_Missing;
	else
		return el->type;
}
csc_jsonType_t csc_json_ndxType(const csc_json_t *js, int ndx)
{	elem_t *el = findByIndex(js, ndx);
	if (el == NULL)
		return csc_jsonType_Missing;
	else
		return el->type;
}
csc_jsonType_t csc_jsonArr_getType(const csc_jsonArr_t *jas, int ndx)
{	return csc_json_ndxType((const csc_json_t *)jas, ndx);
}
const char *csc_json_ndxName(const csc_json_t *js, int ndx)
{	elem_t *el = findByIndex(js, ndx);
	if (el == NULL)
		return NULL;
	else
		return el->name;
}


static int getBool(const elem_t *el,  csc_jsonErr_t *errNum)
{	if (el == NULL)
	{	*errNum = csc_jsonErr_Missing;
		return csc_FALSE;
	}
	else if (el->type == csc_jsonType_Bool)
	{	*errNum = csc_jsonErr_Ok;
		return el->val.bVal;
	}
	else if (el->type == csc_jsonType_Null)
	{	*errNum = csc_jsonErr_Null;
		return csc_FALSE;
	}
	else
	{	*errNum = csc_jsonErr_WrongType;
		return csc_FALSE;
	}
}
csc_bool_t csc_json_getBool(const csc_json_t *js, char *name, csc_jsonErr_t *errNum)
{	return getBool(findByName(js,name), errNum);
}
csc_bool_t csc_json_ndxBool(const csc_json_t *js, int ndx, csc_jsonErr_t *errNum)
{	return getBool(findByIndex(js,ndx), errNum);
}
csc_bool_t csc_jsonArr_getBool(const csc_jsonArr_t *jas, int ndx, csc_jsonErr_t *errNum)
{	return csc_json_ndxBool((csc_json_t*)jas, ndx, errNum);
}


static int getInt(const elem_t *el,  csc_jsonErr_t *errNum)
{	if (el == NULL)
	{	*errNum = csc_jsonErr_Missing;
		return 0;
	}
	else if (el->type == csc_jsonType_Int)
	{	*errNum = csc_jsonErr_Ok;
		return el->val.iVal;
	}
	else if (el->type == csc_jsonType_Null)
	{	*errNum = csc_jsonErr_Null;
		return 0;
	}
	else
	{	*errNum = csc_jsonErr_WrongType;
		return 0;
	}
}
int csc_json_getInt(const csc_json_t *js, char *name, csc_jsonErr_t *errNum)
{	return getInt(findByName(js,name), errNum);
}
int csc_json_ndxInt(const csc_json_t *js, int ndx, csc_jsonErr_t *errNum)
{	return getInt(findByIndex(js,ndx), errNum);
}
int csc_jsonArr_getInt(const csc_jsonArr_t *jas, int ndx, csc_jsonErr_t *errNum)
{	return csc_json_ndxInt((csc_json_t*)jas, ndx, errNum);
}


static double getFloat(const elem_t *el,  csc_jsonErr_t *errNum)
{	if (el == NULL)
	{	*errNum = csc_jsonErr_Missing;
		return 0;
	}
	else if (el->type == csc_jsonType_Int)
	{	*errNum = csc_jsonErr_Ok;
		return el->val.iVal;
	}
	else if (el->type == csc_jsonType_Float)
	{	*errNum = csc_jsonErr_Ok;
		return el->val.fVal;
	}
	else if (el->type == csc_jsonType_Null)
	{	*errNum = csc_jsonErr_Null;
		return 0;
	}
	else
	{	*errNum = csc_jsonErr_WrongType;
		return 0;
	}
}
double csc_json_getFloat(const csc_json_t *js, char *name, csc_jsonErr_t *errNum)
{	return getFloat(findByName(js,name), errNum);
}
double csc_json_ndxFloat(const csc_json_t *js, int ndx, csc_jsonErr_t *errNum)
{	return getFloat(findByIndex(js,ndx), errNum);
}
double csc_jsonArr_getFloat(const csc_jsonArr_t *jas, int ndx, csc_jsonErr_t *errNum)
{	return csc_json_ndxFloat((csc_json_t*)jas, ndx, errNum);
}


static const char *getStr(const elem_t *el,  csc_jsonErr_t *errNum)
{	if (el == NULL)
	{	*errNum = csc_jsonErr_Missing;
		return NULL;
	}
	else if (el->type == csc_jsonType_String)
	{	*errNum = csc_jsonErr_Ok;
		return el->val.sVal;
	}
	else if (el->type == csc_jsonType_Null)
	{	*errNum = csc_jsonErr_Null;
		return NULL;
	}
	else
	{	*errNum = csc_jsonErr_WrongType;
		return NULL;
	}
}
const char *csc_json_getStr(const csc_json_t *js, char *name, csc_jsonErr_t *errNum)
{	return getStr(findByName(js,name), errNum);
}
const char *csc_json_ndxStr(const csc_json_t *js, int ndx, csc_jsonErr_t *errNum)
{	return getStr(findByIndex(js,ndx), errNum);
}
const char *csc_jsonArr_getStr(const csc_jsonArr_t *jas, int ndx, csc_jsonErr_t *errNum)
{	return csc_json_ndxStr((csc_json_t*)jas, ndx, errNum);
}


static const csc_json_t *getObj(const elem_t *el,  csc_jsonErr_t *errNum)
{	if (el == NULL)
	{	*errNum = csc_jsonErr_Missing;
		return NULL;
	}
	else if (el->type == csc_jsonType_Obj)
	{	*errNum = csc_jsonErr_Ok;
		return el->val.oVal;
	}
	else if (el->type == csc_jsonType_Null)
	{	*errNum = csc_jsonErr_Null;
		return NULL;
	}
	else
	{	*errNum = csc_jsonErr_WrongType;
		return NULL;
	}
}
const csc_json_t *csc_json_getObj(const csc_json_t *js, char *name, csc_jsonErr_t *errNum)
{	return getObj(findByName(js,name), errNum);
}
const csc_json_t *csc_json_ndxObj(const csc_json_t *js, int ndx, csc_jsonErr_t *errNum)
{	return getObj(findByIndex(js,ndx), errNum);
}
const csc_json_t *csc_jsonArr_getObj(const csc_jsonArr_t *jas, int ndx, csc_jsonErr_t *errNum)
{	return csc_json_ndxObj((csc_json_t*)jas, ndx, errNum);
}


static const csc_jsonArr_t *getArr(const elem_t *el,  csc_jsonErr_t *errNum)
{	if (el == NULL)
	{	*errNum = csc_jsonErr_Missing;
		return NULL;
	}
	else if (el->type == csc_jsonType_Arr)
	{	*errNum = csc_jsonErr_Ok;
		return el->val.aVal;
	}
	else if (el->type == csc_jsonType_Null)
	{	*errNum = csc_jsonErr_Null;
		return NULL;
	}
	else
	{	*errNum = csc_jsonErr_WrongType;
		return NULL;
	}
}
const csc_jsonArr_t *csc_json_getArr(const csc_json_t *js, char *name, csc_jsonErr_t *errNum)
{	return getArr(findByName(js,name), errNum);
}
const csc_jsonArr_t *csc_json_ndxArr(const csc_json_t *js, int ndx, csc_jsonErr_t *errNum)
{	return getArr(findByIndex(js,ndx), errNum);
}
const csc_jsonArr_t *csc_jsonArr_getArr(const csc_jsonArr_t *jas, int ndx, csc_jsonErr_t *errNum)
{	return csc_json_ndxArr((csc_json_t*)jas, ndx, errNum);
}


static void writeInt(writeStr_t writer, void *context, int val)
{	char buf[MaxIntLen+1];
	sprintf(buf, "%d", val);
	writer(context, buf);
}

static void writeFloat(writeStr_t writer, void *context, double val)
{	char buf[MaxFloatLen+1];
	sprintf(buf, "%16g", val);
	char *p = buf;
	while (*p == ' ')
		p++;
	writer(context, p);
}

static void writeBool(writeStr_t writer, void *context, csc_bool_t val)
{	if (val)
		writer(context, "true");
	else
		writer(context, "false");
}

static void writeStr(writeStr_t writer, void *context, const char *val)
{	char buf[2] = { '\0', '\0' };
	const char *p = val;
	char ch;
	writer(context, "\"");
	while ((ch=*p++) != '\0')
	{	switch(ch)
		{	case '\"':
				writer(context, "\\\"");
				break;
			case '\\':
				writer(context, "\\\\");
				break;
			case '/':
				writer(context, "\\/");
				break;
			case '\n':
				writer(context, "\\\n");
				break;
			case '\r':
				writer(context, "\\\r");
				break;
			case '\f':
				writer(context, "\\\f");
				break;
			case '\b':
				writer(context, "\\\b");
				break;
			case '\t':
				writer(context, "\\\b");
				break;
			default:
				buf[0] = ch;
				writer(context, buf);
		}
	}
	writer(context, "\"");
}

static void writeArr(writeStr_t writer, void *context, const csc_jsonArr_t *jas);
static void writeObj(writeStr_t writer, void *context, const csc_json_t *js);

static void writeEl(writeStr_t writer, void *context, const elem_t *el)
{	if (el->type == csc_jsonType_Bool)
		writeBool(writer, context, el->val.bVal);
	else if (el->type == csc_jsonType_Int)
		writeInt(writer, context, el->val.iVal);
	else if (el->type == csc_jsonType_Float)
		writeFloat(writer, context, el->val.fVal);
	else if (el->type == csc_jsonType_String)
		writeStr(writer, context, el->val.sVal);
	else if (el->type == csc_jsonType_Obj)
		writeObj(writer, context, el->val.oVal);
	else if (el->type == csc_jsonType_Arr)
		writeArr(writer, context, el->val.aVal);
	else
		writer(context, "null");
}

static void writeObj(writeStr_t writer, void *context, const csc_json_t *js)
{	int nEls = js->nEls;
	elem_t *els = js->els;
	writer(context, "{");
	for (int i=0; i<nEls; i++)
	{	writer(context, "\"");
		writer(context, els[i].name);
		writer(context, "\":");
		writeEl(writer, context, &els[i]);
		if (i < nEls-1)
			writer(context, ",");
	}
	writer(context, "}");
}

static void writeArr(writeStr_t writer, void *context, const csc_jsonArr_t *jas)
{	csc_json_t *js = (csc_json_t*)jas;
	int nEls = js->nEls;
	elem_t *els = js->els;
	writer(context, "[");
	for (int i=0; i<nEls; i++)
	{	writeEl(writer, context, &els[i]);
		if (i < nEls-1)
			writer(context, ",");
	}
	writer(context, "]");
}


static void writeFILE(void *context, const char *str)
{	fprintf((FILE*)context, "%s", str);
}
void csc_json_writeFILE(const csc_json_t *js, FILE *fout)
{	writeObj(writeFILE, (void*)fout, js);
}

static void writeCstr(void *context, const char *str)
{	csc_str_append((csc_str_t*)context, str);
}
void csc_json_writeCstr(const csc_json_t *js, csc_str_t *cstr)
{	writeObj(writeCstr, (void*)cstr, js);
}



typedef int (*readCharAnyFunc_t)(void *context);


// Class to that can read single chars from a string.
typedef struct readCharStr_s
{	const char *str;
	const char *p;
} readCharStr_t;
static readCharStr_t *readCharStr_new(const char *str)
{	readCharStr_t *rcs = csc_allocOne(readCharStr_t);
	rcs->str = str;
	rcs->p = str;
}
static void readCharStr_free(readCharStr_t *rcs)
{	free(rcs);
}
static int readCharStr_getc(readCharStr_t *rcs)
{	int ch = *rcs->p++;
	if (ch == '\0')
	{	rcs->p--;
		return EOF;
	}
	else
		return ch;
}

// Class to that can read single chars from whatever.
typedef struct readCharAny_s
{	readCharAnyFunc_t readChar;
	void *context;
} readCharAny_t;
static readCharAny_t *readCharAny_new(readCharAnyFunc_t readChar, void *context)
{	readCharAny_t *rca = csc_allocOne(readCharAny_t);
	rca->readChar = readChar;
	rca->context = context;
}
static void readCharAny_free(readCharAny_t *rca)
{	free(rca);
}
static int readCharAny_getc(readCharAny_t *rca)
{	return rca->readChar(rca->context);
}


// Reading from a string.
static int readCharStr(void *context)
{	return readCharStr_getc((readCharStr_t*)context);
}

// Reading from a file.
static int readCharFile(void *context)
{	return getc((FILE*)context);
}


typedef struct jsonParse_s
{	char *errStr;
	csc_jsonErr_t errNo;
	int charPos;
	int lineNo;
	readCharAny_t *rca;
	int ch;
} jsonParse_t;

static jsonParse_t *jsonParse_new(readCharAny_t *rca)
{	jsonParse_t *jsp = csc_allocOne(jsonParse_t);
	jsp->errStr = NULL;
	jsp->errNo = csc_jsonErr_Ok;
	jsp->rca = rca;
	jsp->ch = ' ';
	jsp->charPos = 0;
	jsp->lineNo = 1;
	return jsp;
}

static void jsonParse_free(jsonParse_t *jsp)
{	if (jsp->errStr != NULL)
		free(jsp->errStr);
	free(jsp);
}

static const char *jsonParse_getReason(jsonParse_t *jsp)
{	return jsp->errStr;
}

static int jsonParse_nextChar(jsonParse_t *jsp)
{	int ch;
	if (jsp->ch != EOF)
	{	ch = jsp->ch = readCharAny_getc(jsp->rca);
		// putc(ch, stderr); // csc_CKCK; // debugging.
		if (ch == '\n')
			jsp->lineNo++;
		jsp->charPos++;
	}
	return ch;
}

static int jsonParse_skipSpace(jsonParse_t *jsp)
{	int ch = jsp->ch;
	while(isspace(ch))
		ch = jsonParse_nextChar(jsp);
	return ch;
}

static csc_bool_t jsonParse_readNum(jsonParse_t *jsp, elem_t *el)
{	csc_bool_t isContinue = csc_TRUE;
	csc_str_t *numStr = csc_str_new(NULL);
	const char *nums;
	csc_bool_t retVal;
 
// Assumes that we are looking at the first digit of a number.
	int ch = jsp->ch;
	assert(isdigit(ch));
 
// Read in a number.
	while (isContinue)
	{	switch(ch)
		{	case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9': 
			case '-': case '+': case '.': case 'e': case 'E':
				csc_str_append_ch(numStr, ch);
				ch = jsonParse_nextChar(jsp);
				break;
			default:
				isContinue = csc_FALSE;
		}
	}
 
// Look at this number.
	nums = csc_str_charr(numStr);
	if (csc_isValid_int(nums))
	{	retVal = csc_TRUE;
		el->val.iVal = atoi(nums);
		el->type = csc_jsonType_Int;
	}
	else if (csc_isValid_float(nums))
	{	retVal = csc_TRUE;
		el->val.fVal = atof(nums);
		el->type = csc_jsonType_Float;
	}
	else
	{	retVal = csc_FALSE;
		el->type = csc_jsonType_Bad;
	}
 
// Free resources.
	csc_str_free(numStr);
 
// Byte.
	return retVal;
}

 
static csc_bool_t jsonParse_readPlainWord(jsonParse_t *jsp, elem_t *el)
{	csc_str_t *wordStr = csc_str_new(NULL);
	const char *word;
	csc_bool_t retVal;
 
// Assumes that we are looking at the first character of a word.
	int ch = jsp->ch;
	assert(islower(ch));
 
// Read in a word.
	while (islower(ch))
	{	csc_str_append_ch(wordStr, ch);
		ch = jsonParse_nextChar(jsp);
	}
 
// Look at this word.
	word = csc_str_charr(wordStr);
	if (csc_streq(word,"true"))
	{	retVal = csc_TRUE;
		el->val.bVal = csc_TRUE;
		el->type = csc_jsonType_Bool;
	}
	else if (csc_streq(word,"false"))
	{	retVal = csc_TRUE;
		el->val.bVal = csc_FALSE;
		el->type = csc_jsonType_Bool;
	}
	else if (csc_streq(word,"null"))
	{	retVal = csc_TRUE;
		el->type = csc_jsonType_Null;
	}
	else
	{	retVal = csc_FALSE;
		el->type = csc_jsonType_Bad;
	}
 
// Free resources.
	csc_str_free(wordStr);
 
// Byte.
	return retVal;
}


static int jsonParse_readHexStr(jsonParse_t *jsp, char *hexStr, int hexStrMaxLen)
{	int ch = jsp->ch;
	int n;
 
// Assumes that first hex digit has been read in.
	assert(isxdigit(ch));
 
// Read in the string.
	n = 0;
	while (n<hexStrMaxLen && isxdigit(ch))
	{	hexStr[n++] = ch;
		ch = jsonParse_nextChar(jsp);
	}
	hexStr[n] = '\0';
 
// Dont skip past any more hex digits, cos they are a legit in a string.
	// while (isxdigit(ch))
	// {	n++;
	// 	ch = jsonParse_nextChar(jsp);
	// }
 
// Bye.
	return n;
}


static csc_bool_t jsonParse_readString(jsonParse_t *jsp, csc_str_t *str)
{	csc_bool_t isContinue = csc_TRUE;
	csc_bool_t isOK = csc_TRUE;
	csc_bool_t isGetNext;
 
// Assumes we have the initial quote of a string.
	int ch = jsp->ch;
	assert(ch == '\"');
	ch = jsonParse_nextChar(jsp);
 
// Read in the string.
	while (isContinue)
	{	isGetNext = csc_TRUE;
		if (ch == '\"')
		{	isContinue = csc_FALSE;
			jsonParse_nextChar(jsp);
			isGetNext = csc_FALSE;
		}
		else if (ch == EOF)
		{	isContinue = csc_FALSE;
			isOK = csc_FALSE;
			isGetNext = csc_FALSE;
		}
		else if (ch == '\\')
		{	ch = jsonParse_nextChar(jsp);
			switch(ch)
			{	case '\"':
					csc_str_append_ch(str, ch);
					break;
				case '\\':
					csc_str_append_ch(str, ch);
					break;
				case '/':
					csc_str_append_ch(str, ch);
					break;
				case 'b':
					csc_str_append_ch(str, '\b');
					break;
				case 'f':
					csc_str_append_ch(str, '\f');
					break;
				case 'n':
					csc_str_append_ch(str, '\n');
					break;
				case 'r':
					csc_str_append_ch(str, '\r');
					break;
				case 't':
					csc_str_append_ch(str, '\t');
					break;
				case 'u':
				{	const int HexStrMaxLen = 4;
					char hexStr[HexStrMaxLen+1];
					ch = jsonParse_nextChar(jsp);
					int nHexDigits = jsonParse_readHexStr(jsp, hexStr, HexStrMaxLen);
					if (nHexDigits == 4)
					{	csc_str_append(str, "$@$");  // TODO: Convert to UTF-8.
					}
					else
					{	csc_str_append(str, "$#$");  // TODO: 
					}
					isGetNext = csc_FALSE;
					break;
				}
				case EOF:
					isGetNext = csc_FALSE;
					break;
				default:
					csc_str_append_ch(str, ch);
					break;
			}
		}
		else
		{	csc_str_append_ch(str, ch);
		}
		if (isGetNext)
			ch = jsonParse_nextChar(jsp);
	}
 
 	return isOK;
}


static csc_bool_t jsonParse_readIdent(jsonParse_t *jsp, csc_str_t *str)
{	csc_bool_t isOK = csc_TRUE;
	int ch = jsp->ch;
 
// Reset the string.
    csc_str_assign(str, NULL);
 
// Get the identifier.
	if (ch == '\"')
	{	isOK = jsonParse_readString(jsp, str);
	}
	else if (isalpha(ch))
	{	while (isalnum(ch) || ch=='_')
		{	csc_str_append_ch(str, ch);
			ch = jsonParse_nextChar(jsp);
		}
	}
	else
	{	isOK = csc_FALSE;
	}
 
// Return the result.
	return isOK;
}


static csc_bool_t jsonParse_readStringEl(jsonParse_t *jsp, elem_t *el)
{	
// Allocate resources.
	csc_str_t *str = csc_str_new(NULL);
 
// Read in the string.
	csc_bool_t isOK = jsonParse_readString(jsp, str);
 
// Assign the result.
	if (isOK)
	{	el->type = csc_jsonType_String;
		el->val.sVal = csc_str_alloc_charr(str);
	}
	else
	{	el->type = csc_jsonType_Bad;
		el->val.sVal = NULL;
	}
 
// Free resources.
	csc_str_free(str);
 
// Return the result.
	return isOK;
}


static csc_json_t *jsonParse_readObj(jsonParse_t *jsp);
static csc_jsonArr_t *jsonParse_readArr(jsonParse_t *jsp);


static csc_bool_t jsonParse_readElem(jsonParse_t *jsp, elem_t *el)
{	int ch = jsonParse_skipSpace(jsp);
	if (islower(ch))
	{
		return jsonParse_readPlainWord(jsp, el);
	}
	else if (isdigit(ch))
	{
		return jsonParse_readNum(jsp, el);
	}
	else if (ch == '\"')
	{
		return jsonParse_readStringEl(jsp, el);
	}
	else if (ch == '{')
	{
		csc_json_t *obj = jsonParse_readObj(jsp);
		if (csc_json_getErrStr(obj) == NULL)
		{
			el->type = csc_jsonType_Obj; 
			el->val.oVal = obj;
			return csc_TRUE;
		}
		else
		{
			el->type = csc_jsonType_Obj; 
			el->val.oVal = obj;
			return csc_FALSE;
		}
	}
	else if (ch == '[')
	{
		csc_jsonArr_t *arr = jsonParse_readArr(jsp);
		if (csc_json_getErrStr((csc_json_t*)arr) == NULL)
		{
			el->type = csc_jsonType_Arr; 
			el->val.aVal = arr;
			return csc_TRUE;
		}
		else
		{
			el->type = csc_jsonType_Arr; 
			el->val.aVal = arr;
			return csc_FALSE;
		}
	}
	else
	{
		return csc_FALSE;
	}
}


static csc_jsonArr_t *jsonParse_readArr(jsonParse_t *jsp)
{	int isOK = csc_TRUE;	
	csc_bool_t isContinue = csc_TRUE;
 
// Allocate resources.
	csc_jsonArr_t *arr = csc_jsonArr_new();
 
// Assumes that we are looking at the opening brace of an object.
	int ch = jsp->ch;
	assert(ch == '[');
 
// Look at next character.
	jsonParse_nextChar(jsp);
	ch = jsonParse_skipSpace(jsp);
	while (isContinue)
	{
		if (ch == ']')
		{
			ch = jsonParse_nextChar(jsp);
			isContinue = csc_FALSE;
		}
		else
		{
		// Now we expect an element.
			ch = jsonParse_skipSpace(jsp);
			elem_t elem;
			isOK = jsonParse_readElem(jsp, &elem);
			if (!isOK)
			{	csc_json_setErr((csc_json_t*)arr, "Expected Element", jsp->charPos, jsp->lineNo);
				break;
			}
 
		// Add element to the object.
			csc_json_addVal((csc_json_t*)arr, elem.type, NULL, elem.val);
 
		// Now we expect a comma or an ending bracket.
			ch = jsonParse_skipSpace(jsp);
			if (ch == ',')
			{	jsonParse_nextChar(jsp);
				ch = jsonParse_skipSpace(jsp);
			}
			else if (ch == ']')
			{
			}
			else
			{	csc_json_setErr((csc_json_t*)arr, "Expected comma or ending brace", jsp->charPos, jsp->lineNo);
				csc_CKCK; printf("ch=\'%c\'\n", ch);
				break;
			}
 
		}
	}
 
// Return result.
	return arr;
}
			


static csc_json_t *jsonParse_readObj(jsonParse_t *jsp)
{	int isOK = csc_TRUE;	
	csc_bool_t isContinue = csc_TRUE;
 
// Allocate resources.
	csc_json_t *obj = csc_json_new();
	csc_str_t *ident = csc_str_new(NULL);
 
// Assumes that we are looking at the opening brace of an object.
	int ch = jsp->ch;
	assert(ch == '{');
 
// Look at next character.
	jsonParse_nextChar(jsp);
	ch = jsonParse_skipSpace(jsp);
	while (isContinue)
	{
		if (ch == '}')
		{
			ch = jsonParse_nextChar(jsp);
			isContinue = csc_FALSE;
		}
		else if (ch=='\"' || isalpha(ch))
		{
			isOK = jsonParse_readIdent(jsp, ident);
			if (!isOK)
			{
				csc_json_setErr(obj, "Expected Ident", jsp->charPos, jsp->lineNo);
				break;
			}
 
		// Now we expect a colon.
			ch = jsonParse_skipSpace(jsp);
			if (ch != ':')
			{
				isOK = csc_FALSE;
				csc_json_setErr(obj, "Expected Colon", jsp->charPos, jsp->lineNo);
				break;
			}
			jsonParse_nextChar(jsp);
 
		// Now we expect an element.
			ch = jsonParse_skipSpace(jsp);
			elem_t elem;
			isOK = jsonParse_readElem(jsp, &elem);
			if (!isOK)
			{	csc_json_setErr(obj, "Expected Element", jsp->charPos, jsp->lineNo);
				break;
			}
 
		// Add element to the object.
			csc_json_addVal(obj, elem.type, csc_str_charr(ident), elem.val);
 
		// Now we expect a comma or an ending brace.
			ch = jsonParse_skipSpace(jsp);
			if (ch == ',')
			{	jsonParse_nextChar(jsp);
				ch = jsonParse_skipSpace(jsp);
			}
			else if (ch == '}')
			{
			}
			else
			{	csc_json_setErr(obj, "Expected comma or ending brace", jsp->charPos, jsp->lineNo);
				csc_CKCK; printf("ch=\'%c\'\n", ch);
				break;
			}
 
		}
		else  // No ending brace and no identifier.
		{	isOK = csc_FALSE;
			csc_json_setErr( obj, "Expected ending brace or new identifier"
						   , jsp->charPos, jsp->lineNo
						   );
				csc_CKCK; printf("ch=\'%c\'\n", ch);
			isContinue = csc_FALSE;
		}
	}
 
// Free resources.
	csc_str_free(ident);
 
// Return result.
	return obj;
}


csc_json_t *csc_json_newParseStr(const char *str)
{
// Allocate resources.
	readCharStr_t *rcs = readCharStr_new(str);
	readCharAny_t *rca = readCharAny_new(readCharStr, (void*)rcs);
	jsonParse_t *jsp = jsonParse_new(rca);
 
// Parse the object.
	jsonParse_skipSpace(jsp);
	csc_json_t *js = jsonParse_readObj(jsp);
 
// Free resources.
	jsonParse_free(jsp);
	readCharAny_free(rca);
	readCharStr_free(rcs);
 
// Return.
	return js;
}


csc_json_t *csc_json_newParseFILE(FILE *fin)
{
// Allocate resources.
	readCharAny_t *rca = readCharAny_new(readCharFile, (void*)fin);
	jsonParse_t *jsp = jsonParse_new(rca);
 
// Parse the object.
	jsonParse_skipSpace(jsp);
	csc_json_t *js = jsonParse_readObj(jsp);
 
// Free resources.
	jsonParse_free(jsp);
	readCharAny_free(rca);
 
// Return.
	return js;
}


// void main(int argc, char **argv)
// {	char *str = "{ name: \"fred\", age: 23, isMale:false, mary:null\n"
// 				", stats:{ height: 45, weight:35.45}\n"
// 				", tharr:[ 1, \"ksd\\\"jf\", {}, false ]\n"
// 				"}";
// 	csc_json_t *js = csc_json_newParseStr(str);
// 	const char *errStr = csc_json_getErrStr(js);
// 	if (errStr)
// 	{	printf("Error:\"%s\"\n", errStr);
// 		printf("\t position:\"%d\"  ", csc_json_getErrPos(js));
// 		printf("line:\"%d\"\n", csc_json_getErrLinePos(js));
// 	}
// 	else
// 	{	csc_json_writeFILE(js, stdout);
// 	}
// 	csc_json_free(js);
// 	fprintf(stdout, "\n");
// 	exit(0);
// }


// void main(int argc, char **argv)
// {	int ch;
//  
// 	// Test reading from a string.
// 	{	char *testStr = "Hello from a string.\n";
// 		readCharStr_t *rcs = readCharStr_new(testStr);
// 		readCharAny_t *rca = readCharAny_new(readCharStr, (void*)rcs);
// 		while ((ch=readCharAny_getc(rca)) != EOF)
// 			putchar(ch);
// 		readCharAny_free(rca);
// 		readCharStr_free(rcs);
// 	}
//  
// 	// Test reading from a stream.
// 	{	readCharAny_t *rca = readCharAny_new(readCharFile, (void*)stdin);
// 		while ((ch=readCharAny_getc(rca)) != EOF)
// 			putchar(ch);
// 		readCharAny_free(rca);
// 	}
//  
// 	exit(0);
// }


#if 0
void main(int argc, char **argv)
{	csc_jsonErr_t errNum;
	csc_json_t *js = csc_json_new();
 
	csc_json_addInt(js, "fred", 7);
	csc_json_addInt(js, "mark", 37);
 
	csc_jsonArr_t *jas = csc_jsonArr_new();
	csc_jsonArr_apndInt(jas, 11);
	csc_jsonArr_apndInt(jas, 12);
	csc_jsonArr_apndInt(jas, 13);
	csc_json_addArr(js, "pvalues", jas);
 
	csc_json_addFloat(js, "freda", 7.1);
	csc_json_addNull(js, "lucy");
 
	csc_json_t *jane = csc_json_new();
	csc_json_addFloat(jane, "height", 1.45);
	csc_json_addFloat(jane, "weight", 31.5);
	csc_json_addObj(js, "pvalues", jane);
 
	csc_json_addBool(js, "Fido", csc_FALSE);
	csc_json_addStr(js, "Rover", "RoverS");
 
	int fred = csc_json_getInt(js, "fred", &errNum);
	printf("%d %d\n", fred, errNum);
	int mark = csc_json_getInt(js, "mark", &errNum);
	printf("%d %d\n", mark, errNum);
	int jack = csc_json_getInt(js, "jack", &errNum);
	printf("%d %d\n", jack, errNum);
 
	double freda = csc_json_getFloat(js, "freda", &errNum);
	printf("%f %d\n", freda, errNum);
	double lucy = csc_json_getFloat(js, "lucy", &errNum);
	printf("%f %d\n", lucy, errNum);
	double erza = csc_json_getFloat(js, "erza", &errNum);
	printf("%f %d\n", erza, errNum);
 
	const char *fido = csc_json_getStr(js, "Fido", &errNum);
	printf("\"%s\" %d\n", fido, errNum);
	const char *rover = csc_json_getStr(js, "Rover", &errNum);
	printf("\"%s\" %d\n", rover, errNum);
	const char *bo = csc_json_getStr(js, "Bo", &errNum);
	printf("\"%s\" %d\n\n", bo, errNum);
 
	csc_json_writeFILE(js, stdout);
	csc_str_t *cjs = csc_str_new(NULL);
	csc_json_writeCstr(js, cjs);
	printf("\n%s\n\n", csc_str_charr(cjs));
	csc_str_free(cjs);
 
	csc_json_free(js);
	exit(0);
}
#endif
