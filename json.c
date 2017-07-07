#include <stdio.h>
#include "std.h"
#include "alloc.h"

#include "json.h"
#include "dynArray.h"

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


static elem_t *findByName(csc_json_t *js, char *name)
{	elem_t *els = js->els;
	int nEls = js->nEls;
	int i;
	for (i=0; i<nEls; i++)
	{	if (csc_streq(els[i].name, name))
			break;
	}
	if (i < nEls)
		return &els[i];
	else
		return NULL;
}


static elem_t *findByIndex(csc_json_t *js, int ndx)
{	if (ndx<0 || ndx>js->nEls)
		return NULL;
	else
		return &js->els[ndx];
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


int csc_json_length(csc_json_t *js)
{	return js->nEls;
}
int csc_jsonArr_length(csc_jsonArr_t *jas)
{	return csc_json_length((csc_json_t*)jas);
}


csc_jsonType_t csc_json_getType(csc_json_t *js, char *name)
{	elem_t *el = findByName(js, name);
	if (el == NULL)
		return csc_jsonType_Missing;
	else
		return el->type;
}


static int getBool(elem_t *el,  csc_jsonErr_t *errNum)
{	if (el == NULL)
	{	*errNum = csc_jsonErr_Missing;
		return csc_FALSE;
	}
	else if (el->type != csc_jsonType_Bool)
	{	*errNum = csc_jsonErr_WrongType;
		return csc_FALSE;
	}
	else
	{	*errNum = csc_jsonErr_Ok;
		return el->val.bVal;
	}
}
csc_bool_t csc_json_getBool(csc_json_t *js, char *name, csc_jsonErr_t *errNum)
{	return getBool(findByName(js,name), errNum);
}
csc_bool_t csc_json_ndxBool(csc_json_t *js, int ndx, csc_jsonErr_t *errNum)
{	return getBool(findByIndex(js,ndx), errNum);
}
csc_bool_t csc_jsonArr_getBool(csc_jsonArr_t *jas, int ndx, csc_jsonErr_t *errNum)
{	return csc_json_ndxBool((csc_json_t*)jas, ndx, errNum);
}


static int getInt(elem_t *el,  csc_jsonErr_t *errNum)
{	if (el == NULL)
	{	*errNum = csc_jsonErr_Missing;
		return 0;
	}
	else if (el->type != csc_jsonType_Int)
	{	*errNum = csc_jsonErr_WrongType;
		return 0;
	}
	else
	{	*errNum = csc_jsonErr_Ok;
		return el->val.iVal;
	}
}
int csc_json_getInt(csc_json_t *js, char *name, csc_jsonErr_t *errNum)
{	return getInt(findByName(js,name), errNum);
}
int csc_json_ndxInt(csc_json_t *js, int ndx, csc_jsonErr_t *errNum)
{	return getInt(findByIndex(js,ndx), errNum);
}
int csc_jsonArr_getInt(csc_jsonArr_t *jas, int ndx, csc_jsonErr_t *errNum)
{	return csc_json_ndxInt((csc_json_t*)jas, ndx, errNum);
}


static double getFloat(elem_t *el,  csc_jsonErr_t *errNum)
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
	else
	{	*errNum = csc_jsonErr_WrongType;
		return 0;
	}
}
double csc_json_getFloat(csc_json_t *js, char *name, csc_jsonErr_t *errNum)
{	return getFloat(findByName(js,name), errNum);
}
double csc_json_ndxFloat(csc_json_t *js, int ndx, csc_jsonErr_t *errNum)
{	return getFloat(findByIndex(js,ndx), errNum);
}
double csc_jsonArr_getFloat(csc_jsonArr_t *jas, int ndx, csc_jsonErr_t *errNum)
{	return csc_json_ndxFloat((csc_json_t*)jas, ndx, errNum);
}


static const char *getStr(elem_t *el,  csc_jsonErr_t *errNum)
{	if (el == NULL)
	{	*errNum = csc_jsonErr_Missing;
		return NULL;
	}
	else if (el->type != csc_jsonType_String)
	{	*errNum = csc_jsonErr_WrongType;
		return NULL;
	}
	else
	{	*errNum = csc_jsonErr_Ok;
		return el->val.sVal;
	}
}
const char *csc_json_getStr(csc_json_t *js, char *name, csc_jsonErr_t *errNum)
{	return getStr(findByName(js,name), errNum);
}
const char *csc_json_ndxStr(csc_json_t *js, int ndx, csc_jsonErr_t *errNum)
{	return getStr(findByIndex(js,ndx), errNum);
}
const char *csc_jsonArr_getStr(csc_jsonArr_t *jas, int ndx, csc_jsonErr_t *errNum)
{	return csc_json_ndxStr((csc_json_t*)jas, ndx, errNum);
}


static const csc_json_t *getObj(elem_t *el,  csc_jsonErr_t *errNum)
{	if (el == NULL)
	{	*errNum = csc_jsonErr_Missing;
		return NULL;
	}
	else if (el->type != csc_jsonType_Obj)
	{	*errNum = csc_jsonErr_WrongType;
		return NULL;
	}
	else
	{	*errNum = csc_jsonErr_Ok;
		return el->val.oVal;
	}
}
const csc_json_t *csc_json_getObj(csc_json_t *js, char *name, csc_jsonErr_t *errNum)
{	return getObj(findByName(js,name), errNum);
}
const csc_json_t *csc_json_ndxObj(csc_json_t *js, int ndx, csc_jsonErr_t *errNum)
{	return getObj(findByIndex(js,ndx), errNum);
}
const csc_json_t *csc_jsonArr_getObj(csc_jsonArr_t *jas, int ndx, csc_jsonErr_t *errNum)
{	return csc_json_ndxObj((csc_json_t*)jas, ndx, errNum);
}


static const csc_jsonArr_t *getArr(elem_t *el,  csc_jsonErr_t *errNum)
{	if (el == NULL)
	{	*errNum = csc_jsonErr_Missing;
		return NULL;
	}
	else if (el->type != csc_jsonType_Arr)
	{	*errNum = csc_jsonErr_WrongType;
		return NULL;
	}
	else
	{	*errNum = csc_jsonErr_Ok;
		return el->val.aVal;
	}
}
const csc_jsonArr_t *csc_json_getArr(csc_json_t *js, char *name, csc_jsonErr_t *errNum)
{	return getArr(findByName(js,name), errNum);
}
const csc_jsonArr_t *csc_json_ndxArr(csc_json_t *js, int ndx, csc_jsonErr_t *errNum)
{	return getArr(findByIndex(js,ndx), errNum);
}
const csc_jsonArr_t *csc_jsonArr_getArr(csc_jsonArr_t *jas, int ndx, csc_jsonErr_t *errNum)
{	return csc_json_ndxArr((csc_json_t*)jas, ndx, errNum);
}


void main(int argc, char **argv)
{	csc_jsonErr_t errNum;
	csc_json_t *js = csc_json_new();

	csc_json_addInt(js, "fred", 7);
	csc_json_addInt(js, "mark", 37);
	csc_json_addFloat(js, "freda", 7.1);
	csc_json_addFloat(js, "lucy", 3.7);
	csc_json_addStr(js, "Fido", "FidoS");
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
	printf("\"%s\" %d\n", bo, errNum);
 
	csc_json_free(js);
	exit(0);
}

