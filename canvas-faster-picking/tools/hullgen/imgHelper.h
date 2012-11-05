#pragma once
#include <string>
#include <assert.h>
#include <math.h>


typedef unsigned char      uint8;
typedef signed char        int8;
typedef unsigned short     uint16;
typedef signed short       int16;
typedef unsigned int       uint32;
typedef uint32             uint;
typedef signed int         int32;
typedef unsigned __int64   uint64;
typedef signed __int64     int64;

const uint8  UINT8_MIN  = 0;
const uint8  UINT8_MAX  = 0xFFU;
const uint16 UINT16_MIN = 0;
const uint16 UINT16_MAX = 0xFFFFU;
const uint32 UINT32_MIN = 0;
const uint32 UINT32_MAX = 0xFFFFFFFFU;
const uint64 UINT64_MIN = 0;
const uint64 UINT64_MAX = 0xFFFFFFFFFFFFFFFFui64;

const int8  INT8_MIN  = -128;
const int8  INT8_MAX  = 127;
const int16 INT16_MIN = -32768;
const int16 INT16_MAX = 32767;
const int32 INT32_MIN = (-2147483647 - 1);
const int32 INT32_MAX = 2147483647;
const int64 INT64_MIN = (-9223372036854775807i64 - 1);
const int64 INT64_MAX = 9223372036854775807i64;

#include "math.h"

struct ImageData
{
   std::string imgFileName;
	unsigned int uniqueID;
	unsigned int width;
	unsigned int height;
	char* pImgData;
	unsigned int imgSizeInBytes;
	unsigned char imgFormat;
};

struct RGBAColor
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};
struct RGBColor
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
};
 
bool loadImageData(const char* pFilename, ImageData& id);
bool saveImageData(const char* pFilename, ImageData& id);