#pragma once
#include "../pch.h"
#include "Structure.h"

class CredsManager
{
private:
	int GenerateRSAKey(RSA_Data* pRSA_Data, C_Array<uint8_t>* pData, C_Array<uint8_t>* pGeneratedKey);
	int GetSM4SaltKey(EntryEncryptionFlag EncryptionFlag, C_Array<uint8_t>* RetSaltKey);
public:
	CredsManager();
	~CredsManager();

	void Init_Index_RSA_Data(RSA_Data* _RSA_Data);
	int GenerateKey(RSA_Data* pRSA_Data, C_Array<uint8_t>* pData, C_Array<uint8_t>* pGeneratedKey);
	int GenerateBlockKeySM4
	(char* Filename, size_t FNameLen, C_Array<uint8_t>* GeneratedKey, EntryEncryptionFlag EncryptionFlag);

	int	GenerateDynKey(int32_t EncryptionKeyID, C_Array<uint8_t>* GeneratedBuff);

};

