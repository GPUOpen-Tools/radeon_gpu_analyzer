//=====================================================================
// Copyright 2016-2025 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osSystemError.h
///
//=====================================================================

//------------------------------ osSystemError.h ------------------------------

#ifndef __OSSYSTEMERROR_H
#define __OSSYSTEMERROR_H

// Infra:
#include <amdt_base_tools/Include/gtString.h>

// Local:
#include <amdt_os_wrappers/Include/osOSDefinitions.h>
#include <amdt_os_wrappers/Include/osOSWrappersDLLBuild.h>


OS_API osSystemErrorCode osGetLastSystemError();
OS_API void osGetLastSystemErrorAsString(gtString& systemErrorAsString);
OS_API void osGetSystemErrorAsString(osSystemErrorCode systemError, gtString& systemErrorAsString);


#endif //__OSSYSTEMERROR_H

