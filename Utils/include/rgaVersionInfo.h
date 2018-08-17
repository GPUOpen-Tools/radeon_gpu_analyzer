#pragma once

const char* const STR_RGA_VERSION = "2.0.1305";
#ifdef RGA_BUILD_NUMBER
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
const char* const STR_RGA_BUILD_NUM = STR(RGA_BUILD_NUMBER);
#else
const char* const STR_RGA_BUILD_NUM = "0";
#endif
const char* const STR_RGA_OUTPUT_MD_DATA_MODEL = "1.0";

const char* const STR_RGA_BUILD_DATE_DEV = "Development build";

#ifdef RGA_BUILD_DATE
const char* const STR_RGA_BUILD_DATE = STR(RGA_BUILD_DATE);
#else
const char* const STR_RGA_BUILD_DATE = STR_RGA_BUILD_DATE_DEV;
#endif