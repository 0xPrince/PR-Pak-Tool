
#ifndef __OODLE_HEADER_INCLUDED__
#define __OODLE_HEADER_INCLUDED__

#if defined(_WIN32)
#define FUNCLINK __stdcall
#else
#define FUNCLINK
#endif
#ifdef _MSC_VER
#pragma pack(push, Oodle, 8)
#endif
//Enums
typedef enum OodleLZ_Decode_ThreadPhase
{
    OodleLZ_Decode_ThreadPhase1 = 1,
    OodleLZ_Decode_ThreadPhase2 = 2,
    OodleLZ_Decode_ThreadPhaseAll = 3,
    OodleLZ_Decode_Unthreaded = OodleLZ_Decode_ThreadPhaseAll
} OodleLZ_Decode_ThreadPhase;

typedef enum OodleLZ_CheckCRC
{
    OodleLZ_CheckCRC_No = 0,
    OodleLZ_CheckCRC_Yes = 1,
    OodleLZ_CheckCRC_Force32 = 0x40000000
} OodleLZ_CheckCRC;

typedef enum OodleLZ_Verbosity
{
    OodleLZ_Verbosity_None = 0,
    OodleLZ_Verbosity_Minimal = 1,
    OodleLZ_Verbosity_Some = 2,
    OodleLZ_Verbosity_Lots = 3,
    OodleLZ_Verbosity_Force32 = 0x40000000
} OodleLZ_Verbosity;




//Functions
extern "C" int FUNCLINK OodleLZ_Decompress(const void* compBuf, int compBufSize, void* rawBuf, int rawLen,
    int fuzzSafe = NULL,
    OodleLZ_CheckCRC checkCRC = OodleLZ_CheckCRC_No,
    OodleLZ_Verbosity verbosity = OodleLZ_Verbosity_None,
    void* decBufBase = NULL,
    int decBufSize = NULL,
    void* fpCallback = NULL,
    void* callbackUserData = NULL,
    void* decoderMemory = NULL,
    int decoderMemorySize = NULL,
    OodleLZ_Decode_ThreadPhase threadPhase = OodleLZ_Decode_Unthreaded
);
#ifdef _MSC_VER
#pragma pack(pop, Oodle)
#endif


#endif