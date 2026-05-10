#pragma once
#include "../pch.h"
struct AES
{
	const int BlockSize = 16;

	static long long Encrypt_Ex(const unsigned char* Buffer, int BufferSz,
		const unsigned char* KeyBytes, const unsigned char* IVBytes,
		unsigned char* RetBuff, const EVP_CIPHER* CipherAlgo)
	{
		EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
		if (!ctx) return -1;

		long long RetBuffLen = 0;
		int ProcessedSize = 0;
		if (EVP_EncryptInit_ex(ctx, CipherAlgo, nullptr, KeyBytes, IVBytes) != 1)
		{
			EVP_CIPHER_CTX_free(ctx);
			return -2;
		}

		if (EVP_EncryptUpdate(ctx, RetBuff, &ProcessedSize, Buffer, BufferSz) != 1)
		{
			EVP_CIPHER_CTX_free(ctx);
			return -3;
		}
		RetBuffLen += ProcessedSize;

		if (EVP_EncryptFinal_ex(ctx, RetBuff + RetBuffLen, &ProcessedSize) == 1) RetBuffLen += ProcessedSize;


		EVP_CIPHER_CTX_free(ctx);
		return RetBuffLen;
	}


	static long long Decrypt_Ex(const unsigned char* Buffer, unsigned int BuffSz,
		const unsigned char* KeyBytes, const unsigned char* IVBytes,
		unsigned char* RetBuff, const EVP_CIPHER* CipherAlgo)
	{
		long long RetBuffLen = 0;
		int ProcessedSize = 0;
		EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
		if (!ctx) return -1;

		if (EVP_DecryptInit_ex(ctx, CipherAlgo, nullptr, KeyBytes, IVBytes) != 1)
		{
			EVP_CIPHER_CTX_free(ctx);
			return -2;
		}

		if (EVP_DecryptUpdate(ctx, RetBuff, &ProcessedSize, Buffer, BuffSz) != 1)
		{
			EVP_CIPHER_CTX_free(ctx);
			return -3;
		}
		RetBuffLen += ProcessedSize;

		if (EVP_DecryptFinal_ex(ctx, RetBuff + RetBuffLen, &ProcessedSize) == 1) RetBuffLen += ProcessedSize;

		EVP_CIPHER_CTX_free(ctx);
		return RetBuffLen;
	}


	static long long EncryptBuffer_CBC(unsigned char* Buffer, unsigned int NumBytes, const unsigned char* KeyBytes, const unsigned char* IVBytes)
	{
		return Encrypt_Ex(Buffer, NumBytes, KeyBytes, IVBytes, Buffer, EVP_aes_256_cbc());
	}
	static long long DecryptBuffer_CBC(unsigned char* Buffer, unsigned int NumBytes, const unsigned char* KeyBytes, const unsigned char* IVBytes)
	{
		return Decrypt_Ex(Buffer, NumBytes, KeyBytes, IVBytes, Buffer, EVP_aes_256_cbc());
	}

};
