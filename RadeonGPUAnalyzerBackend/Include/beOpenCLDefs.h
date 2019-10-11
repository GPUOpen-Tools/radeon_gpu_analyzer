#ifndef _BEOPENCLDEFS_H_
#define _BEOPENCLDEFS_H_

/// Options specific to OpenCL
struct OpenCLOptions : public beKA::CompileOptions
{
    /// OpenCL compilation options passed into clBuildProgram.
    std::vector<std::string> m_openCLCompileOptions;

    /// OpenCL compilation options passed into clBuildProgram as -Ditems.
    std::vector<std::string> m_defines;

    /// OpenCL include paths passed as "-I..."
    std::vector<std::string> m_incPaths;

    /// Set of devices for compilation.
    std::set<std::string> m_selectedDevices;

    /// Vector of devices for compilation - sorted.
    std::vector<std::string> m_selectedDevicesSorted;
};

/// Compiler package paths: bin, include and lib.
struct CmplrPaths
{
    std::string     m_bin;
    std::string     m_inc;
    std::string     m_lib;
};

#endif // _BEOPENCLDEFS_H_
