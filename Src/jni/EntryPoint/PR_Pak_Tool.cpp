#include "Handler/PakHandler.hpp"
bool bShowLogs = false;


char* Exe_Dir_Path = NULL;
char* uOutputPath = NULL;

enum class UserRequest
{
	NONE = 0,
	UNPACK_PAK
};
int HandleRequest(char* _Path, UserRequest Request)
{
	PRINT(PrintType_INFO, "Path: %s", _Path);
	PRINT(PrintType_INFO, "Verbose Mode: %s", GetBoolStateStr(bShowLogs));
	PRINT(PrintType_NONE, "--------------------------------------\n\n");

	if (Request == UserRequest::UNPACK_PAK)
	{
		if ((!uOutputPath || !Utils::IO::CreateDirRecusive(uOutputPath, '/')) ||
			(Utils::IO::ChangeWorkingDir(uOutputPath ? uOutputPath : Exe_Dir_Path/*Or use Pak Source Dir*/) != 1 && uOutputPath)) 
			Utils::IO::ChangeWorkingDir(Exe_Dir_Path);

		auto UnpackStatus = PakHandler::UnPackPakFile(_Path);
		if (UnpackStatus != 1)
		{
			return UnpackStatus;
		}
		return EXIT_SUCCESS;
	}

	PRINT(PrintType_ERROR, "Invalid User Request");
	return EXIT_FAILURE;
}

void GetExeDirPath(char* argv[], int argc)
{
	if (argc <= 0) return;

	Exe_Dir_Path = (char*)Utils::Memory::AllocMem(Utils::Text::GetStringLength(argv[0]));
	if (Exe_Dir_Path) Utils::IO::GetPathWithoutFileName(argv[0], Exe_Dir_Path);
}

bool GetArg(char* Args[], int Count, const char* ReqArgName, bool bReqValue, char** _ArgVal = NULL)
{
	for (int ArgIndex = 2; ArgIndex < Count; ArgIndex++)
	{
		auto ard = Args[ArgIndex];
		if (Utils::Text::isStringContains(Args[ArgIndex], ReqArgName))
		{
			if (!bReqValue) return true;
			if (!_ArgVal) break;

			if (++ArgIndex < Count)
			{
				*_ArgVal = Args[ArgIndex];
				return true;
			}

		}
	}
	return false;
}
bool ParseArguments(char* Args[], int ACount, char** _Path)
{
	if (ACount <= 1) 
	{
		PRINT(PrintType_ERROR, "No Source File Path");
		return false;
	}

	if (Utils::Text::GetStringLength(Args[1]) > 1)
	{
		if (!Utils::IO::isFileExists(Args[1]))
		{
			PRINT(PrintType_ERROR, "Invalid File Path, Not Exists");
			return false;
		}
		*_Path = Args[1];
	}
	if (GetArg(Args, ACount, "-verbose", false))
	{
		bShowLogs = true;
	}
	GetArg(Args, ACount, "-output", true, &uOutputPath);

	return true;

}
void PrintHeader()
{
	PRINT(PrintType_NONE, "=======================================\n");
	PRINT(PrintType_NONE, "\t    PR Pak Tool v%d.%02d\n", MAJOR_VERSION, MINOR_VERSION);
	PRINT(PrintType_NONE, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	PRINT(PrintType_NONE, "\tFree Open Source Project\n");
	PRINT(PrintType_NONE, "\t Developed By: 0xPrince\n");
	PRINT(PrintType_NONE, "=======================================\n\n");
}
void PrintUsage()
{
	PRINT(PrintType_NONE, "\n[USAGE] - <PakFilePath> <options>(Optional)\n");
	PRINT(PrintType_NONE, "Note: Currently Supports Pak Unpacking Only\n");
	PRINT(PrintType_NONE, "--------------Options--------------\n");
	PRINT(PrintType_NONE, " -verbose\tShows Process Logs if Enabled(May Affect The Speed)[%s].\n", GetBoolStateStr(bShowLogs));
	PRINT(PrintType_NONE, " -output\tOutput Dir Path[Default: %s].\n","App_Starting_Path/");
	PRINT(PrintType_NONE, "-----------------------------------\n\n");
}
extern "C" int main(int argc, char* argv[])
{
	setlocale(LC_ALL, ".UTF8");
	
	GetExeDirPath(argv, argc);
	PrintHeader();

	char* uFile_Path = NULL;
	if (!ParseArguments(argv, argc, &uFile_Path) || uFile_Path == NULL)
	{
		PrintUsage();
		return EXIT_FAILURE;
	}

	int RetStatus = HandleRequest(uFile_Path, UserRequest::UNPACK_PAK);

	return RetStatus;
}
