#pragma once

enum class PakFile_Version
{
	NONE = 0x00,
	MultiCompression		=	0x03,
	IndexXOREncryption		=	0x04,
	IndexHashEncryption		=	0x06,
	IndexAESEncryption		=	0x08,
	BlockAESEncryption		=	0x08,
	DictionaryCompression	=	0x0A,
	BlockSM4Encryption		=	0x0A,
	BlockLCGIndex			=	0x0A,
	IndexTextSection		=	0x0A,
	MultipleEncryption		=	0x0C,
	BlockSPEncryption		=	0x0D,
	BlockDynEncryption		=	0x0E
};


enum class InternalEncryptionFlags
{
	UNKNOWN = -1,
	NONE = 0,
	XOR,
	SP,
	SM4,
	DYN
};
enum class EntryEncryptionFlag : int
{
	NONE		= 0,
	XOR			= 1 << 0,
	SM4_PH1		= 1 << 1,
	SM4_PH2		= 1 << 2,
	SP			= 1 << 4,
	DYN,
	DYN_SM4		= DYN,
	DYN__		= 19,
	DYN_END ,
	SM4_PH3		= 1 << 5,

	//CN Internal
	LM_SXR		= 1 << 10,
	LM_STXR		= 1 << 11,
};


enum class GameID
{
	NONE = 0,
	PUBGM_GLOBAL,
	PUBGM_LITE,
	PUBGM_CN

};