#pragma once

class Handler
{
private:
	C_Array<uint8_t> CompressedMemory;
	C_Array<uint8_t> UnCompressedMemory;
	C_Array<uint8_t>* pIndexData;
	PakManager* _PakManager = NULL;
	PakIndexHeader* _IndexHeader = NULL;
private:
	int HandleCompressedEntry(PakFileInfo* _FileInfo, PakCompressedBlock* Block, size_t _EntryPos);
	int HandleNonCompressedEntry(PakFileInfo* _FileInfo);
	int ReadVerifyEntryContent(PakEntry* _entry, C_Array<uint8_t>* _ContentBuffer);
	int GetEntryFullPath(PakFileInfo* _EntryInfo, EntryDirInfo* _DIRinfo, C_Array<char>* OutEntryPath);
	int SaveDataToDisk(FILE* OutFile, C_Array<uint8_t>* Buffer);
	void LogEntry(PakFileInfo* _EntryInfo);

public:
	Handler(PakManager* In_PakManager, PakIndexHeader* In_IndexHeader, C_Array<uint8_t>* In_pIndexData)
		: _PakManager(In_PakManager), _IndexHeader(In_IndexHeader), pIndexData(In_pIndexData)
	{
		CompressedMemory.reserve(Offsets::MaxBlockDataSize);
		UnCompressedMemory.reserve(Offsets::MaxBlockDataSize);
	}
	~Handler()
	{
		CompressedMemory.clear(); UnCompressedMemory.clear();
		_PakManager = NULL; _IndexHeader = NULL;
	}

	int HandleEntry(PakFileInfo* _FileInfo);
	int HandleDataSection(C_Array<PakFileInfo>* _FileMetaList);
	int HandleTextSection(uint8_t* TextSection, C_Array<PakFileInfo>* _FileMetaList);
};



int Handler::HandleDataSection(C_Array<PakFileInfo>* _FileMetaList)
{
	int ProcessedCount = 0;
	C_Array<char> TempFileName;				TempFileName.reserve(512);
	for (size_t DataIndex = 0; DataIndex < _FileMetaList->size(); DataIndex++)
	{
		auto _FileInfo = &_FileMetaList->data()[DataIndex];
		_FileInfo->FileMeta.DataIndex = DataIndex;

		if (GetEntryFullPath(_FileInfo, NULL, &TempFileName) != 1)
		{
			PRINT(PrintType_ERROR, "Failed to Get Entry Path, [%u](%u)",
				_FileInfo->FileMeta.DataIndex, _FileInfo->FileMeta.NameLength);
			continue;
		}
		_FileInfo->FileMeta.Name = TempFileName.data();

		LogEntry(_FileInfo);

		auto EntryHandleStatus = HandleEntry(_FileInfo);
		if (EntryHandleStatus != 1)
		{
			PRINT(PrintType_ERROR, "Failed to Handle File, Status: %04d, Name: %s", EntryHandleStatus, _FileInfo->FileMeta.Name);
			continue;
		}
		++ProcessedCount;
	}
	TempFileName.clear();
	return ProcessedCount;
}


int Handler::HandleTextSection(uint8_t* TextSection, C_Array<PakFileInfo>* _FileMetaList)
{

	int ProcessedCount = 0;
	C_Array<char> TempFileName;				TempFileName.reserve(512);

	auto TotalDirCount = Utils::Memory::ReadBuffer<uint64_t>(TextSection + 0x00); TextSection += sizeof(TotalDirCount);

	FileData _DataFileinfo; EntryDirInfo _DIRinfo = {};
	int64_t DirCurrentIndex = 0, CurrentFileInfoPos = 0;
	int64_t CurrDirBlockSize = 0;

	while (DirCurrentIndex++ < TotalDirCount)
	{
		CurrDirBlockSize = _DIRinfo.Parse(TextSection + CurrentFileInfoPos);
		if (CurrDirBlockSize <= 0)
		{
			PRINT(PrintType_ERROR, "Pak DIR Info Parsing Failed, Status: %02lld", CurrDirBlockSize);
			break;
		}
		CurrentFileInfoPos += CurrDirBlockSize;

		for (size_t FileIndex = 0; FileIndex < _DIRinfo.FileCount; FileIndex++)
		{

			CurrDirBlockSize = _DataFileinfo.Parse(TextSection + CurrentFileInfoPos, &_PakManager->GetPakFileInfo()->Header);
			if (CurrDirBlockSize <= 0)
			{
				PRINT(PrintType_ERROR, "Pak Entry Meta Info Parsing Failed, Status: %02lld", CurrDirBlockSize);
				break;
			}
			CurrentFileInfoPos += CurrDirBlockSize;


			if (_DataFileinfo.DataIndex > _IndexHeader->NumEntries)
			{
				PRINT(PrintType_ERROR, "Invalid Data Index, Index %02d, FilePath: %s", _DataFileinfo.DataIndex, TempFileName.data());
				continue;
			}

			auto _FileInfo = &_FileMetaList->data()[_DataFileinfo.DataIndex];
			_FileInfo->FileMeta = _DataFileinfo;

			if (GetEntryFullPath(_FileInfo, &_DIRinfo, &TempFileName) != 1)
			{
				PRINT(PrintType_ERROR, "Failed to Get Entry Path, [%u](%u)",
					_FileInfo->FileMeta.DataIndex, _FileInfo->FileMeta.NameLength);
				continue;
			}
			_FileInfo->FileMeta.Name = TempFileName.data();

			LogEntry(_FileInfo);

			auto EntryHandleStatus = HandleEntry(_FileInfo);
			if (EntryHandleStatus != 1)
			{
				PRINT(PrintType_ERROR, "Failed to Handle File, Status: %04d, Name: %s", EntryHandleStatus, _FileInfo->FileMeta.Name);
				continue;
			}
			++ProcessedCount;
		}
	}
	TempFileName.clear();
	return ProcessedCount;
}


int Handler::HandleEntry(PakFileInfo* _FileInfo)
{
	auto OutFile = Utils::IO::_CreateFile(_FileInfo->FileMeta.Name, "wb", '/');
	if (!OutFile)
	{
		PRINT(PrintType_ERROR, "Failed to Create Pak Entry File");
		return -1;
	}

	if (ReadVerifyEntryContent(&_FileInfo->entry, &UnCompressedMemory) == 0)
	{
		PRINT(PrintType_WARNING, "Content Hash Mismatched, Pak Entry is Corrupted, Output May NOT be Valid,[0x%llX](%llu) - %s",
			_FileInfo->entry.Offset, _FileInfo->entry.CompressedSize, _FileInfo->FileMeta.Name);
	}

	int EntryStatus = 0;
	size_t ProcessedSize = 0;
	if (_FileInfo->entry.CompressionMethod != (int)CompressionFlags::COMPRESS_None)
	{
		static C_Array<int> BlockIndices;
		_PakManager->GetBlockIndices(&_FileInfo->entry, &BlockIndices, true);

		for (int Indx = 0; Indx < _FileInfo->entry.CompressionBlocks.size; Indx++)
		{
			int BlockIndex = BlockIndices[Indx];
			auto EntryBlock = _FileInfo->entry.CompressionBlocks[BlockIndex];

			EntryStatus = HandleCompressedEntry(_FileInfo, &EntryBlock, ProcessedSize);
			if (EntryStatus != 1)
			{
				PRINT(PrintType_ERROR, "Failed To Handle Compressed Block: %04d(%04d)/%04d, Status: %04d", Indx, BlockIndex, _FileInfo->entry.CompressionBlocks.size, EntryStatus);
				return EntryStatus;
			}

			EntryStatus = SaveDataToDisk(OutFile, &UnCompressedMemory);
			ProcessedSize += UnCompressedMemory.size();
		}

	}
	else
	{
		EntryStatus = HandleNonCompressedEntry(_FileInfo);
		if (EntryStatus != 1)
		{
			PRINT(PrintType_ERROR, "Failed To Handle NonCompressed Entry, Status: %04d", EntryStatus);
			return EntryStatus;
		}

		EntryStatus = SaveDataToDisk(OutFile, &UnCompressedMemory);
		ProcessedSize += UnCompressedMemory.size();
	}

	if (EntryStatus == 1 && ProcessedSize != _FileInfo->entry.Size)
	{
		PRINT(PrintType_WARNING, "Processed Size != Actual Size Mismatched, Output May NOT be valid, FileName %s", _FileInfo->FileMeta.Name);
	}

	fclose(OutFile);

	return  EntryStatus;
}

int Handler::HandleCompressedEntry(PakFileInfo* _FileInfo, PakCompressedBlock* Block, size_t _EntryPos)
{
	size_t BlockSize = Block->GetSize();
	if (BlockSize <= 0) return -1;
	size_t BlockReadSize = _FileInfo->entry.bEncrypted == 0x01 ? Utils::Misc::Align(BlockSize, 16) : BlockSize;

	CompressedMemory.reserve(BlockReadSize); UnCompressedMemory.reserve(_FileInfo->entry.CompressionBlockSize);

	if (_PakManager->Read(Block->CompressedStart, CompressedMemory.data(), BlockReadSize) != BlockReadSize)
	{
		PRINT(PrintType_ERROR, "Failed To Read Compressed Block, Start: 0x%llx[%u]", Block->CompressedStart, BlockReadSize);
		return -1;
	}
	CompressedMemory.resize(BlockReadSize);

	if (_FileInfo->entry.bEncrypted)
	{
		auto DecBlockStatus = _PakManager->DecryptBlock(_FileInfo, &CompressedMemory);
		if (DecBlockStatus != 1)
		{
			PRINT(PrintType_ERROR, "Entry Block Decryption Failed, Status: %04d", DecBlockStatus);
			return -1;
		}
	}

	CompressedMemory.resize(BlockSize);

	auto UnCompressedBlockSize = _min(_FileInfo->entry.Size - _EntryPos, _FileInfo->entry.CompressionBlockSize);

	UnCompressedMemory.resize(0);
	auto UnCompressStatus = Compression::UncompressMemory(
		(CompressionFlags)_FileInfo->entry.CompressionMethod,
		&CompressedMemory, &UnCompressedMemory,
		UnCompressedBlockSize,
		&_IndexHeader->CompressionDictionary);

	if (UnCompressStatus != 1)
	{
		PRINT(PrintType_ERROR, "Failed to Uncompress(%04d) File, Most Likely Decryption(%04d) Failed, Status: %04d",
			_FileInfo->entry.CompressionMethod, _FileInfo->entry.EncryptionFlag, UnCompressStatus);
		return -1;
	}

	auto DecBlockStatus = _PakManager->DecryptBlockPH2(_FileInfo, _EntryPos, &UnCompressedMemory);
	if (DecBlockStatus != 1)
	{
		PRINT(PrintType_ERROR, "Entry Block PH2-Decryption(%04d) Failed, Status: %04d",
			_FileInfo->entry.EncryptionFlag, DecBlockStatus);
		return -1;
	}

	return 1;
}

int Handler::HandleNonCompressedEntry(PakFileInfo* _FileInfo)
{
	uint64_t EntryStart = _FileInfo->entry.Offset + Offsets::IndexData::NonCompressedFileContent;
	int64_t EntrySize = _FileInfo->entry.Size;
	if (EntrySize <= 0) return 1;

	if (UnCompressedMemory.size() != EntrySize)
	{
		PRINT(PrintType_ERROR, "Failed To Read Non-Compressed Entry, Start: 0x%llX(%llu)", EntryStart, EntrySize);
		return -1;
	}

	if (_FileInfo->entry.bEncrypted)
	{
		auto DecBlockStatus = _PakManager->DecryptBlock(_FileInfo, &UnCompressedMemory);
		if (DecBlockStatus != 1)
		{
			PRINT(PrintType_ERROR, "Non-Compressed Entry Decryption Failed, Status: %04d", DecBlockStatus);
			return -1;
		}
	}
	return 1;
}


int Handler::ReadVerifyEntryContent(PakEntry* _entry, C_Array<uint8_t>* _ContentBuffer)
{
	_ContentBuffer->resize(0);

	auto EntryStart = _entry->Offset;
	size_t EntrySize = _entry->CompressedSize;

	if (_entry->CompressionMethod == (int)CompressionFlags::COMPRESS_None)
	{
		EntryStart += Offsets::IndexData::NonCompressedFileContent;
		EntrySize = _entry->Size;//just in case
	}
	if (EntrySize <= 0) return true;

	_ContentBuffer->reserve(EntrySize);
	if (_PakManager->Read(EntryStart, _ContentBuffer->data(), EntrySize) != EntrySize) return -1;/*Read Failure*/
	_ContentBuffer->resize(EntrySize);

	//TODO: FixMe
	if (_entry->CompressionMethod != (int)CompressionFlags::COMPRESS_None)  return 1;

	return Utils::Misc::CMPHash_SHA1(_ContentBuffer, _entry->Hash);
}


int Handler::GetEntryFullPath(PakFileInfo* _EntryInfo, EntryDirInfo* _DIRinfo, C_Array<char>* OutEntryPath)
{
	if (!_EntryInfo || !OutEntryPath) return -1;

	if (_EntryInfo->FileMeta.NameLength <= 0)
	{
		PRINT(PrintType_ERROR, "Invalid Entry NameLen: %02d", _EntryInfo->FileMeta.NameLength);
		return -1;
	}

	snprintf(OutEntryPath->data(), OutEntryPath->capacity(), "%s%s%s",
		_IndexHeader->bIsMiniPak ? _IndexHeader->MountPoint : "",
		(_DIRinfo && _DIRinfo->NameLength > 0) ? _DIRinfo->Name : "",
		_EntryInfo->FileMeta.Name);

	return 1;
}

int Handler::SaveDataToDisk(FILE* OutFile, C_Array<uint8_t>* Buffer)
{
	auto WrittenSize = Utils::IO::Write_File(OutFile, Buffer->data(), Buffer->size());
	if (WrittenSize != Buffer->size())
	{
		PRINT(PrintType_ERROR, "Failed to Write Complete File, Status: %04u != %04u", WrittenSize, Buffer->size());
		return 0x300;
	}
	return 1;
}

void Handler::LogEntry(PakFileInfo* _EntryInfo)
{
	PRINT(PrintType_LOG, "[%04d](%llu) FileName: %s",
		_EntryInfo->FileMeta.DataIndex, _EntryInfo->entry.Size, _EntryInfo->FileMeta.Name);
}
