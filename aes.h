#include <stdint.h>


// -------- aesNi -----------
// Uses AES-NI CPU instruction.
// Will crash the program if that is not available.
// Its much faster than software implementaion.
 
// Type declaration for CPU hardware AES processing.
typedef struct csc_aes_ni_t csc_aes_ni_t;
 
// Constructor.
csc_aes_ni_t *csc_aes_ni_new(const uint8_t key[16]);
 
// Destructor.
void csc_aes_ni_free(csc_aes_ni_t *ani);
 
// Encrypt 16 bytes of 'plainText' as input giving 16 bytes of 'cypherText'.
void csc_aes_ni_enc(csc_aes_ni_t *ani, const uint8_t *plainText, uint8_t *cipherText);
 
// Decrypt 16 bytes of 'cypherText' as input giving 16 bytes of 'plainText'.
void csc_aes_ni_dec(csc_aes_ni_t *ani, const uint8_t *cipherText, uint8_t *plainText);



// ----------- AES by Polar SSL ---------------
// Software implementation of AES128 by Polar SSL.
// To be used if AES-NI CPU instruction is not available.
 
// https://polarssl.org
// https://linux.softpedia.com/get/Security/PolarSSL-52374.shtml
// 
// The AES block cipher was designed by Vincent Rijmen and Joan Daemen.
// http://csrc.nist.gov/encryption/aes/rijndael/Rijndael.pdf
// http://csrc.nist.gov/publications/fips/fips197/fips-197.pdf
// 
// FIPS-197 compliant AES implementation
// Copyright (C) 2006-2014, Brainspark B.V.
// 
// This file is part of PolarSSL (http://www.polarssl.org)
// Lead Maintainer: Paul Bakker <polarssl_maintainer at polarssl.org>
// All rights reserved.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 
// https://www.gnu.org/licenses/old-licenses/gpl-2.0.html
// for more details.
// 
// Modifications including encapsulation, re-formatting and identifier
// renaming by Stephen Braithwaite.
 
// Type declaration for software AES processing.
typedef struct csc_aes_pssl_t csc_aes_pssl_t;
 
// Constructor.
csc_aes_pssl_t *csc_aes_pssl_new(const uint8_t key[16]);
 
// Destructor.
void csc_aes_pssl_free(csc_aes_pssl_t *ani);
 
// Encrypt 16 bytes of 'plainText' as input giving 16 bytes of 'cypherText'.
void csc_aes_pssl_enc(csc_aes_pssl_t *ani, const uint8_t *plainText, uint8_t *cipherText);
 
// Decrypt 16 bytes of 'cypherText' as input giving 16 bytes of 'plainText'.
void csc_aes_pssl_dec(csc_aes_pssl_t *ani, const uint8_t *cipherText, uint8_t *plainText);



// ------------- Combined AES128 ----------------
// Works out if AES-NI CPU instructions are available.
// Uses AES-NI if available.
// Uses software code by Polar SSL if it is not.
 
// What type of encryption is available?
typedef enum csc_aes_e
{	csc_aes_impl_UNKNOWN=0,  // Let constructor discover if AesNi available.
	csc_aes_impl_NI=1,      // AesNi is supported.  Use it.
	csc_aes_impl_PSSL=2    // AesNi is not supported.  Use Polar SSL software.
} csc_aes_impl_t; 
	
// Find out what type of encryption is available.
csc_aes_impl_t csc_aes_impl();
 
// Type declaration for AES processing.
typedef struct csc_aes_t csc_aes_t;
 
// Constructor.
csc_aes_t *csc_aes_new(csc_aes_impl_t impl, const int8_t key[16]);
 
// Destructor.
void csc_aes_free(csc_aes_t *aes);
 
// Encrypt 16 bytes of 'plainText' as input giving 16 bytes of 'cypherText'.
void csc_aes_enc(csc_aes_t *aes, const uint8_t *plainText, uint8_t *cipherText);
 
// Decrypt 16 bytes of 'cypherText' as input giving 16 bytes of 'plainText'.
void csc_aes_dec(csc_aes_t *aes, const uint8_t *cipherText, uint8_t *plainText);


