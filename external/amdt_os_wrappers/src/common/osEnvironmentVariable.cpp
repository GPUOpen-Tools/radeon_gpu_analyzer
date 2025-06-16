//=====================================================================
// Copyright 2016-2025 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osEnvironmentVariable.cpp
///
//=====================================================================

#include <amdt_os_wrappers/Include/osEnvironmentVariable.h>
#include <amdt_os_wrappers/Include/osProcess.h>
#include <amdt_base_tools/Include/gtAssert.h>

osEnvVarScope::osEnvVarScope(const std::vector<osEnvironmentVariable>& envVars) : mEnvVars(envVars.begin(), envVars.end())
{
    bool isOk = false;

    for (size_t i = 0; i < envVars.size(); i++)
    {
        isOk = osSetCurrentProcessEnvVariable(envVars[i]);
        GT_ASSERT(isOk);
    }
}

osEnvVarScope::~osEnvVarScope()
{
    bool isOk = false;

    for (size_t i = 0; i < mEnvVars.size(); i++)
    {
        isOk = osRemoveCurrentProcessEnvVariable(mEnvVars[i]._name);
        GT_ASSERT(isOk);
    }

}
