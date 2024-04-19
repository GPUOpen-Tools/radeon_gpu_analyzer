// C++.
#include <cassert>
#include <string>
#include <string.h>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>
#include <algorithm>
#include <limits>

// Infra.
#include <cxxopts/include/cxxopts.hpp>

// OS.
#ifdef WIN32
#include <Windows.h>
#include <libloaderapi.h>
#elif __linux
#include <unistd.h>
#include <linux/limits.h>
#include <libgen.h>
#endif // __linux

// Local.
#include <beVulkanBackend.h>
#include "source/common/vulkan/rg_pipeline_types.h"
#include "source/common/vulkan/rg_pso_factory_vulkan.h"
#include "source/common/vulkan/rg_pso_serializer_vulkan.h"
#include "source/common/rga_cli_defs.h"

// Vulkan driver major version.
static const char* kStrVulkanDriverVersionMajor = "2.0";

// Bit mask for Vulkan minor driver version (22 bits).
static const uint32_t  kStrVulkanDriverVersionMinorBits = 0x3FFFFF;

// Offsets/widths of supported Vulkan version bit fields.
static const uint32_t  kStrVulkanVersionPatchWidth = 12;
static const uint32_t  kStrVulkanVersionMinorWidth = 10;
static const uint32_t  kStrVulkanVersionMajorWidth = 10;

static const uint32_t  kStrVulkanVersionPatchOffset = 0;
static const uint32_t  kStrVulkanVersionMinorOffset = kStrVulkanVersionPatchOffset + kStrVulkanVersionPatchWidth;
static const uint32_t  kStrVulkanVersionMajorOffset = kStrVulkanVersionMinorOffset + kStrVulkanVersionMinorWidth;

// Vulkan API function that gets shader info.
static const char* kStrFunctionNameGetShaderInfo = "vkGetShaderInfoAMD";

// Name of default pipeline.
static const char* kStrVulkanCreatedPipelineName = "pipeline";

// Name of RGA Vulkan Backend for vulkan application info.
static const char* kStrVulkanAppInfoAppName = "RGAVulkanBackend";


// Tokens used to check for Radeon adapters.
static const char* kStrVulkanRadeonAdapterTokenA = "radeon";
static const char* kStrVulkanRadeonAdapterTokenB = "amd";

// Default Vulkan entry point name.
static const char* kStrVulkanDefaultEntryName = "main";

// Environment variable to dump all supported GPUs.
static const std::string kStrVulkanEnvVarAllGpuName = "AMDVLK_NULL_GPU";
static const std::string kStrVulkanEnvVarAllGpuValue = "ALL";

// Environment variable to enable the RGA driver stack on machines with APUs.
static const std::string kStrVulkanEnvVarDisableLayerAmdSwitchableGraphicsName = "DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1";
static const std::string kStrVulkanEnvVarDisableLayerAmdSwitchableGraphicsValue = "1";

// Error messages.
static const char* kStrVulkanErrorFailedToGetVkGetShaderInfoAddress = "Error: failed to get address of vkGetShaderInfoAMD.";
static const char* kStrVulkanErrorFailedEnumerateDevices = "Error: failed to enumerate Vulkan devices.";
static const char* kStrVulkanErrorNoDevicesAvailable = "Error: no supported target GPUs available.";
static const char* kStrVulkanErrorFailedToGetDeviceProps = "Error: failed to get the device properties.";
static const char* kStrVulkanErrorFailedToCreateVulkanInsatnce = "Error: failed to create Vulkan instance.";
static const char* kStrVulkanErrorFailedToCreateLogicalDevice = "Error: failed to create logical Vulkan device";
static const char* kStrVulkanErrorFailedToCreateLogicalDeviceForGpu = "Error: failed to create logical Vulkan device for selected GPU.";
static const char* kStrVulkanErrorFailedToCreatePipelineLayout = "Error: failed to create Vulkan pipeline layout.";
static const char* kStrVulkanErrorFailedToCreateDescriptorSetLayout = "Error: failed to create Vulkan descriptor set layout.";
static const char* kStrVulkanErrorFailedToCreateRenderPass = "Error: failed to create Vulkan render pass.";
static const char* kStrVulkanErrorFailedToCreateShaderModule = "Error: failed to create Vulkan shader module from SPIR-V file ";
static const char* kStrVulkanErrorFailedToCreateCallback = "Error: failed to create Vulkan callback object.";
static const char* kStrVulkanErrorFailedToFindCompatibleDevices = "Error: failed to find any compatible Vulkan physical devices that "
"support the VK_AMD_shader_info device extension.";
static const char* kStrVulkanErrorFailedToGetDeviceExtProperties = "Error: failed to get Device extension properties.";
static const char* kStrVulkanErrorFailedToGetExtensionProperties = "Error: failed to get Vulkan extension properties.";
static const char* kStrVulkanErrorFailedToGetDeviceQueueFamilyProperties = "Error: failed to get device queue family properties.";
static const char* kStrVulkanErrorDeviceDoesNotSupportExtension = "Error: device does not support required extension.";
static const char* kStrVulkanErrorFailedToGetShaderStats = "Error: failed to get shader statistics.";
static const char* kStrVulkanErrorPipelineStatsNotAvailable = "Error: pipeline statistics information not available.";
static const char* kStrVulkanErrorFailedToGetBinarySize = "Error: failed to get binary size.";
static const char* kStrVulkanErrorBinaryNotAvailable = "Error: pipeline binary not available.";
static const char* kStrVulkanErrorDisassemblyNotAvailable = "Error: disassembly not available.";
static const char* kStrVulkanErrorInvalidGpu = "Error: invalid GPU target: ";
static const char* kStrVulkanErrorUnableToOpenFileForWrite1 = "Error: unable to open ";
static const char* kStrVulkanErrorUnableToOpenFileForWrite2 = " for write.";
static const char* kStrVulkanErrorGraphicsOrComputeShaderExpected = "Error: a compute or graphics shader file is expected.";
static const char* kStrVulkanErrorFailedToLoadPipelineFile = "Error: failed to load the pipeline file.";
static const char* kStrVulkanErrorFailedToSetEnvVar = "Error: failed to set environment variable.";
static const char* kStrVulkanErrorFailedToRemoveEnvVar = "Error: failed to remove environment variable.";
static const char* kStrVulkanErrorFailedToDeserializeComputePipeline = "Error: failed to deserialize compute pipeline.";
static const char* kStrVulkanErrorFailedToDeserializeGraphicsPipeline = "Error: failed to deserialize graphics pipeline.";
static const char* kStrVulkanErrorFailedToCreateComputePipeline = "Error: failed to create compute pipeline.";
static const char* kStrVulkanErrorFailedToCreateGraphicsPipeline = "Error: failed to create graphics pipeline.";
static const char* STR_ERR_FAILED_CREATE_PIPELINE_HINT = "Make sure that the provided pipeline state matches the given shaders.";
static const char* kStrVulkanErrorFailedToCreateDefaultComputePipeline = "Error: failed to create default compute pipeline.";
static const char* kStrVulkanErrorFailedToCreateDefaultGraphicsPipeline = "Error: failed to create default graphics pipeline.";
static const char* kStrVulkanErrorEmptyVulkanInstance = "Error: trying to setup validation callback for empty Vulkan instance.";
static const char* kStrVulkanErrorFailedToReadShaderFile = "Error: failed to read shader file at ";
static const char* kStrVulkanErrorCouldNotLocateVulkanLoader = "Error: could not locate the Vulkan loader";
static const char* kStrVulkanErrorNullVkHandle = "Error: unexpected NULL Vulkan instance handle.";
static const char* kStrVulkanErrorNullPipelineCreateInfo = "Error: unexpected NULL pipeline create info.";
static const char* kStrVulkanErrorNullDescriptorSetLayour = "Error: unexpected NULL descriptor set layout.";
static const char* kStrVulkanErrorNullDescriptorSetLayourCreateInfo = "Error: unexpected NULL descriptor set layout create info.";
static const char* kStrVulkanErrorCouldNotGetDeviceExtensionsCount = "Error: could not get the number of device extensions.";
static const char* kStrVulkanErrorCouldNotGetDeviceExtensions = "Error: could not enumerate device extensions.";

// Warnings
// Extensions and validation layers.
static const char* kStrVulkanWarningFailedToCheckExtensions = "Warning: Some required Vulkan extensions are not available.";
static const char* kStrVulkanWarningFailedToGetVulkanExtensionFunctionPointer = "Warning: failed to obtain pointer to required Vulkan extension function.";
static const char* kStrVulkanWarningValidationLayerNotSupported = "Warning: validation layer not supported by runtime: ";
static const char* kStrVulkanWarningExtensionNotSupported = "Vulkan extension is not supported: ";
static const char* kStrVulkanWarningValidationInfoUnavailable = "Vulkan validation information may not be available or complete.";
static const char* kStrVulkanWarningGraphicsPipelineWithoutVertexShader = "Warning: no vertex shader detected in graphics pipeline.";

// General warnings.
static const char* kStrVulkanAmdvlkLocation1 = "Warning: failed to locate AMD's Vulkan driver on the system: ";
static const char* kStrVulkanAmdvlkLocation2 = ". Falling back to using the amdvlk binary that is packaged with RGA.";

// Info.
static const char* kStrVulkanInfoValidationEnabling = "Enabling Vulkan validation layers... ";
static const char* kStrVulkanInfoLayerOutput1 = "*** Output from ";
static const char* kStrVulkanInfoLayerOutputBegin = " layer - begin *** ";
static const char* kStrVulkanInfoLayerOutputEnd = " layer - end *** ";
static const char* kStrVulkanInfoLayerHint = "To get more detailed information about this failure, try enabling Vulkan validation layers.";
static const char* kStrVulkanInfoUsingCustomIcd = "Using Vulkan ICD from custom location: ";
static const char* kStrVulkanInfoFunctionReturnedErrorCode = " function returned error code: ";
static const char* kStrVulkanInfoFunctionReturnedEmptyList = " function returned empty list.";

// Required Vulkan validation layers.
static const std::vector<const char*> kVulkanValidationLayers =
{
    "VK_LAYER_LUNARG_standard_validation",
    "VK_LAYER_LUNARG_core_validation",
    "VK_LAYER_LUNARG_parameter_validation"
};

// Required Vulkan extensions.
static const std::vector<const char*> kVulkanExtensions =
{
    "VK_EXT_debug_report"
};

// Required device extensions.
static const std::vector<const char*> kVulkanExtensionsDevice =
{
    "VK_KHR_driver_properties"
};

// Validation messages to be filtered.
// RGA's Vulkan driver stack may trigger these validation messages although there is no actual issue.
// In the normal use case, the API is pretty much useless without queues because they cannot submit any
// work to the device. However, queues are not necessary solely for compilation and they are not even exposed
// by RGA's driver stack.
static const char* kVulkanValidationLayerMessageFilter0 = "create_info->queueCreateInfoCount";
static const char* kVulkanValidationLayerMessageFilter1 = "[ VUID_Undefined ] Object: VK_NULL_HANDLE (Type = 0) | vkCreateDevice: parameter create_info->queueCreateInfoCount must be greater than 0.";
static const char* kVulkanValidationLayerMessageFilter2 = "Object: 0x0 | vkCreateDevice : parameter create_info->queueCreateInfoCount must be greater than 0. (null)";

// *** AUXILIARY FUNCTIONS - BEGIN ***

// Returns the name of the current executable.
static std::string GetVulkanBackendExeName()
{
    std::string ret;
#ifdef _WIN32
    ret = "vulkan_backend.exe";
#else
    ret = "vulkan_backend";
#endif // _WIN32
    return ret;
}

// Returns true if substr is a substring of src, false otherwise, while ignoring the case.
static bool IsSubstringIgnoreCase(const std::string& src, const std::string& substr)
{
    std::string s1_u = src, s2_u = substr;
    std::transform(s1_u.begin(), s1_u.end(), s1_u.begin(), [](unsigned char c) { return static_cast<unsigned char>(std::toupper(c)); });
    std::transform(s2_u.begin(), s2_u.end(), s2_u.begin(), [](unsigned char c) { return static_cast<unsigned char>(std::toupper(c)); });
    return (s1_u.find(s2_u) != std::string::npos);
}

// Returns true if the given file name represents a file that exists
// and can be opened by the current process. Returns false otherwise.
static bool IsFileExists(const std::string& filename)
{
    std::ifstream infile(filename);
    return infile.good();
}

// Returns the path for the current executable ("vulkan_backend").
static std::string GetCurrentExecutablePath()
{
    std::string ret;
#ifdef _WIN32
    char filename[MAX_PATH];
    DWORD rc = GetModuleFileNameA(NULL, filename, MAX_PATH);
    if (rc > 0)
    {
        // Extract the directory from the full path.
        ret = std::string(filename, filename + rc);
        size_t exe_name_start = ret.find(GetVulkanBackendExeName());
        if (exe_name_start != std::string::npos)
        {
            ret = ret.substr(0, exe_name_start);
        }
    }
#elif __linux
    // Get the directory name with the executable name.
    char path_with_exe_name[PATH_MAX];
    ssize_t rc = readlink("/proc/self/exe", path_with_exe_name, PATH_MAX);
    if (rc != -1)
    {
        // Extract the directory name (without the executable name).
        const char* dir_path = dirname(path_with_exe_name);
        if (dir_path != nullptr)
        {
            ret = dir_path;

            // Append the directory separator.
            ret.append("/");
        }
    }
#endif // _WIN32
    return ret;
}

// Returns the name of the amdvlk file (AMD's Vulkan ICD).
static std::string GetAmdvlkFileName()
{
    std::string amdvlk_filename;
#ifdef _WIN32
    amdvlk_filename = "amdvlk64.dll";
#else
    amdvlk_filename = "amdvlk64.so";
#endif // _WIN32
    return amdvlk_filename;
}

// Returns the path for the amdvlk .dll or .so file that is packaged with RGA.
static std::string GetOfflineAmdvlkPath()
{
    // Get the path for the current executable (VulkanBackend).
    std::stringstream amdvlk_offline_file;
    amdvlk_offline_file << GetCurrentExecutablePath();

    // Append the sub-directory where the offline amdvlk binary resides.
    const char* kAmdvlkOfflineFolder = "amdvlk";

    // Append the offline amdvlk library name.
    amdvlk_offline_file << kAmdvlkOfflineFolder << "/" << GetAmdvlkFileName();
    return amdvlk_offline_file.str();
}

// *** AUXILIARY FUNCTIONS - END ***

static bool SetEnvironmentVariable(const std::string& name, const std::string& value)
{
    bool result = false;
#ifdef WIN32
    result = (::SetEnvironmentVariable(std::wstring(name.cbegin(), name.cend()).c_str(),
        std::wstring(value.cbegin(), value.cend()).c_str()) == TRUE);
#else
    result = (::setenv(name.c_str(), value.c_str(), 1) == 0);
#endif
    assert(result);
    return result;
}

// Environment variable helper class.
class EnvVar
{
public:
    EnvVar(const std::string name, const std::string& val, bool pred = true)
    {
        if (pred)
        {
            bool result = SetEnvironmentVariable(name, val);
            assert(result);
            if (result)
            {
                name_ = name;
            }
            else
            {
                std::cerr << kStrVulkanErrorFailedToSetEnvVar << std::endl;
            }
        }
    }

    ~EnvVar()
    {
        if (!name_.empty())
        {
#ifdef WIN32
            bool result = (::SetEnvironmentVariable(std::wstring(name_.cbegin(), name_.cend()).c_str(), nullptr) == TRUE);
#else
            bool result = (::unsetenv(name_.c_str()) == 0);
#endif
            assert(result);
            if (!result)
            {
                std::cerr << kStrVulkanErrorFailedToRemoveEnvVar << std::endl;
            }
        }
    }

private:
    std::string  name_;
};

// Decode the 32-bit Vulkan version integer. Returns triple {major, minor, patch}.
static std::tuple<uint32_t, uint32_t, uint32_t> DecodeVulkanVersion(uint32_t version)
{
    const uint32_t major_mask = (1 << kStrVulkanVersionMajorWidth) - 1;
    const uint32_t minor_mask = (1 << kStrVulkanVersionMinorWidth) - 1;
    const uint32_t patch_mask = (1 << kStrVulkanVersionPatchWidth) - 1;

    const uint32_t major = (version >> kStrVulkanVersionMajorOffset) & major_mask;
    const uint32_t minor = (version >> kStrVulkanVersionMinorOffset) & minor_mask;
    const uint32_t patch = (version >> kStrVulkanVersionPatchOffset) & patch_mask;

    return std::tuple<uint32_t, uint32_t, uint32_t>(major, minor, patch);
}

static std::vector<const char*> GetAvailableValidationLayers()
{
    uint32_t layers_num = 0;
    std::vector<const char*> supported_layers;

    // Get the validation layers supported by Vulkan driver installed on the machine.
    if (vkEnumerateInstanceLayerProperties(&layers_num, nullptr) == VK_SUCCESS)
    {
        std::vector<VkLayerProperties> layer_props(layers_num);
        if (vkEnumerateInstanceLayerProperties(&layers_num, layer_props.data()) == VK_SUCCESS)
        {
            // Check if required layers are present in the list of supported layers.
            std::set<std::string> layer_names;
            std::for_each(layer_props.cbegin(), layer_props.cend(),
                [&](const VkLayerProperties& p) { layer_names.insert(p.layerName); });

            for (const char* validation_layer : kVulkanValidationLayers)
            {
                // If the required layer is supported, add it to the "supportedLayers" list.
                // Otherwise, print a warning.
                if (layer_names.find(validation_layer) != layer_names.cend())
                {
                    supported_layers.push_back(validation_layer);
                }
                else
                {
                    // Inform the user that the validation layer is not available by the runtime.
                    std::cerr << kStrVulkanWarningValidationLayerNotSupported << validation_layer << std::endl;
                }
            }
        }
    }

    return supported_layers;
}

RgConfig::Action RgVulkanBackend::ParseCmdLine(int argc, char* argv[])
{
    bool list_targets = false, list_adapters = false;

    assert(argv != nullptr);
    if (argv != nullptr)
    {
        try
        {
            cxxopts::Options opts(argv[0]);
            opts.add_options()
                ("h,help", "Print help.")
                ("list-targets", "List all supported target GPUs.", cxxopts::value<bool>(list_targets))
                ("list-adapters", "List physical display adapters installed on the system.", cxxopts::value<bool>(list_adapters))
                ("target", "The name of the target GPU.", cxxopts::value<std::string>(config_.target_gpu))
                ("enable-layers", "Enable Vulkan validation layers.", cxxopts::value<bool>(config_.enable_layers))
                ("layers-file", "Path to the validation layers output file.", cxxopts::value<std::string>(config_.validation_file))

                // Pipeline Object file.
                ("pso", "Pipeline state file which would be used to set the pipeline state for the compilation process."
                    "If not specified, RGA would create and set a \"default\" pipeline state.",
                    cxxopts::value<std::string>(config_.pipeline_state_file))

                // ICD file path.
                ("icd", kStrCliOptIcdDescription, cxxopts::value<std::string>(config_.icd_path))

                // VK_LOADER_DEBUG environment variable.
                ("loader-debug", "Value for VK_LOADER_DEBUG", cxxopts::value<std::string>(config_.loader_debug))

                // Per-stage shader input source file options.
                ("vert", "Path to the source file which contains the vertex shader to be attached to the pipeline.",
                    cxxopts::value<std::string>(config_.spv_files[RgPipelineStage::kVertex]))
                ("tesc", "Path to the source file which contains the tessellation control shader to be attached to the pipeline.",
                    cxxopts::value<std::string>(config_.spv_files[RgPipelineStage::kTessellationControl]))
                ("tese", "Path to the source file which contains the tessellation evaluation shader to be attached to the pipeline.",
                    cxxopts::value<std::string>(config_.spv_files[RgPipelineStage::kTessellationEvaluation]))
                ("geom", "Path to the source file which contains the geometry shader to be attached to the pipeline.",
                    cxxopts::value<std::string>(config_.spv_files[RgPipelineStage::kGeometry]))
                ("frag", "Path to the source file which contains the fragment shader to be attached to the pipeline.",
                    cxxopts::value<std::string>(config_.spv_files[RgPipelineStage::kFragment]))
                ("comp", "Path to the source file which contains the compute shader to be attached to the pipeline.",
                    cxxopts::value<std::string>(config_.spv_files[RgPipelineStage::kCompute]))

                // Entry points.
                ("vert-entry", "Vertex shader entry point",
                    cxxopts::value<std::string>(config_.entries[RgPipelineStage::kVertex])->default_value(kStrVulkanDefaultEntryName))
                ("tesc-entry", "Tesselation control shader entry point",
                    cxxopts::value<std::string>(config_.entries[RgPipelineStage::kTessellationControl])->default_value(kStrVulkanDefaultEntryName))
                ("tese-entry", "Tesselation evaluation shader entry point",
                    cxxopts::value<std::string>(config_.entries[RgPipelineStage::kTessellationEvaluation])->default_value(kStrVulkanDefaultEntryName))
                ("geom-entry", "Geometry shader entry point",
                    cxxopts::value<std::string>(config_.entries[RgPipelineStage::kGeometry])->default_value(kStrVulkanDefaultEntryName))
                ("frag-entry", "Fragment shader entry point",
                    cxxopts::value<std::string>(config_.entries[RgPipelineStage::kFragment])->default_value(kStrVulkanDefaultEntryName))
                ("comp-entry", "Compute shader entry point",
                    cxxopts::value<std::string>(config_.entries[RgPipelineStage::kCompute])->default_value(kStrVulkanDefaultEntryName))

                // Output binary file.
                ("bin", "Path to the output pipeline binary file.", cxxopts::value<std::string>(config_.binary_file))

                ("vert-isa", "Path to the output file which contains the disassembly for the vertex shader.",
                    cxxopts::value<std::string>(config_.isa_files[RgPipelineStage::kVertex]))
                ("tesc-isa", "Path to the output file which contains the disassembly for the tessellation control shader.",
                    cxxopts::value<std::string>(config_.isa_files[RgPipelineStage::kTessellationControl]))
                ("tese-isa", "Path to the output file which contains the disassembly for the tessellation evaluation shader.",
                    cxxopts::value<std::string>(config_.isa_files[RgPipelineStage::kTessellationEvaluation]))
                ("geom-isa", "Path to the output file which contains the disassembly for the geometry shader.",
                    cxxopts::value<std::string>(config_.isa_files[RgPipelineStage::kGeometry]))
                ("frag-isa", "Path to the output file which contains the disassembly for the fragment shader.",
                    cxxopts::value<std::string>(config_.isa_files[RgPipelineStage::kFragment]))
                ("comp-isa", "Path to the output file which contains the disassembly for the compute shader.",
                    cxxopts::value<std::string>(config_.isa_files[RgPipelineStage::kCompute]))

                ("vert-stats", "Path to the output file which contains the statistics for the vertex shader.",
                    cxxopts::value<std::string>(config_.stats_files[RgPipelineStage::kVertex]))
                ("tesc-stats", "Path to the output file which contains the statistics for the tessellation control shader.",
                    cxxopts::value<std::string>(config_.stats_files[RgPipelineStage::kTessellationControl]))
                ("tese-stats", "Path to the output file which contains the statistics for the tessellation evaluation shader.",
                    cxxopts::value<std::string>(config_.stats_files[RgPipelineStage::kTessellationEvaluation]))
                ("geom-stats", "Path to the output file which contains the statistics for the geometry shader.",
                    cxxopts::value<std::string>(config_.stats_files[RgPipelineStage::kGeometry]))
                ("frag-stats", "Path to the output file which contains the statistics for the fragment shader.",
                    cxxopts::value<std::string>(config_.stats_files[RgPipelineStage::kFragment]))
                ("comp-stats", "Path to the output file which contains the statistics for the compute shader.",
                    cxxopts::value<std::string>(config_.stats_files[RgPipelineStage::kCompute]))
                ;

            // Parse command line.
            auto result = opts.parse(argc, argv);
            if (result.count("help"))
            {
                std::cout << opts.help() << std::endl;
                exit(0);
            }
            else
            {
                config_.action = (list_targets ? RgConfig::Action::kListTargets :
                    (list_adapters ? RgConfig::Action::kListAdapters : RgConfig::Action::kBuild));
            }
        }
        catch (const cxxopts::OptionException& e)
        {
            std::cerr << "Error parsing options: " << e.what() << std::endl;
        }
    }

    return config_.action;
}

static bool WriteBinaryFile(const std::string& filename, const std::vector<char>& content)
{
    bool ret = false;
    std::ofstream output;
    output.open(filename.c_str(), std::ios::binary);

    if (output.is_open() && !content.empty())
    {
        output.write(&content[0], content.size());
        output.close();
        ret = true;
    }
    else
    {
        std::cerr << kStrVulkanErrorUnableToOpenFileForWrite1 << filename << kStrVulkanErrorUnableToOpenFileForWrite2 << std::endl;
    }

    return ret;
}

static bool WriteTextFile(const std::string& filename, const std::string& content)
{
    bool ret = false;
    std::ofstream output;
    output.open(filename.c_str());

    if (output.is_open())
    {
        output << content << std::endl;
        output.close();
        ret = true;
    }
    else
    {
        std::cerr << kStrVulkanErrorUnableToOpenFileForWrite1 << filename << kStrVulkanErrorUnableToOpenFileForWrite2 << std::endl;
    }

    return ret;
}

int main(int argc, char* argv[])
{
    bool status = false;
    bool is_amd_gpu = false;

    RgVulkanBackend vulkan_backend;
    RgConfig::Action required_action = vulkan_backend.ParseCmdLine(argc, argv);
    switch (required_action)
    {
    case RgConfig::Action::kListTargets:
        status = vulkan_backend.ListTargets();
        break;

    case RgConfig::Action::kListAdapters:
        status = vulkan_backend.ListAdapters(is_amd_gpu, true);
        break;

    case RgConfig::Action::kBuild:
        status = vulkan_backend.Build();
        break;
    }

    vulkan_backend.PrintValidationHint();

    return (status ? 0 : 1);
}

void RgVulkanBackend::DestroyValidationCallback()
{
    // If the validation callback was installed, destroy it.
    if (debug_callback_ != nullptr)
    {
        auto vkDestroyCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance_, "vkDestroyDebugReportCallbackEXT");
        if (vkDestroyCallback != nullptr)
        {
            vkDestroyCallback(instance_, debug_callback_, nullptr);
        }
    }
}

bool RgVulkanBackend::InitVulkan(bool should_enable_validation)
{
    bool result = false;

    // Locate the Vulkan loader (or explicitly locate the ICD if requested by the user).
    const char* icd_path = nullptr;
    if (!config_.icd_path.empty())
    {
        icd_path = config_.icd_path.c_str();

        // In case that this is a build session, notify the user that we are using a Vulkan ICD from a custom location.
        if (should_enable_validation)
        {
            std::cout << kStrVulkanInfoUsingCustomIcd << icd_path << std::endl;
        }
    }

    // If required, set the "VK_LOADER_DEBUG" environment variable.
    if (!config_.loader_debug.empty())
    {
        SetEnvironmentVariable("VK_LOADER_DEBUG", config_.loader_debug);
    }

    // Try to initialize using the system's ICD or the one that was given explicitly.
    VkResult volk_rc = volkInitialize(icd_path);
    assert(volk_rc == VK_SUCCESS);
    if (volk_rc != VK_SUCCESS)
    {
        // We failed to locate the Vulkan loader/ICD.
        if (icd_path != nullptr)
        {
            std::cerr << kStrVulkanErrorCouldNotLocateVulkanLoader << "." << std::endl;
        }
        else
        {
            // We will get here in case that there is no Vulkan loader ("vulkan-1")
            // on the system's path. Try loading the amdvlk binary which is packaged with RGA.
            std::string amdvlkOfflineFilePath = GetOfflineAmdvlkPath();
            if (IsFileExists(amdvlkOfflineFilePath))
            {
                // Notify the user.
                std::cerr << kStrVulkanAmdvlkLocation1 << GetAmdvlkFileName() <<
                    kStrVulkanAmdvlkLocation2 << std::endl;

                // Try loading the offline amdvlk binary.
                volk_rc = volkInitialize(amdvlkOfflineFilePath.c_str());
            }
        }
    }

    if (volk_rc == VK_SUCCESS)
    {
        // We managed to locate the Vulkan loader/ICD. Now, let's create a Vulkan instance.
        std::vector<const char*> supported_extensions;
        VkApplicationInfo app_info = { VK_STRUCTURE_TYPE_APPLICATION_INFO, 0 };

        app_info.pApplicationName = kStrVulkanAppInfoAppName;
        app_info.applicationVersion = 1;
        app_info.pEngineName = "";
        app_info.apiVersion = VK_API_VERSION_1_1;

        VkInstanceCreateInfo instance_info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, 0 };
        instance_info.pApplicationInfo = &app_info;

        // Add Vulkan validation layers to the instance info, if needed.
        if (should_enable_validation && config_.enable_layers)
        {
            available_validation_layers_ = GetAvailableValidationLayers();
            if (!available_validation_layers_.empty())
            {
                instance_info.ppEnabledLayerNames = available_validation_layers_.data();
                instance_info.enabledLayerCount = static_cast<uint32_t>(available_validation_layers_.size());
            }
        }

        // Add Vulkan extensions to the instance info.
        supported_extensions = InitExtensions();
        instance_info.ppEnabledExtensionNames = supported_extensions.data();
        instance_info.enabledExtensionCount = static_cast<uint32_t>(supported_extensions.size());

        if (supported_extensions.size() != kVulkanExtensions.size())
        {
            std::cerr << kStrVulkanWarningFailedToCheckExtensions << std::endl;
            std::cerr << kStrVulkanWarningValidationInfoUnavailable << std::endl;
        }

        // Create a Vulkan instance.
        result = (vkCreateInstance(&instance_info, nullptr, &instance_) == VkResult::VK_SUCCESS);
        assert(result && kStrVulkanErrorFailedToCreateVulkanInsatnce);
        if (!result)
        {
            std::cerr << kStrVulkanErrorFailedToCreateVulkanInsatnce << std::endl;
            std::cerr << "vkCreateInstance" << kStrVulkanInfoFunctionReturnedErrorCode << result << std::endl;
            should_print_validation_hint_ = true;
        }

        // Load all required Vulkan entry points through volk.
        volkLoadInstance(instance_);

        // Setup the Vulkan validation debug callback function.
        if (should_enable_validation && config_.enable_layers && result)
        {
            result = SetupValidationCallback();
        }
    }
    else
    {
        // Notify the user about the failure.
        std::cerr << kStrVulkanErrorCouldNotLocateVulkanLoader;
        if (strlen(icd_path) > 0)
        {
            std::cerr << ": " << icd_path;
        }
        std::cerr << std::endl;
    }

    return result;
}

void RgVulkanBackend::DestroyVulkanInstance()
{
    assert(instance_ != VK_NULL_HANDLE);
    if (instance_ != VK_NULL_HANDLE)
    {
        if (config_.enable_layers)
        {
            DestroyValidationCallback();
        }

        if (validation_output_file_.is_open())
        {
            validation_output_file_.close();
        }

        // Destroy the Vulkan device and instance if they were created successfully.
        if (device_ != VK_NULL_HANDLE)
        {
            vkDestroyDevice(device_, nullptr);
        }

        vkDestroyInstance(instance_, nullptr);
        gpus_.clear();
    }
    else
    {
        std::cerr << kStrVulkanErrorNullVkHandle << std::endl;
    }
}

bool RgVulkanBackend::GetTargetGPUNames(std::vector<std::string>& target_names)
{
    // First check if the primary adapter is a Radeon device.
    bool is_amd_gpu = false;
    ListAdapters(is_amd_gpu);

    // Set the DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1
    // environment variable to enable the RGA driver stack on APUs.
    EnvVar varApuWorkaround(kStrVulkanEnvVarDisableLayerAmdSwitchableGraphicsName,
        kStrVulkanEnvVarDisableLayerAmdSwitchableGraphicsValue);

    // Set "AMDVLK_NULL_GPU=ALL" environment variable.
    EnvVar var(kStrVulkanEnvVarAllGpuName, kStrVulkanEnvVarAllGpuValue);

    // Force using the bundled amdvlk if we are on a machine
    // without AMD primary GPU and the user did not already ask
    // to use a custom ICD location.
    if (!is_amd_gpu && config_.icd_path.empty())
    {
        // Try loading the amdvlk binary which is packaged with RGA.
        std::string offline_icd_path = GetOfflineAmdvlkPath();
        if (IsFileExists(offline_icd_path))
        {
            // Notify the user.
            std::cerr << kStrVulkanAmdvlkLocation1 << GetAmdvlkFileName() <<
                kStrVulkanAmdvlkLocation2 << std::endl;

            // Try loading the offline amdvlk library.
            config_.icd_path = offline_icd_path;
        }
    }

    bool status = InitVulkan(false);
    assert(status);
    if (status)
    {
        if ((status = EnumerateDevices()) == true)
        {
            for (const auto& gpu : gpus_)
            {
                target_names.push_back(gpu.first);
            }
        }

        DestroyVulkanInstance();
    }

    return status;
}

bool RgVulkanBackend::ListTargets()
{
    std::vector<std::string> target_names;

    bool status = GetTargetGPUNames(target_names);
    if (status)
    {
        for (const std::string& target : target_names)
        {
            std::cout << target << std::endl;
        }
        std::cout << std::endl;
    }

    return status;
}

bool RgVulkanBackend::ListAdapters(bool& is_amd_gpu, bool should_print)
{
    int i = 0;
    bool status = InitVulkan(false);
    assert(status);
    if (status)
    {
        if ((status = EnumerateDevices()) == true)
        {
            for (const auto& gpu : gpus_)
            {
                if (i == 0)
                {
                    // For the primary adapter, make sure that it is an AMD device.
                    // First, enumerate the extensions that are supported by the primary device.
                    uint32_t extensions_count = 0;
                    VkResult result = VK_SUCCESS;
                    result = vkEnumerateDeviceExtensionProperties(gpu.second, nullptr, &extensions_count, nullptr);
                    if ((result == VK_SUCCESS) && (extensions_count > 0))
                    {
                        std::vector<VkExtensionProperties> available_extensions;
                        available_extensions.resize(extensions_count);
                        result = vkEnumerateDeviceExtensionProperties(gpu.second, nullptr, &extensions_count, available_extensions.data());
                        if ((result == VK_SUCCESS) && (extensions_count > 0))
                        {
                            std::set<std::string> extension_names;
                            std::for_each(available_extensions.cbegin(), available_extensions.cend(),
                                [&](const VkExtensionProperties& p) { extension_names.insert(p.extensionName); });

                            // Check if required extensions is supported by the primary adapter.
                            bool is_extension_supported = true;
                            for (const char* required_extension : kVulkanExtensionsDevice)
                            {
                                if (extension_names.find(required_extension) == extension_names.cend())
                                {
                                    is_extension_supported = false;
                                    break;
                                }
                            }

                            assert(is_extension_supported);
                            if (is_extension_supported)
                            {
                                // Check if this is an AMD device.
                                VkPhysicalDeviceDriverPropertiesKHR driver_props = {};
                                driver_props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES_KHR;
                                VkPhysicalDeviceProperties2 props2 = {};
                                props2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
                                props2.pNext = &driver_props;
                                vkGetPhysicalDeviceProperties2(gpu.second, &props2);
                                is_amd_gpu = (driver_props.driverID == VK_DRIVER_ID_AMD_PROPRIETARY_KHR);
                            }
                        }
                        else
                        {
                            std::cerr << kStrVulkanErrorCouldNotGetDeviceExtensions << std::endl;
                        }
                    }
                    else
                    {
                        std::cerr << kStrVulkanErrorCouldNotGetDeviceExtensionsCount << std::endl;
                    }
                }

                if (should_print)
                {
                    // Get the device name and print it.
                    VkPhysicalDeviceProperties props = {};
                    vkGetPhysicalDeviceProperties(gpu.second, &props);
                    std::cout << kStrCliVkBackendStrAdapter << i++ << ":" << std::endl;
                    std::cout << kStrCliVkBackendStrAdapterOffset << kStrCliVkBackendStrAdapterName << props.deviceName << std::endl;
                    std::cout << kStrCliVkBackendStrAdapterOffset << kStrCliVkBackendStrAdapterDriver;
                    uint32_t driver_ver_minor = props.driverVersion & kStrVulkanDriverVersionMinorBits;
                    std::cout << kStrVulkanDriverVersionMajor << '.' << driver_ver_minor << std::endl;
                    auto vulkan_version = DecodeVulkanVersion(props.apiVersion);
                    std::cout << kStrCliVkBackendStrAdapterOffset << kStrCliVkBackendStrAdapterVulkan;
                    std::cout << std::get<0>(vulkan_version) << '.' << std::get<1>(vulkan_version) << '.' << std::get<2>(vulkan_version) << std::endl;
                }
            }
        }

        DestroyVulkanInstance();
    }

    return status;
}

bool RgVulkanBackend::Build()
{
    std::vector<std::string>  target_names;
    bool status = GetTargetGPUNames(target_names);
    if (status)
    {
        // If target GPU is not specified, build for the 1st physical adapter installed on the system.
        // Otherwise, build for the GPU specified by user.
        if (config_.target_gpu.empty())
        {
            status = BuildForDevice("");
        }
        else
        {
            auto target_gpu = std::find_if(target_names.cbegin(), target_names.cend(),
                [&](const std::string& s) {return IsSubstringIgnoreCase(s, config_.target_gpu); });

            if (target_gpu != target_names.cend())
            {
                // Set the DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1
                // environment variable to enable the RGA driver stack on APUs.
                EnvVar var_apu_workaround(kStrVulkanEnvVarDisableLayerAmdSwitchableGraphicsName,
                    kStrVulkanEnvVarDisableLayerAmdSwitchableGraphicsValue);

                // Set "AMDVLK_NULL_GPU = target_gpu_name" environment variable.
                EnvVar var(kStrVulkanEnvVarAllGpuName, *target_gpu);

                status = BuildForDevice(*target_gpu);
            }
            else
            {
                std::cerr << kStrVulkanErrorInvalidGpu << config_.target_gpu << std::endl;
            }
        }
    }

    return status;
}

void RgVulkanBackend::PrintValidationHint()
{
    if (should_print_validation_hint_ && !config_.enable_layers)
    {
        std::cerr << kStrVulkanInfoLayerHint << std::endl;
    }
}

std::vector<const char*> RgVulkanBackend::InitExtensions()
{
    VkResult result = VK_SUCCESS;
    uint32_t extension_count = 0;
    std::vector<const char*> supported_extensions;

    // Get the extensions supported by Vulkan driver installed on the machine.
    result = vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
    if (result == VK_SUCCESS)
    {
        std::vector<VkExtensionProperties> extnesion_properties(extension_count);
        result = vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extnesion_properties.data());
        if (result == VK_SUCCESS)
        {
            // Check if required layers are present in the list of supported layers.
            std::set<std::string> extension_names;
            std::for_each(extnesion_properties.cbegin(), extnesion_properties.cend(),
                [&](const VkExtensionProperties& p) { extension_names.insert(p.extensionName); });

            for (const char* required_extension : kVulkanExtensions)
            {
                // If the required extension is supported, add it to the supported extensions list.
                // Otherwise, print a warning.
                if (extension_names.find(required_extension) != extension_names.cend())
                {
                    supported_extensions.push_back(required_extension);
                }
                else
                {
                    std::cerr << kStrVulkanWarningExtensionNotSupported << required_extension << std::endl;
                }
            }
        }
    }

    if (result != VK_SUCCESS)
    {
        std::cerr << kStrVulkanErrorFailedToGetExtensionProperties << std::endl;
        std::cerr << "vkEnumerateInstanceExtensionProperties" << kStrVulkanInfoFunctionReturnedErrorCode << result << std::endl;
        should_print_validation_hint_ = true;
    }

    return supported_extensions;
}

// Call-back function for Vulkan validation layers.
static VKAPI_ATTR VkBool32 VKAPI_CALL ValidationCallback(VkDebugReportFlagsEXT ,
    VkDebugReportObjectTypeEXT ,
    uint64_t                   ,
    size_t                     ,
    int32_t                    ,
    const char* layer_prefix,
    const char* message,
    void* user_data)
{
    if (message != nullptr && layer_prefix != nullptr)
    {
        // Filter redundant messages if necessary.
        std::string msg(message);
        bool should_filter = (msg.find(kVulkanValidationLayerMessageFilter0) != std::string::npos) ||
            (msg.find(kVulkanValidationLayerMessageFilter1) != std::string::npos) ||
            (msg.find(kVulkanValidationLayerMessageFilter2) != std::string::npos);

        if (!should_filter)
        {
            std::ostream& out_stream = *reinterpret_cast<std::ostream*>(user_data);
            out_stream << kStrVulkanInfoLayerOutput1 << layer_prefix << kStrVulkanInfoLayerOutputBegin << std::endl << message << std::endl;
            out_stream << kStrVulkanInfoLayerOutput1 << layer_prefix << kStrVulkanInfoLayerOutputEnd << std::endl;
        }
    }
    return VK_FALSE;
}

bool RgVulkanBackend::SetupValidationCallback()
{
    bool result = false;
    void* validation_out_stream = &std::cout;

    // Status message.
    std::stringstream msg;
    msg << kStrVulkanInfoValidationEnabling;

    if (!config_.validation_file.empty())
    {
        validation_output_file_.open(config_.validation_file);
        if (validation_output_file_.good())
        {
            validation_out_stream = &validation_output_file_;
        }
    }

    VkDebugReportCallbackCreateInfoEXT callback_info = {
        VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
        nullptr,
        VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT,
        ValidationCallback,
        validation_out_stream
    };

    assert(instance_ != VK_NULL_HANDLE);
    if (instance_ != VK_NULL_HANDLE)
    {
        auto vk_create_callback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance_, "vkCreateDebugReportCallbackEXT");
        assert(vk_create_callback != nullptr);
        if (vk_create_callback != nullptr)
        {
            result = (vk_create_callback(instance_, &callback_info, nullptr, &debug_callback_) == VK_SUCCESS);
            assert(result);
            if (!result)
            {
                std::cerr << kStrVulkanErrorFailedToCreateCallback << std::endl;
                std::cerr << kStrVulkanInfoFunctionReturnedErrorCode << result << std::endl;
                should_print_validation_hint_ = true;
            }
        }
        else
        {
            std::cerr << kStrVulkanWarningFailedToGetVulkanExtensionFunctionPointer << std::endl;
            std::cerr << kStrVulkanInfoFunctionReturnedErrorCode << "NULL" << std::endl;
            std::cerr << kStrVulkanWarningValidationInfoUnavailable << std::endl;
        }
    }
    else
    {
        std::cerr << kStrVulkanErrorEmptyVulkanInstance << std::endl;
    }

    // Finalize the message and print it.
    msg << (result ? "success." : "failure.");
    std::cout << msg.str() << std::endl;

    return result;
}

bool RgVulkanBackend::EnumerateDevices()
{
    uint32_t gpu_count = 0;
    VkResult vk_result = VK_SUCCESS;
    std::vector<VkPhysicalDevice> gpus;

    vk_result = vkEnumeratePhysicalDevices(instance_, &gpu_count, nullptr);
    bool result = (vk_result == VK_SUCCESS);
    assert(result);

    // Get the device handles.
    if (result && gpu_count > 0)
    {
        gpus.resize(gpu_count);
        vk_result = vkEnumeratePhysicalDevices(instance_, &gpu_count, &gpus[0]);
        result = (vk_result == VK_SUCCESS);
        assert(result);
    }

    if (!result)
    {
        std::cerr << kStrVulkanErrorFailedEnumerateDevices << std::endl;
        std::cerr << "vkEnumeratePhysicalDevices" << kStrVulkanInfoFunctionReturnedErrorCode << vk_result << std::endl;
        should_print_validation_hint_ = true;
    }

    // Get the device names.
    if (result)
    {
        for (VkPhysicalDevice gpu : gpus)
        {
            VkPhysicalDeviceProperties props = {};
            vkGetPhysicalDeviceProperties(gpu, &props);
            if (strnlen(props.deviceName, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE) > 0)
            {
                gpus_[props.deviceName] = gpu;
            }
            else
            {
                std::cerr << kStrVulkanErrorFailedToGetDeviceProps << std::endl;
                std::cerr << kStrVulkanInfoFunctionReturnedEmptyList << std::endl;
                result = false;
                break;
            }
        }

        if (!result)
        {
            std::cerr << kStrVulkanErrorFailedEnumerateDevices << std::endl;
            should_print_validation_hint_ = true;
        }
    }

    return result;
}

// A structure used to track the family index of the graphics and compute queue.
struct QueueFamilyIndices
{
    int graphics_family = -1;
    int compute_family = -1;

    bool IsComplete() const
    {
        return graphics_family >= 0 && compute_family >= 0;
    }
};

// Find the Queue families supported by the given physical device.
QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

    int index = 0;
    for (const auto& queue_family : queue_families)
    {
        if (queue_family.queueCount > 0 && queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphics_family = index;
        }

        if (queue_family.queueCount > 0 && queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            indices.compute_family = index;
        }

        if (indices.IsComplete())
        {
            break;
        }

        index++;
    }

    return indices;
}

bool RgVulkanBackend::VerifyDevice(VkPhysicalDevice gpu)
{
    uint32_t extension_count = 0;
    bool result = false;
    VkResult vk_result = VK_SUCCESS;

    vk_result = vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extension_count, nullptr);
    bool fatal_error = (vk_result != VK_SUCCESS);
    if (!fatal_error && extension_count > 0)
    {
        std::vector<VkExtensionProperties> extension_properties;
        extension_properties.resize(extension_count);

        vk_result = vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extension_count, &extension_properties[0]);
        fatal_error = (vk_result != VK_SUCCESS);
        if (!fatal_error)
        {
            for (uint32_t extIdx = 0; extIdx < extension_count; ++extIdx)
            {
                if (std::string(extension_properties[extIdx].extensionName) == VK_AMD_SHADER_INFO_EXTENSION_NAME)
                {
                    result = true;
                    break;
                }
            }
        }
    }

    if (fatal_error)
    {
        std::cerr << kStrVulkanErrorFailedToGetDeviceExtProperties << std::endl;
        std::cerr << kStrVulkanInfoFunctionReturnedErrorCode << vk_result << std::endl;
        should_print_validation_hint_ = true;
    }

    if (!result)
    {
        std::cerr << kStrVulkanErrorDeviceDoesNotSupportExtension << std::endl;
    }

    return result;
}

bool RgVulkanBackend::CreateDevice(VkPhysicalDevice gpu)
{
    bool result = false;
    VkResult vk_result = VK_SUCCESS;

    VkDeviceCreateInfo device_info = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, 0 };
    const char* enabledDeviceExtensions[] = { VK_AMD_SHADER_INFO_EXTENSION_NAME };

    // Specify the extensions to enable for the device.
    device_info.enabledExtensionCount = sizeof(enabledDeviceExtensions) / sizeof(const char*);
    device_info.ppEnabledExtensionNames = enabledDeviceExtensions;

    QueueFamilyIndices indices = FindQueueFamilies(gpu);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    // Only add the queue family if the indices are complete.
    std::set<int> unique_queue_families;
    if (indices.IsComplete())
    {
        unique_queue_families.emplace(indices.graphics_family);
        unique_queue_families.emplace(indices.compute_family);
    }

    float queue_priority = 1.0f;
    for (int queue_family : unique_queue_families)
    {
        // Add the create info for the type of family needed.
        VkDeviceQueueCreateInfo queue_create_info = {};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;

        queueCreateInfos.push_back(queue_create_info);
    }

    // Add the queue create info to the device create info.
    device_info.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    device_info.pQueueCreateInfos = queueCreateInfos.data();

    // Explicitly enable usage of geometry and tessellation stages.
    VkPhysicalDeviceFeatures features = {};
    features.geometryShader = VK_TRUE;
    features.tessellationShader = VK_TRUE;
    features.multiViewport = VK_TRUE;
    device_info.pEnabledFeatures = &features;

    vk_result = vkCreateDevice(gpu, &device_info, nullptr, &device_);
    result = (vk_result == VK_SUCCESS);
    assert(result && kStrVulkanErrorFailedToCreateLogicalDevice);

    if (!result)
    {
        std::cerr << kStrVulkanErrorFailedToCreateLogicalDeviceForGpu << std::endl;
        std::cerr << "vkCreateDevice" << kStrVulkanInfoFunctionReturnedErrorCode << vk_result << std::endl;
        should_print_validation_hint_ = true;
    }

    // Obtain the address of vkGetShaderInfoAMD function.
    function_get_shader_info_ = (PFN_vkGetShaderInfoAMD)vkGetDeviceProcAddr(device_, kStrFunctionNameGetShaderInfo);
    assert(function_get_shader_info_ != nullptr);
    result = (function_get_shader_info_ != nullptr);
    if (!result)
    {
        std::cerr << kStrVulkanErrorFailedToGetVkGetShaderInfoAddress << std::endl;
        std::cerr << "vkGetDeviceProcAddr" << kStrVulkanInfoFunctionReturnedErrorCode << "NULL" << std::endl;
        should_print_validation_hint_ = true;
    }

    return result;
}

bool RgVulkanBackend::LoadComputePipelineStateFile(const std::string& pso_file_path)
{
    std::string error_string;
    bool is_loaded = RgPsoSerializerVulkan::ReadStructureFromFile(pso_file_path, &compute_pipeline_create_info_, error_string);
    if (!is_loaded)
    {
        std::cerr << kStrVulkanErrorFailedToDeserializeComputePipeline << std::endl;
        std::cerr << error_string << std::endl;
    }

    return is_loaded;
}

bool RgVulkanBackend::CreateComputePipeline()
{
    bool ret = false;
    std::stringstream error_text;

    assert(compute_pipeline_create_info_ != nullptr);
    if ((ret = (compute_pipeline_create_info_ != nullptr)) == true)
    {
        VkComputePipelineCreateInfo* compute_pipeline_create_info = compute_pipeline_create_info_->GetComputePipelineCreateInfo();
        assert(compute_pipeline_create_info != nullptr);
        if ((ret = (compute_pipeline_create_info != nullptr)) == true)
        {
            // These handles need to be created from the deserialized CreateInfo structures,
            // and then assigned into the compute pipeline create info before attempting to
            // create the compute pipeline.
            VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

            // Attempt to create the descriptor set layout.
            std::vector<VkDescriptorSetLayoutCreateInfo*> descriptor_set_layout_collection = compute_pipeline_create_info_->GetDescriptorSetLayoutCreateInfo();
            std::vector<VkDescriptorSetLayout> descriptor_set_layout_handles;
            for (VkDescriptorSetLayoutCreateInfo* descriptor_set_layout_create_info : descriptor_set_layout_collection)
            {
                VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
                assert(descriptor_set_layout_create_info != nullptr);
                if ((ret = (descriptor_set_layout_create_info != nullptr)) == true)
                {
                    // Set the structure type for the Descriptor Set Layout create info.
                    descriptor_set_layout_create_info->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

                    // Attempt to create the Descriptor Set Layout.
                    VkResult create_result = vkCreateDescriptorSetLayout(device_, descriptor_set_layout_create_info, nullptr, &descriptor_set_layout);
                    assert(create_result == VK_SUCCESS);
                    if ((ret = (create_result == VK_SUCCESS)) == true)
                    {
                        descriptor_set_layout_handles.push_back(descriptor_set_layout);
                    }
                    else
                    {
                        error_text << kStrVulkanErrorFailedToCreateDescriptorSetLayout << std::endl;
                        error_text << "vkCreateDescriptorSetLayout" << kStrVulkanInfoFunctionReturnedErrorCode << create_result << std::endl;
                        should_print_validation_hint_ = true;
                    }
                }
                else
                {
                    error_text << kStrVulkanErrorNullDescriptorSetLayourCreateInfo << std::endl;
                }
            }

            // Proceed with creating the pipeline layout from the compute pipeline create info.
            if (ret)
            {
                VkPipelineLayoutCreateInfo* pipeline_layout_create_info = compute_pipeline_create_info_->GetPipelineLayoutCreateInfo();
                assert(pipeline_layout_create_info != nullptr);
                if ((ret = (pipeline_layout_create_info != nullptr)) == true)
                {
                    // Assign the descriptor set layout into the pipeline layout create info.
                    pipeline_layout_create_info->pSetLayouts = descriptor_set_layout_handles.data();
                    pipeline_layout_create_info->setLayoutCount = static_cast<uint32_t>(descriptor_set_layout_handles.size());

                    // Attempt to create the pipeline layout from the given create info structure.
                    VkResult create_result = vkCreatePipelineLayout(device_, pipeline_layout_create_info, nullptr, &pipeline_layout);
                    assert(create_result == VK_SUCCESS);
                    if ((ret = (create_result == VK_SUCCESS)) == false)
                    {
                        error_text << kStrVulkanErrorFailedToCreatePipelineLayout << std::endl;
                        error_text << "vkCreatePipelineLayout" << kStrVulkanInfoFunctionReturnedErrorCode << create_result << std::endl;
                        should_print_validation_hint_ = true;
                    }
                }
            }

            // Proceed with creating the compute stage shader module,
            // and insert it into the pipeline create info.
            if (ret)
            {
                // Initialize the compute stage with the user's shader stage arguments.
                compute_pipeline_create_info->stage.pName = config_.entries[RgPipelineStage::kCompute].c_str();

                // Load the target SPIR-V shader into a shader module.
                VkShaderModule compute_stage_shader_module = VK_NULL_HANDLE;
                std::string error_msg;
                bool is_module_created = CreateShaderModule(config_.spv_files[RgPipelineStage::kCompute], compute_stage_shader_module, error_msg);
                assert(is_module_created);
                if ((ret = is_module_created) == true)
                {
                    // Insert the shader module handle into the pipeline create info.
                    compute_pipeline_create_info->stage.module = compute_stage_shader_module;
                }
                else
                {
                    ret = false;
                    error_text << error_msg;
                }

                // Insert the pipeline layout handle into the compute pipeline create info.
                VkComputePipelineCreateInfo* pipeline_create_info = compute_pipeline_create_info_->GetComputePipelineCreateInfo();
                assert(pipeline_create_info != nullptr);
                if ((ret = (pipeline_create_info != nullptr)) == true)
                {
                    pipeline_create_info->layout = pipeline_layout;
                }
                else
                {
                    error_text << kStrVulkanErrorNullPipelineCreateInfo << std::endl;
                }
            }

            // Attempt to create the compute pipeline.
            VkPipeline compute_pipeline = VK_NULL_HANDLE;
            VkComputePipelineCreateInfo* compute_pipeline_create_info_2 = compute_pipeline_create_info_->GetComputePipelineCreateInfo();
            assert(compute_pipeline_create_info_2 != nullptr);
            if ((ret = (compute_pipeline_create_info_2 != nullptr)) == true)
            {
                VkResult create_result = vkCreateComputePipelines(device_, VK_NULL_HANDLE, 1, compute_pipeline_create_info_2, nullptr, &compute_pipeline);
                assert(create_result == VK_SUCCESS);
                if ((ret = (create_result == VK_SUCCESS)) == false)
                {
                    error_text << kStrVulkanErrorFailedToCreateComputePipeline << std::endl;
                    error_text << "vkCreateComputePipelines" << kStrVulkanInfoFunctionReturnedErrorCode << create_result << std::endl;
                    error_text << STR_ERR_FAILED_CREATE_PIPELINE_HINT << std::endl;
                    should_print_validation_hint_ = true;
                    ret = false;
                }
            }

            assert(ret);
            if (ret)
            {
                pipeline_ = { kStrVulkanCreatedPipelineName, compute_pipeline };

                // Destroy the handles that were required to create the pipeline.
                for (VkDescriptorSetLayout descriptor_set_layout : descriptor_set_layout_handles)
                {
                    assert(descriptor_set_layout != VK_NULL_HANDLE);
                    if ((ret = (descriptor_set_layout != VK_NULL_HANDLE)) == true)
                    {
                        vkDestroyDescriptorSetLayout(device_, descriptor_set_layout, nullptr);
                    }
                    else
                    {
                        error_text << kStrVulkanErrorNullDescriptorSetLayour << std::endl;
                        break;
                    }
                }

                assert(pipeline_layout != VK_NULL_HANDLE);
                if (pipeline_layout != VK_NULL_HANDLE)
                {
                    vkDestroyPipelineLayout(device_, pipeline_layout, nullptr);
                }
            }
            else
            {
                std::cerr << error_text.str();
            }
        }
    }

    return ret;
}

bool RgVulkanBackend::CreateShaderModule(const std::string& spv_file_path, VkShaderModule& shader_module_handle, std::string& error_string)
{
    bool ret = false;

    // Try to read the SPIR-V shader code into an array of bytes.
    std::vector<char> shader_bytes;
    ret = ReadShaderBytes(spv_file_path, shader_bytes, error_string);

    assert(ret);
    if (ret)
    {
        // Create and initialize a shader module create info structure and add the SPIR-V code to it.
        VkShaderModuleCreateInfo module_create_info = {};
        module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        module_create_info.pCode = reinterpret_cast<const uint32_t*>(shader_bytes.data());
        module_create_info.codeSize = shader_bytes.size();

        // Attempt to create the shader module.
        VkResult create_result = vkCreateShaderModule(device_, &module_create_info, nullptr, &shader_module_handle);

        // Was the shader module created successfully?
        ret = (create_result == VK_SUCCESS);
        assert(ret);
        if (!ret)
        {
            std::stringstream error_stream;
            error_stream << kStrVulkanErrorFailedToCreateShaderModule << spv_file_path << std::endl;
            error_stream << "vkCreateShaderModule" << kStrVulkanInfoFunctionReturnedErrorCode << create_result << std::endl;
            error_string = error_stream.str();
            should_print_validation_hint_ = true;
        }
    }

    return ret;
}

bool RgVulkanBackend::LoadGraphicsPipelineStateFile(const std::string& pso_file_path)
{
    std::string error_string;
    bool is_loaded = RgPsoSerializerVulkan::ReadStructureFromFile(pso_file_path, &graphics_pipeline_create_info_, error_string);
    if (!is_loaded)
    {
        std::cerr << kStrVulkanErrorFailedToDeserializeGraphicsPipeline << std::endl;
        std::cerr << error_string << std::endl;
    }

    return is_loaded;
}

bool RgVulkanBackend::CreateGraphicsPipeline()
{
    bool ret = false;
    std::stringstream error_text;

    assert(graphics_pipeline_create_info_ != nullptr);
    if ((ret = (graphics_pipeline_create_info_ != nullptr)) == true)
    {
        // These handles need to be created from the deserialized CreateInfo structures,
        // and then assigned into the graphics pipeline create info before attempting to
        // create the compute pipeline.
        std::vector< VkDescriptorSetLayout> descriptor_set_layout_handles;

        VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
        VkRenderPass renderPass = VK_NULL_HANDLE;

        // Attempt to create the descriptor set layout.
        std::vector<VkDescriptorSetLayoutCreateInfo*> descriptor_set_layout_collection = graphics_pipeline_create_info_->GetDescriptorSetLayoutCreateInfo();

        for (VkDescriptorSetLayoutCreateInfo* descriptor_set_layout_create_info : descriptor_set_layout_collection)
        {
            VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
            assert(descriptor_set_layout_create_info != nullptr);
            if (descriptor_set_layout_create_info != nullptr)
            {
                // Set the structure type for the Descriptor Set Layout create info.
                descriptor_set_layout_create_info->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

                VkResult create_result = vkCreateDescriptorSetLayout(device_, descriptor_set_layout_create_info, nullptr, &descriptor_set_layout);
                if ((ret = (create_result == VK_SUCCESS)) == true)
                {
                    descriptor_set_layout_handles.push_back(descriptor_set_layout);
                }
                else
                {
                    error_text << kStrVulkanErrorFailedToCreateDescriptorSetLayout << std::endl;
                    error_text << "vkCreateDescriptorSetLayout" << kStrVulkanInfoFunctionReturnedErrorCode << create_result << std::endl;
                    should_print_validation_hint_ = true;
                    break;
                }
            }
        }

        // Proceed with creating the pipeline layout from the compute pipeline create info.
        if (ret)
        {
            VkPipelineLayoutCreateInfo* pipeline_layout_create_info = graphics_pipeline_create_info_->GetPipelineLayoutCreateInfo();
            assert(pipeline_layout_create_info != nullptr);
            if ((ret = (pipeline_layout_create_info != nullptr)) == true)
            {
                // Assign the descriptor set layout into the pipeline layout create info.
                pipeline_layout_create_info->pSetLayouts = descriptor_set_layout_handles.data();
                pipeline_layout_create_info->setLayoutCount = static_cast<uint32_t>(descriptor_set_layout_handles.size());

                // Attempt to create the pipeline layout from the given create info structure.
                VkResult create_result = vkCreatePipelineLayout(device_, pipeline_layout_create_info, nullptr, &pipeline_layout);
                assert(ret);
                if ((ret = (create_result == VK_SUCCESS)) == false)
                {
                    error_text << kStrVulkanErrorFailedToCreatePipelineLayout << std::endl;
                    error_text << "vkCreatePipelineLayout" << kStrVulkanInfoFunctionReturnedErrorCode << create_result << std::endl;
                    should_print_validation_hint_ = true;
                }
            }
        }

        // Proceed with creating the render pass.
        if (ret)
        {
            VkRenderPassCreateInfo* render_pass_create_info = graphics_pipeline_create_info_->GetRenderPassCreateInfo();
            assert(render_pass_create_info != nullptr);
            if ((ret = (render_pass_create_info != nullptr)) == true)
            {
                // Attempt to create the render pass from the given create info structure.
                VkResult create_result = vkCreateRenderPass(device_, render_pass_create_info, nullptr, &renderPass);
                assert(ret);
                if ((ret = (create_result == VK_SUCCESS)) == false)
                {
                    error_text << kStrVulkanErrorFailedToCreateRenderPass << std::endl;
                    error_text << "vkCreateRenderPass" << kStrVulkanInfoFunctionReturnedErrorCode << create_result << std::endl;
                    should_print_validation_hint_ = true;
                }
            }
        }

        // Creating each stage shader module and insert them into the pipeline create info.
        if (ret)
        {
            VkGraphicsPipelineCreateInfo* pipeline_create_info = graphics_pipeline_create_info_->GetGraphicsPipelineCreateInfo();
            assert(pipeline_create_info != nullptr);
            if ((ret = (pipeline_create_info != nullptr)) == true)
            {
                std::vector<VkPipelineShaderStageCreateInfo> stage_create_infos;
                char first_stage = static_cast<char>(RgPipelineStage::kVertex);
                char last_stage = static_cast<char>(RgPipelineStage::kFragment);
                for (int stage_index = first_stage; stage_index <= last_stage; ++stage_index)
                {
                    // If the stage has a SPIR-V file, add it to the vector of stage create info.
                    const std::string& stage_shader_file = config_.spv_files[stage_index];
                    if (!stage_shader_file.empty())
                    {
                        // Load the target SPIR-V shader into a shader module.
                        VkShaderModule stage_module = VK_NULL_HANDLE;
                        std::string error_msg;
                        bool is_module_created = CreateShaderModule(config_.spv_files[stage_index], stage_module, error_msg);

                        assert(is_module_created);
                        if ((ret = is_module_created) == true)
                        {
                            // Initialize a shader stage structure using the newly created module.
                            VkPipelineShaderStageCreateInfo stage_info = {};
                            stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

                            // Shift using the stage index to convert to a Vulkan stage flag.
                            stage_info.stage = static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_VERTEX_BIT << stage_index);

                            // Insert the shader module handle and entrypoint name into the stage info.
                            stage_info.module = stage_module;
                            stage_info.pName = config_.entries[stage_index].c_str();

                            // Add the stage info to the vector of stages in the PSO create info.
                            stage_create_infos.push_back(stage_info);
                        }
                        else
                        {
                            error_text << error_msg;
                        }
                    }
                }

                // Insert the stage array into the pipeline create info. This array of
                // stage create info contains the SPIR-V module handles created above.
                pipeline_create_info->pStages = stage_create_infos.data();
                pipeline_create_info->stageCount = static_cast<uint32_t>(stage_create_infos.size());

                // Insert the pipeline layout and render pass handles into the pipeline create info.
                pipeline_create_info->layout = pipeline_layout;
                pipeline_create_info->renderPass = renderPass;

                // Attempt to create the graphics pipeline.
                VkPipeline graphics_pipeline = VK_NULL_HANDLE;
                VkResult create_result = vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, pipeline_create_info, nullptr, &graphics_pipeline);
                assert(create_result == VK_SUCCESS);
                if ((ret = (create_result == VK_SUCCESS)) == false)
                {
                    error_text << kStrVulkanErrorFailedToCreateGraphicsPipeline << std::endl;
                    error_text << "vkCreateGraphicsPipelines" << kStrVulkanInfoFunctionReturnedErrorCode << create_result << std::endl;
                    error_text << STR_ERR_FAILED_CREATE_PIPELINE_HINT << std::endl;
                    should_print_validation_hint_ = true;
                }

                assert(ret);
                if (ret)
                {
                    pipeline_ = { kStrVulkanCreatedPipelineName, graphics_pipeline };

                    // Destroy the handles that were required to create the pipeline.
                    assert(renderPass != VK_NULL_HANDLE);
                    if (renderPass != VK_NULL_HANDLE)
                    {
                        vkDestroyRenderPass(device_, renderPass, nullptr);
                    }

                    for (VkDescriptorSetLayout descriptor_set_layout : descriptor_set_layout_handles)
                    {
                        if (descriptor_set_layout != VK_NULL_HANDLE)
                        {
                            vkDestroyDescriptorSetLayout(device_, descriptor_set_layout, nullptr);
                        }
                    }

                    assert(pipeline_layout != VK_NULL_HANDLE);
                    if (pipeline_layout != VK_NULL_HANDLE)
                    {
                        vkDestroyPipelineLayout(device_, pipeline_layout, nullptr);
                    }
                }
                else
                {
                    std::cerr << error_text.str() << std::endl;
                }
            }
        }
    }

    return ret;
}

bool RgVulkanBackend::ReadShaderBytes(const std::string& file_path, std::vector<char>& file_bytes, std::string& error_message)
{
    bool is_ok = false;

    std::ifstream file_stream(file_path, std::ios::ate | std::ios::binary);

    bool isFileOpened = file_stream.is_open();

    assert(isFileOpened);
    if (isFileOpened)
    {
        size_t fileSize = (size_t)file_stream.tellg();
        file_bytes.resize(fileSize);

        file_stream.seekg(0);
        file_stream.read(file_bytes.data(), fileSize);
        file_stream.close();

        is_ok = true;
    }
    else
    {
        error_message = kStrVulkanErrorFailedToReadShaderFile + file_path;
        assert(false);
    }

    return is_ok;
}

VkPhysicalDevice RgVulkanBackend::GetDeviceHandle(const std::string& device_name)
{
    VkPhysicalDevice device_handle = VK_NULL_HANDLE;

    assert(!gpus_.empty());
    if (!gpus_.empty())
    {
        // If device name is empty, return handle of the 1st supported device.
        // Otherwise, look for the device with matching name.
        if (device_name.empty())
        {
            device_handle = gpus_.cbegin()->second;
        }
        else
        {
            for (const auto& gpu : gpus_)
            {
                if (gpu.first == device_name)
                {
                    device_handle = gpu.second;
                    break;
                }
            }
        }

        if (device_handle == VK_NULL_HANDLE)
        {
            std::cerr << kStrVulkanErrorInvalidGpu << device_name << std::endl;
        }
    }
    else
    {
        std::cerr << kStrVulkanErrorNoDevicesAvailable << std::endl;
    }

    return device_handle;
}

bool RgVulkanBackend::BuildForDevice(const std::string& device_name)
{
    // Create a Vulkan instance.
    bool status = InitVulkan(true);
    assert(status);

    // Get the device info.
    status = status && EnumerateDevices();

    VkPhysicalDevice device_handle = GetDeviceHandle(device_name);

    if (device_handle != VK_NULL_HANDLE)
    {
        // Check if target GPU supports required extensions.
        status = status && VerifyDevice(device_handle);

        // Create a Vulkan device compatible with VK_AMD_shader_info (no queues needed)
        status = status && CreateDevice(device_handle);

        // Create a Vulkan pipeline.
        status = status && CreatePipeline();

        // Build the pipeline.
        status = status && BuildPipeline();
    }

    return status;
}

bool RgVulkanBackend::CreatePipeline()
{
    bool ret = false;
    bool should_abort = false;

    // Determine the type of pipeline being created by searching for SPIR-V files attached to each stage.
    bool has_compute_shader = !config_.spv_files[RgPipelineStage::kCompute].empty();

    bool has_graphics_shaders = (!config_.spv_files[RgPipelineStage::kVertex].empty() ||
        !config_.spv_files[RgPipelineStage::kTessellationControl].empty() ||
        !config_.spv_files[RgPipelineStage::kTessellationEvaluation].empty() ||
        !config_.spv_files[RgPipelineStage::kGeometry].empty() ||
        !config_.spv_files[RgPipelineStage::kFragment].empty());

    if (has_graphics_shaders && config_.spv_files[RgPipelineStage::kVertex].empty())
    {
        std::cerr << kStrVulkanWarningGraphicsPipelineWithoutVertexShader << std::endl;
    }

    if (!should_abort)
    {
        // If the user specified a direct path to a PSO file, deserialize it.
        if (!config_.pipeline_state_file.empty())
        {
            // Expect one of compute or graphic shader is specified (not both).
            if (has_compute_shader != has_graphics_shaders)
            {
                if (has_graphics_shaders)
                {
                    ret = LoadGraphicsPipelineStateFile(config_.pipeline_state_file);
                }
                else
                {
                    ret = LoadComputePipelineStateFile(config_.pipeline_state_file);
                }

                assert(ret && kStrVulkanErrorFailedToLoadPipelineFile);
                if (!ret)
                {
                    std::cerr << kStrVulkanErrorFailedToLoadPipelineFile << std::endl;
                }
            }
            else
            {
                assert(false && kStrVulkanErrorGraphicsOrComputeShaderExpected);
                std::cerr << kStrVulkanErrorGraphicsOrComputeShaderExpected << std::endl;
            }
        }
        else
        {
            // The user didn't provide a Pipeline State file, so create one using the pipeline factory.
            RgPsoFactoryVulkan pipelineFactory;

            if (has_compute_shader)
            {
                compute_pipeline_create_info_ = pipelineFactory.GetDefaultComputePsoCreateInfo();
                assert(compute_pipeline_create_info_ != nullptr);
                if (compute_pipeline_create_info_ == nullptr)
                {
                    std::cerr << kStrVulkanErrorFailedToCreateDefaultComputePipeline << std::endl;
                }
            }
            else if (has_graphics_shaders)
            {
                graphics_pipeline_create_info_ = pipelineFactory.GetDefaultGraphicsPsoCreateInfo();
                assert(graphics_pipeline_create_info_ != nullptr);
                if (graphics_pipeline_create_info_ == nullptr)
                {
                    std::cerr << kStrVulkanErrorFailedToCreateDefaultGraphicsPipeline << std::endl;
                }
            }
            else
            {
                // Can't determine the type of pipeline being created. This shouldn't happen.
                assert(false);
            }
        }

        // If the user provided a path to a compute shader, create a compute pipeline.
        if (has_compute_shader)
        {
            ret = CreateComputePipeline();
        }
        else if (has_graphics_shaders)
        {
            ret = CreateGraphicsPipeline();
        }
        else
        {
            // Couldn't correctly determine the pipeline type based on the provided shader files.
            // We shouldn't ever get here.
            assert(false);
            ret = false;
        }
    }

    return ret;
}

bool RgVulkanBackend::BuildPipeline() const
{
    bool status = true;

    if (!config_.binary_file.empty())
    {
        // Build the pipeline if it's present.
        if (!pipeline_.first.empty())
        {
            status = status && BuildToBinary(pipeline_.first, pipeline_.second);
        }
    }

    // "target_stages": a vector of tuples {stage_bits, out_isa_file_name, out_stats_file_name}.
    std::vector<std::tuple<VkShaderStageFlagBits, std::string, std::string>>  target_stages;

    if (!config_.spv_files[RgPipelineStage::kVertex].empty())
    {
        target_stages.push_back(std::tuple<VkShaderStageFlagBits, std::string, std::string>(
            VK_SHADER_STAGE_VERTEX_BIT,
            config_.isa_files[RgPipelineStage::kVertex],
            config_.stats_files[RgPipelineStage::kVertex]
            ));
    }
    if (!config_.spv_files[RgPipelineStage::kTessellationControl].empty())
    {
        target_stages.push_back(std::tuple<VkShaderStageFlagBits, std::string, std::string>(
            VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
            config_.isa_files[RgPipelineStage::kTessellationControl],
            config_.stats_files[RgPipelineStage::kTessellationControl]
            ));
    }
    if (!config_.spv_files[RgPipelineStage::kTessellationEvaluation].empty())
    {
        target_stages.push_back(std::tuple<VkShaderStageFlagBits, std::string, std::string>(
            VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
            config_.isa_files[RgPipelineStage::kTessellationEvaluation],
            config_.stats_files[RgPipelineStage::kTessellationEvaluation]
            ));
    }
    if (!config_.spv_files[RgPipelineStage::kGeometry].empty())
    {
        target_stages.push_back(std::tuple<VkShaderStageFlagBits, std::string, std::string>(
            VK_SHADER_STAGE_GEOMETRY_BIT,
            config_.isa_files[RgPipelineStage::kGeometry],
            config_.stats_files[RgPipelineStage::kGeometry]
            ));
    }
    if (!config_.spv_files[RgPipelineStage::kFragment].empty())
    {
        target_stages.push_back(std::tuple<VkShaderStageFlagBits, std::string, std::string>(
            VK_SHADER_STAGE_FRAGMENT_BIT,
            config_.isa_files[RgPipelineStage::kFragment],
            config_.stats_files[RgPipelineStage::kFragment]
            ));
    }
    if (!config_.spv_files[RgPipelineStage::kCompute].empty())
    {
        target_stages.push_back(std::tuple<VkShaderStageFlagBits, std::string, std::string>(
            VK_SHADER_STAGE_COMPUTE_BIT,
            config_.isa_files[RgPipelineStage::kCompute],
            config_.stats_files[RgPipelineStage::kCompute]
            ));
    }

    for (const auto& stage : target_stages)
    {
        if (!std::get<1>(stage).empty())
        {
            // Build ISA for the pipeline if it's present.
            if (!pipeline_.first.empty())
            {
                status = status && BuildToDisasm(std::get<1>(stage), pipeline_.second, std::get<0>(stage));
            }
        }

        if (!std::get<2>(stage).empty())
        {
            // Build stats for the pipeline if it's present.
            if (!pipeline_.first.empty())
            {
                status = status && BuildToStatistics(std::get<2>(stage), pipeline_.first, pipeline_.second, std::get<0>(stage));
            }
        }
    }

    // We're done with dumping data from the pipeline. Destroy the pipeline before exiting.
    VkPipeline pipelineHandle = pipeline_.second;
    if (pipelineHandle != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(device_, pipelineHandle, nullptr);
    }

    return status;
}

bool RgVulkanBackend::BuildToStatistics(const std::string& stats_filename, const std::string&,
    VkPipeline pipeline, VkShaderStageFlagBits stage) const
{
    VkShaderStatisticsInfoAMD stats = {};
    size_t size = sizeof(stats);

    assert(function_get_shader_info_ != nullptr);
    bool status = (function_get_shader_info_ != nullptr);

    status = status && (function_get_shader_info_(device_, pipeline, stage, VK_SHADER_INFO_TYPE_STATISTICS_AMD, &size, &stats) == VK_SUCCESS);
    assert(status && kStrVulkanErrorFailedToGetShaderStats);
    if (status)
    {
        std::stringstream statistics_stream;

        statistics_stream << "Statistics:" << std::endl;
        statistics_stream << "    - shaderStageMask                           = " << stats.shaderStageMask << std::endl;
        statistics_stream << "    - resourceUsage.numUsedVgprs                = " << stats.resourceUsage.numUsedVgprs << std::endl;
        statistics_stream << "    - resourceUsage.numUsedSgprs                = " << stats.resourceUsage.numUsedSgprs << std::endl;
        statistics_stream << "    - resourceUsage.ldsSizePerLocalWorkGroup    = " << stats.resourceUsage.ldsSizePerLocalWorkGroup << std::endl;
        statistics_stream << "    - resourceUsage.ldsUsageSizeInBytes         = " << stats.resourceUsage.ldsUsageSizeInBytes << std::endl;
        statistics_stream << "    - resourceUsage.scratchMemUsageInBytes      = " << stats.resourceUsage.scratchMemUsageInBytes << std::endl;
        statistics_stream << "    - numPhysicalVgprs                          = " << stats.numPhysicalVgprs << std::endl;
        statistics_stream << "    - numPhysicalSgprs                          = " << stats.numPhysicalSgprs << std::endl;
        statistics_stream << "    - numAvailableVgprs                         = " << stats.numAvailableVgprs << std::endl;
        statistics_stream << "    - numAvailableSgprs                         = " << stats.numAvailableSgprs << std::endl;

        if (stage == VK_SHADER_STAGE_COMPUTE_BIT)
        {
            for (int i = 0; i < 3; ++i)
            {
                statistics_stream << "    - computeWorkGroupSize" << i << " = " << stats.computeWorkGroupSize[i] << std::endl;
            }
        }

        status = WriteTextFile(stats_filename, statistics_stream.str());
    }

    if (!status)
    {
        std::cerr << kStrVulkanErrorPipelineStatsNotAvailable << std::endl;
    }

    return status;
}

bool RgVulkanBackend::BuildToBinary(const std::string&, VkPipeline pipeline) const
{
    size_t binary_size = 0;

    assert(function_get_shader_info_ != nullptr);
    bool status = (function_get_shader_info_ != nullptr);

    // Note: When retrieving the binary for an entire pipeline, the shader stage argument is
    // irrelevant, and is ignored by the extension. Despite this, the validation layer will still
    // complain about some specific stage arguments being invalid.
    // Therefore, the "VK_SHADER_STAGE_VERTEX_BIT" argument is provided here to avoid emitting a
    // validation layer warning, and the resulting binary will include all relevant pipeline stages.
    VkShaderStageFlagBits stage_flag_bits = VK_SHADER_STAGE_VERTEX_BIT;

    // Query the size of the binary file being dumped.
    status = status && (function_get_shader_info_(device_, pipeline, stage_flag_bits,
        VK_SHADER_INFO_TYPE_BINARY_AMD, &binary_size, nullptr) == VK_SUCCESS);
    assert(status && kStrVulkanErrorFailedToGetBinarySize);

    if (status && binary_size > 0)
    {
        // Allocate storage for the binary, fill it with the binary bytes, and write to disk.
        uint8_t* binary = new uint8_t[binary_size];

        if (binary && function_get_shader_info_(device_, pipeline, stage_flag_bits,
            VK_SHADER_INFO_TYPE_BINARY_AMD, &binary_size, binary) == VK_SUCCESS)
        {
            status = WriteBinaryFile(config_.binary_file, std::vector<char>(binary, binary + binary_size));
        }

        delete[] binary;
    }

    if (!status)
    {
        std::cerr << kStrVulkanErrorBinaryNotAvailable << std::endl;
    }

    return status;
}

bool RgVulkanBackend::BuildToDisasm(const std::string& isa_filename, VkPipeline pipeline, VkShaderStageFlagBits stage) const
{
    size_t disassembly_size = 0;

    assert(function_get_shader_info_ != nullptr);
    bool status = (function_get_shader_info_ != nullptr);

    status = status && (function_get_shader_info_(device_, pipeline, stage, VK_SHADER_INFO_TYPE_DISASSEMBLY_AMD,
        &disassembly_size, nullptr) == VK_SUCCESS);
    assert(status && disassembly_size > 0);
    if (status && disassembly_size > 0)
    {
        char* disassembly = new char[disassembly_size];

        if (disassembly && function_get_shader_info_(device_, pipeline, stage,
            VK_SHADER_INFO_TYPE_DISASSEMBLY_AMD, &disassembly_size, disassembly) == VK_SUCCESS)
        {
            status = WriteTextFile(isa_filename, disassembly);
        }

        delete[] disassembly;
    }

    if (!status)
    {
        std::cerr << kStrVulkanErrorDisassemblyNotAvailable << std::endl;
    }

    return status;
}

