//======================================================================
// Copyright 2020-2023 Advanced Micro Devices, Inc. All rights reserved.
//======================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_VULKAN_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_VULKAN_H_

// Local.
#include "radeon_gpu_analyzer_backend/be_data_types.h"
#include "radeon_gpu_analyzer_backend/be_program_builder.h"
#include "source/radeon_gpu_analyzer_cli/kc_config.h"

using namespace beKA;

// Metadata Parsed from Amdgpu-dis code object Metadata string.
class BeAmdPalMetaData
{
public:

    // Enum to represent different stages in hardware pipeline.
    enum class StageType
    {
        kLS,
        kHS,
        kES,
        kGS,
        kVS,
        kPS,
        kCS
    };

    // Enum to represent different shader types.
    enum class ShaderType
    {
        kVertex,
        kHull,
        kDomain,
        kGeometry,
        kPixel,
        kCompute
    };

    // Enum to represent different shader subtypes.
    enum class ShaderSubtype
    {
        kUnknown,
        kRayGeneration,
        kMiss,
        kAnyHit,
        kClosestHit,
        kIntersection,
        kCallable,
        kTraversal
    };

    // Struct to hold hardware stage details.
    struct HardwareStageMetaData
    {
        StageType          stage_type;
        beKA::AnalysisData stats;
    };

    // Struct to hold shader function details.
    struct ShaderFunctionMetaData
    {
        std::string        name;
        ShaderSubtype      shader_subtype;
        beKA::AnalysisData stats;
    };

    // Struct to hold shader details.
    struct ShaderMetaData
    {
        ShaderType    shader_type;
        StageType     hardware_mapping;
        ShaderSubtype shader_subtype = ShaderSubtype::kUnknown;
    };

    // Struct to hold amdpal pipeline details.
    struct PipelineMetaData
    {
        std::string                         api;
        std::vector<HardwareStageMetaData>  hardware_stages;
        std::vector<ShaderFunctionMetaData> shader_functions;
        std::vector<ShaderMetaData>         shaders;
    };

    // Function to convert string stage name to StageType enum
    static StageType GetStageType(const std::string& stage_name);

    // Function to convert string shader name to ShaderType enum
    static ShaderType GetShaderType(const std::string& shader_name);

    // Function to convert string shader subtype to ShaderSubtype enum
    static ShaderSubtype GetShaderSubtype(const std::string& subtype_name);

    // Function to convert StageType enum to string stage name.
    static std::string GetStageName(StageType stage_type);

    // Function to convert ShaderType enum to string shader name.
    static std::string GetShaderName(ShaderType shader_type);

    // Function to convert ShaderSubtype enum to string shader subtype.
    static std::string GetShaderSubtypeName(ShaderSubtype subtype);
};

// SPIR-V tool.
enum class BeVulkanSpirvTool
{
    kAssembler,
    kDisassembler
};

//
// ProgramBuilder implementation for live-driver Vulkan mode.
//
class beProgramBuilderVulkan : public BeProgramBuilder
{
public:
    beProgramBuilderVulkan()  = default;
    ~beProgramBuilderVulkan() = default;

    // Get the list of target GPUs supported by the Vulkan driver.
    static beStatus GetVulkanDriverTargetGPUs(const std::string& loader_debug, const std::string& icd_file,
        std::set<std::string>& target_gpus, bool should_print_cmd, std::string& error_text);

    // Get the list of physically installed GPUs.
    static beStatus GetPhysicalGPUs(const std::string& icd_file, std::vector<BeVkPhysAdapterInfo>& gpu_info,
        bool should_print_cmd, std::string& error_text);

    // Compile single glsl or hlsl source file to a binary SPIR-V file.
    // If compilation fails, the error text returned by the compiler is returned in "errText" string.
    static beStatus CompileSrcToSpirvBinary(const Config& config, const std::string& src_file, const std::string& spv_file,
        BePipelineStage stage, bool is_hlsl, std::string& error_text);

    // Compile SPIR-V binary file(s) to pipeline binary, ISA disassembly & statistics using the Vulkan Backend executable.
    static beStatus CompileSpirv(const std::string& loader_debug, const BeVkPipelineFiles& spirv_files, const BeVkPipelineFiles& isa_files,
        const BeVkPipelineFiles& statsFiles, const std::string& bin_file, const std::string& pso_file, const std::string& icd_file,
        const std::string& validation_output, const std::string& validation_output_redirection, const std::string& device,
        bool should_print_cmd, std::string& errMsg);

    // Disassemble SPIR-V binary file to disassembly text file.
    static beStatus DisassembleSpv(const std::string& spv_tool_bin_dir, const std::string& spv_file_path,
        const std::string& spv_dis_file_path, bool should_print_cmd, std::string& error_msg);

    // Assemble SPIR-V text file to SPIR-V binary file.
    static beStatus AssembleSpv(const std::string& spv_tools_bin_dir, const std::string& spv_txt_file_path,
        const std::string& spv_file_path, bool should_print_cmd, std::string& error_message);

    // Preprocess input file using the glslang compiler. The preprocessed text is returned in "output" string.
    static beStatus PreprocessSource(const Config& config, const std::string& glslang_bin_dir, const std::string& input_file,
        bool is_hlsl, bool should_print_cmd, std::string& output, std::string& error_msg);

   // Invoke the amdgpu-dis executable.
    static beStatus InvokeAmdgpudis(const std::string& cmd_line_options, bool should_print_cmd,
        std::string& out_text, std::string& error_txt);

    // Parses amdgpu-dis output and extracts a table with the amdgpu shader stage name being the key ("vs", "ps", etc.) 
    // and that shader stage's disassembly the value.
    static beStatus ParseAmdgpudisOutput(const std::string& amdgpu_dis_output, 
        std::map<std::string, std::string>& shader_to_disassembly, std::string& error_msg);

    // Invoke amdgpu-dis on the binary file and parse the output into disassembly.
    static beStatus AmdgpudisBinaryToDisassembly(const std::string&                  bin_file,
                                                 const BeVkPipelineFiles&            isa_files,
                                                 bool                                should_print_cmd,
                                                 std::string&                        amdgpu_dis_stdout,
                                                 std::map<std::string, std::string>& shader_to_disassembly,
                                                 std::string&                        error_msg);

    // Helper function to Extract from AmdgpuDis Metadata Hardware Maapping for a given api shader stage.
    static bool GetAmdgpuDisApiShaderToHwMapping(const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline, 
                                                 const std::string&                        api_shader_stage_name, 
                                                 std::string&                              hw_mapping_str);

    // Helper function to Write Isa Files with Hw mapping extracted from AmdgpuDis Metadata per api shader stage.
    static bool WriteIsaFileWithHwMapping(uint32_t                                  stage,
                                          const BeAmdPalMetaData::PipelineMetaData& amdpal_pipeline,
                                          const std::map<std::string, std::string>& shader_to_disassembly,
                                          const std::string&                        isa_file);

    // Parses amdgpu-dis output and extracts code object metadata.
    static beKA::beStatus ParseAmdgpudisMetadata(const std::string&                  amdgpu_dis_output, 
                                                 BeAmdPalMetaData::PipelineMetaData& pipeline);

private:
    // Invoke the glslang compiler executable.
    static beStatus InvokeGlslang(const std::string& glsl_bin_path, const std::string& cmd_line_options,
        bool should_print_cmd, std::string& out_text, std::string& error_text);

    // Invoke one of SPIR-V tools.
    static beStatus InvokeSpvTool(BeVulkanSpirvTool tool, const std::string& spv_tools_bin_dir, const std::string& cmd_line_options,
        bool should_print_cmd, std::string& out_msg, std::string& err_msg);

    // Invoke the VulkanBackend executable.
    static beStatus InvokeVulkanBackend(const std::string& cmd_line_options, bool should_print_cmd,
        std::string& out_text, std::string& error_txt);
 
};

#endif // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_VULKAN_H_
