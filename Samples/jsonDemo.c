#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <CscNetLib/std.h>
#include <CscNetLib/json.h>


void processData(FILE *fin, FILE *fout)
{	csc_jsonErr_t errNum;
	double width, length, area;
 
// Read in the data.
	csc_json_t *inData = csc_json_newParseFILE(fin);
	width = csc_json_getFloat(inData, "width", &errNum);
	length = csc_json_getFloat(inData, "length", &errNum);
	csc_json_free(inData);
 
// Calculate results.
	area = width * length;
 
// Send out the result.
	csc_json_t *results = csc_json_new();
	csc_json_addFloat(results, "area", area);
	csc_json_writeFILE(results, fout);
	fprintf(fout, "\n");
	csc_json_free(results);
}


int main(int argc, char **argv)
{	FILE *fin = fopen("house.json", "r");
	processData(fin, stdout);
	fclose(fin);
	exit(0);
}


