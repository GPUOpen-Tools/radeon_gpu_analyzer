// -*- C++ -*-
//=====================================================================
// Copyright 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/ShaderAnalyzer/RadeonGPUAnalyzerBackend/include/beRGADllBuild.h $
/// \version $Revision: #1 $
/// \brief  Thing to decorate exported/imported backend functions.
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/ShaderAnalyzer/RadeonGPUAnalyzerBackend/include/beRGADllBuild.h#1 $
// Last checkin:   $DateTime: 2016/02/28 16:32:28 $
// Last edited by: $Author: igal $
// Change list:    $Change: 561710 $
//=====================================================================

// TODO do we need another header for this?  I have duplicates.


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
