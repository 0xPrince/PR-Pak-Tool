#pragma once
#include "../pch.h"

/*
Author: 0xPrince
*/

namespace Utils
{
	namespace Memory
	{
		inline void* MemSet(void* _Dst, int _Val, size_t _Size)
		{
			return memset(_Dst, _Val, _Size);
		}

		inline void* MemCopy(void* _Dst, void const* _Src, size_t _Size)
		{
			return memcpy(_Dst, _Src, _Size);
		}

		inline void* AllocMem(size_t size)
		{
			return malloc(size);
		}

		inline void FreeMem(void* _Mem)
		{
			free(_Mem);
		}

		inline void ReadBuffer(void* dst, const uint8_t* src, size_t _size)
		{
			MemCopy(dst, src, _size);
		}

		template <typename type>
		inline type ReadBuffer(const uint8_t* src)
		{
			return *(type*)(src);
		}
	}

	namespace Text
	{
		inline size_t GetStringLength(const char* _String)
		{
			return strlen(_String);
		}
		inline int isStringContains(const char* _String, const char* MatchString)
		{
			return strstr(_String, MatchString) != NULL;
		}
		inline char* ToLowerInvariant(char* str, size_t len)
		{
			size_t index = 0;
			while (index < len)
			{
				str[index] = (char)tolower(str[index]);
				++index;
			}
			return str;
		}
		
		static int ConvertUnicodeToUTF8(W_CHAR* _src, size_t srcLen, char* _dst, size_t dstLen)
		{
			C_Array<char> UTF8_Buff; UTF8_Buff.reserve(dstLen);
			int32_t dIndx = 0;

			for (int32_t Indx = 0; Indx < srcLen; ++Indx)
			{
				size_t RemainingSize = dstLen - dIndx;
				if (RemainingSize <= 1 ) break;
				unsigned int code_point = (uint32_t)(_src[Indx]);

				if (code_point <= 0x7F)
				{
					UTF8_Buff[dIndx++] = (char)code_point;
					continue;
				}
				if (code_point <= 0x7FF)
				{
					if (RemainingSize < 2) break;
					UTF8_Buff[dIndx++] = (char)(code_point >> 6) | 0x80 | 0x40;
					UTF8_Buff[dIndx++] = (char)(code_point & 0x3F) | 0x80 ;
					continue;
				}
				if (code_point <= 0xFFFF)
				{
					if (RemainingSize < 3) break;
					UTF8_Buff[dIndx++] = (char)(code_point >> 12) | 0x80 | 0x40 | 0x20 ;
					UTF8_Buff[dIndx++] = (char)((code_point >> 6) & 0x3F) | 0x80;
					UTF8_Buff[dIndx++] = (char)(code_point & 0x3F) | 0x80 ;
					continue;
				}
				if (code_point <= 0x10FFFF)
				{
					if (RemainingSize < 4) break;
					UTF8_Buff[dIndx++] = (char)(code_point >> 18) | 0x80 | 0x40 | 0x20 | 0x10;
					UTF8_Buff[dIndx++] = (char)((code_point >> 12) & 0x3F) | 0x80;
					UTF8_Buff[dIndx++] = (char)((code_point >> 6) & 0x3F) | 0x80;
					UTF8_Buff[dIndx++] = (char)(code_point & 0x3F) | 0x80;
					continue;
				}
			}

			if (dIndx > 0)
			{
				if (dIndx > dstLen) dIndx = dstLen;
				Memory::MemCopy(_dst, UTF8_Buff.data(), dIndx);
			}
			
			UTF8_Buff.clear();

			return dIndx;
		}
		
	}



	namespace IO
	{
		inline char* GetCurrWorkingDir(char* _DstBuf, int   _DstBufSz)
		{
			return __getcwd(_DstBuf, _DstBufSz);
		}

		inline int ChangeWorkingDir(const char* _Path)
		{
			if (!_Path) return -1;
			return __chdir(_Path) == 0;
		}

		static const char* FixNTPathLimit(const char* _Path)
		{
			#if !defined(_WIN32)
			return _Path;
			#endif
			//mmmmmmmmmmmmmmmmmmmmmmmmm
			const char NT_Prefix[] = "\\\\?\\";
			const char WinDelimiter = '\\';
			if (_Path[2] == NT_Prefix[2]) return _Path;

			static C_Array<char> NT_TmpPath;  NT_TmpPath.reserve(512);

			NT_TmpPath.assign((char*)NT_Prefix, sizeof(NT_Prefix) - 1);
			
			int indx = NT_TmpPath.size();
			if (_Path[1] != ':' && GetCurrWorkingDir(NT_TmpPath.data() + NT_TmpPath.size(), NT_TmpPath.capacity() - NT_TmpPath.size()))
			{
				indx = Text::GetStringLength(NT_TmpPath.data());
				if (NT_TmpPath[indx] != WinDelimiter &&
					_Path[0] != WinDelimiter && _Path[0] != '/')
				{
					NT_TmpPath[indx] = WinDelimiter;
					++indx;
				}
				NT_TmpPath.resize(indx);
			}
			NT_TmpPath.append((char*)_Path, Text::GetStringLength(_Path) + 1);

			while (++indx < NT_TmpPath.size())
			{
				if (NT_TmpPath[indx] == '/') NT_TmpPath[indx] = WinDelimiter;
			}
			return NT_TmpPath.data();

		}

		inline FILE* _OpenFile(const char* _Path, const char* Mode)
		{
			_Path = FixNTPathLimit(_Path);
			return fopen(_Path, Mode);
		}

		inline int _CloseFile(FILE* _file)
		{
			if (!_file) return 0;
			return fclose(_file);
		}

		inline int isDirExists(const char* _Path)
		{
			_Path = FixNTPathLimit(_Path);
			//Not Really Good
			struct stat tstat;
			return stat(_Path, &tstat) == 0 &&
				_S_ISTYPE(tstat.st_mode, S_IFDIR);
		}

		inline int isFileExists(const char* _Path)
		{
			//Not Really Good
			struct stat tstat;
			return stat(_Path, &tstat) == 0 &&
				_S_ISTYPE(tstat.st_mode, S_IFREG);
		}

		inline int CreateDir(const char* _Path)
		{
			_Path = FixNTPathLimit(_Path);
			return __mkdir(_Path, 777);
		}


		static bool CreateDirRecusive(const char* _Path, const char Delimiter)
		{
			static C_Array<char> TempPath; //clear? whatever
			TempPath.resize(0);
			auto last_slash = strrchr(_Path+1, Delimiter);
			
			auto Path_Len = last_slash != NULL ? (last_slash - _Path) + 1 : Text::GetStringLength(_Path);
			if (Path_Len <= 0) return false;
			TempPath.assign((char*)_Path, Path_Len+1);
			TempPath[TempPath.size()-1] = '\0';

			if (isDirExists(TempPath.data()))
				return true;

			uint32_t Index = 0;
			while (++Index < TempPath.size())
			{
				if (TempPath.data()[Index] != Delimiter) continue;

				TempPath.data()[Index] = '\0';//Null Terminator
				CreateDir(TempPath.data());
				TempPath.data()[Index] = Delimiter;
			}

			return isDirExists(TempPath.data());
		}
		
		inline FILE* _CreateFile(const char* _Path, const char* _FileMode, const char Delimiter)
		{
			CreateDirRecusive(_Path, Delimiter);
			return _OpenFile(_Path, _FileMode);
		}
		
		inline int SeekFile(FILE* file, int64_t Offset, int Origin)
		{
			return __fseek(file, Offset, Origin);
		}

		inline int64_t GetFileSize(FILE* _file)
		{
			SeekFile(_file, 0, SEEK_END);
			auto file_size = __ftell(_file);
			SeekFile(_file, 0, SEEK_SET);
			return file_size;
		}

		inline size_t Read_File(FILE* file, int64_t Offset, void* _Buffer, size_t size, int Origin = SEEK_SET)
		{
			//seek ret
			SeekFile(file, Offset, Origin);
			return fread(_Buffer, 1, size, file);
		}

		inline size_t Write_File(FILE* file, uint8_t* _Buffer, size_t size)
		{
			return fwrite(_Buffer, 1, size, file);
		}

		inline size_t Write_File(FILE* file, int64_t Offset, uint8_t* _Buffer, size_t size)
		{
			//seek ret
			SeekFile(file, Offset, SEEK_SET);
			return Write_File(file, _Buffer, size);
		}

		inline bool isFileExtension(const char* FileName, const char* MatchExt)
		{
			const char* Lastdt = NULL;
			return (Lastdt = strrchr(FileName, '.')) != NULL
				&& strcmp(Lastdt, MatchExt) == NULL;
		}

		static void GetPathWithoutFileName(const char* _Path, char* _RetPath)
		{
		
			const char* LastSlash = strrchr(_Path+1, '/');
			if (LastSlash == NULL) LastSlash = strrchr(_Path + 1, '\\');

			if (LastSlash != NULL)
			{
				size_t PathLen = LastSlash - _Path;
				strncpy(_RetPath, _Path, PathLen);
				_RetPath[PathLen] = '\0';
				return;
			}
			strcpy(_RetPath, _Path);
		}

		static void GetFileNameWithoutExt(const char* _Path, char* _FileName)
		{
			const char* LastSlash = strrchr(_Path, '/');
			if (LastSlash == NULL) LastSlash = strrchr(_Path, '\\');

			const char* FileNameStart = (LastSlash != NULL) ? LastSlash + 1 : _Path;

			const char* FileExt = strrchr(FileNameStart, '.');
			if (FileExt != NULL)
			{
				size_t NameLen = FileExt - FileNameStart;
				strncpy(_FileName, FileNameStart, NameLen);
				_FileName[NameLen] = '\0';
				return;
			}
			strcpy(_FileName, FileNameStart);
		}
	}

	namespace Ticks
	{
		inline long GetCurrTick()
		{
			return clock();
		}
		inline double GetSeconds(long ticks)
		{
			return (double)ticks / CLOCKS_PER_SEC;
		}
	}

	namespace Misc
	{
		template <typename T>
		inline constexpr T Align(T Val, uint64_t Alignment)
		{
			//(Val + Alignment) & (1 - Alignment)
			return (T)(((uint64_t)Val + Alignment - 1) & ~(Alignment - 1));
		}

		template <typename T>
		inline constexpr T AlignDown(T Val, uint64_t Alignment)
		{
			return (T)(((uint64_t)Val) & ~(Alignment - 1));
		}

		inline void ReverseBuffer(unsigned char* arr, size_t len)
		{
			unsigned char tmp_ch = 0x00;
			for (size_t Index = 0; Index < len / 2; Index++)
			{
				tmp_ch = arr[Index];
				arr[Index] = arr[len - 1 - Index];
				arr[len - 1 - Index] = tmp_ch;
			}
		}

		inline void XORBuffers(unsigned char* Buff_a, size_t _size, unsigned char* Buff_b, size_t Buff_b_Sz)
		{
			for (size_t index = 0; index < _size; index++)
			{
				Buff_a[index] ^= *(uint8_t*)(Buff_b + (index % Buff_b_Sz));
			}
		}
		inline int CmpBuffers(void const* _Buf1, void const* _Buf2, size_t _Len)
		{
			return !memcmp(_Buf1, _Buf2, _Len);
		}
		
		static int CMPHash_SHA1(C_Array<uint8_t>* _Data, uint8_t* _Hash)
		{
			static C_Array<uint8_t> HashBuff;
			HashBuff.reserve(SHA1::HASH_SIZE);

			SHA1::Hash(_Data->data(), _Data->size(), HashBuff.data());
			HashBuff.resize(HashBuff.capacity());

			bool bMatched = Utils::Misc::CmpBuffers(_Hash, HashBuff.data(), HashBuff.size());
			HashBuff.resize(0);
		//	HashBuff.clear();
			return bMatched;
		}

	}
}
