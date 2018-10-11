
typedef struct csc_blacklist_t csc_blacklist_t;

csc_blacklist_t *csc_blacklist_new(int expireTime);
void csc_blacklist_free(csc_blacklist_t *bl);
int csc_blacklist_blackness(csc_blacklist_t *bl, const char *idStr);
void csc_blacklist_clean(csc_blacklist_t *bl);
int csc_blacklist_accessCount(csc_blacklist_t *bl);

