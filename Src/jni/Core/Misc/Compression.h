#pragma once
#include "../pch.h"

enum class CompressionFlags : int
{
	COMPRESS_None = 0x00,
	COMPRESS_ZLIB = 0x01,
	COMPRESS_ZSTD = 0x06,
	COMPRESS_OODLELZ = 0x07,
	COMPRESS_ZSTDDICT = 0x08,
	COMPRESS_MAX = 0x0F
};


class Compression
{
public:
	static int UncompressMemory(CompressionFlags Flags,
		C_Array<uint8_t>* CompressedMem, C_Array<uint8_t>* UnCompressedMem,
		size_t CompBlockSize, C_Array<uint8_t>* CompDictionary);

};

