#pragma once

#include "../pch.h"
struct SHAHash
{
	//EVP_MAX_MD_SIZE
	static int Hash_Buffer_Ex(const void* Buff, unsigned int BuffSz, unsigned char* RetHash, const EVP_MD* hashAlgo) {
		EVP_MD_CTX* ctx = EVP_MD_CTX_new();
		if (!ctx)  return -1;

		if (EVP_DigestInit_ex(ctx, hashAlgo, nullptr) != 1)
		{
			EVP_MD_CTX_free(ctx);
			return -2;
		}

		if (EVP_DigestUpdate(ctx, Buff, BuffSz) != 1)
		{
			EVP_MD_CTX_free(ctx);
			return -3;
		}

		unsigned int digest_len;
		if (EVP_DigestFinal_ex(ctx, RetHash, &digest_len) != 1)
		{
			EVP_MD_CTX_free(ctx);
			return -4;
		}

		EVP_MD_CTX_free(ctx);
		return digest_len;
	}
	
};
struct SHA1 : private SHAHash
{
	static const int HASH_SIZE = 20;
	static int Hash(const void* Buffer, unsigned int BuffSz, unsigned char* Ret_Hash)
	{
		return Hash_Buffer_Ex(Buffer, BuffSz, Ret_Hash, EVP_sha1());
	}
};