#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <CscNetLib/std.h>
#include <CscNetLib/alloc.h>
#include <CscNetLib/json.h>


void testReport_bVal( FILE *fout , const char *testName
                    , csc_bool_t required , csc_bool_t got)
{
//  printf("req=\"%d\"  got=\"%d\"\n", required, got);
    if (got == required)    
        fprintf(fout, "pass (%s)\n", testName);
    else
        fprintf(fout, "FAIL (%s)\n", testName);
}


void testReport_iVal(FILE *fout, const char *testName, int required, int got)
{
//  printf("req=\"%d\"  got=\"%d\"\n", required, got);
    if (got == required)    
        fprintf(fout, "pass (%s)\n", testName);
    else
        fprintf(fout, "FAIL (%s)\n", testName);
}


void testReport_fVal(FILE *fout, const char *testName, double required, double got)
{
//  printf("req=\"%f\"  got=\"%f\"\n", required, got);
    if (got == required)    
        fprintf(fout, "pass (%s)\n", testName);
    else
        fprintf(fout, "FAIL (%s)\n", testName);
}


void testReport_sVal(FILE *fout, const char *testName, const char *required, const char *got)
{
//  printf("req=\"%s\"  got=\"%s\"\n", required, got);
    if (required==NULL || got==NULL)
    {   if (required == got)
            fprintf(fout, "pass (%s)\n", testName);
        else
            fprintf(fout, "FAIL (%s)\n", testName);
    }
    else
    {   if (csc_streq(got,required))    
            fprintf(fout, "pass (%s)\n", testName);
        else
            fprintf(fout, "FAIL (%s)\n", testName);
    }
}


void testIO_1()
{   csc_jsonErr_t errNum;
    const char *errStr;
 
// Resources.
    csc_json_t *js = NULL;
    FILE *fin = NULL;
    FILE *fout = NULL;
 
// Read in starting JSON.
    char *str = "{ name: \"fred\", age: 23, isMale:false, mary:null\n"
                 ", stats:{ height: 45, weight:35.45}\n"
                 ", tharr:[ 9, \"ksd\\\"jf\", {}, false ]\n"
                 "}";
    js = csc_json_newParseStr(str);
    errStr = csc_json_getErrStr(js);
    assert(errStr == NULL);
 
// Write it to a file.
    fout = fopen("csc_temp_1.json", "w");  assert(fout);
    csc_json_writeFILE(js, fout);
 
// Clean up.
    if (fout)
    {   fclose(fout);
        fout=NULL;
    }
    if (js)
    {   csc_json_free(js);
        js=NULL;
    }
 
// Read in the file.
    fin = fopen("csc_temp_1.json", "r");  assert(fin);
    js = csc_json_newParseFILE(fin);
    errStr = csc_json_getErrStr(js);
    assert(errStr == NULL);
 
// Perform the tests.
    testReport_sVal( stdout, "json_name", "fred"
                   , csc_json_getStr(js, "name", &errNum)
                   );
    testReport_iVal( stdout, "json_age", 23
                   , csc_json_getInt(js, "age", &errNum)
                   );
    testReport_bVal( stdout, "json_isMale", csc_FALSE
                   , csc_json_getBool(js, "isMale", &errNum)
                   );
    testReport_bVal( stdout, "json_mary", csc_TRUE
                   , csc_json_getType(js, "mary")==csc_jsonType_Null
                   );
 
// Examine stats object.
    const csc_json_t *stats = csc_json_getObj(js, "stats", &errNum);
    testReport_bVal( stdout, "json_stats", csc_FALSE, stats==NULL);
    testReport_iVal( stdout, "json_height", 45
                   , csc_json_getInt(stats, "height", &errNum)
                   );
    testReport_iVal( stdout, "json_weight", 35.45
                   , csc_json_getFloat(stats, "weight", &errNum)
                   );
 
// Examine tharr array.
    const csc_jsonArr_t *tharr = csc_json_getArr(js, "tharr", &errNum);
    testReport_iVal( stdout, "json_tharr", 4, csc_jsonArr_length(tharr));
    
    testReport_iVal( stdout, "json_tharr_0", 9
                   , csc_jsonArr_getInt(tharr, 0, &errNum)
                   );
    testReport_sVal( stdout, "json_tharr_1", "ksd\"jf"
                   , csc_jsonArr_getStr(tharr, 1, &errNum)
                   );

    testReport_iVal( stdout, "json_tharr_2a", csc_jsonType_Obj
                   , csc_jsonArr_getType(tharr, 2)
                   );
    const csc_json_t *tharrObj = csc_jsonArr_getObj(tharr, 2, &errNum);
    testReport_iVal( stdout, "json_tharr_2b", 0, csc_json_length(tharrObj));
    testReport_bVal( stdout, "json_tharr_3", csc_FALSE
                   , csc_jsonArr_getBool(tharr, 4, &errNum)
                   );
    
// Clean up.
    if (fin)
    {   fclose(fin);
        fout=NULL;
    }
    if (js)
    {   csc_json_free(js);
        js=NULL;
    }
}


int main(int argc, char **argv)
{   
    testIO_1();

    if (csc_mck_nchunks() == 0)
        fprintf(stdout, "pass (%s)\n", "http_memory");
    else
        fprintf(stdout, "FAIL (%s)\n", "http_memory");
    csc_mck_print(stdout);
}
