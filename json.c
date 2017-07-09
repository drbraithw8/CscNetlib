#include <stdio.h>

#include "std.h"
#include "alloc.h"
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
} csc_json_t;


typedef void (*writeStr_t)(void *context, const char *str);
typedef int (*readChar_t)(void *context);


// Class to that can read single chars from a string.
typedef struct readCharStr_s
{	const char *str;
	const char *p;
} readCharStr_t;
readCharStr_t *readCharStr_new(const char *str)
{	readCharStr_t *rcs = csc_allocOne(readCharStr_t);
	rcs->str = str;
	rcs->p = str;
}
void readCharStr_free(readCharStr_t *rcs)
{	free(rcs);
}
int readCharStr_getc(readCharStr_t *rcs)
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
{	readChar_t readChar;
	void *context;
} readCharAny_t;
readCharAny_t *readCharAny_new(readChar_t readChar, void *context)
{	readCharAny_t *rca = csc_allocOne(readCharAny_t);
	rca->readChar = readChar;
	rca->context = context;
}
void readCharAny_free(readCharAny_t *rca)
{	free(rca);
}
int readCharAny_getc(readCharAny_t *rca)
{	return rca->readChar(rca->context);
}


// Reading from a string.
int readCharStr(void *context)
{	return readCharStr_getc((readCharStr_t*)context);
}

// Reading from a file.
int readCharFile(void *context)
{	return getc((FILE*)context);
}


void main(int argc, char **argv)
{	int ch;
 
	// Test reading from a string.
	{	char *testStr = "Hello from a string.\n";
		readCharStr_t *rcs = readCharStr_new(testStr);
		readCharAny_t *rca = readCharAny_new(readCharStr, (void*)rcs);
		while ((ch=readCharAny_getc(rca)) != EOF)
			putchar(ch);
		readCharAny_free(rca);
		readCharStr_free(rcs);
	}
 
	// Test reading from a stream.
	{	readCharAny_t *rca = readCharAny_new(readCharFile, (void*)stdin);
		while ((ch=readCharAny_getc(rca)) != EOF)
			putchar(ch);
		readCharAny_free(rca);
	}
 
	exit(0);
}


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
 
// Free the object.
	free(js);
}
void csc_jsonArr_free(csc_jsonArr_t *jas)
{	csc_json_free((csc_json_t*)jas);
}


static void csc_json_add(csc_json_t *js, csc_jsonType_t type, char *name, val_t val)
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
	csc_json_add(js, csc_jsonType_Null, name, v);
}
void csc_jsonArr_apndNull(csc_jsonArr_t *jas)
{	csc_json_addNull((csc_json_t*)jas, NULL);
}


void csc_json_addBool(csc_json_t *js, char *name, csc_bool_t val)
{	val_t v;
	v.bVal = val;
	csc_json_add(js, csc_jsonType_Bool, name, v);
}
void csc_jsonArr_apndBool(csc_jsonArr_t *jas, csc_bool_t val)
{	csc_json_addBool((csc_json_t*)jas, NULL, val);
}


void csc_json_addInt(csc_json_t *js, char *name, int val)
{	val_t v;
	v.iVal = val;
	csc_json_add(js, csc_jsonType_Int, name, v);
}
void csc_jsonArr_apndInt(csc_jsonArr_t *jas, int val)
{	csc_json_addInt((csc_json_t*)jas, NULL, val);
}


void csc_json_addFloat(csc_json_t *js, char *name, double val)
{	val_t v;
	v.fVal = val;
	csc_json_add(js, csc_jsonType_Float, name, v);
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
	csc_json_add(js, csc_jsonType_String, name, v);
}
void csc_jsonArr_apndStr(csc_jsonArr_t *jas, const char *val)
{	csc_json_addStr((csc_json_t*)jas, NULL, val);
}


void csc_json_addObj(csc_json_t *js, char *name, csc_json_t *val)
{	val_t v;
	v.oVal = val;
	csc_json_add(js, csc_jsonType_Obj, name, v);
}
void csc_jsonArr_apndObj(csc_jsonArr_t *jas, csc_json_t *val)
{	csc_json_addObj((csc_json_t*)jas, NULL, val);
}


void csc_json_addArr(csc_json_t *js, char *name, csc_jsonArr_t *val)
{	val_t v;
	v.aVal = val;
	csc_json_add(js, csc_jsonType_Arr, name, v);
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
{	writer(context, "\"");
	writer(context, val);
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
	{	writeStr(writer, context, els[i].name);
		writer(context, ":");
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


static void writeStream(void *context, const char *str)
{	fprintf((FILE*)context, "%s", str);
}
void csc_json_writeStream(const csc_json_t *js, FILE *fout)
{	writeObj(writeStream, (void*)fout, js);
}

static void writeCstr(void *context, const char *str)
{	csc_str_append((csc_str_t*)context, str);
}
void csc_json_writeCstr(const csc_json_t *js, csc_str_t *cstr)
{	writeObj(writeCstr, (void*)cstr, js);
}



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
 
	csc_json_writeStream(js, stdout);
	csc_str_t *cjs = csc_str_new(NULL);
	csc_json_writeCstr(js, cjs);
	printf("\n%s\n\n", csc_str_charr(cjs));
	csc_str_free(cjs);
 
	csc_json_free(js);
	exit(0);
}
#endif
