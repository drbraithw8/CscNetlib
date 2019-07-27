#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <CscNetLib/std.h>




int main(int argc, char **argv)
{	int isPass;
	const int testSiz = 256;
	csc_uchar a[csc_bm_nBytes(testSiz)];
	const int seed = 3;
	FILE *fout = stdout;
	const char *testName = "Bitmap1";

// Pass or fail.
	isPass = csc_TRUE;
	
// Set values.
	srandom(seed);
	for (int i=0; i<testSiz; i++)
	{	uint64_t val = random();
		// printf("%lu\n", val);
		if (val & 1)
			csc_bm_set(a,i);
		else
			csc_bm_clr(a,i);
	}

// Check values.
	srandom(seed);
	for (int i=0; i<testSiz; i++)
	{	uint64_t val = random();
		if (val & 1)
		{	if (!(csc_bm_isSet(a,i)))
				isPass = csc_FALSE;
			if (csc_bm_isClr(a,i))
				isPass = csc_FALSE;
		}
		else
		{	if (!(csc_bm_isClr(a,i)))
				isPass = csc_FALSE;
			if (csc_bm_isSet(a,i))
				isPass = csc_FALSE;
		}
	}

// Report pass or fail.
	if (isPass)	
		fprintf(fout, "pass (%s)\n", testName);
	else
		fprintf(fout, "FAIL (%s)\n", testName);
}
