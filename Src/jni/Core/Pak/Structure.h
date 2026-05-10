#pragma once
#include "Enums.h"

#pragma pack(push, 1)
struct PakEncryptionInfo
{
	uint8_t IndexKey_Data[0x100];
	uint8_t IndexIV_Data[0x100];
	uint8_t IndexHash_Data[0x100];

};
struct PakInfoHeader
{
	uint8_t bEncryptedIndex;
	uint32_t Magic;
	uint32_t Version;
	uint8_t IndexHash[20];
	uint64_t IndexSize;
	uint64_t IndexOffset;

};

struct PakInfo
{
	PakEncryptionInfo EncryptionData;
	PakInfoHeader Header;
	
};


struct PakCompressedBlock
{
	int64_t CompressedStart;
	int64_t CompressedEnd;

	int64_t GetSize()
	{
		return CompressedEnd - CompressedStart;
	}
};
template<typename type>
struct PArray
{
	int32_t size;
	type* Data;

	inline type& operator[](size_t index)
	{
		__ASSERT(index < size, "PArray: index out of range");
		return Data[index];
	};
};

struct PakEntry
{
	uint8_t Hash[20]; 
	int64_t Offset; 
	int64_t Size; 
	int32_t CompressionMethod;
	uint64_t CompressedSize;
	uint8_t u_00; 
	uint8_t ContentHash[20]; 

	//Present: CompressionMethod != NONE
	PArray<PakCompressedBlock> CompressionBlocks;

	uint32_t CompressionBlockSize;
	uint8_t bEncrypted;
	uint32_t EncryptionFlag;
	int32_t EncryptionKeyID;
};

struct CompressionDictEntry
{
	uint64_t DictSize;
	uint64_t _psz_;
	uint8_t Buffer;
};
#pragma pack(pop) 

static_assert(sizeof(PakInfo) == 0x32D, "Invalid Structures Alignment");


struct FileData
{
	int32_t NameLength;
	char* Name;
	bool bNameUnicode;
	int32_t DataIndex;

	int Parse(uint8_t* DirEntryStart, PakInfoHeader* Header);
};
struct EntryDirInfo
{
	int32_t NameLength;
	char* Name;
	int64_t FileCount;
	//FileData* _filedata;

	int Parse(uint8_t* FileNameEntry);

};


struct PakFileInfo
{
	FileData FileMeta;
	PakEntry entry;

	int Parse(uint8_t* PakFileStart, PakInfoHeader* Header);
};

struct PakIndexHeader
{
	uint32_t MountPointLength;
	char* MountPoint;
	uint32_t NumEntries;
	uint8_t* FileEntryList;

	
	bool bIsMiniPak;
	bool bDictionaryPak;
	C_Array<uint8_t> CompressionDictionary;

	int Parse(C_Array<uint8_t>* IndexData);
};

