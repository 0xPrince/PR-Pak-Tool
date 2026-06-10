#include "PakHandler.hpp"
//#include "../Packer/Packer.h"


int PakHandler::UnPackPakFile(const char* _PakFilePath)
{
	FILE* PakFile = Utils::IO::_OpenFile(_PakFilePath, "rb");
	if (!PakFile)
	{
		PRINT(PrintType_ERROR, "Pak File Access Failed");
		return -1;
	}
	PakLoadData LoadData = PakLoadData(new PakManager());

	PRINT(PrintType_INFO, "Loading Pak File...");
	auto LoadPakStatus = LoadPakFile(PakFile,&LoadData);
	if (LoadPakStatus != 1)
	{
		PRINT(PrintType_ERROR, "Pak File Loading Failed, Status: %04d", LoadPakStatus);
		Utils::IO::_CloseFile(PakFile);
		return -0x10AD;
	}
	PRINT(PrintType_SUCCESS, "Pak File Loaded.");

	PRINT(PrintType_NONE, "----------Unpacking Pak File----------\n\n");

	LoadData._PakFileName = (char*)Utils::Memory::AllocMem(Utils::Text::GetStringLength(_PakFilePath));
	Utils::IO::GetFileNameWithoutExt(_PakFilePath, LoadData._PakFileName);

	auto UnpackStatus = UnPacker::Start(&LoadData);

	Utils::Memory::FreeMem(LoadData._PakFileName);
	Utils::IO::_CloseFile(PakFile);

	return UnpackStatus;
}





int PakHandler::LoadPakFile(FILE* _PakFile, PakLoadData* _LoadData)
{
	auto _PakManager = _LoadData->_PakManager;
	if (!_PakManager) return -1;

	auto SetupStatus = _PakManager->Setup(_PakFile);
	if (SetupStatus != 1)
	{
		PRINT(PrintType_ERROR, "Failed To Setup Pak Data, Status: %04d", SetupStatus);
		return -1;
	}

	auto PakHeader = &_PakManager->GetPakFileInfo()->Header;
	GameID& Game_ID = _PakManager->GetGameID();
	if ((Game_ID = Offsets::GetGameID(PakHeader->Magic)) == GameID::NONE)
	{
		PRINT(PrintType_ERROR, "Unsupported or Invalid File");
		return -2;
	}

	PRINT(PrintType_INFO, "Pak Version: %u", PakHeader->Version);
	Offsets::Init(Game_ID, PakHeader->Version);
	auto IdxInf_Status = _PakManager->InitIndexInfo();
	if (IdxInf_Status != 1)
	{
		PRINT(PrintType_ERROR, "Invalid IndexInfo - Unsupported or Invalid File, Status: %04d", IdxInf_Status);
		return -3;
	}

GetIndexDataCode:
	auto IndexDataStatus = _PakManager->GetIndexData(&_LoadData->IndexData);
	if (IndexDataStatus != 1 || _LoadData->IndexData.size() <= 0)
	{
		PRINT(PrintType_ERROR, "Index Data Decryption Failed, Status: %04d", IndexDataStatus);

		if (IndexDataStatus == -3/*Decryption Failure*/ &&
			Game_ID == GameID::PUBGM_GLOBAL)
		{
			//There's no obvious difference in both Game's Pak Files To Detect Game
			Offsets::Init(Game_ID = GameID::PUBGM_LITE, PakHeader->Version);
			PRINT(PrintType_INFO, "Retrying AS GameID: %04d", Game_ID);
			goto GetIndexDataCode;
		}

		return -4;
	}



	if (_PakManager->ValidateIndexHash(&_LoadData->IndexData) != 1)
	{
		_LoadData->IndexData.clear();
		PRINT(PrintType_ERROR, "Hash Mismatched, Possible an Invalid IndexData or a Decryption Failure");
		return -5;
	}

	_LoadData->IndexHeader = {};
	int IndxHeaderStatus = _LoadData->IndexHeader.Parse(&_LoadData->IndexData);
	if (IndxHeaderStatus != 1 || _LoadData->IndexData.size() <= 0 ||
		_LoadData->IndexHeader.NumEntries < 1 || _LoadData->IndexHeader.NumEntries > 1000 * 1000/*Just a Safe Number*/)
	{
		PRINT(PrintType_ERROR, "Status: %04d - Invalid Entries, Possible IndexData is Invalid or a IndexData Decryption Failure",
		IndxHeaderStatus);
		return -6;
	}



	auto EntryDataSectionEnd = _PakManager->GetIndexEntries(&_LoadData->IndexEntries, &_LoadData->IndexHeader);
	if (EntryDataSectionEnd <= 0)
	{
		PRINT(PrintType_ERROR, "Status: %04lld - Failed to Get File Entries, %04u, %04u", EntryDataSectionEnd, _LoadData->IndexEntries.size(), _LoadData->IndexHeader.NumEntries);
		return -7;
	}

	_LoadData->TextSection = NULL;
	if (PakHeader->Version >= (int)PakFile_Version::IndexTextSection)
	{
		_LoadData->TextSection = _PakManager->GetTextSection(_LoadData->IndexHeader.FileEntryList + EntryDataSectionEnd, &_LoadData->IndexData);
		if (_LoadData->TextSection == NULL)
		{
			PRINT(PrintType_ERROR, "Failed to Get Text Section,Index: 0x%llX(0x%llX), Version: %03d",
				PakHeader->IndexOffset, PakHeader->IndexSize, PakHeader->Version);

			return -8;
		}
	}

	return 1;
}
