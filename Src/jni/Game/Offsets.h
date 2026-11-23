#pragma once
#include "../pch.h"
#include "../Core/Pak/Structure.h"

namespace Offsets
{

	extern GameID Game_ID;
	const auto MaxCompressedBlockSize = 64 * 1024;
	const auto MaxBlockDataSize = 1024 * 1024;
	namespace Header
	{
		extern C_Array<unsigned char> Z_Fields_Key;
		extern C_Array<unsigned char> Z_Fields_IV;
		extern int Z_Field_Count;
		
		extern PakInfoHeader Keys;

		extern int PakFileHeaderPadding ;

	}
	namespace IndexData
	{
		extern int NonCompressedFileContent;
		
		extern C_Array<unsigned char> XORKey ;

		extern unsigned char* RSA_Exponent;
		extern unsigned int* RSA_Modulus;
	}
	namespace EntryBlock
	{
		extern C_Array<unsigned char*> KeyArraySM4;
		extern C_Array<unsigned char*> DynKeyArray;

		extern C_Array<unsigned char> XORKey;
		extern C_Array<unsigned char> SPKey;

		extern C_Array<unsigned char> SLM_Key;
		extern unsigned char ST_XorKey;
	}

	extern GameID GetGameID(unsigned int magic_num);
	extern void Init(GameID _GameID, unsigned int PakVersion);

}
