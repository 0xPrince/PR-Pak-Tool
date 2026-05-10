#include "Compression.h"

#if !defined(NO_ZLIB)
#include <zlib/include/zlib.h>
#endif 

#if !defined(NO_ZSTD)
#include <zstd/include/zstd.h>
#endif

#if !defined(NO_OODLE)
#include <oodle/include/oodle.hpp>
#endif 

long long ZLIB_Decompress(const uint8_t* CompressedBuffer, uint32_t CompressedSize, uint8_t* UncompressedBuffer, uint32_t UncompressedSize, int32_t BitWindow = 15);


int Compression::UncompressMemory(CompressionFlags Flags,
	C_Array<uint8_t>* CompressedMem, C_Array<uint8_t>* UnCompressedMem,
	size_t CompBlockSize, C_Array<uint8_t>* CompDictionary)
{
	UnCompressedMem->resize(0);
	long long UncompressedLen = 0;
	int Ret_Status = 1;
	switch (Flags)
	{
	case CompressionFlags::COMPRESS_ZLIB:
	{

#if !defined(NO_ZLIB)
		UncompressedLen = ZLIB_Decompress(
			CompressedMem->data(), CompressedMem->size(),
			UnCompressedMem->data(), UnCompressedMem->capacity());
		if (UncompressedLen < 0)
			Ret_Status = UncompressedLen;
#else
		PRINT(PrintType_ALERT, "No ZLIB Included");
		return -403;
#endif

	}break;
	case CompressionFlags::COMPRESS_ZSTD:
	{

#if !defined(NO_ZSTD)
		UncompressedLen = ZSTD_decompress(
			UnCompressedMem->data(), UnCompressedMem->capacity(),
			CompressedMem->data(), CompressedMem->size());

		if (ZSTD_isError(UncompressedLen))
		{
			PRINT(PrintType_ERROR, "ZTD Decryption Failure, Msg: %s", ZSTD_getErrorName(UncompressedLen));
			Ret_Status = -1;
		}
#else
		PRINT(PrintType_ALERT, "No ZSTD Included");
		return -403;
#endif

	}break;
	case CompressionFlags::COMPRESS_OODLELZ:
	{

#if !defined(NO_OODLE)
		UncompressedLen = OodleLZ_Decompress(
			CompressedMem->data(), CompressedMem->size(),
			UnCompressedMem->data(), CompBlockSize);

		if (UncompressedLen == 0)
		{
			PRINT(PrintType_ERROR, "OodleLZ decompression failed");
			Ret_Status = -1;
		}
#else
		PRINT(PrintType_ALERT, "No Oodle Included");
		return -403;
#endif

	}break;
	case CompressionFlags::COMPRESS_ZSTDDICT:
	{
#if !defined(NO_ZSTD)
		if (!CompDictionary || !CompDictionary->data() || CompDictionary->size() < 8) return -3;
		auto zstdDictStream = ZSTD_createDStream();
		UncompressedLen = ZSTD_decompress_usingDict(zstdDictStream,
			UnCompressedMem->data(), UnCompressedMem->capacity(),
			CompressedMem->data(), CompressedMem->size(),
			CompDictionary->data(), CompDictionary->size());
		ZSTD_freeDStream(zstdDictStream);

		if (ZSTD_isError(UncompressedLen))
		{
			PRINT(PrintType_ERROR, "ZSTDDICT Decompression Failure, Msg:%s\n", ZSTD_getErrorName(UncompressedLen));

			Ret_Status = -1;
		}
#else
		PRINT(PrintType_ALERT, "No ZSTD Included");
		return -403;
#endif

	}break;
	default:
	{
		PRINT(PrintType_ERROR, "UnSupported Compression: %02d", (int)Flags);
		Ret_Status = -2;
		//break;
	}break;
	}

	if (Ret_Status < 0) PRINT(PrintType_INFO, "Compressed Memory Header: %llX", *(uint64_t*)CompressedMem->data());

	if (Ret_Status == 1)
		UnCompressedMem->resize(UncompressedLen);

	return Ret_Status;
}




void* zalloc(void* opaque, unsigned int items, unsigned int size)
{
	return Utils::Memory::AllocMem(items * size);
}

void zfree(void* opaque, void* address)
{
	Utils::Memory::FreeMem(address);
}

long long ZLIB_Decompress(const uint8_t* CompressedBuffer, uint32_t CompressedSize, uint8_t* UncompressedBuffer, uint32_t UncompressedSize, int32_t BitWindow)
{
	long long ZCompressedSize = CompressedSize;
	long long ZUncompressedSize = UncompressedSize;
#if !defined(NO_ZLIB)

	z_stream stream;
	stream.zalloc = &zalloc;
	stream.zfree = &zfree;
	stream.opaque = Z_NULL;
	stream.next_in = (uint8_t*)CompressedBuffer;
	stream.avail_in = ZCompressedSize;
	stream.next_out = UncompressedBuffer;
	stream.avail_out = ZUncompressedSize;

	int32_t Result = inflateInit2(&stream, BitWindow);

	if (Result != Z_OK)
		return -1;

	Result = inflate(&stream, Z_FINISH);

	ZUncompressedSize = stream.total_out;

	if (Result != Z_STREAM_END)
		ZUncompressedSize = -2;

	Result = inflateEnd(&stream);

	if (Result != Z_OK)
		ZUncompressedSize = -3;


#endif 
	return ZUncompressedSize;
}
