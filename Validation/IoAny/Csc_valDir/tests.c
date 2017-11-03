#include <stdlib.h>
#include <stdio.h>

#include <CscNetLib/std.h>
#include <CscNetLib/alloc.h>
#include <CscNetLib/cstr.h>
#include <CscNetLib/ioAny.h>


int get_line(csc_ioAnyRead_t *ior, csc_str_t *line)
{   int ch;
    csc_str_reset(line);
 
// Read in line. 
	ch = csc_ioAnyRead_getc(ior);
    while (ch!=EOF && ch!='\n')
    {   if (ch != '\r')
        {   csc_str_append_ch(line, ch);
		}
		ch = csc_ioAnyRead_getc(ior);
    }
 
// Return result. 
    if (ch == EOF)
        return -1;
    else
        return csc_str_length(line);
}


// ------------------------------------------------------------
// Test 1: Read a line from a string into a file and back again.
// ------------------------------------------------------------
void test1(void)
{	
	char *origLine1 = "original line #1";
	char *origLine2 = "original line #2";
	char *tempFilePath = "csc_temp_ioAny.txt";
 
// Resources.
	FILE *fin = NULL;
	FILE *fout = NULL;
	csc_ioAny_readChStr_t *inStr = NULL;
	csc_ioAnyRead_t *ioIn = NULL;
	csc_ioAnyWrite_t *ioOut = NULL;
    csc_str_t *line1 = NULL;
    csc_str_t *line2 = NULL;
 
// Create output for phase 1.
	fout = fopen(tempFilePath , "w");
	ioOut = csc_ioAnyWrite_new(csc_ioAny_writeFILE, fout);
 
// Transfer lines.
	csc_ioAnyWrite_puts(ioOut, origLine1);
	csc_ioAnyWrite_puts(ioOut, "\n");
	csc_ioAnyWrite_puts(ioOut, origLine2);
	csc_ioAnyWrite_puts(ioOut, "\n");
 
// Clean up some.
	if (ioOut)
	{	csc_ioAnyWrite_free(ioOut);
		ioOut=NULL;
	}
	if (fout)
	{	fclose(fout);
		fout=NULL;
	}
 
// Create input for phase 2.
	fin = fopen(tempFilePath , "r");
	ioIn = csc_ioAnyRead_new(csc_ioAny_readCharFILE, fin);
 
// Transfer lines.
	line1 = csc_str_new(NULL);
	line2 = csc_str_new(NULL);
	csc_ioAnyRead_getline(ioIn, line1);
	csc_ioAnyRead_getline(ioIn, line2);
 
// Results for test 1.
	if (  csc_streq(csc_str_charr(line1),origLine1)
	   && csc_streq(csc_str_charr(line2),origLine2)
	   )
	{	fprintf(stdout, "pass (%s)\n", "ioAny_ioFile");
	}
	else
	{	fprintf(stdout, "FAIL (%s)\n", "ioAny_ioFile");
	}
 
// Clean up.
	if (ioIn)
		csc_ioAnyRead_free(ioIn);
	if (fin)
		fclose(fin);
	if (line1)
		csc_str_free(line1);   
	if (line2)
		csc_str_free(line2);   
}


// ---------------------------------------------------------------
// Test 2: Read a line from a string into a csc_str and back again.
// ---------------------------------------------------------------
void test2(void)
{	
	char *origLine1 = "original line #1";
	char *origLine2 = "original line #2";
	char *tempFilePath = "csc_temp_ioAny.txt";
 
// Resources.
	csc_ioAny_readChStr_t *inStr = NULL;
	csc_ioAnyRead_t *ioIn = NULL;
	csc_ioAnyWrite_t *ioOut = NULL;
    csc_str_t *line1 = NULL;
    csc_str_t *line2 = NULL;
	csc_str_t *store = NULL;
 
// Create output for phase 1.
	store = csc_str_new(NULL);
	ioOut = csc_ioAnyWrite_new(csc_ioAny_writeCstr, store);
 
// Transfer lines.
	csc_ioAnyWrite_puts(ioOut, origLine1);
	csc_ioAnyWrite_puts(ioOut, "\n");
	csc_ioAnyWrite_puts(ioOut, origLine2);
	csc_ioAnyWrite_puts(ioOut, "\n");
 
// Create input for phase 2.
	inStr = csc_ioAny_readChStr_new(csc_str_charr(store));
	ioIn = csc_ioAnyRead_new(csc_ioAny_readCharStr, inStr);
 
// Transfer lines.
	line1 = csc_str_new(NULL);
	line2 = csc_str_new(NULL);
	csc_ioAnyRead_getline(ioIn, line1);
	csc_ioAnyRead_getline(ioIn, line2);
 
// Results for test 2.
	if (  csc_streq(csc_str_charr(line1),origLine1)
	   && csc_streq(csc_str_charr(line2),origLine2)
	   )
	{	fprintf(stdout, "pass (%s)\n", "ioAny_ioString");
	}
	else
	{	fprintf(stdout, "FAIL (%s)\n", "ioAny_ioString");
	}
 
// Clean up.
	if (ioOut)
		csc_ioAnyWrite_free(ioOut);
	if (ioIn)
		csc_ioAnyRead_free(ioIn);
	if (inStr)
		csc_ioAny_readChStr_free(inStr);
	if (store)
		csc_str_free(store);
	if (line1)
		csc_str_free(line1);   
	if (line2)
		csc_str_free(line2);   
}


int main(int argc, char **argv)
{	test1();
	test2();
	if (csc_mck_nchunks() == 0)
		fprintf(stdout, "pass (%s)\n", "ioAny_memory");
	else
		fprintf(stdout, "FAIL (%s)\n", "ioAny_memory");
}


// ------------------------------------------------
// -- Test 2: Read a line from a file into a cstr.
// ------------------------------------------------

// // Create output for phase 2.
//     replica = csc_str_new(NULL);
// 	ioOut = csc_ioAnyWrite_new(csc_ioAny_writeCstr, fp);
// 
// // Transfer one line.
// 	xferLine(ioIn, ioOut);
// 
// // Look at the result.
// 	if (csc_streq(origStr, csc_str_charr(replica)))
// 		fprintf(fout, "pass (%s)\n", testName);
// 	else
// 		fprintf(fout, "FAIL (%s)\n", testName);
// 
// // Clean up Phase 2.
// 	if (ioIN)
// 	{	csc_ioAnyRead_free(ioIn);
// 		ioIn=NULL;
// 	}
// 	if (fp)
// 	{	fclose(fp);
// 		fp=NULL;
// 	}
// 	if (ioOut)
// 	{	csc_ioAnyWrite_free(ioOut);
// 		ioOut=NULL;
// 	}
// 	if (replica)
// 	{	csc_str_free(replica);
// 		replica=NULL;
// 	}

