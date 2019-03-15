// C++.
#include <cassert>
#include <string>
#include <string.h>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>
#include <algorithm>

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
#include <Utils/Vulkan/Include/rgPipelineTypes.h>
#include <Utils/Vulkan/Include/rgPsoFactoryVulkan.h>
#include <Utils/Vulkan/Include/rgPsoSerializerVulkan.h>
#include <Utils/Include/rgaCliDefs.h>

// Vulkan driver major version.
static const char* STR_VUKAN_DRIVER_VERSION_MAJOR = "2.0";

// Bit mask for Vulkan minor driver version (22 bits).
static const uint32_t  VULKAN_DRIVER_VERSION_MINOR_BITS = 0x3FFFFF;

// Offsets/widths of supported Vulkan version bit fields.
static const uint32_t  VULKAN_VERSION_PATCH_WIDTH = 12;
static const uint32_t  VULKAN_VERSION_MINOR_WIDTH = 10;
static const uint32_t  VULKAN_VERSION_MAJOR_WIDTH = 10;

static const uint32_t  VULKAN_VERSION_PATCH_OFFSET = 0;
static const uint32_t  VULKAN_VERSION_MINOR_OFFSET = VULKAN_VERSION_PATCH_OFFSET + VULKAN_VERSION_PATCH_WIDTH;
static const uint32_t  VULKAN_VERSION_MAJOR_OFFSET = VULKAN_VERSION_MINOR_OFFSET + VULKAN_VERSION_MINOR_WIDTH;

// Vulkan API function that gets shader info.
static const char*  STR_FUNC_GET_SHADER_INFO = "vkGetShaderInfoAMD";

// Name of default pipeline.
static const char* STR_CREATED_PIPELINE_NAME = "pipeline";

// Name of RGA Vulkan Backend for vulkan application info.
static const char* STR_VULKAN_APP_INFO_APP_NAME = "RGAVulkanBackend";

// GPU ID string.
static const char* STR_GPU_ID = "GPU ID ";

// Tokens used to check for Radeon adapters.
static const char* STR_RADEON_ADAPTER_TOKEN_A = "radeon";
static const char* STR_RADEON_ADAPTER_TOKEN_B = "amd";

// Output file suffixes corresponding to the pipeline stages.
static const char* STR_OUTPUT_SUFFIX_COMP = "comp";
static const char* STR_OUTPUT_SUFFIX_VERT = "vert";
static const char* STR_OUTPUT_SUFFIX_FRAG = "frag";
static const char* STR_OUTPUT_SUFFIX_TESE = "tese";
static const char* STR_OUTPUT_SUFFIX_TESC = "tesc";
static const char* STR_OUTPUT_SUFFIX_GEOM = "geom";

// Default Vulkan entry point name.
static const char* STR_DEFAULT_VULKAN_ENTRY_NAME = "main";

// Paths to default pipeline object files.
static const char* STR_FIRST_SERIALIZE_COMPUTE_PSO_FILE_NAME  = "computePso1.pso";
static const char* STR_SECOND_SERIALIZE_COMPUTE_PSO_FILE_NAME = "computePso2.pso";
static const char* STR_FIRST_SERIALIZE_GRAPHIC_PSO_FILE_NAME  = "graphicsPso1.pso";
static const char* STR_SECOND_SERIALIZE_GRAPHIC_PSO_FILE_NAME = "graphicsPso2.pso";

// Environment variable to dump all supported GPUs.
static const std::string STR_ENV_VAR_ALL_GPUS_NAME  = "AMDVLK_NULL_GPU";
static const std::string STR_ENV_VAR_ALL_GPUS_VALUE = "ALL";

// Environment variable to enable the RGA driver stack on machines with APUs.
static const std::string STR_ENV_VAR_DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_NAME = "DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1";
static const std::string STR_ENV_VAR_DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_VALUE = "1";

// Error messages.
static const char* STR_ERR_FAILED_GET_TEMP_DIR                  = "Error: failed to get temporary directory.";
static const char* STR_ERR_FAILED_GET_VK_GET_SHADER_INFO_ADDR   = "Error: failed to get address of vkGetShaderInfoAMD.";
static const char* STR_ERR_FAILED_ENUMERATE_DEVICES             = "Error: failed to enumerate Vulkan devices.";
static const char* STR_ERR_NO_DEVICES_AVAILABLE                 = "Error: no supported target GPUs available.";
static const char* STR_ERR_FAILED_GET_DEVICE_PROPS              = "Error: failed to get the device properties.";
static const char* STR_ERR_FAILED_CREATE_VULKAN_INSTANCE        = "Error: failed to create Vulkan instance.";
static const char* STR_ERR_FAILED_CREATE_LOGICAL_DEVICE         = "Error: failed to create logical Vulkan device";
static const char* STR_ERR_FAILED_CREATE_LOGICAL_DEVICE_FOR_GPU = "Error: failed to create logical Vulkan device for selected GPU.";
static const char* STR_ERR_FAILED_CREATE_PIPELINE_LAYOUT        = "Error: failed to create Vulkan pipeline layout.";
static const char* STR_ERR_FAILED_CREATE_DESCRIPTOR_SET_LAYOUT  = "Error: failed to create Vulkan descriptor set layout.";
static const char* STR_ERR_FAILED_CREATE_RENDER_PASS            = "Error: failed to create Vulkan render pass.";
static const char* STR_ERR_FAILED_CREATE_SHADER_MODULE          = "Error: failed to create Vulkan shader module from SPIR-V file ";
static const char* STR_ERR_FAILED_CREATE_CALLBACK               = "Error: failed to create Vulkan callback object.";
static const char* STR_ERR_FAILED_FIND_COMPATIBLE_DEVICES       = "Error: failed to find any compatible Vulkan physical devices that "
                                                                  "support the VK_AMD_shader_info device extension.";
static const char* STR_ERR_FAILED_GET_DEVICE_EXT_PROPS          = "Error: failed to get Device extension properties.";
static const char* STR_ERR_FAILED_GET_EXT_PROPS                 = "Error: failed to get Vulkan extension properties.";
static const char* STR_ERR_FAILED_GET_DEVICE_QUEUE_FAMILY_PROPS = "Error: failed to get device queue family properties.";
static const char* STR_ERR_DEVICE_DOES_NOT_SUPPORT_EXTENSION    = "Error: device does not support required extension.";
static const char* STR_ERR_FAILED_GET_SHADER_STATS              = "Error: failed to get shader statistics.";
static const char* STR_ERR_PIPELINE_STATS_NOT_AVAILABLE         = "Error: pipeline statistics information not available.";
static const char* STR_ERR_FAILED_GET_BINARY_SIZE               = "Error: failed to get binary size.";
static const char* STR_ERR_PIPELINE_BINARY_NOT_AVAILABLE        = "Error: pipeline binary not available.";
static const char* STR_ERR_DISASM_NOT_AVAILABLE                 = "Error: disassembly not available.";
static const char* STR_ERR_INVALID_GPU                          = "Error: invalid GPU target: ";
static const char* STR_ERR_UNABLE_OPEN_FILE_WRITE_1             = "Error: unable to open ";
static const char* STR_ERR_UNABLE_OPEN_FILE_WRITE_2             = " for write.";
static const char* STR_ERR_GRAPH_OR_COMP_SHADERS_EXPECTED       = "Error: a compute or graphics shader file is expected.";
static const char* STR_ERR_FAILED_LOAD_PIPELINE_FILE            = "Error: failed to load the pipeline file.";
static const char* STR_ERR_FAILED_SET_ENV_VAR                   = "Error: failed to set environment variable.";
static const char* STR_ERR_FAILED_REMOVE_ENV_VAR                = "Error: failed to remove environment variable.";
static const char* STR_ERR_FAILED_DESERIALIZE_COMP_PIPELINE     = "Error: failed to deserialize compute pipeline.";
static const char* STR_ERR_FAILED_DESERIALIZE_GRAPH_PIPELINE    = "Error: failed to deserialize graphics pipeline.";
static const char* STR_ERR_FAILED_CREATE_COMP_PIPELINE          = "Error: failed to create compute pipeline.";
static const char* STR_ERR_FAILED_CREATE_GRAPH_PIPELINE         = "Error: failed to create graphics pipeline.";
static const char* STR_ERR_FAILED_CREATE_PIPELINE_HINT          = "Make sure that the provided pipeline state matches the given shaders.";
static const char* STR_ERR_FAILED_CREATE_DEFAULT_COMP_PIPELINE  = "Error: failed to create default compute pipeline.";
static const char* STR_ERR_FAILED_CREATE_DEFAULT_GRAPH_PIPELINE = "Error: failed to create default graphics pipeline.";
static const char* STR_ERR_FAILED_VERIFY_COMP_PIPELINE          = "Error: failed to verify the compute pipeline serialization.";
static const char* STR_ERR_FAILED_VERIFY_GRAPH_PIPELINE         = "Error: failed to verify the graphics pipeline serialization.";
static const char* STR_ERR_NULL_VULKAN_INSTANCE                 = "Error: trying to setup validation callback for empty Vulkan instance.";
static const char* STR_ERR_FAILED_TO_READ_SHADER_FILE           = "Error: failed to read shader file at ";
static const char* STR_ERR_COULD_NOT_LOCATE_VULKAN_LOADER       = "Error: could not locate the Vulkan loader";
static const char* STR_ERR_NULL_VK_HANDLE                       = "Error: unexpected NULL Vulkan instance handle.";
static const char* STR_ERR_NULL_PPLN_CREATE_INFO                = "Error: unexpected NULL pipeline create info.";
static const char* STR_ERR_NULL_DESC_SET_LAYOUT                 = "Error: unexpected NULL descriptor set layout.";
static const char* STR_ERR_NULL_DESC_SET_LAYOUT_CREATE_INFO     = "Error: unexpected NULL descriptor set layout create info.";
static const char* STR_ERR_GRAPHICS_PIPELINE_WITHOUT_VERTEX     = "Error: graphics pipeline must contain a vertex shader.";

// Warnings
static const char* STR_WRN_FAILED_CHECK_VALIDATION_LAYERS       = "Warning: Some required Vulkan validation layers are not available.";
static const char* STR_WRN_FAILED_CHECK_EXTENSIONS              = "Warning: Some required Vulkan extensions are not available.";
static const char* STR_WRN_FAILED_GET_VULKAN_EXT_FUNC_PTR       = "Warning: failed to obtain pointer to required Vulkan extension function.";
static const char* STR_WRN_VALIDATION_LAYER_NOT_SUPPORTED       = "Warning: validation layer not supported by runtime: ";

// Extensions and validation layers.
static const char* STR_EXTENSION_NOT_SUPPORTED                  = "Vulkan extension is not supported: ";
static const char* STR_VALIDATION_INFO_UNAVAILABLE              = "Vulkan validation information may not be available or complete.";
static const char* STR_VALIDATION_ENABLING                      = "Enabling Vulkan validation layers... ";
static const char* STR_LAYER_OUTPUT_A                           = "*** Output from ";
static const char* STR_LAYER_OUTPUT_BEGIN                       = " layer - begin *** ";
static const char* STR_LAYER_OUTPUT_END                         = " layer - end *** ";
static const char* STR_LAYER_HINT                               = "To get more detailed information about this failure, try enabling Vulkan validation layers.";

// Notifications.
static const char* STR_USING_CUSTOM_ICD_LOCATION                = "Using Vulkan ICD from custom location: ";
static const char* STR_FUNCTION_RETURNED_ERR_CODE               = " function returned error code: ";
static const char* STR_FUNCTION_RETURNED_EMPTY_LIST             = " function returned empty list.";

// Warnings.
static const char* STR_USING_OFFLINE_AMDVLK_LOCATION_A          = "Warning: failed to locate AMD's Vulkan driver on the system: ";
static const char* STR_USING_OFFLINE_AMDVLK_LOCATION_B          = ". Falling back to using the amdvlk binary that is packaged with RGA.";

// Required Vulkan validation layers.
static const std::vector<const char*> VULKAN_VALIDATION_LAYERS =
{
    "VK_LAYER_LUNARG_standard_validation",
    "VK_LAYER_LUNARG_core_validation",
    "VK_LAYER_LUNARG_parameter_validation"
};

// Required Vulkan extensions.
static const std::vector<const char*> VULKAN_EXTENSIONS =
{
    "VK_EXT_debug_report"
};

// Validation messages to be filtered.
// RGA's Vulkan driver stack may trigger these validation messages although there is no actual issue.
// In the normal use case, the API is pretty much useless without queues because they cannot submit any
// work to the device. However, queues are not necessary solely for compilation and they are not even exposed
// by RGA's driver stack.
static const char* STR_VALIDATION_LAYER_MSG_FILTER_0 = "pCreateInfo->queueCreateInfoCount";
static const char* STR_VALIDATION_LAYER_MSG_FILTER_1 = "[ VUID_Undefined ] Object: VK_NULL_HANDLE (Type = 0) | vkCreateDevice: parameter pCreateInfo->queueCreateInfoCount must be greater than 0.";
static const char* STR_VALIDATION_LAYER_MSG_FILTER_2 = "Object: 0x0 | vkCreateDevice : parameter pCreateInfo->queueCreateInfoCount must be greater than 0. (null)";

// *** AUXILIARY FUNCTIONS - BEGIN ***

// Returns the name of the current executable.
static std::string GetVulkanBackendExeName()
{
    std::string ret;
#ifdef _WIN32
    ret = "VulkanBackend.exe";
#else
    ret = "VulkanBackend";
#endif // _WIN32
    return ret;
}

// Returns true if substr is a substring of src, false otherwise, while ignoring the case.
static bool IsSubstringIgnoreCase(const std::string& src, const std::string& substr)
{
    std::string s1_u = src, s2_u = substr;
    std::transform(s1_u.begin(), s1_u.end(), s1_u.begin(), [](unsigned char c) {return std::toupper(c); });
    std::transform(s2_u.begin(), s2_u.end(), s2_u.begin(), [](unsigned char c) {return std::toupper(c); });
    return (s1_u.find(s2_u) != std::string::npos);
}

// Returns true if the given file name represents a file that exists
// and can be opened by the current process. Returns false otherwise.
static bool IsFileExists(const std::string& fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

// Returns the path for the current executable ("VulkanBackend").
static std::string GetCurrentExecutablePath()
{
    std::string ret;
#ifdef _WIN32
    char fileName[MAX_PATH];
    DWORD rc = GetModuleFileNameA(NULL, fileName, MAX_PATH);
    if (rc > 0)
    {
        // Extract the directory from the full path.
        ret = std::string(fileName, fileName + rc);
        size_t exeNameStart = ret.find(GetVulkanBackendExeName());
        if (exeNameStart != std::string::npos)
        {
            ret = ret.substr(0, exeNameStart);
        }
    }
#elif __linux
    // Get the directory name with the executable name.
    char pathWithExeName[PATH_MAX];
    ssize_t rc = readlink("/proc/self/exe", pathWithExeName, PATH_MAX);
    if (rc != -1)
    {
        // Extract the directory name (without the executable name).
        const char *pPath = dirname(pathWithExeName);
        if (pPath != nullptr)
        {
            ret = pPath;

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
    std::string amdvlkFileName;
#ifdef _WIN32
    amdvlkFileName = "amdvlk64.dll";
#else
    amdvlkFileName = "amdvlk64.so";
#endif // _WIN32
    return amdvlkFileName;
}

// Returns the path for the amdvlk .dll or .so file that is packaged with RGA.
static std::string GetOfflineAmdvlkPath()
{
    // Get the path for the current executable (VulkanBackend).
    std::stringstream amdvlkOfflineFile;
    amdvlkOfflineFile << GetCurrentExecutablePath();

    // Append the sub-directory where the offline amdvlk binary resides.
    const char* AMDVLK_OFFLINE_FOLDER = "amdvlk";

    // Append the offline amdvlk library name.
    amdvlkOfflineFile << AMDVLK_OFFLINE_FOLDER << "/" << GetAmdvlkFileName();
    return amdvlkOfflineFile.str();
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
                m_name = name;
            }
            else
            {
                std::cerr << STR_ERR_FAILED_SET_ENV_VAR << std::endl;
            }
        }
    }

    ~EnvVar()
    {
        if (!m_name.empty())
        {
#ifdef WIN32
            bool result = (::SetEnvironmentVariable(std::wstring(m_name.cbegin(), m_name.cend()).c_str(), nullptr) == TRUE);
#else
            bool result = (::unsetenv(m_name.c_str()) == 0);
#endif
            assert(result);
            if (!result)
            {
                std::cerr << STR_ERR_FAILED_REMOVE_ENV_VAR << std::endl;
            }
        }
    }

private:
    std::string  m_name;
};

// Returns path to OS temp dir (adds trailing slash).
static std::string GetTempDir()
{
    std::string ret;

#ifdef WIN32
    const int tmpDirLen = MAX_PATH;
    wchar_t   tmpDirW[tmpDirLen];
    if (GetTempPath(tmpDirLen, tmpDirW) != 0)
    {
        std::wstring  tmpDirWStr(tmpDirW);
        std::string   tmpDirStr(tmpDirWStr.begin(), tmpDirWStr.end());
        ret = tmpDirStr;
        if (!ret.empty())
        {
            ret += "\\";
        }
    }
#else
    std::vector<std::string>  tmpDirEnvVars = { "TMPDIR", "TMP", "TEMP", "TEMPDIR" };
    for (const std::string tmpDirEnvVar : tmpDirEnvVars)
    {
        const char* tmpDir = std::getenv(tmpDirEnvVar.c_str());
        if (tmpDir != nullptr)
        {
            ret = tmpDir;
            break;
        }
    }
    if (!ret.empty())
    {
        ret += "/";
    }
#endif
    return ret;
}

// Builds a file name by appending a suffix to the base file name specified by "fileName".
// The suffix is chosen based on the Vulkan stage specified by "stage".
// If the base file name has an extension, the suffix is added before the extension.
// Examples:
//     "out_isa.txt" + VK_SHADER_STAGE_VERTEX_BIT  -->  "out_isa_vertex.txt"
//     "stats" + VK_SHADER_STAGE_FRAGMENT_BIT  -->  "stats_fragment"
//
static void BuildFileName(std::string& fileName, VkShaderStageFlagBits stage)
{
    size_t extOffset = fileName.rfind('.');
    std::string  suffix;
    switch (stage)
    {
        case VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT:                 suffix = STR_OUTPUT_SUFFIX_COMP; break;
        case VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT:                suffix = STR_OUTPUT_SUFFIX_FRAG; break;
        case VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT:                suffix = STR_OUTPUT_SUFFIX_GEOM; break;
        case VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:    suffix = STR_OUTPUT_SUFFIX_TESC; break;
        case VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: suffix = STR_OUTPUT_SUFFIX_TESE; break;
        case VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT:                  suffix = STR_OUTPUT_SUFFIX_VERT; break;
    }

    assert(!suffix.empty());

    fileName.insert((extOffset == std::string::npos ? fileName.size() : extOffset), "_" + suffix);
}

// Decode the 32-bit Vulkan version integer. Returns triple {major, minor, patch}.
static std::tuple<uint32_t, uint32_t, uint32_t>
DecodeVulkanVersion(uint32_t version)
{
    const uint32_t majorMask = (1 << VULKAN_VERSION_MAJOR_WIDTH) - 1;
    const uint32_t minorMask = (1 << VULKAN_VERSION_MINOR_WIDTH) - 1;
    const uint32_t patchMask = (1 << VULKAN_VERSION_PATCH_WIDTH) - 1;

    const uint32_t major = (version >> VULKAN_VERSION_MAJOR_OFFSET) & majorMask;
    const uint32_t minor = (version >> VULKAN_VERSION_MINOR_OFFSET) & minorMask;
    const uint32_t patch = (version >> VULKAN_VERSION_PATCH_OFFSET) & patchMask;

    return std::tuple<uint32_t, uint32_t, uint32_t>(major, minor, patch);
}

static std::vector<const char*> GetAvailableValidationLayers()
{
    uint32_t layersNum = 0;
    std::vector<const char*> supportedLayers;

    // Get the validation layers supported by Vulkan driver installed on the machine.
    if (vkEnumerateInstanceLayerProperties(&layersNum, nullptr) == VK_SUCCESS)
    {
        std::vector<VkLayerProperties> layerProps(layersNum);
        if (vkEnumerateInstanceLayerProperties(&layersNum, layerProps.data()) == VK_SUCCESS)
        {
            // Check if required layers are present in the list of supported layers.
            std::set<std::string> layerNames;
            std::for_each(layerProps.cbegin(), layerProps.cend(),
                [&](const VkLayerProperties& p) { layerNames.insert(p.layerName); });

            for (const char* pValidationLayer : VULKAN_VALIDATION_LAYERS)
            {
                // If the required layer is supported, add it to the "supportedLayers" list.
                // Otherwise, print a warning.
                if (layerNames.find(pValidationLayer) != layerNames.cend())
                {
                    supportedLayers.push_back(pValidationLayer);
                }
                else
                {
                    // Inform the user that the validation layer is not available by the runtime.
                    std::cerr << STR_WRN_VALIDATION_LAYER_NOT_SUPPORTED << pValidationLayer << std::endl;
                }
            }
        }
    }

    return supportedLayers;
}

rgConfig::Action rgVulkanBackend::ParseCmdLine(int argc, char* argv[])
{
    bool listTargets = false, listAdapters = false;

    assert(argv != nullptr);
    if (argv != nullptr)
    {
        try
        {
            cxxopts::Options opts(argv[0]);
            opts.add_options()
            ("h,help", "Print help.")
            ("list-targets",   "List all supported target GPUs.", cxxopts::value<bool>(listTargets))
            ("list-adapters",  "List physical display adapters installed on the system.", cxxopts::value<bool>(listAdapters))
            ("target",         "The name of the target GPU.", cxxopts::value<std::string>(m_config.m_targetGPU))
            ("enable-layers",  "Enable Vulkan validation layers.", cxxopts::value<bool>(m_config.m_enableLayers))
            ("layers-file",    "Path to the validation layers output file.", cxxopts::value<std::string>(m_config.m_validationFile))

            // Pipeline Object file.
            ("pso", "Pipeline state file which would be used to set the pipeline state for the compilation process."
                    "If not specified, RGA would create and set a \"default\" pipeline state.",
                    cxxopts::value<std::string>(m_config.m_pipelineStateFile))

            // ICD file path.
            ("icd", CLI_OPT_ICD_DESCRIPTION, cxxopts::value<std::string>(m_config.m_icdPath))

            // VK_LOADER_DEBUG environment variable.
            ("loader-debug", "Value for VK_LOADER_DEBUG", cxxopts::value<std::string>(m_config.m_loaderDebug))

            // Per-stage shader input source file options.
            ("vert", "Path to the source file which contains the vertex shader to be attached to the pipeline.",
                        cxxopts::value<std::string>(m_config.m_spvFiles[rgPipelineStage::Vertex]))
            ("tesc", "Path to the source file which contains the tessellation control shader to be attached to the pipeline.",
                        cxxopts::value<std::string>(m_config.m_spvFiles[rgPipelineStage::TessellationControl]))
            ("tese", "Path to the source file which contains the tessellation evaluation shader to be attached to the pipeline.",
                        cxxopts::value<std::string>(m_config.m_spvFiles[rgPipelineStage::TessellationEvaluation]))
            ("geom", "Path to the source file which contains the geometry shader to be attached to the pipeline.",
                        cxxopts::value<std::string>(m_config.m_spvFiles[rgPipelineStage::Geometry]))
            ("frag", "Path to the source file which contains the fragment shader to be attached to the pipeline.",
                        cxxopts::value<std::string>(m_config.m_spvFiles[rgPipelineStage::Fragment]))
            ("comp", "Path to the source file which contains the compute shader to be attached to the pipeline.",
                        cxxopts::value<std::string>(m_config.m_spvFiles[rgPipelineStage::Compute]))

            // Entry points.
            ("vert-entry", "Vertex shader entry point",
                           cxxopts::value<std::string>(m_config.m_entries[rgPipelineStage::Vertex])->default_value(STR_DEFAULT_VULKAN_ENTRY_NAME))
            ("tesc-entry", "Tesselation control shader entry point",
                           cxxopts::value<std::string>(m_config.m_entries[rgPipelineStage::TessellationControl])->default_value(STR_DEFAULT_VULKAN_ENTRY_NAME))
            ("tese-entry", "Tesselation evaluation shader entry point",
                           cxxopts::value<std::string>(m_config.m_entries[rgPipelineStage::TessellationEvaluation])->default_value(STR_DEFAULT_VULKAN_ENTRY_NAME))
            ("geom-entry", "Geometry shader entry point",
                           cxxopts::value<std::string>(m_config.m_entries[rgPipelineStage::Geometry])->default_value(STR_DEFAULT_VULKAN_ENTRY_NAME))
            ("frag-entry", "Fragment shader entry point",
                           cxxopts::value<std::string>(m_config.m_entries[rgPipelineStage::Fragment])->default_value(STR_DEFAULT_VULKAN_ENTRY_NAME))
            ("comp-entry", "Compute shader entry point",
                           cxxopts::value<std::string>(m_config.m_entries[rgPipelineStage::Compute])->default_value(STR_DEFAULT_VULKAN_ENTRY_NAME))

            // Output binary file.
            ("bin",   "Path to the output pipeline binary file.", cxxopts::value<std::string>(m_config.m_binaryFile))

            ("vert-isa", "Path to the output file which contains the disassembly for the vertex shader.",
                         cxxopts::value<std::string>(m_config.m_isaFiles[rgPipelineStage::Vertex]))
            ("tesc-isa", "Path to the output file which contains the disassembly for the tessellation control shader.",
                         cxxopts::value<std::string>(m_config.m_isaFiles[rgPipelineStage::TessellationControl]))
            ("tese-isa", "Path to the output file which contains the disassembly for the tessellation evaluation shader.",
                         cxxopts::value<std::string>(m_config.m_isaFiles[rgPipelineStage::TessellationEvaluation]))
            ("geom-isa", "Path to the output file which contains the disassembly for the geometry shader.",
                         cxxopts::value<std::string>(m_config.m_isaFiles[rgPipelineStage::Geometry]))
            ("frag-isa", "Path to the output file which contains the disassembly for the fragment shader.",
                         cxxopts::value<std::string>(m_config.m_isaFiles[rgPipelineStage::Fragment]))
            ("comp-isa", "Path to the output file which contains the disassembly for the compute shader.",
                         cxxopts::value<std::string>(m_config.m_isaFiles[rgPipelineStage::Compute]))

            ("vert-stats", "Path to the output file which contains the statistics for the vertex shader.",
                           cxxopts::value<std::string>(m_config.m_statsFiles[rgPipelineStage::Vertex]))
            ("tesc-stats", "Path to the output file which contains the statistics for the tessellation control shader.",
                           cxxopts::value<std::string>(m_config.m_statsFiles[rgPipelineStage::TessellationControl]))
            ("tese-stats", "Path to the output file which contains the statistics for the tessellation evaluation shader.",
                           cxxopts::value<std::string>(m_config.m_statsFiles[rgPipelineStage::TessellationEvaluation]))
            ("geom-stats", "Path to the output file which contains the statistics for the geometry shader.",
                           cxxopts::value<std::string>(m_config.m_statsFiles[rgPipelineStage::Geometry]))
            ("frag-stats", "Path to the output file which contains the statistics for the fragment shader.",
                           cxxopts::value<std::string>(m_config.m_statsFiles[rgPipelineStage::Fragment]))
            ("comp-stats", "Path to the output file which contains the statistics for the compute shader.",
                           cxxopts::value<std::string>(m_config.m_statsFiles[rgPipelineStage::Compute]))
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
                m_config.m_action = (listTargets ? rgConfig::Action::ListTargets :
                                    (listAdapters ? rgConfig::Action::ListAdapters : rgConfig::Action::Build));
            }
        }
        catch (const cxxopts::OptionException& e)
        {
            std::cerr << "Error parsing options: " << e.what() << std::endl;
        }
    }

    return m_config.m_action;
}

static bool WriteBinaryFile(const std::string& fileName, const std::vector<char>& content)
{
    bool ret = false;
    std::ofstream output;
    output.open(fileName.c_str(), std::ios::binary);

    if (output.is_open() && !content.empty())
    {
        output.write(&content[0], content.size());
        output.close();
        ret = true;
    }
    else
    {
        std::cerr << STR_ERR_UNABLE_OPEN_FILE_WRITE_1 << fileName << STR_ERR_UNABLE_OPEN_FILE_WRITE_2 << std::endl;
    }

    return ret;
}

static bool WriteTextFile(const std::string& fileName, const std::string& content)
{
    bool ret = false;
    std::ofstream output;
    output.open(fileName.c_str());

    if (output.is_open())
    {
        output << content << std::endl;
        output.close();
        ret = true;
    }
    else
    {
        std::cerr << STR_ERR_UNABLE_OPEN_FILE_WRITE_1 << fileName << STR_ERR_UNABLE_OPEN_FILE_WRITE_2 << std::endl;
    }

    return ret;
}

int main(int argc, char* argv[])
{
    bool status = false;
    bool isAmdGpu = false;

    rgVulkanBackend  vulkanBackend;
    rgConfig::Action requiredAction = vulkanBackend.ParseCmdLine(argc, argv);
    switch (requiredAction)
    {
        case rgConfig::Action::ListTargets:
            status = vulkanBackend.ListTargets();
            break;

        case rgConfig::Action::ListAdapters:
            status = vulkanBackend.ListAdapters(isAmdGpu, true);
            break;

        case rgConfig::Action::Build:
            status = vulkanBackend.Build();
            break;
    }

    vulkanBackend.PrintValidationHint();

    return (status ? 0 : 1);
}

void rgVulkanBackend::DestroyValidationCallback()
{
    // If the validation callback was installed, destroy it.
    if (m_debugCallback != nullptr)
    {
        auto vkDestroyCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugReportCallbackEXT");
        if (vkDestroyCallback != nullptr)
        {
            vkDestroyCallback(m_instance, m_debugCallback, nullptr);
        }
    }
}

bool rgVulkanBackend::InitVulkan(bool shouldEnableValidation)
{
    bool result = false;

    // Locate the Vulkan loader (or explicitly locate the ICD if requested by the user).
    const char* pIcdPath = nullptr;
    if (!m_config.m_icdPath.empty())
    {
        pIcdPath = m_config.m_icdPath.c_str();

        // In case that this is a build session, notify the user that we are using a Vulkan ICD from a custom location.
        if (shouldEnableValidation)
        {
            std::cout << STR_USING_CUSTOM_ICD_LOCATION << pIcdPath << std::endl;
        }
    }

    // If required, set the "VK_LOADER_DEBUG" environment variable.
    if (!m_config.m_loaderDebug.empty())
    {
        SetEnvironmentVariable("VK_LOADER_DEBUG", m_config.m_loaderDebug);
    }

    // Try to initialize using the system's ICD or the one that was given explicitly.
    VkResult volkRc = volkInitialize(pIcdPath);
    assert(volkRc == VK_SUCCESS);
    if (volkRc != VK_SUCCESS)
    {
        // We failed to locate the Vulkan loader/ICD.
        if (pIcdPath != nullptr)
        {
            std::cerr << STR_ERR_COULD_NOT_LOCATE_VULKAN_LOADER << "." << std::endl;
        }
        else
        {
            // We will get here in case that there is no Vulkan loader ("vulkan-1")
            // on the system's path. Try loading the amdvlk binary which is packaged with RGA.
            std::string amdvlkOfflineFilePath = GetOfflineAmdvlkPath();
            if (IsFileExists(amdvlkOfflineFilePath))
            {
                // Notify the user.
                std::cerr << STR_USING_OFFLINE_AMDVLK_LOCATION_A << GetAmdvlkFileName() <<
                    STR_USING_OFFLINE_AMDVLK_LOCATION_B << std::endl;

                // Try loading the offline amdvlk binary.
                volkRc = volkInitialize(amdvlkOfflineFilePath.c_str());
            }
        }
    }

    if (volkRc == VK_SUCCESS)
    {
        // We managed to locate the Vulkan loader/ICD. Now, let's create a Vulkan instance.
        std::vector<const char*> supportedExts;
        VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO, 0 };

        appInfo.pApplicationName = STR_VULKAN_APP_INFO_APP_NAME;
        appInfo.applicationVersion = 1;
        appInfo.pEngineName = "";
        appInfo.apiVersion = VK_API_VERSION_1_1;

        VkInstanceCreateInfo instanceInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, 0 };

        instanceInfo.pApplicationInfo = &appInfo;

        // Add Vulkan validation layers to the instance info, if needed.
        if (shouldEnableValidation && m_config.m_enableLayers)
        {
            m_availableValidationLayers = GetAvailableValidationLayers();
            if (!m_availableValidationLayers.empty())
            {
                instanceInfo.ppEnabledLayerNames = m_availableValidationLayers.data();
                instanceInfo.enabledLayerCount = static_cast<uint32_t>(m_availableValidationLayers.size());
            }
        }

        // Add Vulkan extensions to the instance info.
        supportedExts = InitExtensions();
        instanceInfo.ppEnabledExtensionNames = supportedExts.data();
        instanceInfo.enabledExtensionCount = static_cast<uint32_t>(supportedExts.size());

        if (supportedExts.size() != VULKAN_EXTENSIONS.size())
        {
            std::cerr << STR_WRN_FAILED_CHECK_EXTENSIONS << std::endl;
            std::cerr << STR_VALIDATION_INFO_UNAVAILABLE << std::endl;
        }

        // Create a Vulkan instance.
        result = (vkCreateInstance(&instanceInfo, nullptr, &m_instance) == VkResult::VK_SUCCESS);
        assert(result && STR_ERR_FAILED_CREATE_VULKAN_INSTANCE);
        if (!result)
        {
            std::cerr << STR_ERR_FAILED_CREATE_VULKAN_INSTANCE << std::endl;
            std::cerr << "vkCreateInstance" << STR_FUNCTION_RETURNED_ERR_CODE << result << std::endl;
            m_printValidationHint = true;
        }

        // Load all required Vulkan entry points through volk.
        volkLoadInstance(m_instance);

        // Setup the Vulkan validation debug callback function.
        if (shouldEnableValidation && m_config.m_enableLayers && result)
        {
            result = SetupValidationCallback();
        }
    }
    else
    {
        // Notify the user about the failure.
        std::cerr << STR_ERR_COULD_NOT_LOCATE_VULKAN_LOADER;
        if (strlen(pIcdPath) > 0)
        {
            std::cerr << ": " << pIcdPath;
        }
        std::cerr << std::endl;
    }

    return result;
}

void rgVulkanBackend::DestroyVulkanInstance()
{
    assert(m_instance != VK_NULL_HANDLE);
    if (m_instance != VK_NULL_HANDLE)
    {
        if (m_config.m_enableLayers)
        {
            DestroyValidationCallback();
        }

        if (m_validationOutFile.is_open())
        {
            m_validationOutFile.close();
        }

        // Destroy the Vulkan device and instance if they were created successfully.
        if (m_device != VK_NULL_HANDLE)
        {
            vkDestroyDevice(m_device, nullptr);
        }

        vkDestroyInstance(m_instance, nullptr);
        m_gpus.clear();
    }
    else
    {
        std::cerr << STR_ERR_NULL_VK_HANDLE << std::endl;
    }
}

bool rgVulkanBackend::GetTargetGPUNames(std::vector<std::string>& targetNames)
{
    // First check if the primary adapter is a Radeon device.
    bool isAmdGpu = false;
    ListAdapters(isAmdGpu);

    // Set the DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1
    // environment variable to enable the RGA driver stack on APUs.
    EnvVar varApuWorkaround(STR_ENV_VAR_DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_NAME,
        STR_ENV_VAR_DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_VALUE);

    // Set "AMDVLK_NULL_GPU=ALL" environment variable.
    EnvVar var(STR_ENV_VAR_ALL_GPUS_NAME, STR_ENV_VAR_ALL_GPUS_VALUE);

    // Force using the bundled amdvlk if we are on a machine
    // without AMD primary GPU and the user did not already ask
    // to use a custom ICD location.
    if (!isAmdGpu && m_config.m_icdPath.empty())
    {
        // Try loading the amdvlk binary which is packaged with RGA.
        std::string offlineIcdPath = GetOfflineAmdvlkPath();
        if (IsFileExists(offlineIcdPath))
        {
            // Notify the user.
            std::cerr << STR_USING_OFFLINE_AMDVLK_LOCATION_A << GetAmdvlkFileName() <<
                STR_USING_OFFLINE_AMDVLK_LOCATION_B << std::endl;

            // Try loading the offline amdvlk library.
            m_config.m_icdPath = offlineIcdPath;
        }
    }

    bool status = InitVulkan(false);
    assert(status);
    if (status)
    {
        if ((status = EnumerateDevices()) == true)
        {
            for (const auto& gpu : m_gpus)
            {
                targetNames.push_back(gpu.first);
            }
        }

        DestroyVulkanInstance();
    }

    return status;
}

bool rgVulkanBackend::ListTargets()
{
    std::vector<std::string> targetNames;

    bool status = GetTargetGPUNames(targetNames);
    if (status)
    {
        for (const std::string& target : targetNames)
        {
            std::cout << target << std::endl;
        }
        std::cout << std::endl;
    }

    return status;
}

bool rgVulkanBackend::ListAdapters(bool& isAmdGpu, bool shouldPrint)
{
    int i = 0;
    bool status = InitVulkan(false);
    assert(status);
    if (status)
    {
        if ((status = EnumerateDevices()) == true)
        {
            for (const auto& gpu : m_gpus)
            {
                VkPhysicalDeviceProperties props = {};
                vkGetPhysicalDeviceProperties(gpu.second, &props);

                if (i == 0)
                {
                    // Check if the primary adapter is a Radeon device.
                    std::string deviceName = props.deviceName;
                    std::transform(deviceName.begin(), deviceName.end(), deviceName.begin(), ::tolower);
                    isAmdGpu = deviceName.find(STR_RADEON_ADAPTER_TOKEN_A) != std::string::npos ||
                        deviceName.find(STR_RADEON_ADAPTER_TOKEN_B) != std::string::npos;
                }

                if (shouldPrint)
                {
                    std::cout << CLI_VK_BACKEND_STR_ADAPTER << i++ << ":" << std::endl;
                    std::cout << CLI_VK_BACKEND_STR_ADAPTER_OFFSET << CLI_VK_BACKEND_STR_ADAPTER_NAME << props.deviceName << std::endl;
                    std::cout << CLI_VK_BACKEND_STR_ADAPTER_OFFSET << CLI_VK_BACKEND_STR_ADAPTER_DRIVER;
                    uint32_t driverVerMinor = props.driverVersion & VULKAN_DRIVER_VERSION_MINOR_BITS;
                    std::cout << STR_VUKAN_DRIVER_VERSION_MAJOR << '.' << driverVerMinor << std::endl;
                    auto vulkanVer = DecodeVulkanVersion(props.apiVersion);
                    std::cout << CLI_VK_BACKEND_STR_ADAPTER_OFFSET << CLI_VK_BACKEND_STR_ADAPTER_VULKAN;
                    std::cout << std::get<0>(vulkanVer) << '.' << std::get<1>(vulkanVer) << '.' << std::get<2>(vulkanVer) << std::endl;
                }
            }
        }

        DestroyVulkanInstance();
    }

    return status;
}

bool rgVulkanBackend::Build()
{
    std::vector<std::string>  targetNames;
    bool status = GetTargetGPUNames(targetNames);
    if (status)
    {
        // If target GPU is not specified, build for the 1st physical adapter installed on the system.
        // Otherwise, build for the GPU specified by user.
        if (m_config.m_targetGPU.empty())
        {
            status = BuildForDevice("");
        }
        else
        {
            auto targetGPU = std::find_if(targetNames.cbegin(), targetNames.cend(),
                                          [&](const std::string& s) {return IsSubstringIgnoreCase(s, m_config.m_targetGPU);});

            if (targetGPU != targetNames.cend())
            {
                // Set the DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_1
                // environment variable to enable the RGA driver stack on APUs.
                EnvVar varApuWorkaround(STR_ENV_VAR_DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_NAME,
                    STR_ENV_VAR_DISABLE_LAYER_AMD_SWITCHABLE_GRAPHICS_VALUE);

                // Set "AMDVLK_NULL_GPU = target_gpu_name" environment variable.
                EnvVar var(STR_ENV_VAR_ALL_GPUS_NAME, *targetGPU);

                status = BuildForDevice(*targetGPU);
            }
            else
            {
                std::cerr << STR_ERR_INVALID_GPU << m_config.m_targetGPU << std::endl;
            }
        }
    }

    return status;
}

void rgVulkanBackend::PrintValidationHint()
{
    if (m_printValidationHint && !m_config.m_enableLayers)
    {
        std::cerr << STR_LAYER_HINT << std::endl;
    }
}

std::vector<const char*> rgVulkanBackend::InitExtensions()
{
    VkResult result = VK_SUCCESS;
    uint32_t extsNum;
    std::vector<const char*> supportedExts;

    // Get the extensions supported by Vulkan driver installed on the machine.
    if ((result = vkEnumerateInstanceExtensionProperties(nullptr, &extsNum, nullptr)) == VK_SUCCESS)
    {
        std::vector<VkExtensionProperties> extProps(extsNum);
        if ((result = vkEnumerateInstanceExtensionProperties(nullptr, &extsNum, extProps.data())) == VK_SUCCESS)
        {
            // Check if required layers are present in the list of supported layers.
            std::set<std::string> extNames;
            std::for_each(extProps.cbegin(), extProps.cend(), [&](const VkExtensionProperties& p) { extNames.insert(p.extensionName); });

            for (const char* reqdExt : VULKAN_EXTENSIONS)
            {
                // If the required extension is supported, add it to the "supportedExts" list.
                // Otherwise, print a warning.
                if (extNames.find(reqdExt) != extNames.cend())
                {
                    supportedExts.push_back(reqdExt);
                }
                else
                {
                    std::cerr << STR_EXTENSION_NOT_SUPPORTED << reqdExt << std::endl;
                }
            }
        }
    }

    if (result != VK_SUCCESS)
    {
        std::cerr << STR_ERR_FAILED_GET_EXT_PROPS << std::endl;
        std::cerr << "vkEnumerateInstanceExtensionProperties" << STR_FUNCTION_RETURNED_ERR_CODE << result << std::endl;
        m_printValidationHint = true;
    }

    return supportedExts;
}

// Call-back function for Vulkan validation layers.
static VKAPI_ATTR VkBool32 VKAPI_CALL ValidationCallback(VkDebugReportFlagsEXT      flags,
                                                         VkDebugReportObjectTypeEXT objectType,
                                                         uint64_t                   object,
                                                         size_t                     location,
                                                         int32_t                    messageCode,
                                                         const char*                pLayerPrefix,
                                                         const char*                pMessage,
                                                         void*                      pUserData)
{
    if (pMessage != nullptr && pLayerPrefix != nullptr)
    {
        // Filter redundant messages if necessary.
        std::string msg(pMessage);
        bool shouldFilter = (msg.find(STR_VALIDATION_LAYER_MSG_FILTER_0) != std::string::npos) ||
            (msg.find(STR_VALIDATION_LAYER_MSG_FILTER_1) != std::string::npos) ||
            (msg.find(STR_VALIDATION_LAYER_MSG_FILTER_2) != std::string::npos);

        if (!shouldFilter)
        {
            std::ostream& outStream = *reinterpret_cast<std::ostream*>(pUserData);
            outStream << STR_LAYER_OUTPUT_A << pLayerPrefix << STR_LAYER_OUTPUT_BEGIN << std::endl << pMessage << std::endl;
            outStream << STR_LAYER_OUTPUT_A << pLayerPrefix << STR_LAYER_OUTPUT_END << std::endl;
        }
    }
    return VK_FALSE;
}

bool rgVulkanBackend::SetupValidationCallback()
{
    bool result = false;
    void* validationOutStream = &std::cout;

    // Status message.
    std::stringstream msg;
    msg << STR_VALIDATION_ENABLING;

    if (!m_config.m_validationFile.empty())
    {
        m_validationOutFile.open(m_config.m_validationFile);
        if (m_validationOutFile.good())
        {
            validationOutStream = &m_validationOutFile;
        }
    }

    VkDebugReportCallbackCreateInfoEXT callbackInfo = {
        VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
        nullptr,
        VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT,
        ValidationCallback,
        validationOutStream
    };

    assert(m_instance != VK_NULL_HANDLE);
    if (m_instance != VK_NULL_HANDLE)
    {
        auto vkCreateCallback = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(m_instance, "vkCreateDebugReportCallbackEXT");
        assert(vkCreateCallback != nullptr);
        if (vkCreateCallback != nullptr)
        {
            result = (vkCreateCallback(m_instance, &callbackInfo, nullptr, &m_debugCallback) == VK_SUCCESS);
            assert(result);
            if (!result)
            {
                std::cerr << STR_ERR_FAILED_CREATE_CALLBACK << std::endl;
                std::cerr << STR_FUNCTION_RETURNED_ERR_CODE << result << std::endl;
                m_printValidationHint = true;
            }
        }
        else
        {
            std::cerr << STR_WRN_FAILED_GET_VULKAN_EXT_FUNC_PTR << std::endl;
            std::cerr << STR_FUNCTION_RETURNED_ERR_CODE << "NULL" << std::endl;
            std::cerr << STR_VALIDATION_INFO_UNAVAILABLE << std::endl;
        }
    }
    else
    {
        std::cerr << STR_ERR_NULL_VULKAN_INSTANCE << std::endl;
    }

    // Finalize the message and print it.
    msg << (result ? "success." : "failure.");
    std::cout << msg.str() << std::endl;

    return result;
}

bool rgVulkanBackend::EnumerateDevices()
{
    uint32_t gpuCount = 0;
    VkResult vkResult = VK_SUCCESS;
    std::vector<VkPhysicalDevice> gpus;

    vkResult = vkEnumeratePhysicalDevices(m_instance, &gpuCount, nullptr);
    bool result = (vkResult == VK_SUCCESS);
    assert(result);

    // Get the device handles.
    if (result && gpuCount > 0)
    {
        gpus.resize(gpuCount);
        vkResult = vkEnumeratePhysicalDevices(m_instance, &gpuCount, &gpus[0]);
        result = (vkResult == VK_SUCCESS);
        assert(result);
    }

    if (!result)
    {
        std::cerr << STR_ERR_FAILED_ENUMERATE_DEVICES << std::endl;
        std::cerr << "vkEnumeratePhysicalDevices" << STR_FUNCTION_RETURNED_ERR_CODE << vkResult << std::endl;
        m_printValidationHint = true;
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
                m_gpus[props.deviceName] = gpu;
            }
            else
            {
                std::cerr << STR_ERR_FAILED_GET_DEVICE_PROPS << std::endl;
                std::cerr << STR_FUNCTION_RETURNED_EMPTY_LIST << std::endl;
                result = false;
                break;
            }
        }
    }

    if (!result)
    {
        std::cerr << STR_ERR_FAILED_ENUMERATE_DEVICES << std::endl;
        m_printValidationHint = true;
    }

    return result;
}

// A structure used to track the family index of the graphics and compute queue.
struct QueueFamilyIndices
{
    int m_graphicsFamily = -1;
    int m_computeFamily  = -1;

    bool IsComplete() const
    {
        return m_graphicsFamily >= 0 && m_computeFamily >= 0;
    }
};

// Find the Queue families supported by the given physical device.
QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int index = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.m_graphicsFamily = index;
        }

        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            indices.m_computeFamily = index;
        }

        if (indices.IsComplete())
        {
            break;
        }

        index++;
    }

    return indices;
}

bool rgVulkanBackend::VerifyDevice(VkPhysicalDevice gpu)
{
    uint32_t extensionCount = 0;
    bool result = false;
    VkResult vkResult = VK_SUCCESS;

    vkResult = vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, nullptr);
    bool fatalError = (vkResult != VK_SUCCESS);
    if (!fatalError && extensionCount > 0)
    {
        std::vector<VkExtensionProperties> extProps;
        extProps.resize(extensionCount);

        vkResult = vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, &extProps[0]);
        fatalError = (vkResult != VK_SUCCESS);
        if (!fatalError)
        {
            for (uint32_t extIdx = 0; extIdx < extensionCount; ++extIdx)
            {
                if (std::string(extProps[extIdx].extensionName) == VK_AMD_SHADER_INFO_EXTENSION_NAME)
                {
                    result = true;
                    break;
                }
            }
        }
    }

    if (fatalError)
    {
        std::cerr << STR_ERR_FAILED_GET_DEVICE_EXT_PROPS << std::endl;
        std::cerr << STR_FUNCTION_RETURNED_ERR_CODE << vkResult << std::endl;
        m_printValidationHint = true;
    }

    if (!result)
    {
        std::cerr << STR_ERR_DEVICE_DOES_NOT_SUPPORT_EXTENSION << std::endl;
    }

    return result;
}

bool rgVulkanBackend::CreateDevice(VkPhysicalDevice gpu)
{
    bool result = false;
    VkResult vkResult = VK_SUCCESS;

    VkDeviceCreateInfo deviceInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, 0 };
    const char* enabledDeviceExtensions[] = { VK_AMD_SHADER_INFO_EXTENSION_NAME };

    // Specify the extensions to enable for the device.
    deviceInfo.enabledExtensionCount = sizeof(enabledDeviceExtensions) / sizeof(const char*);
    deviceInfo.ppEnabledExtensionNames = enabledDeviceExtensions;

    QueueFamilyIndices indices = FindQueueFamilies(gpu);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    // Only add the queue family if the indices are complete.
    std::set<int> uniqueQueueFamilies;
    if (indices.IsComplete())
    {
        uniqueQueueFamilies.emplace(indices.m_graphicsFamily);
        uniqueQueueFamilies.emplace(indices.m_computeFamily);
    }

    float queuePriority = 1.0f;
    for (int queueFamily : uniqueQueueFamilies)
    {
        // Add the create info for the type of family needed.
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Add the queue create info to the device create info.
    deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceInfo.pQueueCreateInfos = queueCreateInfos.data();

    // Explicitly enable usage of geometry and tessellation stages.
    VkPhysicalDeviceFeatures features = {};
    features.geometryShader = VK_TRUE;
    features.tessellationShader = VK_TRUE;
    features.multiViewport = VK_TRUE;
    deviceInfo.pEnabledFeatures = &features;

    vkResult = vkCreateDevice(gpu, &deviceInfo, nullptr, &m_device);
    result = (vkResult == VK_SUCCESS);
    assert(result && STR_ERR_FAILED_CREATE_LOGICAL_DEVICE);

    if (!result)
    {
        std::cerr << STR_ERR_FAILED_CREATE_LOGICAL_DEVICE_FOR_GPU << std::endl;
        std::cerr << "vkCreateDevice" << STR_FUNCTION_RETURNED_ERR_CODE << vkResult << std::endl;
        m_printValidationHint = true;
    }

    // Obtain the address of vkGetShaderInfoAMD function.
    m_pFuncGetShaderInfo = (PFN_vkGetShaderInfoAMD)vkGetDeviceProcAddr(m_device, STR_FUNC_GET_SHADER_INFO);
    assert(m_pFuncGetShaderInfo != nullptr);
    result = (m_pFuncGetShaderInfo != nullptr);
    if (!result)
    {
        std::cerr << STR_ERR_FAILED_GET_VK_GET_SHADER_INFO_ADDR << std::endl;
        std::cerr << "vkGetDeviceProcAddr" << STR_FUNCTION_RETURNED_ERR_CODE << "NULL" << std::endl;
        m_printValidationHint = true;
    }

    return result;
}

bool rgVulkanBackend::LoadComputePipelineStateFile(const std::string& psoFilePath)
{
    std::string errorString;
    bool isLoaded = rgPsoSerializerVulkan::ReadStructureFromFile(psoFilePath, &m_pComputePipelineCreateInfo, errorString);
    if (!isLoaded)
    {
        std::cerr << STR_ERR_FAILED_DESERIALIZE_COMP_PIPELINE << std::endl;
        std::cerr << errorString << std::endl;
    }

    return isLoaded;
}

bool rgVulkanBackend::CreateComputePipeline()
{
    bool ret = false;
    std::stringstream errText;

    assert(m_pComputePipelineCreateInfo != nullptr);
    if ((ret = (m_pComputePipelineCreateInfo != nullptr)) == true)
    {
        VkComputePipelineCreateInfo* pComputePipelineCreateInfo = m_pComputePipelineCreateInfo->GetComputePipelineCreateInfo();
        assert(pComputePipelineCreateInfo != nullptr);
        if ((ret = (pComputePipelineCreateInfo != nullptr)) == true)
        {
            // These handles need to be created from the deserialized CreateInfo structures,
            // and then assigned into the compute pipeline create info before attempting to
            // create the compute pipeline.
            VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

            // Attempt to create the descriptor set layout.
            std::vector<VkDescriptorSetLayoutCreateInfo*> descriptorSetLayoutCollection = m_pComputePipelineCreateInfo->GetDescriptorSetLayoutCreateInfo();
            std::vector<VkDescriptorSetLayout> descriptorSetLayoutHandles;
            for (VkDescriptorSetLayoutCreateInfo* pDescriptorSetLayoutCreateInfo : descriptorSetLayoutCollection)
            {
                VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
                assert(pDescriptorSetLayoutCreateInfo != nullptr);
                if ((ret = (pDescriptorSetLayoutCreateInfo != nullptr)) == true)
                {
                    // Set the structure type for the Descriptor Set Layout create info.
                    pDescriptorSetLayoutCreateInfo->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

                    // Attempt to create the Descriptor Set Layout.
                    VkResult createResult = vkCreateDescriptorSetLayout(m_device, pDescriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout);
                    assert(createResult == VK_SUCCESS);
                    if ((ret = (createResult == VK_SUCCESS)) == true)
                    {
                        descriptorSetLayoutHandles.push_back(descriptorSetLayout);
                    }
                    else
                    {
                        errText << STR_ERR_FAILED_CREATE_DESCRIPTOR_SET_LAYOUT << std::endl;
                        errText << "vkCreateDescriptorSetLayout" << STR_FUNCTION_RETURNED_ERR_CODE << createResult << std::endl;
                        m_printValidationHint = true;
                    }
                }
                else
                {
                    errText << STR_ERR_NULL_DESC_SET_LAYOUT_CREATE_INFO << std::endl;
                }
            }

            // Proceed with creating the pipeline layout from the compute pipeline create info.
            if (ret)
            {
                VkPipelineLayoutCreateInfo* pPipelineLayoutCreateInfo = m_pComputePipelineCreateInfo->GetPipelineLayoutCreateInfo();
                assert(pPipelineLayoutCreateInfo != nullptr);
                if ((ret = (pPipelineLayoutCreateInfo != nullptr)) == true)
                {
                    // Assign the descriptor set layout into the pipeline layout create info.
                    pPipelineLayoutCreateInfo->pSetLayouts = descriptorSetLayoutHandles.data();
                    pPipelineLayoutCreateInfo->setLayoutCount = static_cast<uint32_t>(descriptorSetLayoutHandles.size());

                    // Attempt to create the pipeline layout from the given create info structure.
                    VkResult createResult = vkCreatePipelineLayout(m_device, pPipelineLayoutCreateInfo, nullptr, &pipelineLayout);
                    assert(createResult == VK_SUCCESS);
                    if ((ret = (createResult == VK_SUCCESS)) == false)
                    {
                        errText << STR_ERR_FAILED_CREATE_PIPELINE_LAYOUT << std::endl;
                        errText << "vkCreatePipelineLayout" << STR_FUNCTION_RETURNED_ERR_CODE << createResult << std::endl;
                        m_printValidationHint = true;
                    }
                }
            }

            // Proceed with creating the compute stage shader module,
            // and insert it into the pipeline create info.
            if (ret)
            {
                // Initialize the compute stage with the user's shader stage arguments.
                pComputePipelineCreateInfo->stage.pName = m_config.m_entries[rgPipelineStage::Compute].c_str();

                // Load the target SPIR-V shader into a shader module.
                VkShaderModule computeStageShaderModule = VK_NULL_HANDLE;
                std::string errMsg;
                bool isModuleCreated = CreateShaderModule(m_config.m_spvFiles[rgPipelineStage::Compute], computeStageShaderModule, errMsg);
                assert(isModuleCreated);
                if ((ret = isModuleCreated) == true)
                {
                    // Insert the shader module handle into the pipeline create info.
                    pComputePipelineCreateInfo->stage.module = computeStageShaderModule;
                }
                else
                {
                    ret = false;
                    errText << errMsg;
                }

                // Insert the pipeline layout handle into the compute pipeline create info.
                VkComputePipelineCreateInfo* pPipelineCreateInfo = m_pComputePipelineCreateInfo->GetComputePipelineCreateInfo();
                assert(pPipelineCreateInfo != nullptr);
                if ((ret = (pPipelineCreateInfo != nullptr)) == true)
                {
                    pPipelineCreateInfo->layout = pipelineLayout;
                }
                else
                {
                    errText << STR_ERR_NULL_PPLN_CREATE_INFO << std::endl;
                }
            }

            // Attempt to create the compute pipeline.
            VkPipeline computePipeline = VK_NULL_HANDLE;
            VkComputePipelineCreateInfo* pComputePipelineCreateInfo = m_pComputePipelineCreateInfo->GetComputePipelineCreateInfo();
            assert(pComputePipelineCreateInfo != nullptr);
            if ((ret = (pComputePipelineCreateInfo != nullptr)) == true)
            {
                VkResult createResult = vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, pComputePipelineCreateInfo, nullptr, &computePipeline);
                assert(createResult == VK_SUCCESS);
                if ((ret = (createResult == VK_SUCCESS)) == false)
                {
                    errText << STR_ERR_FAILED_CREATE_COMP_PIPELINE << std::endl;
                    errText << "vkCreateComputePipelines" << STR_FUNCTION_RETURNED_ERR_CODE << createResult << std::endl;
                    errText << STR_ERR_FAILED_CREATE_PIPELINE_HINT << std::endl;
                    m_printValidationHint = true;
                    ret = false;
                }
            }

            assert(ret);
            if (ret)
            {
                m_pipeline = { STR_CREATED_PIPELINE_NAME, computePipeline };

                // Destroy the handles that were required to create the pipeline.
                for (VkDescriptorSetLayout descriptorSetLayout : descriptorSetLayoutHandles)
                {
                    assert(descriptorSetLayout != VK_NULL_HANDLE);
                    if ((ret = (descriptorSetLayout != VK_NULL_HANDLE)) == true)
                    {
                        vkDestroyDescriptorSetLayout(m_device, descriptorSetLayout, nullptr);
                    }
                    else
                    {
                        errText << STR_ERR_NULL_DESC_SET_LAYOUT << std::endl;
                        break;
                    }
                }

                assert(pipelineLayout != VK_NULL_HANDLE);
                if (pipelineLayout != VK_NULL_HANDLE)
                {
                    vkDestroyPipelineLayout(m_device, pipelineLayout, nullptr);
                }
            }
            else
            {
                std::cerr << errText.str();
            }
        }
    }

    return ret;
}

bool rgVulkanBackend::CreateShaderModule(const std::string& spvFilePath, VkShaderModule& shaderModuleHandle, std::string& errorString)
{
    bool ret = false;

    // Try to read the SPIR-V shader code into an array of bytes.
    std::vector<char> shaderBytes;
    ret = ReadShaderBytes(spvFilePath, shaderBytes, errorString);

    assert(ret);
    if (ret)
    {
        // Create and initialize a shader module create info structure and add the SPIR-V code to it.
        VkShaderModuleCreateInfo moduleCreateInfo = {};
        moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        moduleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shaderBytes.data());
        moduleCreateInfo.codeSize = shaderBytes.size();

        // Attempt to create the shader module.
        VkResult createResult = vkCreateShaderModule(m_device, &moduleCreateInfo, nullptr, &shaderModuleHandle);

        // Was the shader module created successfully?
        ret = (createResult == VK_SUCCESS);
        assert(ret);
        if (!ret)
        {
            std::stringstream errorStream;
            errorStream << STR_ERR_FAILED_CREATE_SHADER_MODULE << spvFilePath << std::endl;
            errorStream << "vkCreateShaderModule" << STR_FUNCTION_RETURNED_ERR_CODE << createResult << std::endl;
            errorString = errorStream.str();
            m_printValidationHint = true;
        }
    }

    return ret;
}

bool rgVulkanBackend::LoadGraphicsPipelineStateFile(const std::string& psoFilePath)
{
    std::string errorString;
    bool isLoaded = rgPsoSerializerVulkan::ReadStructureFromFile(psoFilePath, &m_pGraphicsPipelineCreateInfo, errorString);
    if (!isLoaded)
    {
        std::cerr << STR_ERR_FAILED_DESERIALIZE_GRAPH_PIPELINE << std::endl;
        std::cerr << errorString << std::endl;
    }

    return isLoaded;
}

bool rgVulkanBackend::CreateGraphicsPipeline()
{
    bool ret = false;
    std::stringstream errText;

    assert(m_pGraphicsPipelineCreateInfo != nullptr);
    if ((ret = (m_pGraphicsPipelineCreateInfo != nullptr)) == true)
    {
        // These handles need to be created from the deserialized CreateInfo structures,
        // and then assigned into the graphics pipeline create info before attempting to
        // create the compute pipeline.
        std::vector< VkDescriptorSetLayout> descriptorSetLayoutHandles;

        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkRenderPass renderPass = VK_NULL_HANDLE;

        // Attempt to create the descriptor set layout.
        std::vector<VkDescriptorSetLayoutCreateInfo*> descriptorSetLayoutCollection = m_pGraphicsPipelineCreateInfo->GetDescriptorSetLayoutCreateInfo();

        for (VkDescriptorSetLayoutCreateInfo* pDescriptorSetLayoutCreateInfo : descriptorSetLayoutCollection)
        {
            VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
            assert(pDescriptorSetLayoutCreateInfo != nullptr);
            if (pDescriptorSetLayoutCreateInfo != nullptr)
            {
                // Set the structure type for the Descriptor Set Layout create info.
                pDescriptorSetLayoutCreateInfo->sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

                VkResult createResult = vkCreateDescriptorSetLayout(m_device, pDescriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout);
                if ((ret = (createResult == VK_SUCCESS)) == true)
                {
                    descriptorSetLayoutHandles.push_back(descriptorSetLayout);
                }
                else
                {
                    errText << STR_ERR_FAILED_CREATE_DESCRIPTOR_SET_LAYOUT << std::endl;
                    errText << "vkCreateDescriptorSetLayout" << STR_FUNCTION_RETURNED_ERR_CODE << createResult << std::endl;
                    m_printValidationHint = true;
                    break;
                }
            }
        }

        // Proceed with creating the pipeline layout from the compute pipeline create info.
        if (ret)
        {
            VkPipelineLayoutCreateInfo* pPipelineLayoutCreateInfo = m_pGraphicsPipelineCreateInfo->GetPipelineLayoutCreateInfo();
            assert(pPipelineLayoutCreateInfo != nullptr);
            if ((ret = (pPipelineLayoutCreateInfo != nullptr)) == true)
            {
                // Assign the descriptor set layout into the pipeline layout create info.
                pPipelineLayoutCreateInfo->pSetLayouts = descriptorSetLayoutHandles.data();
                pPipelineLayoutCreateInfo->setLayoutCount = static_cast<uint32_t>(descriptorSetLayoutHandles.size());

                // Attempt to create the pipeline layout from the given create info structure.
                VkResult createResult = vkCreatePipelineLayout(m_device, pPipelineLayoutCreateInfo, nullptr, &pipelineLayout);
                assert(ret);
                if ((ret = (createResult == VK_SUCCESS)) == false)
                {
                    errText << STR_ERR_FAILED_CREATE_PIPELINE_LAYOUT << std::endl;
                    errText << "vkCreatePipelineLayout" << STR_FUNCTION_RETURNED_ERR_CODE << createResult << std::endl;
                    m_printValidationHint = true;
                }
            }
        }

        // Proceed with creating the render pass.
        if (ret)
        {
            VkRenderPassCreateInfo* pRenderPassCreateInfo = m_pGraphicsPipelineCreateInfo->GetRenderPassCreateInfo();
            assert(pRenderPassCreateInfo != nullptr);
            if ((ret = (pRenderPassCreateInfo != nullptr)) == true)
            {
                // Attempt to create the render pass from the given create info structure.
                VkResult createResult = vkCreateRenderPass(m_device, pRenderPassCreateInfo, nullptr, &renderPass);
                assert(ret);
                if ((ret = (createResult == VK_SUCCESS)) == false)
                {
                    errText << STR_ERR_FAILED_CREATE_RENDER_PASS << std::endl;
                    errText << "vkCreateRenderPass" << STR_FUNCTION_RETURNED_ERR_CODE << createResult << std::endl;
                    m_printValidationHint = true;
                }
            }
        }

        // Creating each stage shader module and insert them into the pipeline create info.
        if (ret)
        {
            VkGraphicsPipelineCreateInfo* pPipelineCreateInfo = m_pGraphicsPipelineCreateInfo->GetGraphicsPipelineCreateInfo();
            assert(pPipelineCreateInfo != nullptr);
            if ((ret = (pPipelineCreateInfo != nullptr)) == true)
            {
                std::vector<VkPipelineShaderStageCreateInfo> stageCreateInfos;
                char firstStage = static_cast<char>(rgPipelineStage::Vertex);
                char lastStage = static_cast<char>(rgPipelineStage::Fragment);
                for (int stageIndex = firstStage; stageIndex <= lastStage; ++stageIndex)
                {
                    // If the stage has a SPIR-V file, add it to the vector of stage create info.
                    const std::string& stageShaderFile = m_config.m_spvFiles[stageIndex];
                    if (!stageShaderFile.empty())
                    {
                        // Load the target SPIR-V shader into a shader module.
                        VkShaderModule stageModule = VK_NULL_HANDLE;
                        std::string errMsg;
                        bool isModuleCreated = CreateShaderModule(m_config.m_spvFiles[stageIndex], stageModule, errMsg);

                        assert(isModuleCreated);
                        if ((ret = isModuleCreated) == true)
                        {
                            // Initialize a shader stage structure using the newly created module.
                            VkPipelineShaderStageCreateInfo stageInfo = {};
                            stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

                            // Shift using the stage index to convert to a Vulkan stage flag.
                            stageInfo.stage = static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_VERTEX_BIT << stageIndex);

                            // Insert the shader module handle and entrypoint name into the stage info.
                            stageInfo.module = stageModule;
                            stageInfo.pName = m_config.m_entries[stageIndex].c_str();

                            // Add the stage info to the vector of stages in the PSO create info.
                            stageCreateInfos.push_back(stageInfo);
                        }
                        else
                        {
                            errText << errMsg;
                        }
                    }
                }

                // Insert the stage array into the pipeline create info. This array of
                // stage create info contains the SPIR-V module handles created above.
                pPipelineCreateInfo->pStages = stageCreateInfos.data();
                pPipelineCreateInfo->stageCount = static_cast<uint32_t>(stageCreateInfos.size());

                // Insert the pipeline layout and render pass handles into the pipeline create info.
                pPipelineCreateInfo->layout = pipelineLayout;
                pPipelineCreateInfo->renderPass = renderPass;

                // Attempt to create the graphics pipeline.
                VkPipeline graphicsPipeline = VK_NULL_HANDLE;
                VkResult createResult = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, pPipelineCreateInfo, nullptr, &graphicsPipeline);
                assert(createResult == VK_SUCCESS);
                if ((ret = (createResult == VK_SUCCESS)) == false)
                {
                    errText << STR_ERR_FAILED_CREATE_GRAPH_PIPELINE << std::endl;
                    errText << "vkCreateGraphicsPipelines" << STR_FUNCTION_RETURNED_ERR_CODE << createResult << std::endl;
                    errText << STR_ERR_FAILED_CREATE_PIPELINE_HINT << std::endl;
                    m_printValidationHint = true;
                }

                assert(ret);
                if (ret)
                {
                    m_pipeline = { STR_CREATED_PIPELINE_NAME, graphicsPipeline };

                    // Destroy the handles that were required to create the pipeline.
                    assert(renderPass != VK_NULL_HANDLE);
                    if (renderPass != VK_NULL_HANDLE)
                    {
                        vkDestroyRenderPass(m_device, renderPass, nullptr);
                    }

                    for (VkDescriptorSetLayout descriptorSetLayout : descriptorSetLayoutHandles)
                    {
                        if (descriptorSetLayout != VK_NULL_HANDLE)
                        {
                            vkDestroyDescriptorSetLayout(m_device, descriptorSetLayout, nullptr);
                        }
                    }

                    assert(pipelineLayout != VK_NULL_HANDLE);
                    if (pipelineLayout != VK_NULL_HANDLE)
                    {
                        vkDestroyPipelineLayout(m_device, pipelineLayout, nullptr);
                    }
                }
                else
                {
                    std::cerr << errText.str() << std::endl;
                }
            }
        }
    }

    return ret;
}

bool rgVulkanBackend::ReadShaderBytes(const std::string& filePath, std::vector<char>& fileBytes, std::string& errorMessage)
{
    bool isOk = false;

    std::ifstream fileStream(filePath, std::ios::ate | std::ios::binary);

    bool isFileOpened = fileStream.is_open();

    assert(isFileOpened);
    if (isFileOpened)
    {
        size_t fileSize = (size_t)fileStream.tellg();
        fileBytes.resize(fileSize);

        fileStream.seekg(0);
        fileStream.read(fileBytes.data(), fileSize);
        fileStream.close();

        isOk = true;
    }
    else
    {
        errorMessage = STR_ERR_FAILED_TO_READ_SHADER_FILE + filePath;
        assert(false);
    }

    return isOk;
}

VkPhysicalDevice rgVulkanBackend::GetDeviceHandle(const std::string& deviceName)
{
    VkPhysicalDevice deviceHandle = VK_NULL_HANDLE;

    assert(!m_gpus.empty());
    if (!m_gpus.empty())
    {
        // If device name is empty, return handle of the 1st supported device.
        // Otherwise, look for the device with matching name.
        if (deviceName.empty())
        {
            deviceHandle = m_gpus.cbegin()->second;
        }
        else
        {
            for (const auto& gpu : m_gpus)
            {
                if (gpu.first == deviceName)
                {
                    deviceHandle = gpu.second;
                    break;
                }
            }
        }

        if (deviceHandle == VK_NULL_HANDLE)
        {
            std::cerr << STR_ERR_INVALID_GPU << deviceName << std::endl;
        }
    }
    else
    {
        std::cerr << STR_ERR_NO_DEVICES_AVAILABLE << std::endl;
    }

    return deviceHandle;
}

bool rgVulkanBackend::BuildForDevice(const std::string& deviceName)
{
    // Create a Vulkan instance.
    bool status = InitVulkan(true);
    assert(status);

    // Get the device info.
    status = status && EnumerateDevices();

    VkPhysicalDevice deviceHandle = GetDeviceHandle(deviceName);

    if (deviceHandle != VK_NULL_HANDLE)
    {
        // Check if target GPU supports required extensions.
        status = status && VerifyDevice(deviceHandle);

        // Create a Vulkan device compatible with VK_AMD_shader_info (no queues needed)
        status = status && CreateDevice(deviceHandle);

        // Create a Vulkan pipeline.
        status = status && CreatePipeline();

        // Build the pipeline.
        status = status && BuildPipeline();
    }

    return status;
}

bool rgVulkanBackend::CreatePipeline()
{
    bool ret = false;
    bool shouldAbort = false;

    // Determine the type of pipeline being created by searching for SPIR-V files attached to each stage.
    bool hasComputeShader = !m_config.m_spvFiles[rgPipelineStage::Compute].empty();

    bool hasGraphicsShaders = (!m_config.m_spvFiles[rgPipelineStage::Vertex].empty() ||
                               !m_config.m_spvFiles[rgPipelineStage::TessellationControl].empty() ||
                               !m_config.m_spvFiles[rgPipelineStage::TessellationEvaluation].empty() ||
                               !m_config.m_spvFiles[rgPipelineStage::Geometry].empty() ||
                               !m_config.m_spvFiles[rgPipelineStage::Fragment].empty());

    if (hasGraphicsShaders && m_config.m_spvFiles[rgPipelineStage::Vertex].empty())
    {
        std::cerr << STR_ERR_GRAPHICS_PIPELINE_WITHOUT_VERTEX << std::endl;
        shouldAbort = true;
    }

    if (!shouldAbort)
    {
        // If the user specified a direct path to a PSO file, deserialize it.
        if (!m_config.m_pipelineStateFile.empty())
        {
            // Expect one of compute or graphic shader is specified (not both).
            if (hasComputeShader != hasGraphicsShaders)
            {
                if (hasGraphicsShaders)
                {
                    ret = LoadGraphicsPipelineStateFile(m_config.m_pipelineStateFile);
                }
                else
                {
                    ret = LoadComputePipelineStateFile(m_config.m_pipelineStateFile);
                }

                assert(ret && STR_ERR_FAILED_LOAD_PIPELINE_FILE);
                if (!ret)
                {
                    std::cerr << STR_ERR_FAILED_LOAD_PIPELINE_FILE << std::endl;
                }
            }
            else
            {
                assert(false && STR_ERR_GRAPH_OR_COMP_SHADERS_EXPECTED);
                std::cerr << STR_ERR_GRAPH_OR_COMP_SHADERS_EXPECTED << std::endl;
            }
        }
        else
        {
            // The user didn't provide a Pipeline State file, so create one using the pipeline factory.
            rgPsoFactoryVulkan pipelineFactory;

            if (hasComputeShader)
            {
                m_pComputePipelineCreateInfo = pipelineFactory.GetDefaultComputePsoCreateInfo();
                assert(m_pComputePipelineCreateInfo != nullptr);
                if (m_pComputePipelineCreateInfo == nullptr)
                {
                    std::cerr << STR_ERR_FAILED_CREATE_DEFAULT_COMP_PIPELINE << std::endl;
                }
            }
            else if (hasGraphicsShaders)
            {
                m_pGraphicsPipelineCreateInfo = pipelineFactory.GetDefaultGraphicsPsoCreateInfo();
                assert(m_pGraphicsPipelineCreateInfo != nullptr);
                if (m_pGraphicsPipelineCreateInfo == nullptr)
                {
                    std::cerr << STR_ERR_FAILED_CREATE_DEFAULT_GRAPH_PIPELINE << std::endl;
                }
            }
            else
            {
                // Can't determine the type of pipeline being created. This shouldn't happen.
                assert(false);
            }
        }

        // If the user provided a path to a compute shader, create a compute pipeline.
        if (hasComputeShader)
        {
            ret = CreateComputePipeline();
        }
        else if (hasGraphicsShaders)
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

bool rgVulkanBackend::BuildPipeline() const
{
    bool status = true;

    if (!m_config.m_binaryFile.empty())
    {
        // Build the pipeline if it's present.
        if (!m_pipeline.first.empty())
        {
            status = status && BuildToBinary(m_pipeline.first, m_pipeline.second);
        }
    }

    // "targetStages": a vector of tuples {stage_bits, out_isa_file_name, out_stats_file_name}.
    std::vector<std::tuple<VkShaderStageFlagBits, std::string, std::string>>  targetStages;

    if (!m_config.m_spvFiles[rgPipelineStage::Vertex].empty())
    {
        targetStages.push_back(std::tuple<VkShaderStageFlagBits, std::string, std::string>(
                                 VK_SHADER_STAGE_VERTEX_BIT,
                                 m_config.m_isaFiles[rgPipelineStage::Vertex],
                                 m_config.m_statsFiles[rgPipelineStage::Vertex]
                               ));
    }
    if (!m_config.m_spvFiles[rgPipelineStage::TessellationControl].empty())
    {
        targetStages.push_back(std::tuple<VkShaderStageFlagBits, std::string, std::string>(
                                 VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                                 m_config.m_isaFiles[rgPipelineStage::TessellationControl],
                                 m_config.m_statsFiles[rgPipelineStage::TessellationControl]
                               ));
    }
    if (!m_config.m_spvFiles[rgPipelineStage::TessellationEvaluation].empty())
    {
        targetStages.push_back(std::tuple<VkShaderStageFlagBits, std::string, std::string>(
                                 VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                                 m_config.m_isaFiles[rgPipelineStage::TessellationEvaluation],
                                 m_config.m_statsFiles[rgPipelineStage::TessellationEvaluation]
                               ));
    }
    if (!m_config.m_spvFiles[rgPipelineStage::Geometry].empty())
    {
        targetStages.push_back(std::tuple<VkShaderStageFlagBits, std::string, std::string>(
                                 VK_SHADER_STAGE_GEOMETRY_BIT,
                                 m_config.m_isaFiles[rgPipelineStage::Geometry],
                                 m_config.m_statsFiles[rgPipelineStage::Geometry]
                               ));
    }
    if (!m_config.m_spvFiles[rgPipelineStage::Fragment].empty())
    {
        targetStages.push_back(std::tuple<VkShaderStageFlagBits, std::string, std::string>(
                                 VK_SHADER_STAGE_FRAGMENT_BIT,
                                 m_config.m_isaFiles[rgPipelineStage::Fragment],
                                 m_config.m_statsFiles[rgPipelineStage::Fragment]
                               ));
    }
    if (!m_config.m_spvFiles[rgPipelineStage::Compute].empty())
    {
        targetStages.push_back(std::tuple<VkShaderStageFlagBits, std::string, std::string>(
                                 VK_SHADER_STAGE_COMPUTE_BIT,
                                 m_config.m_isaFiles[rgPipelineStage::Compute],
                                 m_config.m_statsFiles[rgPipelineStage::Compute]
                               ));
    }

    for (const auto& stage : targetStages)
    {
        if (!std::get<1>(stage).empty())
        {
            // Build ISA for the pipeline if it's present.
            if (!m_pipeline.first.empty())
            {
                status = status && BuildToDisasm(std::get<1>(stage), m_pipeline.second, std::get<0>(stage));
            }
        }

        if (!std::get<2>(stage).empty())
        {
            // Build stats for the pipeline if it's present.
            if (!m_pipeline.first.empty())
            {
                status = status && BuildToStatistics(std::get<2>(stage), m_pipeline.first, m_pipeline.second, std::get<0>(stage));
            }
        }
    }

    // We're done with dumping data from the pipeline. Destroy the pipeline before exiting.
    VkPipeline pipelineHandle = m_pipeline.second;
    if (pipelineHandle != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(m_device, pipelineHandle, nullptr);
    }

    return status;
}

bool rgVulkanBackend::BuildToStatistics(const std::string& statsFileName, const std::string& pipelineName,
                                        VkPipeline pipeline, VkShaderStageFlagBits stage) const
{
    VkShaderStatisticsInfoAMD stats = {};
    size_t size = sizeof(stats);

    assert(m_pFuncGetShaderInfo != nullptr);
    bool status = (m_pFuncGetShaderInfo != nullptr);

    status = status && (m_pFuncGetShaderInfo(m_device, pipeline, stage, VK_SHADER_INFO_TYPE_STATISTICS_AMD, &size, &stats) == VK_SUCCESS);
    assert(status && STR_ERR_FAILED_GET_SHADER_STATS);
    if (status)
    {
        std::stringstream statisticsStream;

        statisticsStream << "Statistics:" << std::endl;
        statisticsStream << "    - shaderStageMask                           = " << stats.shaderStageMask << std::endl;
        statisticsStream << "    - resourceUsage.numUsedVgprs                = " << stats.resourceUsage.numUsedVgprs << std::endl;
        statisticsStream << "    - resourceUsage.numUsedSgprs                = " << stats.resourceUsage.numUsedSgprs << std::endl;
        statisticsStream << "    - resourceUsage.ldsSizePerLocalWorkGroup    = " << stats.resourceUsage.ldsSizePerLocalWorkGroup << std::endl;
        statisticsStream << "    - resourceUsage.ldsUsageSizeInBytes         = " << stats.resourceUsage.ldsUsageSizeInBytes << std::endl;
        statisticsStream << "    - resourceUsage.scratchMemUsageInBytes      = " << stats.resourceUsage.scratchMemUsageInBytes << std::endl;
        statisticsStream << "    - numPhysicalVgprs                          = " << stats.numPhysicalVgprs << std::endl;
        statisticsStream << "    - numPhysicalSgprs                          = " << stats.numPhysicalSgprs << std::endl;
        statisticsStream << "    - numAvailableVgprs                         = " << stats.numAvailableVgprs << std::endl;
        statisticsStream << "    - numAvailableSgprs                         = " << stats.numAvailableSgprs << std::endl;

        if (stage == VK_SHADER_STAGE_COMPUTE_BIT)
        {
            for (int i = 0; i < 3; ++i)
            {
                statisticsStream << "    - computeWorkGroupSize" << i << " = " << stats.computeWorkGroupSize[i] << std::endl;
            }
        }

        status = WriteTextFile(statsFileName, statisticsStream.str());
    }

    if (!status)
    {
        std::cerr << STR_ERR_PIPELINE_STATS_NOT_AVAILABLE << std::endl;
    }

    return status;
}

bool rgVulkanBackend::BuildToBinary(const std::string& pipelineName, VkPipeline pipeline) const
{
    size_t binarySize = 0;

    assert(m_pFuncGetShaderInfo != nullptr);
    bool status = (m_pFuncGetShaderInfo != nullptr);

    // Note: When retrieving the binary for an entire pipeline, the shader stage argument is
    // irrelevant, and is ignored by the extension. Despite this, the validation layer will still
    // complain about some specific stage arguments being invalid.
    // Therefore, the "VK_SHADER_STAGE_VERTEX_BIT" argument is provided here to avoid emitting a
    // validation layer warning, and the resulting binary will include all relevant pipeline stages.
    VkShaderStageFlagBits stageFlagBits = VK_SHADER_STAGE_VERTEX_BIT;

    // Query the size of the binary file being dumped.
    status = status && (m_pFuncGetShaderInfo(m_device, pipeline, stageFlagBits,
        VK_SHADER_INFO_TYPE_BINARY_AMD, &binarySize, nullptr) == VK_SUCCESS);
    assert(status && STR_ERR_FAILED_GET_BINARY_SIZE);

    if (status && binarySize > 0)
    {
        // Allocate storage for the binary, fill it with the binary bytes, and write to disk.
        uint8_t* binary = new uint8_t[binarySize];

        if (binary && m_pFuncGetShaderInfo(m_device, pipeline, stageFlagBits,
            VK_SHADER_INFO_TYPE_BINARY_AMD, &binarySize, binary) == VK_SUCCESS)
        {
            status = WriteBinaryFile(m_config.m_binaryFile, std::vector<char>(binary, binary + binarySize));
        }

        delete[] binary;
    }

    if (!status)
    {
        std::cerr << STR_ERR_PIPELINE_BINARY_NOT_AVAILABLE << std::endl;
    }

    return status;
}

bool rgVulkanBackend::BuildToDisasm(const std::string& isaFileName, VkPipeline pipeline, VkShaderStageFlagBits stage) const
{
    size_t disassemblySize = 0;

    assert(m_pFuncGetShaderInfo != nullptr);
    bool status = (m_pFuncGetShaderInfo != nullptr);

    status = status && (m_pFuncGetShaderInfo(m_device, pipeline, stage, VK_SHADER_INFO_TYPE_DISASSEMBLY_AMD,
                                             &disassemblySize, nullptr) == VK_SUCCESS);
    assert(status && disassemblySize > 0);
    if (status && disassemblySize > 0)
    {
        char* disassembly = new char[disassemblySize];

        if (disassembly && m_pFuncGetShaderInfo(m_device, pipeline, stage,
            VK_SHADER_INFO_TYPE_DISASSEMBLY_AMD, &disassemblySize, disassembly) == VK_SUCCESS)
        {
            status = WriteTextFile(isaFileName, disassembly);
        }

        delete[] disassembly;
    }

    if (!status)
    {
        std::cerr << STR_ERR_DISASM_NOT_AVAILABLE << std::endl;
    }

    return status;
}

