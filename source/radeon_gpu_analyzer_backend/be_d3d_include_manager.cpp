//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for rga backend d3d include manager.
//=============================================================================

#ifdef _WIN32

// C++.
#include <cassert>
#include <fstream>

// Local.
#include "source/radeon_gpu_analyzer_backend/be_d3d_include_manager.h"

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

static bool OpenIncludeFile(const std::string& include_file_full_path, char*& file_buffer, unsigned& file_size_in_bytes)
{
    bool ret = false;

    // Open the file.
    std::ifstream includeFile(include_file_full_path.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

    if (includeFile.is_open())
    {
        // Get the file size.
        file_size_in_bytes = (unsigned int)includeFile.tellg();

        if (file_size_in_bytes > 0)
        {
            // Allocate the buffer.
            file_buffer = new char[file_size_in_bytes];

            // Read the file's contents into the buffer.
            includeFile.seekg(0, std::ios::beg);
            includeFile.read(file_buffer, file_size_in_bytes);

            // Close the file.
            includeFile.close();

            // We are done.
            ret = true;
        }
    }

    return ret;
}

// Returns true if fullString starts with substring.
static bool IsBeginsWith(std::string const& full_string, std::string const& substring)
{
    bool ret = (full_string.size() >= substring.size());
    ret = ret && (full_string.compare(0, substring.length(), substring) == 0);
    return ret;
}

// Returns true if fullString ends with substring.
static bool IsEndsWith(std::string const& full_string, std::string const& substring)
{
    bool ret = false;

    if (full_string.length() >= substring.length())
    {
        ret = (full_string.compare(full_string.length() - substring.length(), substring.length(), substring) == 0);
    }

    return ret;
}

// Adds a directory separator character to the path, if required.
static void AdjustIncludePath(std::string& include_path)
{
    const std::string kDirSeparator = "\\";
    if (!IsEndsWith(include_path, kDirSeparator))
    {
        include_path += kDirSeparator;
    }
}

// ***************************************
// *** INTERNALLY LINKED SYMBOLS - END ***
// ***************************************

D3dIncludeManager::D3dIncludeManager(const std::string& shader_dir,
    const std::vector<std::string>& include_search_dirs) : shader_dir_(shader_dir), include_search_dirs_(include_search_dirs)
{
}

D3dIncludeManager::~D3dIncludeManager()
{
}

STDMETHODIMP D3dIncludeManager::Open(THIS_ D3D_INCLUDE_TYPE include_type, LPCSTR filename, LPCVOID parent_data, LPCVOID* ppData, UINT* pBytes)
{
    (void)parent_data;
    bool is_done = false;

    if (filename != nullptr)
    {
        std::string include_file_full_path;
        switch (include_type)
        {
            case D3D_INCLUDE_LOCAL:
            {
                // First, try the shader's directory.
                include_file_full_path = shader_dir_;

                // Is it a relative path to the shader's directory.
                bool is_relative = IsBeginsWith(filename, "\\");

                if (!is_relative)
                {
                    AdjustIncludePath(include_file_full_path);
                }

                include_file_full_path += filename;
                is_done = OpenIncludeFile(include_file_full_path, (char*&) * ppData, *pBytes);
                if (!is_done)
                {
                    // Search in the user-defined directories.
                    for (const std::string& includeDir : include_search_dirs_)
                    {
                        include_file_full_path = includeDir;
                        if (!is_relative)
                        {
                            AdjustIncludePath(include_file_full_path);
                        }

                        include_file_full_path += filename;
                        is_done = OpenIncludeFile(include_file_full_path, (char*&) * ppData, *pBytes);

                        if (is_done)
                        {
                            break;
                        }
                    }
                }

                break;
            }

            case D3D_INCLUDE_SYSTEM:
            {
                // First, try the shader's directory.
                include_file_full_path = shader_dir_;
                AdjustIncludePath(include_file_full_path);
                include_file_full_path += filename;
                is_done = OpenIncludeFile(include_file_full_path, (char*&) * ppData, *pBytes);

                if (!is_done)
                {
                    // Go through the directories which the user specified.
                    for (const std::string& include_dir : include_search_dirs_)
                    {
                        include_file_full_path = include_dir;
                        AdjustIncludePath(include_file_full_path);
                        include_file_full_path += filename;
                        is_done = OpenIncludeFile(include_file_full_path, (char*&) * ppData, *pBytes);

                        if (is_done)
                        {
                            break;
                        }
                    }
                }
                break;
            }

            default:
                // Unknown D3D include type.
                assert(false);
                break;
        }
    }

    // Must return S_OK according to the documentation.
    return (is_done ? S_OK : E_FAIL);
}

STDMETHODIMP D3dIncludeManager::Close(THIS_ LPCVOID data)
{
    char* buf = (char*)data;
    delete[] buf;
    return S_OK;
}
#endif
