// Author: Dr Stephen Braithwaite.
// This work is licensed under a Creative Commons Attribution-ShareAlike 4.0 International License.

#include <time.h>
#include "std.h"

typedef struct csc_blacklist_t csc_blacklist_t;

csc_blacklist_t *csc_blacklist_new(int expireTime);
void csc_blacklist_free(csc_blacklist_t *bl);
int csc_blacklist_blackness(csc_blacklist_t *bl, const char *idStr);
void csc_blacklist_clean(csc_blacklist_t *bl);
int csc_blacklist_accessCount(csc_blacklist_t *bl);
void csc_blacklist_setTimeFaked(csc_blacklist_t *bl, csc_bool_t isFaked);
void csc_blacklist_setFakeTime(csc_blacklist_t *bl, time_t fakeNow);


