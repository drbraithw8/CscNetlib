#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <CscNetLib/std.h>
#include <CscNetLib/alloc.h>
#include <CscNetLib/aes.h>

void aesPssl_test()
{   csc_bool_t isOK = csc_TRUE;
 
	uint8_t enc_key[]    = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6
						   , 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
						   };
	uint8_t plain[]      = { 0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d
						   , 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34
						   };
    uint8_t cipher[]     = { 0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb
						   , 0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32
						   };
    uint8_t computed[16];
 
// Create the encryption object.
    csc_aes_pssl_t *aes = csc_aes_pssl_new(enc_key);
 
// Test the enc.
	csc_bool_t isOk = csc_TRUE;
	if (isOk)
    {	csc_aes_pssl_enc(aes, plain, computed);
		if (memcmp(cipher, computed, sizeof(computed)))
		{	fprintf(stdout, "FAIL (%s)\n", "aes_pssl_enc");
			isOk = csc_FALSE;
		}
		else
			fprintf(stdout, "pass: (%s)\n", "aes_pssl_enc");
	}
 
// Test the dec.
	if (isOk)
    {	csc_aes_pssl_dec(aes, cipher, computed);
		if (memcmp(plain, computed, sizeof(computed)))
		{	fprintf(stdout, "FAIL (%s)\n", "aes_pssl_dec");
			isOk = csc_FALSE;
		}
		else
			fprintf(stdout, "pass: (%s)\n", "aes_pssl_dec");
	}
 
// Free the encryption object.
	csc_aes_pssl_free(aes);
}  


void aesNi_test()
{   csc_bool_t isOK = csc_TRUE;
 
	uint8_t enc_key[]    = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6
						   , 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
						   };
	uint8_t plain[]      = { 0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d
						   , 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34
						   };
    uint8_t cipher[]     = { 0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb
						   , 0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32
						   };
    uint8_t computed[16];
 
// Create the encryption object.
    csc_aes_ni_t *aes = csc_aes_ni_new(enc_key);
 
// Test the enc.
	csc_bool_t isOk = csc_TRUE;
	if (isOk)
    {	csc_aes_ni_enc(aes, plain, computed);
		if (memcmp(cipher, computed, sizeof(computed)))
		{	fprintf(stdout, "FAIL (%s)\n", "aes_ni_enc");
			isOk = csc_FALSE;
		}
		else
			fprintf(stdout, "pass: (%s)\n", "aes_ni_enc");
	}
 
// Test the dec.
	if (isOk)
    {	csc_aes_ni_dec(aes, cipher, computed);
		if (memcmp(plain, computed, sizeof(computed)))
		{	fprintf(stdout, "FAIL (%s)\n", "aes_ni_dec");
			isOk = csc_FALSE;
		}
		else
			fprintf(stdout, "pass: (%s)\n", "aes_ni_dec");
	}
 
// Free the encryption object.
	csc_aes_ni_free(aes);
}  


void aes_test()
{   csc_bool_t isOK = csc_TRUE;
 
	uint8_t enc_key[]    = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6
						   , 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
						   };
	uint8_t plain[]      = { 0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d
						   , 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34
						   };
    uint8_t cipher[]     = { 0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb
						   , 0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32
						   };
    uint8_t computed[16];
 
// Create the encryption object.
    csc_aes_t *aes = csc_aes_new(csc_aes_impl_UNKNOWN, enc_key);
 
// Test the enc.
	csc_bool_t isOk = csc_TRUE;
	if (isOk)
    {	csc_aes_enc(aes, plain, computed);
		if (memcmp(cipher, computed, sizeof(computed)))
		{	fprintf(stdout, "FAIL (%s)\n", "aes_enc");
			isOk = csc_FALSE;
		}
		else
			fprintf(stdout, "pass: (%s)\n", "aes_enc");
	}
 
// Test the dec.
	if (isOk)
    {	csc_aes_dec(aes, cipher, computed);
		if (memcmp(plain, computed, sizeof(computed)))
		{	fprintf(stdout, "FAIL (%s)\n", "aes_dec");
			isOk = csc_FALSE;
		}
		else
			fprintf(stdout, "pass: (%s)\n", "aes_dec");
	}
 
// Free the encryption object.
	csc_aes_free(aes);
}  


void main(int argc, char **argv)
{
// Test software implementation.
	aesPssl_test();

// Test hardware implementation.
	if (csc_aes_impl())
	{	fprintf(stdout, "%s", "AES_NI supported\n");
		aesNi_test();
	}
	else
	{	fprintf(stdout, "%s", "AES_NI not supported\n");
	}

// Test combined software-hardware implementation.
	aes_test();

// Memory check.
    csc_mck_print(stdout);
}
