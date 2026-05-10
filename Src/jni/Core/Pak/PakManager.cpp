#include "PakManager.h"
#include "../Cred/CredsManager.h"

CredsManager* Cred_Manager = NULL;


PakManager::PakManager()
{
	_info = (PakInfo*)Utils::Memory::AllocMem(sizeof(PakInfo));
	Cred_Manager = new CredsManager();
}

PakManager::~PakManager()
{
	if (_info) Utils::Memory::FreeMem(_info);

	_info = NULL;
	_PakFileHandle = NULL;
	HeaderKeys.clear();
}


int PakManager::Setup(FILE* _PakFile)
{
	if (!_info) return -1;
	if (!_PakFile) return -2;

	_PakFileHandle = _PakFile;

	PakFileSize = Utils::IO::GetFileSize(_PakFileHandle);
	if (PakFileSize < 0x100) return -3;

	auto HeaderStartOffset = PakFileSize - sizeof(PakInfoHeader);
	if (Read(HeaderStartOffset, (uint8_t*)&_info->Header, sizeof(PakInfoHeader)) != sizeof(PakInfoHeader))
		return -4;

	return 1;
}


int PakManager::ValidateIndexHash(C_Array<uint8_t>* IndexData)
{
	return Utils::Misc::CMPHash_SHA1(IndexData, _info->Header.IndexHash);
}



int PakManager::GetIndexAES_Key(C_Array<uint8_t>* GeneratedKey, C_Array<uint8_t>* GeneratedIV)
{
	RSA_Data* IndexRSA_Data = (RSA_Data*)Utils::Memory::AllocMem(sizeof(RSA_Data));

	Cred_Manager->Init_Index_RSA_Data(IndexRSA_Data);

	C_Array<uint8_t> m_data;
	m_data.assign(_info->EncryptionData.IndexKey_Data, sizeof(_info->EncryptionData.IndexKey_Data));

	auto KeyStatus = Cred_Manager->GenerateKey(IndexRSA_Data, &m_data, GeneratedKey);
	if (KeyStatus != 1)
	{
		Utils::Memory::FreeMem(IndexRSA_Data); m_data.clear();
		return 0x100 + KeyStatus;
	}

	m_data.resize(0);
	m_data.assign(_info->EncryptionData.IndexIV_Data, sizeof(_info->EncryptionData.IndexIV_Data));
	KeyStatus = Cred_Manager->GenerateKey(IndexRSA_Data, &m_data, GeneratedIV);
	
	Utils::Memory::FreeMem(IndexRSA_Data); m_data.clear();
	
	if (KeyStatus != 1)
	{
		return 0x200 + KeyStatus;
	}
	return 1;
}



int PakManager::GetIndexData(C_Array<uint8_t>* IndexData)
{
	IndexData->reserve(_info->Header.IndexSize);

	if (IndexData->capacity() != _info->Header.IndexSize) return -1/*Allocation Failure*/;

	if (Read(_info->Header.IndexOffset, IndexData->data(), IndexData->capacity()) != _info->Header.IndexSize)
		return -2/*Read Failure*/;

	IndexData->resize(_info->Header.IndexSize);

	if (_info->Header.bEncryptedIndex == 1)
	{
		if (_info->Header.Version >= (int)PakFile_Version::IndexAESEncryption
			&& Game_ID != GameID::PUBGM_CN/*Different Version System */)
		{
			auto PakEncData_Status = GetPakEncryptionData();
			if (PakEncData_Status != 1)
			{
				return 0x100 + PakEncData_Status;
			}
		}
		if (DecryptIndexData(IndexData) != 1)return -3/*Decryption Failure*/;
	}

	return 1;
}



int PakManager::InitIndexInfo()
{
	if (_info->Header.IndexOffset < 10 || _info->Header.IndexSize < 10) return -1;
	if (HeaderKeys.size() > 0) return -2/*Already Initialized*/;

	HeaderKeys.clear();
	HeaderKeys.reserve(Offsets::Header::Z_Field_Count);
	if (HeaderKeys.capacity() != Offsets::Header::Z_Field_Count) return -3/*Alloc Failure*/;

	ZUC::GenerateKeyArray(Offsets::Header::Z_Fields_Key, Offsets::Header::Z_Fields_IV,
		HeaderKeys.data(), HeaderKeys.capacity());

	if (HeaderKeys[0] == HeaderKeys[Offsets::Header::Z_Field_Count - 1])
		return -4/*Keys Generation Failure*/;
	HeaderKeys.resize(HeaderKeys.capacity());

	_info->Header.IndexOffset ^= CON_INTS(HeaderKeys[(int)Offsets::Header::Keys.IndexOffset],
		HeaderKeys[(int)Offsets::Header::Keys.IndexOffset + 1], uint64_t);

	_info->Header.IndexSize ^= CON_INTS(HeaderKeys[(int)Offsets::Header::Keys.IndexSize],
		HeaderKeys[(int)Offsets::Header::Keys.IndexSize + 1], uint64_t);

	_info->Header.bEncryptedIndex ^=
		_info->Header.Version > (int)PakFile_Version::IndexXOREncryption ?
		*(uint8_t*)(&HeaderKeys[(int)Offsets::Header::Keys.bEncryptedIndex]) :
		_info->Header.bEncryptedIndex;

	if (_info->Header.IndexOffset <1 || _info->Header.IndexOffset > PakFileSize ||
		_info->Header.IndexSize <1 || _info->Header.IndexSize > PakFileSize)
		return -5/*Likely Decryption Failure*/;

	//_info->Header.Magic ^= HeaderKeys[Offsets::Header::PakMagicKeyIndex];

	if (_info->Header.Version >= (int)PakFile_Version::IndexHashEncryption)
	{
		for (size_t i = 0; i < sizeof(_info->Header.IndexHash) / 4; i++)
		{
			((unsigned int*)(_info->Header.IndexHash))[i] ^= HeaderKeys[Offsets::Header::Keys.IndexHash[0] + i];
		}
	}

	return 1;
}
int PakManager::GetPakEncryptionData()
{
	auto StartOffset = PakFileSize - (sizeof(PakInfoHeader) + Offsets::Header::PakFileHeaderPadding + sizeof(PakEncryptionInfo));
	if (StartOffset <= 1000) return -1;

	if (Read(StartOffset, &_info->EncryptionData, sizeof(PakEncryptionInfo)) != sizeof(PakEncryptionInfo)) return -1;

	return 1;
}

int PakManager::DecryptIndexData(C_Array<uint8_t>* IndexData)
{
	if (_info->Header.Version < (int)PakFile_Version::IndexAESEncryption ||
		Game_ID == GameID::PUBGM_CN/*Have Different Version System */)
	{
		Utils::Misc::XORBuffers(IndexData->data(), _info->Header.IndexSize, Offsets::IndexData::XORKey, 16/*Why const?*/);
		return 1;
	}

	C_Array<uint8_t> GeneratedKey = {}, GeneratedIV = {};
	auto AES_KeyStatus = GetIndexAES_Key(&GeneratedKey, &GeneratedIV);

	if (AES_KeyStatus != 1 ||
		GeneratedKey.size() != 32 || GeneratedIV.size() != 32)
	{
		PRINT(PrintType_ERROR, "Failed to Get Index AES Key - Status: 0x%04X", AES_KeyStatus);
		return AES_KeyStatus;
	}

	auto DecryptedLen = AES::DecryptBuffer_CBC(IndexData->data(), _info->Header.IndexSize, GeneratedKey.data(), GeneratedIV.data());
	if (DecryptedLen <= 0)
	{
		PRINT(PrintType_ERROR, "Failed to Decrypt Index Data AES: 0x%llX, Status: 02lld", _info->Header.IndexSize, DecryptedLen);
		return 0x532;
	}

	IndexData->resize(DecryptedLen);

	GeneratedKey.clear(); GeneratedIV.clear();

	return 1;
}

//Internal Encryption Method
InternalEncryptionFlags PakManager::GetBlockEncryptionMethod(PakEntry* _entry)
{

	if (_info->Header.Version < (int)PakFile_Version::BlockSM4Encryption ||
		_entry->EncryptionFlag == (int)EntryEncryptionFlag::XOR ||
		Game_ID == GameID::PUBGM_CN/*Different Version System */
		|| (_entry->EncryptionFlag == 0 //????
			&& _entry->CompressionMethod == (int)CompressionFlags::COMPRESS_None)

		)
	{
		return InternalEncryptionFlags::XOR;
	}

	if ((_info->Header.Version >= (int)PakFile_Version::BlockSM4Encryption
		&& _info->Header.Version < (int)PakFile_Version::MultipleEncryption) ||
		(_entry->EncryptionFlag == (int)EntryEncryptionFlag::SM4_PH1
			|| _entry->EncryptionFlag == (int)EntryEncryptionFlag::SM4_PH2
			|| ((int)_entry->EncryptionFlag & (int)EntryEncryptionFlag::SM4_PH3) == (int)EntryEncryptionFlag::SM4_PH3))
	{
		return InternalEncryptionFlags::SM4;
	}

	if (_entry->EncryptionFlag == (int)EntryEncryptionFlag::SP
		&& _info->Header.Version >= (int)PakFile_Version::BlockSPEncryption
		&& Game_ID != GameID::PUBGM_CN)
		return InternalEncryptionFlags::SP;


	if (_info->Header.Version >= (int)PakFile_Version::BlockDynEncryption
		&& (_entry->EncryptionFlag - (int)EntryEncryptionFlag::DYN)
		< ((int)EntryEncryptionFlag::DYN_END - (int)EntryEncryptionFlag::DYN))
	{
		return InternalEncryptionFlags::DYN;
	}

	return InternalEncryptionFlags::UNKNOWN;
}



int PakManager::DecryptBlockSP(C_Array<uint8_t>* _Block)
{
	if (_Block->size() < 0x8) return 1;
	auto _Data = _Block->data(); auto Size = _Block->size();


	_Data[0] ^= Offsets::EntryBlock::SPKey[(Size + 0) & 3];
	_Data[1] ^= Offsets::EntryBlock::SPKey[(Size + 1) & 3];
	_Data[2] ^= Offsets::EntryBlock::SPKey[(Size + 2) & 3];
	_Data[3] ^= Offsets::EntryBlock::SPKey[(Size - 1) & 3];


	auto _DataInt = (uint32_t*)_Data;


	for (size_t BlockIndex = 1; BlockIndex < (Size / 4); BlockIndex++)
	{
		_DataInt[BlockIndex] ^= _DataInt[BlockIndex - 1];
	}
	return 1;
}

int PakManager::DecryptBlockSM4(PakFileInfo* _FileInfo, C_Array<uint8_t>* _Block)
{

	/*static*/ C_Array<uint8_t> GeneratedBlockKey;
	if (Cred_Manager->GenerateBlockKeySM4(
		_FileInfo->FileMeta.Name, _FileInfo->FileMeta.NameLength,
		&GeneratedBlockKey, (EntryEncryptionFlag)_FileInfo->entry.EncryptionFlag) != 1
		|| GeneratedBlockKey.size() != 16)
	{
		return -0x200;
	}
	SM4::Decrypt(_Block->data(), _Block->size(), _Block->data(), GeneratedBlockKey.data());
	GeneratedBlockKey.clear();

	return 1;
}

int PakManager::DecryptBlockDYN(PakFileInfo* _FileInfo, C_Array<uint8_t>* _Block)
{
	if (_FileInfo->entry.EncryptionFlag != (int)EntryEncryptionFlag::DYN_SM4)
	{
		PRINT(PrintType_ERROR, "UnImplemented DYN Decryption[%02u]", _FileInfo->entry.EncryptionFlag);
		return -1;
	}

	C_Array<uint8_t> GeneratedBlockKey;
	if (Cred_Manager->GenerateDynKey(_FileInfo->entry.EncryptionKeyID, &GeneratedBlockKey) != 1 || GeneratedBlockKey.size() != 16)
	{
		PRINT(PrintType_ERROR, "Failed to Generate DYN Key [%02u],[%02u]", _FileInfo->entry.EncryptionFlag, _FileInfo->entry.EncryptionKeyID);
		return -1;
	}

	SM4::Decrypt(_Block->data(), _Block->size(), _Block->data(), GeneratedBlockKey.data());
	GeneratedBlockKey.clear();
	return 1;
}

int PakManager::DecryptBlock(PakFileInfo* _FileInfo, C_Array<uint8_t>* Block)
{

	switch (GetBlockEncryptionMethod(&_FileInfo->entry))
	{
	case InternalEncryptionFlags::XOR:
	{
		Utils::Misc::XORBuffers(Block->data(), Block->size(), Offsets::EntryBlock::XORKey, 16/*Why const?*/);
		return 1;
	}
	case InternalEncryptionFlags::SP:
	{
		return DecryptBlockSP(Block);
	}
	case InternalEncryptionFlags::SM4:
	{
		return DecryptBlockSM4(_FileInfo, Block);
	}
	case InternalEncryptionFlags::DYN:
	{
		return DecryptBlockDYN(_FileInfo, Block);
	}
	default:
		break;
	}

	PRINT(PrintType_ALERT, "UnImplemented Encryption: %02u,[%02u]", _FileInfo->entry.EncryptionFlag, _FileInfo->entry.EncryptionKeyID);

	return -141;
}


EntryEncryptionFlag GetGOPBlockEncryptionFlag(C_Array<uint8_t>* pMemory)
{
	if (pMemory->size() >= 8)
	{
		const uint64_t MatchRes = 0x7373616D696E694C;
		if ((*(uint64_t*)(pMemory->data() + 0x00) ^ *(uint64_t*)(Offsets::EntryBlock::SLM_Key + 0x00)) == MatchRes)
		{
			return EntryEncryptionFlag::LM_SXR;
		}
		
		{
			int n = sizeof(MatchRes);
			while (--n > 0 && (*(uint8_t*)(pMemory->data() + n) ^ (Offsets::EntryBlock::ST_XorKey + n)) == ((uint8_t*)&MatchRes)[n]);
			if (n == 0)
			{
				return EntryEncryptionFlag::LM_STXR;
			}
		}
	}
	return EntryEncryptionFlag::NONE;
}

int PakManager::DecryptBlock_LM(PakFileInfo* _FileInfo, size_t FilePos, C_Array<uint8_t>* pMemory)
{
//	if (!Utils::IO::isFileExtension(_FileInfo->FileMeta.Name, ".ini")) return 1;

	if (FilePos == 0 && _FileInfo->entry.EncryptionFlag == (int)EntryEncryptionFlag::NONE)
	{
		_FileInfo->entry.EncryptionFlag = (uint32_t)GetGOPBlockEncryptionFlag(pMemory);
	}

	if (_FileInfo->entry.EncryptionFlag == (int)EntryEncryptionFlag::NONE) return 1;

	if (_FileInfo->entry.EncryptionFlag == (int)EntryEncryptionFlag::LM_SXR)
	{
		Utils::Misc::XORBuffers(pMemory->data(), pMemory->size(), Offsets::EntryBlock::SLM_Key, 16);
		return 1;
	}

	if (_FileInfo->entry.EncryptionFlag == (int)EntryEncryptionFlag::LM_STXR)
	{
		const int MaxN = sizeof(uint64_t)+1;

		size_t Indx = 0, nIndx = FilePos, SkpNum = 0xFF;
		if (nIndx >= Offsets::MaxCompressedBlockSize)  nIndx += ((nIndx / Offsets::MaxCompressedBlockSize) - 1);

		while (Indx < pMemory->size())
		{
			uint32_t nNum = (nIndx++) % MaxN;
			if (nNum == SkpNum) continue;

			pMemory->data()[Indx] ^= Offsets::EntryBlock::ST_XorKey + nNum;

			SkpNum = ((Indx + FilePos) / MaxN) % MaxN;
			++Indx;
		}
		return 1;
	}
	
	return 1;
}

int PakManager::DecryptBlockPH2(PakFileInfo* _FileInfo, size_t FilePos, C_Array<uint8_t>* pMemory)
{
	if (this->Game_ID == GameID::PUBGM_CN)
	{
		return DecryptBlock_LM(_FileInfo, FilePos, pMemory);
	}

	return 1;
}

void PakManager::GetBlockIndices(PakEntry* _entry, C_Array<int>* RetIds, bool bInvert)
{
	int NumBlocks = _entry->CompressionBlocks.size;
	RetIds->reserve(NumBlocks); RetIds->resize(0);

	auto EncMethod = GetBlockEncryptionMethod(_entry);
	if (NumBlocks < 2 || _entry->bEncrypted != 0x1 ||
		EncMethod != InternalEncryptionFlags::SM4 &&
		!(EncMethod == InternalEncryptionFlags::DYN &&
			_entry->EncryptionFlag == (int)EntryEncryptionFlag::DYN_SM4))
	{
		for (size_t i = 0; i < NumBlocks; i++)
			RetIds->insert(i);

		return;
	}


	static C_Array<int> Indices; 
	Indices.reserve(NumBlocks); Indices.resize(0);

	int seed = NumBlocks;
	while (Indices.size() < NumBlocks)
	{
		seed = 0x41C64E6D * seed + 12345;
		unsigned int idx = (seed / 0x10000 % 0x7FFFu) % NumBlocks;

		if (Indices.FindIndexOf(idx) != -1) continue;
		Indices.insert(idx); RetIds->insert(idx);
	}

	if (bInvert)
	{
		int i = 0;
		do
		{
			int j = 0;
			while (Indices[j] != i && ++j < Indices.size());

			if (j >= Indices.size()) continue;

			RetIds->data()[i] = j;
		} while (++i < Indices.size());

	}

	//Indices.clear();
}


uint8_t* PakManager::GetTextSection(uint8_t* TextSectionStart, C_Array<uint8_t>* pIndexData)
{
	static C_Array<uint8_t> TextSection;

	size_t TextSectionPakStart = TextSectionStart - pIndexData->data();
	if (TextSectionPakStart >= pIndexData->size())
	{
		//Most Probably TextSection Seperated From IndexData Chunk

		auto TextSectionPakOffset = this->GetPakFileInfo()->Header.IndexOffset + TextSectionPakStart;

		int64_t  EmbededTextSectionSize = 0;
		this->Read(TextSectionPakOffset, (uint8_t*)&EmbededTextSectionSize, sizeof(EmbededTextSectionSize)); TextSectionPakOffset += sizeof(EmbededTextSectionSize);
		if (EmbededTextSectionSize < 10 ||
			EmbededTextSectionSize >(uint64_t)this->GetSize() - pIndexData->size())
		{
			return NULL;//Invalid Text Section
		}

		TextSection.reserve(EmbededTextSectionSize);
		if (this->Read(TextSectionPakOffset, TextSection.data(), TextSection.capacity()) != EmbededTextSectionSize)
		{
			return NULL;//Read or Allocation Failed
		}
		TextSection.resize(EmbededTextSectionSize);
		return TextSection.data();
	}

	return TextSectionStart;
}

int64_t PakManager::GetIndexEntries(C_Array<PakFileInfo>* PakEntries, PakIndexHeader* _IndexHeader)
{
	PakEntries->reserve(_IndexHeader->NumEntries); PakEntries->resize(0);

	int64_t CurrentFileEntryEnd = 0;
	InternalEncryptionFlags IEncFlag;
	for (uint32_t EntryIndex = 0; EntryIndex < _IndexHeader->NumEntries; EntryIndex++)
	{
		auto _FileInfo = &PakEntries->data()[EntryIndex];
		auto CurrEntrySize = _FileInfo->Parse(_IndexHeader->FileEntryList + CurrentFileEntryEnd, &_info->Header);
		if (CurrEntrySize <= 0)
		{
			PRINT(PrintType_ERROR, "Pak Entry Parsing Failed, Status: %02d", CurrEntrySize);
			return CurrEntrySize;
		}

		if (_FileInfo->entry.bEncrypted &&
			(IEncFlag = GetBlockEncryptionMethod(&_FileInfo->entry)) == InternalEncryptionFlags::UNKNOWN)
		{
			PRINT(PrintType_ERROR, "Unknown Encryption: %02d",
				_FileInfo->entry.EncryptionFlag);
		}
		if (!_IndexHeader->bDictionaryPak)
			_IndexHeader->bDictionaryPak = _FileInfo->entry.CompressionMethod == (int)CompressionFlags::COMPRESS_ZSTDDICT;

		CurrentFileEntryEnd += CurrEntrySize;
		PakEntries->resize(PakEntries->size() + 1);
	}


	if (PakEntries->size() != _IndexHeader->NumEntries) return -1;

	if (_IndexHeader->bDictionaryPak)
	{
		auto CompDictStatus = InitCompressionDictionary(PakEntries, _IndexHeader);
		if (CompDictStatus != 1)
		{
			PRINT(PrintType_ERROR, "Failed to Initialize Compression Dictionary, Status: %02d", CompDictStatus);
			return -103;
		}
	}
	return CurrentFileEntryEnd;
}


int PakManager::InitCompressionDictionary(C_Array<PakFileInfo>* PakEntries, PakIndexHeader* _IndexHeader)
{

	if (_info->Header.Version < (int)PakFile_Version::DictionaryCompression) return 1;
	if (!_IndexHeader->bDictionaryPak || !PakEntries || PakEntries->size() < 2) return -1;

	auto dictEntry = PakEntries->data()[PakEntries->size() - 1];
	if (dictEntry.entry.CompressedSize != dictEntry.entry.Size
		|| dictEntry.entry.Size < 8 || dictEntry.entry.Offset <= 0)
		return -1/*Invaid Dict Entry*/;

	CompressionDictEntry CompDictionary = {};
	if (Read(dictEntry.entry.Offset, (uint8_t*)&CompDictionary, sizeof(CompressionDictEntry)) != sizeof(CompressionDictEntry))
		return -2/*Read Error*/;

	_IndexHeader->CompressionDictionary.reserve(CompDictionary.DictSize);

	if (Read(dictEntry.entry.Offset + offset_of(CompressionDictEntry, CompressionDictEntry::Buffer),
		_IndexHeader->CompressionDictionary.data(), CompDictionary.DictSize) != CompDictionary.DictSize)
		return -3/*Buff Read Error*/;

	_IndexHeader->CompressionDictionary.resize(CompDictionary.DictSize);

	PakEntries->resize(PakEntries->size() - 1);

	return 1;
}

int PakFileInfo::Parse(uint8_t* _EntryStart, PakInfoHeader* Header)
{
	//MESS
	size_t EntryStartOffset = 0;

	if (Header->Version < (int)PakFile_Version::IndexTextSection)
	{
		int MetaSize = FileMeta.Parse(_EntryStart, Header);
		if (MetaSize < 1) return -0x800 + MetaSize;
		EntryStartOffset += MetaSize;
	}

	uint8_t* EntryStart = _EntryStart + EntryStartOffset;

	entry = Utils::Memory::ReadBuffer<PakEntry>(EntryStart);


	entry.CompressionMethod &= (int)CompressionFlags::COMPRESS_MAX;
	if (entry.CompressionMethod > (int)CompressionFlags::COMPRESS_MAX) return -1; //Possibly Invalid or Unsupported File

	int offset = offset_of(PakEntry, PakEntry::CompressionBlocks.size);
	if (Header->Version < 5)
		offset = offset_of(PakEntry, PakEntry::u_00);

	if (entry.CompressionMethod != (int)CompressionFlags::COMPRESS_None)
	{
		entry.CompressionBlocks.Data = entry.CompressionBlocks.size > 0 ? (PakCompressedBlock*)(EntryStart + offset + sizeof(entry.CompressionBlocks.size)) : NULL;
		offset += sizeof(entry.CompressionBlocks.size) + (entry.CompressionBlocks.size * sizeof(PakCompressedBlock));
	}

	if (Header->Version >= (int)PakFile_Version::MultiCompression)
	{
		entry.CompressionBlockSize = Utils::Memory::ReadBuffer<uint32_t>(EntryStart + offset); offset += sizeof(entry.CompressionBlockSize);
	
		if (entry.CompressionBlockSize > Offsets::MaxCompressedBlockSize) return -2;/*Invalid ?*/

		entry.bEncrypted = Utils::Memory::ReadBuffer<uint8_t>(EntryStart + offset); offset += sizeof(entry.bEncrypted);
	}

	entry.EncryptionFlag = 0; entry.EncryptionKeyID = -1;
	if (Header->Version >= (int)PakFile_Version::MultipleEncryption)
	{
		if (Offsets::Game_ID == GameID::PUBGM_LITE)
		{
			if (entry.bEncrypted)
				entry.EncryptionFlag = (uint32_t)EntryEncryptionFlag::SM4_PH1;
		}
		else
		{
			entry.EncryptionFlag = Utils::Memory::ReadBuffer<uint32_t>(EntryStart + offset);
			offset += sizeof(entry.EncryptionFlag);
		}

	}

	if (Header->Version >= (int)PakFile_Version::BlockDynEncryption)
	{
		entry.EncryptionKeyID = Utils::Memory::ReadBuffer<uint32_t>(EntryStart + offset); offset += sizeof(entry.EncryptionKeyID);
	}

	return EntryStartOffset + offset;
}


const char RootMountPoint[] = "../../../";
int PakIndexHeader::Parse(C_Array<uint8_t>* IndexData)
{
	int Offset = 0;
	MountPointLength = Utils::Memory::ReadBuffer<uint32_t>(IndexData->data() + Offset); Offset += sizeof(MountPointLength);
	if (MountPointLength <= 0 || MountPointLength > 200) return -1;

	bIsMiniPak = MountPointLength > sizeof(RootMountPoint);

	MountPoint = (char*)(IndexData->data() + Offset + (bIsMiniPak ? sizeof(RootMountPoint) - 1 : 0)); Offset += MountPointLength;
	
	NumEntries = Utils::Memory::ReadBuffer<uint32_t>(IndexData->data() + Offset); Offset += sizeof(NumEntries);
	FileEntryList = (IndexData->data() + Offset);
	return 1;
}

int FileData::Parse(uint8_t* DirEntryStart, PakInfoHeader* Header)
{
	int Offset = 0;
	NameLength = Utils::Memory::ReadBuffer<int32_t>(DirEntryStart + Offset); Offset += sizeof(NameLength);
	Name = (char*)(DirEntryStart + Offset);//Maybe Allocate?
	bNameUnicode = NameLength < 0;

	if (NameLength < 0)
	{
		NameLength = (~NameLength + 1);
	}

	if (NameLength > 0x100) return -3;/*Invalid Entry*/

	Offset += bNameUnicode ? NameLength * 2 : NameLength;

	if (bNameUnicode)
	{
		Utils::Text::ConvertUnicodeToUTF8((W_CHAR*)Name, NameLength, (char*)Name, NameLength * 2);
	}

	if (Header->Version >= (int)PakFile_Version::IndexTextSection)
	{
		DataIndex = Utils::Memory::ReadBuffer<int32_t>(DirEntryStart + Offset);  Offset += sizeof(DataIndex);
		if (DataIndex < 0)
			DataIndex = ~DataIndex;
	}

	return Offset;
}
int EntryDirInfo::Parse(uint8_t* FileNameEntry)
{
	int Offset = 0;
	NameLength = Utils::Memory::ReadBuffer<int32_t>(FileNameEntry + Offset); Offset += sizeof(NameLength);

	if (NameLength > 0x100) return -1/*Invalid Entry*/;

	if (NameLength < 0)
	{
		PRINT(PrintType_ERROR, "Negetive Dir NameLength: %04d, No Implementation", NameLength);
	
	}
	else
	{
		Name = (char*)(FileNameEntry + Offset);
		Offset += NameLength;
	}

	FileCount = Utils::Memory::ReadBuffer<int64_t>(FileNameEntry + Offset); Offset += sizeof(FileCount);

	return Offset;
}
