//=============================================================================
/// Copyright (c) 2017-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for dx11 string constants.
//=============================================================================

#include <string>

static const std::string  USAGE_MESSAGE = ""
"Usage:\n"
"RGADX11.exe --list-adapters               Dump the list of display adapters installed on the system.\n"
"RGADX11.exe --get-adapter-info <ID>       Dump the name and driver folder for the adapter specified by ID.\n";

static const std::wstring  STR_AMD_DISPLAY_ADAPTER_TOKEN_1  = L"AMD";
static const std::wstring  STR_AMD_DISPLAY_ADAPTER_TOKEN_2  = L"Radeon";
static const std::wstring  STR_AMD_DXX_MODULE_NAME          = L"atidxx64.dll";

static const std::wstring  STR_ERR_GET_DISPLAY_ADAPTERS_FAILED      = L"Error: failed to detect the list of supported display adapters.";
static const std::wstring  STR_ERR_GET_DISPLAY_ADAPTER_INFO_FAILED  = L"Error: failed to retrieve the display adapter info for ID: ";
static const std::wstring  STR_ERR_BAD_DISPLAY_ADAPTER_ID           = L"Error: invalid display adapter ID: ";
