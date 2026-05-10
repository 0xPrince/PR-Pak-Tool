#pragma once
#include "../pch.h"
#include "../Unpacker/Unpacker.h"

class PakHandler
{
private:
	static int LoadPakFile(FILE* _PakFile, PakLoadData* _LoadData/*out*/);

public:
	static int UnPackPakFile(const char* _PakFilePath);

};