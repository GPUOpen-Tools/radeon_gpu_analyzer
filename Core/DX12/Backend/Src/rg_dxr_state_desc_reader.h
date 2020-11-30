#pragma once

// C++.
#include <string>
#include <vector>
#include <memory>

// Local.
#include "rg_dx12_data_types.h"

namespace rga
{
    class RgDxrStateDescReader
    {
    public:
        RgDxrStateDescReader() = delete;
        ~RgDxrStateDescReader() = delete;
        static bool ReadDxrStateDesc(const std::string& json_file_path,
            RgDxrStateDesc& state_desc, std::string& error_msg);
    };
};