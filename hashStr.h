#include <stdint.h>

typedef struct
{	uint64_t h0;
	uint64_t h1;
} csc_hash_hval128_t;


// Its really a char*, coz we use strlen() to find the length of the key.
csc_hash_hval128_t csc_hash_str128(const char *key);

// But hash.h functions are very generic and require void*.
unsigned long csc_hash_str(void *key);

