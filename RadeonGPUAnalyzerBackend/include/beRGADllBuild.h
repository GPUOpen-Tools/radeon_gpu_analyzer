//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================


#ifdef _WIN32
#pragma warning(disable:4005)
#if defined(RGA_BACKEND_EXPORTS)
    #define RGA_BACKEND_DECLDIR __declspec(dllexport)
#elif defined(RGA_BACKEND_STATIC)
    #define RGA_BACKEND_DECLDIR
#else
    #define RGA_BACKEND_DECLDIR __declspec(dllimport)
#endif
#else
// TODO We could use g++ __attribute syntax to control symbol visibility.
#  define RGA_BACKEND_DECLDIR
#endif
