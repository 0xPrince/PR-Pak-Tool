
#ifndef MACROS_H
#define MACROS_H

#define W_CHAR unsigned short

#define GetBoolStateStr(bState) (bState ? "Enabled" : "Disabled")
#define _max(a,b) (((a) > (b)) ? (a) : (b))
#define _min(a,b) (((a) < (b)) ? (a) : (b))
#define offset_of(s,m) ((unsigned int)&(((s*)0)->m))


#ifndef _S_ISTYPE
#define _S_ISTYPE(mode, mask)  (((mode) & S_IFMT) == (mask))
#endif
#ifndef CON_INTS
#define CON_INTS(a,b,type) ((type)a << sizeof(a) * sizeof(type)) | b
#endif

#if !defined(__ASSERT)
#if defined(NDEBUG)
#define __ASSERT(expression,msg) ((void)0)
#else
//_STL_ASSERT
#include <assert.h>
#define __ASSERT(expression,msg) (void)(                                                       \
            (!!(expression)) ||                                                              \
            (_wassert(_CRT_WIDE(#expression), _CRT_WIDE(#msg), (unsigned)(__LINE__)), 0))
#endif 
#endif//__ASSERT

#if defined(_WIN32)
#define __getcwd _getcwd
#define __chdir _chdir
#define __mkdir(_Path,perm) _mkdir(_Path)
#define __fseek(_Stream,_Offset,_Origin) _fseeki64(_Stream,_Offset,_Origin)
#define __ftell(_Stream) _ftelli64(_Stream)


#elif defined(__ANDROID__) || defined(UNIX)
#define __getcwd getcwd
#define __chdir chdir
#define __mkdir(_Path,perm) mkdir(_Path,perm)
#define __fseek(_Stream,_Offset,_Origin) fseek(_Stream,_Offset,_Origin)
#define __ftell(_Stream) ftell(_Stream)
#endif

#endif //MACROS_H