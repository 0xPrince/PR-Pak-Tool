#include "UnPacker.h"
#include "Handler.hpp"

const char* Prefix_WDir = "Output/Unpack";

bool SetupOutputWD(const char* _FileName, uint64_t uid);

int UnPacker::Start(PakLoadData* _LoadData)
{
	auto PakHeader = _LoadData->_PakManager->GetPakFileInfo()->Header;
	if (!SetupOutputWD(_LoadData->_PakFileName, PakHeader.IndexOffset ^ PakHeader.IndexSize))
	{
		PRINT(PrintType_ERROR, "Failed To Setup Output Directory");
	}

	C_Array<char> TmpBuffer; TmpBuffer.reserve(512);
	Utils::IO::GetCurrWorkingDir(TmpBuffer.data(), TmpBuffer.capacity());

	PRINT(PrintType_INFO, "Output Path: %s\n", TmpBuffer.data());
	TmpBuffer.clear();

	auto StartTick = Utils::Ticks::GetCurrTick();
	Handler EntryUnPackHandler = Handler(_LoadData->_PakManager, &_LoadData->IndexHeader, &_LoadData->IndexData);

	int ProcessedEntryCount = 0;
	if (PakHeader.Version >= (int)PakFile_Version::IndexTextSection)
	{
		ProcessedEntryCount = EntryUnPackHandler.HandleTextSection(_LoadData->TextSection, &_LoadData->IndexEntries);
	}
	else
	{
		ProcessedEntryCount = EntryUnPackHandler.HandleDataSection(&_LoadData->IndexEntries);
	}

	if (ProcessedEntryCount < 0)
	{
		PRINT(PrintType_ERROR, "Pak Entries(%04u) Handing Failure, Status: %04d.", ProcessedEntryCount, _LoadData->IndexHeader.NumEntries);
		return -22;
	}
	
	PRINT(PrintType_INFO, "%04u/%04u Entries Unpacked in %f Seconds.",
		ProcessedEntryCount, _LoadData->IndexHeader.NumEntries,
		Utils::Ticks::GetSeconds(Utils::Ticks::GetCurrTick() - StartTick));

	if (ProcessedEntryCount < _LoadData->IndexHeader.NumEntries) return -416;
	return 1;
}


bool SetupOutputWD(const char* _FileName, uint64_t uid)
{
	C_Array<char> WD_Buffer;
	WD_Buffer.reserve(strlen(Prefix_WDir) + strlen(_FileName) + 0x20);
	snprintf(WD_Buffer.data(), WD_Buffer.capacity(), "%s/%s_%10llu/", Prefix_WDir, _FileName, uid ^ 0x41676B07);

	bool bSuccess = (Utils::IO::CreateDirRecusive(WD_Buffer.data(), '/') &&
		Utils::IO::ChangeWorkingDir(WD_Buffer.data()) == 1);
	WD_Buffer.clear();
	return bSuccess;
}