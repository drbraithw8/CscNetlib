#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

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


typedef int (*readCharAnyFunc_t)(void *context);


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
{	readCharAnyFunc_t readChar;
	void *context;
} readCharAny_t;
readCharAny_t *readCharAny_new(readCharAnyFunc_t readChar, void *context)
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


typedef struct jsonParse_s
{	char *errStr;
	csc_jsonErr_t errNo;
	readCharAny_t rca;
	int ch;
} jsonParse_t;

jsonParse_t *jsonParse_new(readCharAny_t rca)
{	jsp = csc_allocOne(jsonParse_t);
	jsp->errStr = NULL;
	jsp->errNo = csc_jsonErr_Ok;
	jsp->rca = rca;
	jsp->ch = ' ';
	return jsp;
}

void jsonParse_free(jsonParse_t *jsp)
{	if (jsp->errStr != NULL)
		free(jsp->errStr);
	free(jsp);
}

const char *jsonParse_getReason(jsonParse_t *jsp)
{	return jsp->errStr;
}

void jsonParse_nextChar(jsonParse_t *jsp)
{	if (jsp->ch != -1)
		jsp->ch = readCharAny_getc(jsp->rca);
}

void jsonParse_skipSpace(jsonParse_t *jsp)
{	while(isspace(jsp->ch))
		jsonParse_nextChar(jsp);
}

csc_bool_t jsonParse_readNum(jsonParse_t *jsp, elem_t *el)
{	csc_bool_t isContinue = csc_TRUE;
	csc_str_t *numStr = csc_str_new();
	const char *nums;
	csc_bool_t retVal;
	int ch = jsp->ch;
 
// Read in a number.
	while (isContinue)
	{	switch(ch)
		{	case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9': 
			case '-': case '+': case '.': case 'e': case 'E':
				csc_str_append_ch(numStr, ch);
				ch = readCharAny_getc(jsp->rca);
				break;
			default:
				jsp->ch = ch;
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


csc_bool_t jsonParse_readPlainWord(jsonParse_t *jsp, elem_t *el)
{	csc_bool_t isContinue = csc_TRUE;
	csc_str_t *wordStr = csc_str_new();
	const char *word;
	csc_bool_t retVal;
	int ch = jsp->ch;
 
// Read in a number.
	while (isContinue)
	{	switch(ch)
		{	case 't': case 'r': case 'u': case 'e': case 'f':
			case 'a': case 'l': case 's': case 'n':
				csc_str_append_ch(wordStr, ch);
				ch = readCharAny_getc(jsp->rca);
				break;
			default:
				jsp->ch = ch;
				isContinue = csc_FALSE;
		}
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


csc_bool_t jsonParse_readString(jsonParse_t *jsp, elem_t *el)
{	csc_bool_t isContinue = csc_TRUE;
	csc_str_t *cStr = csc_str_new();
	csc_bool_t retVal;
	int ch;
 
// Read past the initial quote.
	ch = readCharAny_getc(jsp->rca);
	while (ch!=-1 && ch!='\"')
	{	
		if (ch == '\\')
		{	ch = readCharAny_getc(jsp->rca);
			switch(ch)
			{	case '\"':
					csc_str_append_ch(cStr, ch);
					break;
				case '\\':
					csc_str_append_ch(cStr, ch);
					break;
				case '/':
					csc_str_append_ch(cStr, ch);
					break;
				case 'b':
					csc_str_append_ch(cStr, '\b');
					break;
				case 'f':
					csc_str_append_ch(cStr, '\f');
					break;
				case 'n':
					csc_str_append_ch(cStr, '\n');
					break;
				case 'r':
					csc_str_append_ch(cStr, '\r');
					break;
				case 't':
					csc_str_append_ch(cStr, '\t');
					break;
				case 'u':
	

"truefalsn
}


	
				
				
	
//




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

