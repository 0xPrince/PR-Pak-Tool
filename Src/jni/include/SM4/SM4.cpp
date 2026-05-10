#include "SM4.h"
#include "Data.h"
#include <string.h>  // For memset

#ifdef __cplusplus
extern "C" {
#endif
#ifndef GET_ULONG_BE
#define GET_ULONG_BE(n,b,i)                             \
{                                                       \
    (n) = ( (unsigned long) (b)[(i)    ] << 24 )        \
        | ( (unsigned long) (b)[(i) + 1] << 16 )        \
        | ( (unsigned long) (b)[(i) + 2] <<  8 )        \
        | ( (unsigned long) (b)[(i) + 3]       );       \
}
#endif

#ifndef SET_ULONG_BE
#define SET_ULONG_BE(n,b,i)                             \
{                                                       \
    (b)[(i)    ] = (unsigned char) ( (n) >> 24 );       \
    (b)[(i) + 1] = (unsigned char) ( (n) >> 16 );       \
    (b)[(i) + 2] = (unsigned char) ( (n) >>  8 );       \
    (b)[(i) + 3] = (unsigned char) ( (n)       );       \
}
#endif


#define  SHL(x,n) (((x) & 0xFFFFFFFF) << n)
#define ROTL(x,n) (SHL((x),n) | ((x) >> (32 - n)))

#define SWAP(a,b) { unsigned long t = a; a = b; b = t; t = 0; }




static unsigned char SM4Sbox(unsigned char inch)
{
	unsigned char* pTable = (unsigned char*)SboxTable;
	unsigned char retVal = (unsigned char)(pTable[inch]);
	return retVal;
}

static unsigned long SM4Lt(unsigned long ka)
{
	unsigned long bb = 0;
	unsigned long c = 0;
	unsigned char a[4];
	unsigned char b[4];
	SET_ULONG_BE(ka, a, 0)
		b[0] = SM4Sbox(a[0]);
	b[1] = SM4Sbox(a[1]);
	b[2] = SM4Sbox(a[2]);
	b[3] = SM4Sbox(a[3]);
	GET_ULONG_BE(bb, b, 0)
		c = bb ^ (ROTL(bb, 2)) ^ (ROTL(bb, 10)) ^ (ROTL(bb, 18)) ^ (ROTL(bb, 24));
	return c;
}

static unsigned long SM4F(unsigned long x0, unsigned long x1, unsigned long x2, unsigned long x3, unsigned long rk)
{
	return (x0 ^ SM4Lt(x1 ^ x2 ^ x3 ^ rk));
}

unsigned long CalciRK(unsigned long ka)
{
	unsigned long bb = 0;
	unsigned long rk = 0;
	unsigned char a[4];
	unsigned char b[4];
	SET_ULONG_BE(ka, a, 0)
		b[0] = SM4Sbox(a[0]);
	b[1] = SM4Sbox(a[1]);
	b[2] = SM4Sbox(a[2]);
	b[3] = SM4Sbox(a[3]);
	GET_ULONG_BE(bb, b, 0)
		rk = bb ^ (ROTL(bb, 13)) ^ (ROTL(bb, 23));
	return rk;
}

static void Setkey(unsigned long* SK, unsigned char* key)
{
	unsigned long MK[4];
	unsigned long k[36];
	unsigned long i = 0;

	GET_ULONG_BE(MK[0], key, 0);
	GET_ULONG_BE(MK[1], key, 4);
	GET_ULONG_BE(MK[2], key, 8);
	GET_ULONG_BE(MK[3], key, 12);
	k[0] = MK[0] ^ FK[0];
	k[1] = MK[1] ^ FK[1];
	k[2] = MK[2] ^ FK[2];
	k[3] = MK[3] ^ FK[3];
	for (; i < 32; i++)
	{
		k[i + 4] = k[i] ^ (CalciRK(k[i + 1] ^ k[i + 2] ^ k[i + 3] ^ CK[i]));
		SK[i] = k[i + 4];
	}


	for (i = 0; i < SM4::Block_Size; i++)
	{
		SWAP(SK[i], SK[31 - i]);
	}
}

static void Process_Round(unsigned long* sk,
	unsigned char* input,
	unsigned char* output)
{
	unsigned long i = 0;
	unsigned long ulbuf[36];
	
	memset(ulbuf, 0, sizeof(ulbuf));
	GET_ULONG_BE(ulbuf[0], input, 0)
		GET_ULONG_BE(ulbuf[1], input, 4)
		GET_ULONG_BE(ulbuf[2], input, 8)
		GET_ULONG_BE(ulbuf[3], input, 12)
		while (i < 32)
		{
			ulbuf[i + 4] = SM4F(ulbuf[i], ulbuf[i + 1], ulbuf[i + 2], ulbuf[i + 3], sk[i]);
			i++;
		}
	SET_ULONG_BE(ulbuf[35], output, 0);
	SET_ULONG_BE(ulbuf[34], output, 4);
	SET_ULONG_BE(ulbuf[33], output, 8);
	SET_ULONG_BE(ulbuf[32], output, 12);
}

#ifdef __cplusplus
}
#endif

void SM4::Decrypt(unsigned char* Content, unsigned int ContentLength, unsigned char* Output, unsigned char* _key)
{
	unsigned long sk[32];
	memset(sk, 0, sizeof(sk));
	if(_key) Setkey(sk, _key);

	for (size_t Index = 0; Index < ContentLength; Index += SM4::Block_Size)
	{
		Process_Round(sk, Content + Index, Output + Index);
	}
}


