//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_OPENCL_DEFINITIONS_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_OPENCL_DEFINITIONS_H_

// Options specific to OpenCL
struct OpenCLOptions : public beKA::CompileOptions
{
    // OpenCL compilation options passed into clBuildProgram.
    std::vector<std::string> opencl_compile_options;

    // OpenCL compilation options passed into clBuildProgram as -Ditems.
    std::vector<std::string> defines;

    // OpenCL include paths passed as "-I..."
    std::vector<std::string> include_paths;

    // Set of devices for compilation.
    std::set<std::string> selected_devices;

    // Vector of devices for compilation - sorted.
    std::vector<std::string> selected_devices_sorted;
};

// Compiler package paths: bin, include and lib.
struct CmpilerPaths
{
    std::string bin;
    std::string inc;
    std::string lib;
};

#endif // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_OPENCL_DEFINITIONS_H_
