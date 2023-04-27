//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_LIGHTNING_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_LIGHTNING_H_

// C++.
#include <string>
#include <sstream>
#include <unordered_map>

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include "external/amdt_base_tools/Include/gtString.h"
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Local.
#include "radeon_gpu_analyzer_backend/be_program_builder.h"
#include "radeon_gpu_analyzer_backend/be_opencl_definitions.h"

using namespace beKA;

// ObjDump operation types.
enum class ObjDumpOp
{
    kDisassemble,
    kDisassembleWithLineNumbers,
    kGetMetadata,
    kGetKernelCodeSize
};

// Kernel statistics.
struct KernelCodeProperties
{
    size_t wavefront_num_sgprs = 0;
    size_t work_item_num_vgprs = 0;
    size_t wavefront_size = 0;
    size_t workgroup_segment_size = 0;
    size_t private_segment_size = 0;
    size_t sgpr_spills = 0;
    size_t vgpr_spills = 0;
    size_t isa_size = 0;
};

// Maps  kernel_name --> KernelCodeProperties.
typedef  std::map<std::string, KernelCodeProperties> CodePropsMap;

class BeProgramBuilderLightning : public BeProgramBuilder
{
public:
    BeProgramBuilderLightning()               = default;
    ~BeProgramBuilderLightning(void) override = default;

    virtual beStatus GetKernelIlText(const std::string& device, const std::string& kernel, std::string& il) override;
    virtual beStatus GetKernelIsaText(const std::string& device, const std::string& kernel, std::string& isa) override;
    virtual beStatus GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis) override;
    virtual beStatus GetDeviceTable(std::vector<GDT_GfxCardInfo>& table) override;

    // Retrieve the LightningCompiler version.
    // The version reported by the compiler is returned in "outText" string.
    static beStatus GetCompilerVersion(RgaMode mode, const std::string& user_bin_dir, bool should_print_cmd, std::string& outText);

    // Compile a set of OpenCL source files to a single executable.
    // The "error_msg" is the content of stderr dumped by the compiler in case of compilation failure.
    static beStatus CompileOpenCLToBinary(const CmpilerPaths& compiler_paths, const OpenCLOptions& user_options, const std::vector<std::string>& src_filename,
        const std::string& bin_filename,const std::string& device, bool  should_print_cmd, std::string& error_msg);

    // Compile a set of OpenCL source files to LLVM IR representation.
    // The "error_msg" is the content of stderr dumped by the compiler in case of compilation failure.
    static beStatus CompileOpenCLToLlvmIr(const CmpilerPaths&             compiler_paths,
                                          const OpenCLOptions&            user_options,
                                          const std::vector<std::string>& src_filename,
                                          const std::string&              il_output_filename,
                                          const std::string&              device,
                                          bool                            should_print_cmd,
                                          std::string&                    error_msg);

    // Preprocess input file using the OpenCL Lightning Compiler. The preprocessed program text
    // is returned in "output" string.
    static beStatus PreprocessOpencl(const std::string& user_bin_dir, const std::string& input_file,
        const std::string& args, bool should_print_cmd, std::string& output);

    // Disassemble binary to ISA text.
    static beStatus  DisassembleBinary(const std::string& user_bin_dir, const std::string& bin_filename, const std::string& device,
        bool line_numbers, bool should_print_cmd, std::string& isa_disassembly, std::string& error_msg);

    // Extract CodeObject metadata.
    static beStatus  ExtractMetadata(const std::string& user_bin_dir, const std::string& bin_filename,
        bool should_print_cmd, std::string& metadata_text);

    // Extract CodeObject Metadata kernels properties.
    static beStatus  ExtractKernelCodeProps(const std::string& user_bin_dir, const std::string& bin_filename,
        bool should_print_cmd, CodePropsMap& code_props);

    // Extract names of kernels from the binary.
    static beStatus  ExtractKernelNames(const std::string& user_bin_dir, const std::string& bin_filename,
        bool should_print_cmd, std::vector<std::string>& kernel_names);

    // Check if the output file exists and not empty.
    static bool  VerifyOutputFile(const std::string& filename);

    // Get the ISA size.
    static int  GetIsaSize(const std::string& isa_disassembly);

    // Extract the size of binary section for the provided kernel.
    static int  GetKernelCodeSize(const std::string& user_bin_dir, const std::string& bin_file,
                                  const std::string& kernel_name, bool should_print_cmd);

protected:
    // Adds standard options required to compile source language with LC compiler to the "options" string.
    static beStatus AddCompilerStandardOptions(RgaMode mode, const CmpilerPaths& compiler_paths, const std::string& device, std::string& options);

    // Builds a string containing all necessary options for OpenCL compiler.
    // Writes constructed options string to "options".
    static beStatus ConstructOpenCLCompilerOptions(const CmpilerPaths& compiler_paths,
        const OpenCLOptions& user_options,
        const std::vector<std::string>& src_file_names,
        const std::string& bin_filename,
        const std::string& device,
        std::string& options);

    // Compile for the provided language with the given command line options.
    // The stdout and stderr generated by compiler are returned in "stdOut" and "stdErr".
    static beStatus InvokeCompiler(RgaMode mode, const std::string& user_bin_dir, const std::string& cmd_line_options,
        bool should_print_cmd, std::string& std_out, std::string& std_err, unsigned long timeout = 0);

    // Check if the compiler generated output file and reported no errors.
    static beStatus VerifyCompilerOutput(const std::string& outpuf_filename, const std::string& error_text);

    // Builds a string containing the standard options required by the "op" operation of ObjDump.
    static beStatus ConstructObjDumpOptions(ObjDumpOp          op,
        const std::string& compiler_bin_dir,
        const std::string& bin_filename,
        const std::string& device,
        bool               should_print_cmd,
        std::string& options);

    // Launch the LC ObjDump of ReadObj (for CodeObj metadata).
    static beStatus InvokeObjDump(ObjDumpOp op, const std::string& user_bin_dir, const std::string& cmd_line_options,
        bool should_print_cmd, std::string& out_text, std::string& error_msg);

    // Check if the ObjDump output is an error message or ISA text.
    static beStatus VerifyObjDumpOutput(const std::string& objdump_output);

    // Filter the output text generated by ObjDump. Keep only CodeObj metadata or ISA (specified by "op") and remove everything else.
    static beStatus FilterCodeObjOutput(ObjDumpOp op, std::string& text);

    // Checks if the readobj supports "-amdgpu-code-object-metadata" option.
    static bool DoesReadobjSupportMetadata(const std::string& user_bin_dir, bool should_print_cmd);
};

#endif // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_LIGHTNING_H_
