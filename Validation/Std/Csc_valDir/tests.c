#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <CscNetLib/std.h>
#include <CscNetLib/alloc.h>
#include <CscNetLib/cstr.h>

FILE *fout;

void testTrimE(const char *testName, const char *orig, const char *expect)
{	char str[100];
	strcpy(str, orig);
	csc_trim(str);
	if (csc_streq(str,expect))
        fprintf(fout, "pass (%s)\n", testName);
    else
        fprintf(fout, "FAIL (%s)\n", testName);
}


#define longStr1 "aaabb bcccdddeeefff ggghhhiiijjj"
#define longStr2 "  aaabb bcccdddeeefff ggghhhiiijjj  "
#define nullStr ""
#define space " "
#define spaces "    "
#define shortStr "ae"
#define spaceAfter "ae "
#define spacesAfter "ae   "
#define spaceBefore " ae"
#define spacesBefore "   ae"
#define spaceBA " ae "
#define spacesBA "   ae   "

void testTrim()
{	testTrimE("trim_longStr1", longStr1, longStr1);
	testTrimE("trim_longStr2", longStr2, longStr1);
	testTrimE("trim_nullStr", nullStr, nullStr);
	testTrimE("trim_space", space, nullStr);
	testTrimE("trim_spaces", spaces, nullStr);
	testTrimE("trim_shortStr", shortStr, shortStr);
	testTrimE("trim_spaceAfter", spaceAfter, shortStr);
	testTrimE("trim_spacesAfter", spacesAfter, shortStr);
	testTrimE("trim_spaceBefore", spaceBefore, shortStr);
	testTrimE("trim_spacesBefore", spacesBefore, shortStr);
	testTrimE("trim_spaceBA", spaceBA, shortStr);
	testTrimE("trim_spacesBA", spacesBA, shortStr);
}


int main(int argc, char **argv)
{
    fout = fopen("csc_testOut.txt", "a"); assert(fout);
	testTrim();
    fclose(fout);
}



