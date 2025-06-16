//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for AMDDXXModuleWrapper class.
//=============================================================================

#pragma once

// Infra.
#include "DXXModule.h"

class AMDDXXModuleWrapper : private AMDDXXModule
{
private:
    std::string module_name_;

public:

    AMDDXXModuleWrapper() : AMDDXXModule(AMDDXXModule::s_DefaultModuleName)
    {
        if (IsLoaded())
        {
            module_name_ = AMDDXXModule::s_DefaultModuleName;
        }
    };

    bool LoadModule(const std::string& name)
    {
        bool is_ok = AMDDXXModule::LoadModule(name);
        if (is_ok)
        {
            module_name_ = name;
        }
        else
        {
            module_name_ = "";
        }
        return is_ok;        
    };

    const std::string& GetModuleName() const
    {
        return module_name_;
    };

    using AMDDXXModule::AmdDxGsaCompileShader;
    
    using AMDDXXModule::AmdDxGsaFreeCompiledShader;
	
    using AMDDXXModule::UnloadModule;
    
    using AMDDXXModule::IsLoaded;

};
