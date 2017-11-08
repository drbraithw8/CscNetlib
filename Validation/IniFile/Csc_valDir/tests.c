#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <CscNetLib/std.h>
#include <CscNetLib/alloc.h>
#include <CscNetLib/iniFile.h>



void testReport_sVal(const char *testName, const char *required, const char *got)
{
// 	printf("req=\"%s\"  got=\"%s\"\n", required, got);
	if (required==NULL || got==NULL)
	{	if (required == got)
			fprintf(stdout, "pass (%s)\n", testName);
		else
			fprintf(stdout, "FAIL (%s)\n", testName);
	}
	else
	{	if (csc_streq(got,required))	
			fprintf(stdout, "pass (%s)\n", testName);
		else
			fprintf(stdout, "FAIL (%s)\n", testName);
	}
}

void testReport_iVal(const char *testName, int required, int got)
{	if (got == required)	
		fprintf(stdout, "pass (%s)\n", testName);
	else
		fprintf(stdout, "FAIL (%s)\n", testName);
}


void testLookup(csc_ini_t *ini, char *section, const char *key, const char *required) 
{   const char *value = csc_ini_getStr(ini, section, key);
	char *testName = csc_alloc_str7("ini", "_", section, "_", key, NULL, NULL);
	testReport_sVal(testName, required, value);
	free(testName);
}


int main()
{   
    char *iniFilePath;
    csc_ini_t *ini;
    int errLineNo;
    char *section;
 
    iniFilePath = "test.ini";
    ini = csc_ini_new();
    errLineNo = csc_ini_read(ini, iniFilePath);
 
    if (errLineNo == -1)
    {   fprintf(stderr, "Error opening in ini file \"%s\"\n", iniFilePath);
    }
    else if (errLineNo > 0)
    {   fprintf(stderr, "Error at line number %d in ini file \"%s\"\n",
                        errLineNo, iniFilePath);
    }
    else
    {   section = "ServerBase"; 
        testLookup(ini,section,"IP", "127.0.0.1");
        testLookup(ini,section,"PortNum", "9991");
        testLookup(ini,section,"dob", NULL);
 
        section = "Demo";   
        testLookup(ini,section,"Width", "5.7");
        testLookup(ini,section,"Height", "7.2");
        testLookup(ini,section,"Depth", "0.2");
    }   
        
    csc_ini_free(ini);

	if (csc_mck_nchunks() == 0)
		fprintf(stdout, "pass (%s)\n", "ini_memory");
	else
		fprintf(stdout, "FAIL (%s)\n", "ini_memory");
	csc_mck_print(stdout);
}
