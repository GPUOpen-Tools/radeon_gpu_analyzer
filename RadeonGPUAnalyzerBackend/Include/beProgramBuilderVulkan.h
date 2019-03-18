//=================================================================
// Copyright 2018 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#pragma once

// Local.
#include <RadeonGPUAnalyzerBackend/Include/beDataTypes.h>
#include <RadeonGPUAnalyzerBackend/Include/beProgramBuilder.h>
#include <RadeonGPUAnalyzerCLI/Src/kcConfig.h>

using namespace beKA;

// SPIR-V tool.
enum class beSpvTool
{
    Assembler,
    Disassembler
};

//
// ProgramBuilder implementation for online Vulkan mode.
//
class beProgramBuilderVulkan : public beProgramBuilder
{
public:
    beProgramBuilderVulkan()  = default;
    ~beProgramBuilderVulkan() = default;

    // Get the list of target GPUs supported by the Vulkan driver.
    static beStatus GetVulkanDriverTargetGPUs(const std::string& loaderDebug, const std::string& icdFile,
                                              std::set<std::string>& targetGPUs,
                                              bool printCmd, std::string& errText);

    // Get the list of physically installed GPUs.
    static beStatus GetPhysicalGPUs(const std::string&                icdFile,
                                    std::vector<beVkPhysAdapterInfo>& gpuInfo,
                                    bool                              printCmd,
                                    std::string&                      errText);

    // Compile single glsl or hlsl source file to a binary SPIR-V file.
    // If compilation fails, the error text returned by the compiler is returned in "errText" string.
    static beStatus CompileSrcToSpirvBinary(const Config& config,
                                            const std::string& srcFile,
                                            const std::string& spvFile,
                                            bePipelineStage stage,
                                            bool isHlsl,
                                            std::string& errText);

    // Link multiple SPIR-V binary files into a single SPIR-V binary file.
    //static beStatus LinkSpvBinaries(const std::vector<std::string>& spvFileNames, const std::string& outSpvFileName);

    // Compile SPIR-V binary file(s) to pipeline binary, ISA disassembly & statistics using the Vulkan Backend executable.
    static beStatus CompileSpirv(const std::string& loaderDebug,
                                 const beVkPipelineFiles& spirvFiles,
                                 const beVkPipelineFiles& isaFiles,
                                 const beVkPipelineFiles& statsFiles,
                                 const std::string& binFile,
                                 const std::string& psoFile,
                                 const std::string& icdFile,
                                 const std::string& validationOutput,
                                 const std::string& validationOutputRedirection,
                                 const std::string& device,
                                 bool printCmd,
                                 std::string& errMsg);

    // Disassemble SPIR-V binary file to disassembly text file.
    static beStatus DisassembleSpv(const std::string& spvToolsBinDir,
                                   const std::string& spvFilePath,
                                   const std::string& spvDisFilePath,
                                   bool               printCmd,
                                   std::string&       errMsg);

    // Assemble SPIR-V text file to SPIR-V binary file.
    static beStatus AssembleSpv(const std::string& spvToolsBinDir,
                                const std::string& spvTxtFilePath,
                                const std::string& spvFilePath,
                                bool               printCmd,
                                std::string&       errMsg);

    // Preprocess input file using the glslang compiler. The preprocessed text is returned in "output" string.
    static beStatus PreprocessSource(const Config& config, const std::string& glslangBinDir, const std::string& inputFile,
                                     bool isHlsl, bool printCmd, std::string& output, std::string& errMsg);

private:

    // Invoke the Glslang compiler executable.
    static beStatus InvokeGlslang(const std::string& glslangBinPath, const std::string& cmdLineOptions,
                                  bool printCmd, std::string& outText, std::string& errText);

    // Invoke one of SPIR-V tools.
    static beStatus InvokeSpvTool(beSpvTool tool, const std::string& spvToolsBinDir, const std::string& cmdLineOptions,
                                  bool printCmd, std::string& outMsg, std::string& errMsg);

    // Invoke the VulkanBackend executable.
    static beStatus InvokeVulkanBackend(const std::string& cmdLineOptions, bool printCmd,
                                        std::string& outText, std::string& errText);
};
