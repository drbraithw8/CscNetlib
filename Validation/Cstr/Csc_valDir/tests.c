#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <CscNetLib/std.h>
#include <CscNetLib/alloc.h>
#include <CscNetLib/cstr.h>

FILE *fout;

void printPassFail(const char *testName, csc_bool_t isPass)
{	if (isPass)
		fprintf(fout, "pass (%s)\n", testName);
	else
		fprintf(fout, "FAIL (%s)\n", testName);
}

#define longStr "aaabbbcccdddeeefffggghhhiiijjj"

int main(int argc, char **argv)
{
	const char *testName;
	fout = fopen("csc_testOut.txt", "a"); assert(fout);
	csc_str_t *strA;

	strA = csc_str_new(NULL);
	printPassFail("cstr_NULLNew", csc_streq(csc_str_charr(strA),""));

	csc_str_free(strA);
	printPassFail("cstr_NULLFree", csc_mck_nchunks()==0);

	strA = csc_str_new("");
	printPassFail("cstr_EmptyNew", csc_streq(csc_str_charr(strA),""));

	csc_str_free(strA);
	printPassFail("cstr_EmptyFree", csc_mck_nchunks()==0);

	strA = csc_str_new("a");
	printPassFail("cstr_SingleNew", csc_streq(csc_str_charr(strA),"a"));

	csc_str_assign(strA, longStr);
	printPassFail("cstr_AssignLong1", csc_streq(csc_str_charr(strA),longStr));

	csc_str_free(strA);
	printPassFail("cstr_SingleFree1", csc_mck_nchunks()==0);

	strA = csc_str_new(longStr);
	printPassFail("cstr_LongNew", csc_streq(csc_str_charr(strA),longStr));

	csc_str_free(strA);
	printPassFail("cstr_LongFree", csc_mck_nchunks()==0);

	strA = csc_str_new(NULL);
	csc_str_append(strA, longStr);
	csc_str_append_ch(strA, 'k');
	csc_str_append(strA, longStr);
	csc_str_append_ch(strA, 'k');
	csc_str_append(strA, longStr);
	csc_str_append_ch(strA, 'k');
	csc_str_append(strA, longStr);
	csc_str_append_ch(strA, 'k');
	csc_str_append(strA, longStr);
	csc_str_append_ch(strA, 'k');
	csc_str_append(strA, longStr);
	printPassFail( "append1", csc_streq(csc_str_charr(strA),
										  longStr "k" longStr "k" longStr
										  "k"
										  longStr "k" longStr "k" longStr
										));

	csc_str_assign(strA, longStr);
	printPassFail("AssignLong2", csc_streq(csc_str_charr(strA),longStr));

	csc_str_t *strB = csc_str_new("Jacky");
	csc_str_assign(strA, "");
	csc_str_append_f(strA, "Here %c%d%% %X %7.3f %S %s"
						 , '$', 23, 1023, 12.34567, strB, "there."); 
	char *expected="Here $23% 3FF  12.346 Jacky there.";
	// fprintf(stderr, "expected:\"%s\"\n", expected);
	// fprintf(stderr, "strA    :\"%s\"\n", csc_str_charr(strA));
	printPassFail("append_f", csc_streq(csc_str_charr(strA),expected));


	csc_str_free(strA);
	csc_str_free(strB);
	printPassFail("cstr_Free2", csc_mck_nchunks()==0);

	fclose(fout);
}



