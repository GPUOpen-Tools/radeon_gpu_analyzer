#pragma once

#define RGA_VERSION_MAJOR  2
#define RGA_VERSION_MINOR  9
#define RGA_VERSION_UPDATE 0
#define GEN_RGA_VERSION(MAJOR, MINOR, UPDATE)  MAJOR.MINOR.UPDATE
#define RGA_VERSION_MAJOR_MINOR  GEN_RGA_VERSION(RGA_VERSION_MAJOR, RGA_VERSION_MINOR, RGA_VERSION_UPDATE)
#define GEN_RGA_VERSION_STRING(VER) #VER
#define GEN_kStrRgaVersion(VER) GEN_RGA_VERSION_STRING(VER)

const char* const kStrRgaVersion = GEN_kStrRgaVersion(RGA_VERSION_MAJOR_MINOR);

#ifdef RGA_BUILD_NUMBER
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
const char* const kStrRgaBuildNum = STR(RGA_BUILD_NUMBER);
#else
#define RGA_BUILD_NUMBER 0
const char* const kStrRgaBuildNum = "0";
#endif
const char* const kStrRgaOutputMdDataModel = "1.0";

const char* const kStrRgaBuildDateDev = "Development build";

#ifdef RGA_BUILD_DATE
const char* const kStrRgaBuildDate = STR(RGA_BUILD_DATE);
#else
const char* const kStrRgaBuildDate = kStrRgaBuildDateDev;
#endif

#ifdef _DEBUG
const char* const kStrRgaUpdatecheckUrl = ".";
#else
const char* const kStrRgaUpdatecheckUrl = "https://api.github.com/repos/GPUOpen-Tools/RGA/releases/latest";
#endif

const char* const kStrRgaUpdatecheckAssetName = "RGA-Updates.json";
