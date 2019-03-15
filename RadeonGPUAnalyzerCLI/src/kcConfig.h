//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <RadeonGPUAnalyzerBackend/Include/beBackend.h>
#include <RadeonGPUAnalyzerCLI/Src/kcDataTypes.h>

/// A place to collect command line options.
/// This is a POD with a dump method.
class Config
{
public:

    static std::string sourceKindHLSL;
    static std::string sourceKindAMDIL;
    static std::string sourceKindOpenCL;
    static std::string sourceKindOpenGL;
    static std::string sourceKindGLSLVulkanOffline;
    static std::string sourceKindSpirvBinOffline;
    static std::string sourceKindSpirvTxtOffline;
    static std::string sourceKindRocmOpenCL;
    static std::string sourceKindVulkan;

    enum ConfigCommand
    {
        ccNone,
        ccInvalid,
        ccCompile,
        ccListEntries,
        ccHelp,
        ccListAsics,
        ccListAdapters,
        ccVersion,
        ccGenVersionInfoFile,
        ccUpdate
    };

    Config();

    beKA::RgaMode            m_mode;             ///< RGA mode.
    ConfigCommand            m_RequestedCommand; ///< What the user requested to do
    std::vector<std::string> m_InputFiles;       ///< Source file for processing.
    std::string              m_AnalysisFile;     ///< Output analysis file.
    std::string              m_ILFile;           ///< Output IL Text file template.
    std::string              m_ISAFile;          ///< Output ISA Text file template.
    std::string              m_LiveRegisterAnalysisFile;///< Live register analysis output file.
    std::string              m_blockCFGFile;     ///< Output file for per-block control flow graph.
    std::string              m_instCFGFile;      ///< Output file for per-instruction control flow graph.
    std::string              m_BinaryOutputFile; ///< Output binary file template.
    std::string              m_Function;         ///< Kernel/Function of interest in analysis.
    std::string              m_CSVSeparator;     ///< Override for CSV list separator.
    std::string              m_MetadataFile;     ///< Output .metadata Text file template.
    std::vector<std::string> m_ASICs;            ///< Target GPUs for compilation.
    std::vector<std::string> m_SuppressSection;  ///< List of sections to suppress in generated binary files.
    std::vector<std::string> m_OpenCLOptions;    ///< Options to be added to OpenCL compile.
    std::vector<std::string> m_Defines;          ///< Macros to be added to compile.
    std::vector<std::string> m_IncludePath;      ///< Additional Include paths
    bool                     m_isRetainUserBinaryPath; ///< If true then CLI will not add the asic name to the generated binary output file
    std::string              m_versionInfoFile;  ///< RGA CLI config file name.
    std::string              m_sessionMetadataFile;  ///< RGA CLI session metadata file name.
    std::string              m_logFile;          ///< RGA CLI log file name (full path).
    int                      m_optLevel;         ///< Optimization level.

    // DX/GL
    std::string              m_SourceKind;             ///< Kind of source HLSL or GLSL (maybe more later like ASM kinds).
    std::string              m_Profile;                ///< Profile used with GSA compilations. Target in DX
    int                      m_DXAdapter;              ///< ID of GPU adapter to use for DX.
    unsigned int             m_DXFlags;                ///< Flags to pass to D3DCompile.
    std::string              m_DXLocation;             ///< D3DCompiler dll location
    std::string              m_FXC;                    ///< FXC path and arguments
    std::string              m_DumpMSIntermediate;     /// the location where to save the ms blob as text
    bool                     m_EnableShaderIntrinsics; /// true to enable DX shader intrinsics.
    bool                     m_AMDILInput;             /// true when the input language is AMDIL rather than HLSL.
    int                      m_UAVSlot;                /// User-defined UAV slot for shader intrinsics.

    // Vulkan.
    std::string              m_programOutputDir;     ///< Output directory for the compiler.
    std::string              m_VertexShader;         ///< Vertex shader full path
    std::string              m_TessControlShader;    ///< Tessellation control shader full path
    std::string              m_TessEvaluationShader; ///< Tessellation evaluation shader full path
    std::string              m_GeometryShader;       ///< Geometry shader full path
    std::string              m_FragmentShader;       ///< Fragment shader full path
    std::string              m_ComputeShader;        ///< Compute shader full path
    std::string              m_pso;                  ///< Vulkan Pipeline State Object file.
    std::string              m_spvBin;               ///< Path to SPIR-V binary file.
    std::string              m_icdFile;              ///< Full path to an alternative Vulkan ICD to load instead of loader.
    std::string              m_loaderDebug;          ///< Value for the VK_LOADER_DEBUG environment variable.
    std::string              m_glslangOpt;           ///< Additional options for glslang (the Vulkan front-end compiler).
    std::string              m_spvTxt;               ///< Path to SPIR-V text file.
    std::string              m_vulkanValidation;     ///< Path to output Vulkan validation info file.
    std::string              m_parsedSpv;            ///< Path to parsed SPIR-V text output file.
    rgVulkanInputType        m_vertShaderFileType;   ///< Type of vertex shader file.
    rgVulkanInputType        m_tescShaderFileType;   ///< Type of tesselation control shader file.
    rgVulkanInputType        m_teseShaderFileType;   ///< Type of tesselation evaluation shader file.
    rgVulkanInputType        m_fragShaderFileType;   ///< Type of fragment shader file.
    rgVulkanInputType        m_geomShaderFileType;   ///< Type of geometry shader file.
    rgVulkanInputType        m_compShaderFileType;   ///< Type of compute shader file.
    bool                     m_isHlslInput;          ///< Input files are HLSL shaders.
    bool                     m_isGlslInput;          ///< Input files are GLSL shaders.
    bool                     m_isSpvInput;           ///< Input files are SPIR-V binary files.
    bool                     m_isSpvTxtInput;        ///< Input files are SPIR-V text files.

    // Compiler paths
    std::string              m_cmplrBinPath;         ///< Path to user-provided compiler "bin" folder.
    std::string              m_cmplrIncPath;         ///< Path to user-provided compiler "include" folder.
    std::string              m_cmplrLibPath;         ///< Path to user-provided compiler "lib" folder.

    bool                     m_isAmdPalIlBinariesRequired;       ///< True to generate AMD PAL IL binaries
    bool                     m_isAmdPalIlDisassemblyRequired;    ///< True to generate AMD PAL IL disassembly
    bool                     m_isAmdIsaBinariesRequired;         ///< True to generate AMD ISA binaries
    bool                     m_isAmdIsaDisassemblyRequired;      ///< True to generate AMD ISA binaries
    bool                     m_isParsedISARequired;              ///< True to generate "parsed" ISA in CSV format.
    bool                     m_isLineNumbersRequired;            ///< True to generate source lines in the ISA disassembly.
    bool                     m_isWarningsRequired;               ///< True to print warnings reported by the compiler.

    bool                     m_printProcessCmdLines;             ///< True to print command lines that RGA uses to launch
                                                                 ///  external processed.
private:
};
