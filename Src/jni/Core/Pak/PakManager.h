#pragma once
#include "../Misc/Compression.h"
#include "Structure.h"

class PakManager
{
private:
	/*Vars*/
	FILE* _PakFileHandle = NULL;
	PakInfo* _info = NULL;
	long long PakFileSize = NULL;
	C_Array<unsigned int> HeaderKeys;

	GameID Game_ID = GameID::NONE;

	/*Funcs*/
	int DecryptIndexData(C_Array<uint8_t>* IndexData);
	int GetIndexAES_Key(C_Array<uint8_t>* GeneratedKey, C_Array<uint8_t>* GeneratedIV);
	int GetPakEncryptionData();
	int InitCompressionDictionary(C_Array<PakFileInfo>* PakEntries, PakIndexHeader* _IndexHeader);

	int DecryptBlockSP(C_Array<uint8_t>* _Block);
	int DecryptBlockSM4(PakFileInfo* _FileInfo, C_Array<uint8_t>* _Block);
	int DecryptBlockDYN(PakFileInfo* _FileInfo, C_Array<uint8_t>* _Block);
	int DecryptBlock_LM(PakFileInfo* _FileInfo, size_t FilePos, C_Array<uint8_t>* pMemory);

public:
	PakManager();
	~PakManager();

	size_t Read(int64_t Offset, void* _Buffer, size_t size){return Utils::IO::Read_File(_PakFileHandle, Offset, _Buffer, size);}
	PakInfo* GetPakFileInfo() {return _info;}
	GameID& GetGameID() {return Game_ID;}
	long long GetSize() {return PakFileSize;}


	int Setup(FILE* _PakFile);
	int ValidateIndexHash(C_Array<uint8_t>* IndexData);

	int InitIndexInfo();
	int GetIndexData(C_Array<uint8_t>* IndexData);
	InternalEncryptionFlags GetBlockEncryptionMethod(PakEntry* _FileInfo);
	int DecryptBlock(PakFileInfo* _FileInfo, C_Array<uint8_t>* Block);
	int DecryptBlockPH2(PakFileInfo* _FileInfo, size_t FilePos, C_Array<uint8_t>* pMemory);
	void GetBlockIndices(PakEntry* _entry, C_Array<int>* RetIds, bool bInvert);

	uint8_t* GetTextSection( uint8_t* TextSectionStart, C_Array<uint8_t>* pIndexData);
	int64_t GetIndexEntries(C_Array<PakFileInfo>* PakEntries, PakIndexHeader* _IndexHeader);
};
