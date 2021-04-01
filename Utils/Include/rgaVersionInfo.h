#pragma once

#define RGA_VERSION_MAJOR  2
#define RGA_VERSION_MINOR  4
#define RGA_VERSION_UPDATE 2
#define GEN_RGA_VERSION(MAJOR, MINOR, UPDATE)  MAJOR.MINOR.UPDATE
#define RGA_VERSION_MAJOR_MINOR  GEN_RGA_VERSION(RGA_VERSION_MAJOR, RGA_VERSION_MINOR, RGA_VERSION_UPDATE)
#define GEN_RGA_VERSION_STRING(VER) #VER
#define GEN_STR_RGA_VERSION(VER) GEN_RGA_VERSION_STRING(VER)

const char* const STR_RGA_VERSION = GEN_STR_RGA_VERSION(RGA_VERSION_MAJOR_MINOR);

#ifdef RGA_BUILD_NUMBER
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
const char* const STR_RGA_BUILD_NUM = STR(RGA_BUILD_NUMBER);
#else
#define RGA_BUILD_NUMBER 0
const char* const STR_RGA_BUILD_NUM = "0";
#endif
const char* const STR_RGA_OUTPUT_MD_DATA_MODEL = "1.0";

const char* const STR_RGA_BUILD_DATE_DEV = "Development build";

#ifdef RGA_BUILD_DATE
const char* const STR_RGA_BUILD_DATE = STR(RGA_BUILD_DATE);
#else
const char* const STR_RGA_BUILD_DATE = STR_RGA_BUILD_DATE_DEV;
#endif

#ifdef _DEBUG
const char* const STR_RGA_UPDATECHECK_URL = ".";
#else
const char* const STR_RGA_UPDATECHECK_URL = "https://api.github.com/repos/GPUOpen-Tools/RGA/releases/latest";
#endif

const char* const STR_RGA_UPDATECHECK_ASSET_NAME = "RGA-Updates.json";
