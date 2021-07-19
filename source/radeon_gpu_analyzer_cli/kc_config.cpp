//======================================================================
// Copyright 2020-2021 Advanced Micro Devices, Inc. All rights reserved.
//======================================================================

#if GSA_BUILD
    #include <sstream>
#endif

#include "radeon_gpu_analyzer_cli/kc_config.h"
#include "radeon_gpu_analyzer_cli/kc_cli_string_constants.h"

using namespace std;

std::string Config::source_kind_dx11                    = "DX11";
std::string Config::source_kind_dx12                    = "DX12";
std::string Config::source_kind_dxr                     = "DXR";
std::string Config::source_kind_amdil                   = "AMDIL";
std::string Config::source_kind_opencl                  = "OPENCL";
std::string Config::source_kind_opengl                  = "OPENGL";
std::string Config::source_kind_glsl_vulkan_offline     = "VK-OFFLINE";
std::string Config::source_kind_spirv_bin_offline       = "VK-SPV-OFFLINE";
std::string Config::source_kind_spirv_txt_offline       = "VK-SPV-TXT-OFFLINE";
std::string Config::source_kind_vulkan                  = "VULKAN";

Config::Config() :
    mode(kModeNone),
    requested_command(kNone),
    analysis_file(),
    il_file(),
    isa_file(),
    livereg_analysis_file(),
    binary_output_file(),
    function(),
    csv_separator(),
    metadata_file(),
    asics(),
    suppress_section(),
    opencl_options(),
    defines(),
    include_path(),
    should_avoid_binary_device_prefix(false),
    is_parsed_isa_required(false),
    is_line_numbers_required(false),
    is_warnings_required(false),
    is_hlsl_input(false),
    is_glsl_input(false),
    is_spv_input(false),
    is_spv_txt_input(false),
    vert_shader_file_type(RgVulkanInputType::kUnknown),
    tesc_shader_file_type(RgVulkanInputType::kUnknown),
    tese_shader_file_type(RgVulkanInputType::kUnknown),
    frag_shader_file_type(RgVulkanInputType::kUnknown),
    geom_shader_file_type(RgVulkanInputType::kUnknown),
    comp_shader_file_type(RgVulkanInputType::kUnknown),
    source_kind(),
    profile(),
    dx_flags(0),
    dx_compiler_location(),
    dx_adapter(-1),
    fxc(),
    dump_ms_intermediate(),
    enable_shader_intrinsics(false),
    dxbc_input_dx11(false),
    uav_slot(-1),
    opt_level(-1),
    print_process_cmd_line(false)
{
}
