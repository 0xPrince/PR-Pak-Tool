#include "CredsManager.h"


CredsManager::CredsManager() {}

CredsManager::~CredsManager() {}

void CredsManager::Init_Index_RSA_Data(RSA_Data* _RSA_Data)
{
	Utils::Memory::MemSet(_RSA_Data->Exponent, 0, sizeof(_RSA_Data->Exponent));
	Utils::Memory::MemSet(_RSA_Data->Modulus, 0, sizeof(_RSA_Data->Modulus));
	
	Utils::Memory::MemCopy(_RSA_Data->Exponent, Offsets::IndexData::RSA_Exponent, sizeof(_RSA_Data->Exponent));
	Utils::Memory::MemCopy(_RSA_Data->Modulus, Offsets::IndexData::RSA_Modulus, sizeof(_RSA_Data->Modulus));
}

int HashRSAKey(C_Array<uint8_t>* RSA_Key, C_Array<uint8_t>* GeneratedBuff)
{
	if (RSA_Key->size() < 43 || RSA_Key->data()[0]) return 0;

	int RetStatus = 0;
	size_t HashSize = 20;

	C_Array<uint8_t> FirstHash;
	FirstHash.reserve(HashSize);
	Utils::Memory::MemCopy(FirstHash.data(), (const void*)(RSA_Key->data() + 1), FirstHash.capacity());
	FirstHash.resize(FirstHash.capacity());

	C_Array<uint8_t> LastHash = {};
	LastHash.reserve(RSA_Key->size() - 1 - FirstHash.size());
	Utils::Memory::MemCopy(LastHash.data(), (const void*)(RSA_Key->data() + FirstHash.size() + 1), LastHash.capacity());
	LastHash.resize(LastHash.capacity());


	C_Array<uint8_t> TmpHash = {};
	TmpHash.reserve(HashSize);
	TmpHash.resize(TmpHash.capacity());

	SHA1::Hash(LastHash.data(), LastHash.size(), TmpHash.data());
	Utils::Misc::XORBuffers(FirstHash.data(), FirstHash.size(), TmpHash.data(), TmpHash.size());

	SHA1::Hash(FirstHash.data(), FirstHash.size(), TmpHash.data());
	Utils::Misc::XORBuffers(LastHash.data(), LastHash.size(), TmpHash.data(), TmpHash.size());

	Utils::Memory::MemSet(TmpHash.data(), 0, TmpHash.capacity());

	SHA1::Hash(TmpHash.data(), TmpHash.capacity(), TmpHash.data());

	GeneratedBuff->resize(0);
	if (!memcmp(TmpHash.data(), LastHash.data(), HashSize))//Check If == NullHash
	{
		uint8_t* _LastHash = &LastHash[HashSize];
		int ValidIndex = -1;
		while (ValidIndex + 1 < (LastHash.capacity() - HashSize) && !_LastHash[++ValidIndex]);

		if (_LastHash[ValidIndex] == 1)
		{
			signed int HashedBuffSize = LastHash.size() + ~ValidIndex - HashSize;

			if (GeneratedBuff->capacity() < HashedBuffSize)
				GeneratedBuff->reserve(HashedBuffSize);

			Utils::Memory::MemCopy((void*)GeneratedBuff->data(), _LastHash + ValidIndex + 1, HashedBuffSize);

			GeneratedBuff->resize(HashedBuffSize);
			RetStatus = 1;
		}
	}
	FirstHash.clear(); LastHash.clear(); TmpHash.clear();

	return RetStatus;
}

int CredsManager::GenerateRSAKey(RSA_Data* pRSA_Data, C_Array<uint8_t>* pData, C_Array<uint8_t>* pGeneratedKey)
{
	if (!pRSA_Data || !pData || !pGeneratedKey) return -1;
	pGeneratedKey->resize(0);

	BIGNUM* bn_exp = BN_bin2bn(pRSA_Data->Exponent, sizeof(pRSA_Data->Exponent), NULL);
	BIGNUM* bn_mod = BN_bin2bn(pRSA_Data->Modulus, sizeof(pRSA_Data->Modulus), NULL);
	BIGNUM* bn_data = BN_bin2bn(pData->data(), pData->size(), NULL);
	
	BIGNUM* GeneratedKeyBN = BN_new();
	BN_CTX* ctx = BN_CTX_new();
	BN_mod_exp(GeneratedKeyBN, bn_data, bn_exp, bn_mod, ctx);

	
	int GeneratedKey_size = BN_num_bytes(GeneratedKeyBN);

	if (GeneratedKey_size < 1) return -2;

	pGeneratedKey->reserve(GeneratedKey_size);


	BN_bn2bin(GeneratedKeyBN, pGeneratedKey->data());
	pGeneratedKey->resize(GeneratedKey_size);


	Utils::Misc::ReverseBuffer(pGeneratedKey->data(), pGeneratedKey->size());

	BN_free(bn_exp); BN_free(bn_mod); BN_free(bn_data); BN_free(GeneratedKeyBN);
	BN_CTX_free(ctx);


	return 1;
}

int CredsManager::GenerateKey(RSA_Data* pRSA_Data, C_Array<uint8_t>* pData, C_Array<uint8_t>* pGeneratedKey)
{
	pGeneratedKey->resize(0);
	Utils::Misc::ReverseBuffer(pData->data(), pData->size());
	auto RSA_KeyStatus = GenerateRSAKey(pRSA_Data, pData, pGeneratedKey);

	if (RSA_KeyStatus != 1
		|| !pGeneratedKey || pGeneratedKey->size() == NULL)
	{
		if (pGeneratedKey) pGeneratedKey->clear();
		return 0x1000 + RSA_KeyStatus;
	}

	if (!HashRSAKey(pGeneratedKey, pGeneratedKey) ||
		pGeneratedKey->size() != 32)
	{
		pGeneratedKey->clear();
		return 0x2000;
	}

	return 1;
}



int CredsManager::GenerateBlockKeySM4
(char* Filename, size_t FNameLen, C_Array<uint8_t>* GeneratedKey, EntryEncryptionFlag EncryptionFlag)
{
	GeneratedKey->resize(0);
	if (FNameLen < 1) return -1;

	C_Array<uint8_t> SM4KeySalt;
	if (GetSM4SaltKey(EncryptionFlag, &SM4KeySalt) != 1) return -0x300;

	/*static*/ C_Array<char> TmpFNameBlock;

	TmpFNameBlock.reserve(FNameLen);
	Utils::IO::GetFileNameWithoutExt(Filename, TmpFNameBlock.data());

	TmpFNameBlock.resize(strlen(TmpFNameBlock.data()));
	TmpFNameBlock.assign(Utils::Text::ToLowerInvariant(TmpFNameBlock.data(), TmpFNameBlock.size()), TmpFNameBlock.size());
	//if (FNameLen < 1) FNameLen = 1;

	TmpFNameBlock.append((char*)SM4KeySalt.data(), SM4KeySalt.size());
	SM4KeySalt.clear();

	GeneratedKey->reserve(20);
	SHA1::Hash(TmpFNameBlock.data(), TmpFNameBlock.size(), GeneratedKey->data());
	TmpFNameBlock.clear();

	GeneratedKey->resize(16);

	return 1;
}


int CredsManager::GetSM4SaltKey(EntryEncryptionFlag EncryptionFlag, C_Array<uint8_t>* RetSaltKey)
{
	RetSaltKey->resize(0);
	//< MultipleEncryption
	if (EncryptionFlag == EntryEncryptionFlag::NONE || EncryptionFlag == EntryEncryptionFlag::SM4_PH1)
	{
		RetSaltKey->assign(Offsets::EntryBlock::KeyArraySM4[0], 20);
		return 1;
	}
	if (EncryptionFlag == EntryEncryptionFlag::SM4_PH2)
	{
		RetSaltKey->assign(Offsets::EntryBlock::KeyArraySM4[1], 20);
		return 1;
	}

	int KeyIndx = (int)EncryptionFlag - ((int)EntryEncryptionFlag::SM4_PH3 - 1);
	const int MaxKeyCount = Offsets::EntryBlock::KeyArraySM4Count - (2);
	if (KeyIndx < 0 || KeyIndx >= MaxKeyCount)
	{
		PRINT(PrintType_ERROR, "SM4 Key Index Outside Bounds (%d)[%02d] > [%02d]", EncryptionFlag, KeyIndx, MaxKeyCount-1);
		return -1;
	}

	auto SM4KeyBuff = Offsets::EntryBlock::KeyArraySM4[2 + KeyIndx];
	if (SM4KeyBuff == NULL)
	{
		PRINT(PrintType_ALERT, "SM4 Key Buffer NULL at Index: [%02d]", 2 + KeyIndx);
		return -3;
	}

	RetSaltKey->reserve(20 + 4);
	RetSaltKey->assign(SM4KeyBuff, 20);
	auto FlagNumLen = snprintf((char*)RetSaltKey->data() + RetSaltKey->size(), RetSaltKey->size(), "%d", (int)EncryptionFlag);
	if (FlagNumLen < 1) return -2;
	RetSaltKey->resize(RetSaltKey->size() + FlagNumLen);
	return 1;

}

int	CredsManager::GenerateDynKey(int32_t EncryptionKeyID, C_Array<uint8_t>* GeneratedBuff)
{
	if (EncryptionKeyID == -1) return -1;

	int KeyIndex = EncryptionKeyID & 0xFFFFFF;
	if (KeyIndex >= Offsets::EntryBlock::DynKeyArrayCount)
	{
		PRINT(PrintType_ALERT, "DYN Key Index Outside Bounds [%02d] > [%02d] ", KeyIndex, Offsets::EntryBlock::DynKeyArrayCount-1);
		return -2;
	}
	auto DynKeyBuff = Offsets::EntryBlock::DynKeyArray[KeyIndex];
	if (DynKeyBuff == NULL)
	{
		PRINT(PrintType_ALERT, "DYN Key Buffer NULL at Index: [%02d]", KeyIndex);
		return -3;
	}

	C_Array<uint8_t> tmpBuff;
	tmpBuff.assign(DynKeyBuff, 20);

	GeneratedBuff->resize(20);
	SHA1::Hash(tmpBuff.data(), tmpBuff.size(), GeneratedBuff->data());

	GeneratedBuff->resize(16);
	tmpBuff.clear();
	return 1;
}