//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for the isa decoder manager.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_ISA_DECODE_MANAGER_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_ISA_DECODE_MANAGER_H_

// C++.
#include <memory>

// Isa Decoder.
#include "amdisa/isa_decoder.h"

class RgIsaDecodeManager
{
public:
    // Returns the singleton instance.
    static RgIsaDecodeManager& GetInstance()
    {
        static RgIsaDecodeManager instance;
        return instance;
    }

    // Get the handle to the isa decode manager.
    amdisa::DecodeManager* Get();

private:
    // Private constructor to prevent instantiation from outside.
    RgIsaDecodeManager()  = default;
    ~RgIsaDecodeManager() = default;

    // Delete copy and move constructors to prevent copying.
    RgIsaDecodeManager(const RgIsaDecodeManager&)            = delete;
    RgIsaDecodeManager& operator=(const RgIsaDecodeManager&) = delete;

    // The manager of all the architectures.
    std::unique_ptr<amdisa::DecodeManager> decode_manager_;
};

#endif  // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_ISA_DECODE_MANAGER_H_
