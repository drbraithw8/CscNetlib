#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <CscNetLib/std.h>
#include <CscNetLib/blacklist.h>


time_t tStampToTimeT(char *tStamp)
{   struct tm ts;
    bzero(&ts, sizeof(ts));
    sscanf( tStamp, "%4d%2d%2d.%2d%2d%2d"
          , &ts.tm_year
          , &ts.tm_mon
          , &ts.tm_mday
          , &ts.tm_hour
          , &ts.tm_min
          , &ts.tm_sec
          );
    ts.tm_year -= 1900;
    return mktime(&ts);
}


int main(int argc, char **argv)
{   
    const int LineMax = 99;
    char line[LineMax+1];
    char *words[3];
    int nWords;
    time_t fakeNow;
    int blackness;
 
    csc_blacklist_t *bl = csc_blacklist_new(15); assert(bl);
    csc_blacklist_setTimeFaked(bl, csc_TRUE);
 
    while (csc_fgetline(stdin,line,LineMax) > 0)
    {   if (csc_streq(line, "clean"))
        {
//          printf("nchunks=%ld\n", csc_mck_nchunks());
            csc_blacklist_clean(bl);
//          printf("nchunks=%ld\n", csc_mck_nchunks());
        }
        else
        {
        // Separate the time/date and the IP.
            nWords = csc_param(words, line, 3);  assert(nWords==2);

        // Extract and set the time.
            fakeNow = tStampToTimeT(line);
            csc_blacklist_setFakeTime(bl, fakeNow);

        // Do the blacklisting.
            blackness = csc_blacklist_blackness(bl, words[1]);
//          printf( "%ld %s %s %d %d\n"
//                , fakeNow
//                , words[0], words[1]
//                , blackness
//                , csc_blacklist_accessCount(bl)
//                );
        }
    }
 
    csc_blacklist_free(bl);

    if (csc_mck_nchunks() == 0)
        printf("pass (blacklist_mem)\n");
    else
        printf("FAIL (blacklist_mem)\n");
}

