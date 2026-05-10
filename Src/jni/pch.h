#ifndef PCH_H
#define PCH_H

#define MAJOR_VERSION 1
#define MINOR_VERSION 0


//#ifndef NO_ZLIB
//#define NO_ZLIB
//#endif 
//
//#ifndef NO_ZSTD
//#define NO_ZSTD
//#endif

//#ifndef NO_OODLE
//#define NO_OODLE
//#endif 

#if defined(_WIN32)
#pragma comment(lib,"crypt32.lib")
#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"openssl\\libcrypto.lib")

#if !defined(NO_ZLIB)
#pragma comment(lib,"zlib\\libzlib.lib")
#endif 

#if !defined(NO_ZSTD)

#pragma comment(lib,"zstd\\libzstd.lib")
#endif
#if !defined(NO_OODLE)
#pragma comment(lib,"oodle\\oo2core.lib")
#endif
#elif !defined(__ANDROID__) && !defined(UNIX)
	compiling for a rocket? Non-Linux One?
#endif



#include <openssl/bn.h>
#include <openssl/evp.h>


#include <AES/AES.hpp>
#include <SHA/SHA.hpp>
#include <SM4/SM4.h>
#include <ZUC/ZUC.h>


#include <iostream>

#if defined(_WIN32)
#include <direct.h>
#elif defined(__ANDROID__) || defined(UNIX)
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "Macros.h"

#include "Utils/Custom/PRLog.h"

#include "Utils/Custom/CustomDataArray.hpp"
#include "Utils/Utils.h"
#include "Game/Offsets.h"

#include "Core/Pak/PakManager.h"

#endif //PCH_H
