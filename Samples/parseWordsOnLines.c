#include <stdio.h>
#include <CscNetLib/std.h>

#define MaxLineLen 255
#define MaxWords 10

void main(int argc, char **argv)
{   char line[MaxLineLen+1];
    char *words[MaxWords];
    int nWords, iWord;
 
    while (csc_fgetline(stdin,line,MaxLineLen) != -1)
    {   nWords = csc_param_quote(words, line, MaxWords);
        for (iWord=0; iWord<nWords; iWord++)
            printf("*\t%s\n", words[iWord]);
        printf("\n");
    }

	exit(0);
}


