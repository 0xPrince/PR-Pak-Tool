#pragma once
/*
Author: 0xPrince
*/
extern bool bShowLogs;

enum PrintType
{
	PrintType_NONE = 0,
	PrintType_DEBUG,
	PrintType_LOG,
	PrintType_INFO,
	PrintType_ALERT,
	PrintType_ERROR,
	PrintType_WARNING,
	PrintType_SUCCESS
};
#define _NL "\n"

#define GetPref(_type,_fmt) \
_type == PrintType_DEBUG   ? 	"[DEBUG] - "	_fmt _NL :	\
_type == PrintType_LOG     ? 	/*"[LOG] - "*/	_fmt _NL :	\
_type == PrintType_INFO    ?	"[INFO] - "		_fmt _NL :	\
_type == PrintType_ALERT   ?	"[ALERT] - "	_fmt _NL :	\
_type == PrintType_ERROR   ?	"[ERROR] - "	_fmt _NL :	\
_type == PrintType_WARNING ?	"[WARN] - "		_fmt _NL :	\
_type == PrintType_SUCCESS ? 	"[SUCCESS] - "	_fmt _NL :	\
_fmt


#define PRINT(_type, _fmt, ...) \
!(bShowLogs == 0 && _type == PrintType_LOG) ? \
printf(GetPref(_type, _fmt /*_NL*/),##__VA_ARGS__) : \
NULL 
