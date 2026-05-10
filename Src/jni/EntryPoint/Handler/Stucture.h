#pragma once

struct PakLoadData
{
	char* _PakFileName;
	PakManager* _PakManager;
	PakIndexHeader IndexHeader;
	C_Array<uint8_t> IndexData;
	C_Array<PakFileInfo> IndexEntries;
	uint8_t* TextSection;
	PakLoadData(PakManager* In_PakManager = NULL)
		: _PakFileName(NULL), _PakManager(In_PakManager), IndexHeader({}), TextSection(NULL)
	{ }
	~PakLoadData()
	{
		IndexEntries.clear(); IndexData.clear();
		IndexHeader = {};
	}
};