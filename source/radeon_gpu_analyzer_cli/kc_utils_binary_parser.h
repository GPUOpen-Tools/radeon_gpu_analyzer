//=============================================================================
/// Copyright (c) 2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for parsing binary code object disassembly.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_BINARY_PARSER_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_BINARY_PARSER_H_

// C++.
#include <string>
#include <utility>
#include <vector>

// Backend.
#include "radeon_gpu_analyzer_backend/be_opencl_definitions.h"

// Local.
#include "radeon_gpu_analyzer_cli/kc_config.h"

// Substitute source path pair: {from_prefix, to_prefix} for llvm-objdump --substitute-path.
using SubstituteSourcePath = std::pair<std::string, std::string>;

// Code object binary disassembly strategy.
class KcCliBinaryDisassemblyStrategy
{
public:
    // Defaulted Virtual Destructor.
    virtual ~KcCliBinaryDisassemblyStrategy() = default;

    // Invoke the disassembler and to generate raw text disassembly from code object binary.
    virtual beKA::beStatus Disassemble(const std::string&              bin_file,
                                       const std::vector<std::string>& source_code_dirs,
                                       const std::vector<SubstituteSourcePath>& substitute_paths,
                                       bool                            verbose,
                                       std::string&                    text_disassembly) = 0;

    // Parses amdgpu-dis output and extracts a table with
    // the amdgpu kernel name being the key and that shader stage's disassembly the value.
    virtual beKA::beStatus ParseKernels(const std::string&                  text_disassembly,
                                        std::map<std::string, std::string>& kernel_to_disassembly,
                                        std::string&                        error_msg) const = 0;
};

// Amdgpu-Dis disassembly parser for compute workflows.
class KcCliAmdgpuDisComputeStrategy : public KcCliBinaryDisassemblyStrategy
{
public:
    // Invoke amdgpu-dis executable and to generate raw text disassembly from code object binary.
    beKA::beStatus Disassemble(const std::string&              bin_file,
                               const std::vector<std::string>& source_code_dirs,
                               const std::vector<SubstituteSourcePath>& substitute_paths,
                               bool                            verbose,
                               std::string&                    text_disassembly) override;

    // Parses amdgpu-dis output and extracts a table with
    // the amdgpu shader name being the key and that shader stage's disassembly the value.
    beKA::beStatus ParseKernels(const std::string&                  text_disassembly,
                                std::map<std::string, std::string>& kernel_to_disassembly,
                                std::string&                        error_msg) const override;
};

// Amdgpu-Dis disassembly parser for graphics workflows.
class KcCliAmdgpuDisGraphicsStrategy : public KcCliAmdgpuDisComputeStrategy
{
public:
    // Parses amdgpu-dis output and extracts a table with
    // the amdgpu shader name being the key and that shader stage's disassembly the value.
    beKA::beStatus ParseKernels(const std::string&                  text_disassembly,
                                std::map<std::string, std::string>& kernel_to_disassembly,
                                std::string&                        error_msg) const override;
};

// LLVM disassembly parser for compute workflows.
class KcCliLlvmObjdumpComputeStrategy : public KcCliBinaryDisassemblyStrategy
{
public:
    KcCliLlvmObjdumpComputeStrategy(const std::string&                      binary_codeobj_file,
                                    const std::string&                      clang_device,
                                    const CmpilerPaths&                     compiler_paths,
                                    bool                                    is_line_correlation_enabled,
                                    bool                                    verbose,
                                    const BeAmdHsaMetaData::AmdHsaMetaData& md)
        : binary_codeobj_file_(binary_codeobj_file)
        , clang_device_(clang_device)
        , compiler_paths_(compiler_paths)
        , is_line_correlation_enabled_(is_line_correlation_enabled)
        , verbose_(verbose)
        , amdhsa_kernels_md_(md)
    {
    }

    // Invoke llvm-objdump executable and to generate raw text disassembly from code object binary.
    beKA::beStatus Disassemble(const std::string&              bin_file,
                               const std::vector<std::string>& source_code_dirs,
                               const std::vector<SubstituteSourcePath>& substitute_paths,
                               bool                            verbose,
                               std::string&                    text_disassembly) override;

    // Parses  llvm-objdump disassembly output and extracts a table with
    // the amdgpu shader name being the key and that shader stage's disassembly the value.
    beKA::beStatus ParseKernels(const std::string&                  text_disassembly,
                                std::map<std::string, std::string>& kernel_to_disassembly,
                                std::string&                        error_msg) const override;

protected:
    // Binary code object file name.
    std::string binary_codeobj_file_;

    // Clang device used for compilation/disassembly.
    std::string clang_device_;

    // Alternative compiler paths specified by a user.
    CmpilerPaths compiler_paths_;

    // Boolean flag speifying if line numbers are required.
    bool is_line_correlation_enabled_ = false;

    // boolean flag specifying if should print cmd.
    bool verbose_ = false;

    // Amdhsa metadata;
    BeAmdHsaMetaData::AmdHsaMetaData amdhsa_kernels_md_;
};

// LLVM disassembly parser for compute workflows.
class KcCliLlvmObjdumpGraphicsStrategy : public KcCliLlvmObjdumpComputeStrategy
{
public:
    KcCliLlvmObjdumpGraphicsStrategy(const std::string&                        binary_codeobj_file,
                                     const std::string&                        clang_device,
                                     const CmpilerPaths&                       compiler_paths,
                                     bool                                      is_line_correlation_enabled,
                                     bool                                      verbose,
                                     const BeAmdPalMetaData::PipelineMetaData& md)
        : KcCliLlvmObjdumpComputeStrategy(binary_codeobj_file, clang_device, compiler_paths, is_line_correlation_enabled, verbose, {})
        , amdpal_pipeline_md_(md)
    {
    }

    // Parses  llvm-objdump disassembly output and extracts a table with
    // the amdgpu shader name being the key and that shader stage's disassembly the value.
    beKA::beStatus ParseKernels(const std::string&                  text_disassembly,
                                std::map<std::string, std::string>& kernel_to_disassembly,
                                std::string&                        error_msg) const override;

private:
    // Amdhsa metadata;
    BeAmdPalMetaData::PipelineMetaData amdpal_pipeline_md_;
};

#endif  // RGA_RADEONGPUANALYZERCLI_SRC_KC_UTILS_BINARY_PARSER_H_
