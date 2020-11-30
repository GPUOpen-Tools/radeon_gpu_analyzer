// C++.
#include <string>
#include <vector>
#include <array>
#include <fstream>

// Volk.
#include <volk/volk.h>

// Local.
#include <Utils/Include/rgaSharedDataTypes.h>
#include <Utils/Vulkan/Include/rgPipelineTypes.h>
//
// Vulkan Backend configuration specified by the command line arguments.
//
struct RgConfig
{
    // Requested action.
    enum class Action
    {
        kListTargets,
        kListAdapters,
        kBuild,
        kInvalid
    };

    // Requested action.
    Action  action = Action::kInvalid;

    // Selected target GPU.
    std::string  target_gpu;

    // Path to the Pipeline State file.
    std::string pipeline_state_file;

    // Path to the ICD file to load instead of the loader (optional).
    std::string icd_path;

    // Value of the VK_LOADER_DEBUG environment variable (optional).
    std::string loader_debug;

    // Stage-specific shader SPV file inputs.
    std::array<std::string, rgPipelineStage::Count> spv_files;

    // Entry points.
    std::array<std::string, rgPipelineStage::Count> entries;

    // Stage-specific output ISA file paths.
    std::array<std::string, rgPipelineStage::Count> isa_files;

    // Stage-specific output statistics file paths.
    std::array<std::string, rgPipelineStage::Count> stats_files;

    // Shader binary output file path.
    std::string   binary_file;

    // Validation output file path.
    std::string   validation_file;

    // Enable Vulkan validation layers.
    bool  enable_layers = false;
};

//
// The Vulkan Backend class responsible for interaction with Vulkan interface of AMD Graphics Driver.
//
class RgVulkanBackend
{
public:
    // CTOR
    RgVulkanBackend() = default;

    // DTOR
    ~RgVulkanBackend() = default;

    // Parse the command line options and fill the configuration.
    // Returns requested action. If parsing command line failed, returns "Invalid" action.
    RgConfig::Action ParseCmdLine(int argc, char* argv[]);

    // List supported Vulkan targets.
    bool ListTargets();

    // List physical graphic adapters installed on the system.
    // If adapter #0 is an AMD GPU, is_amd_gpu is set to true.
    // If should_print is set to true, the list of adapters would be printed to stdout.
    bool ListAdapters(bool& is_amd_gpu, bool should_print = false);

    // Perform build process.
    bool Build();

    // Print a hint about using Vulkan validation layers if required.
    void PrintValidationHint();

private:
    // Initialize Vulkan API.
    // shouldEnableValidation - true to enable validation layers, false otherwise.
    // Note: the driver does not support enabling validation layers in ALL mode.
    bool InitVulkan(bool shouldEnableValidation);

    // Destroy initialized Vulkan instance.
    void DestroyVulkanInstance();

    // Collect Vulkan devices (names and Vulkan handles).
    bool EnumerateDevices();

    // Get the list of names of supported target GPUs.
    // The target names are returned in "target_names" vector.
    bool GetTargetGPUNames(std::vector<std::string>& target_names);

    // Verifies that the device supports required extensions.
    bool VerifyDevice(VkPhysicalDevice gpu);

    // Create Vulkan device object.
    // The device is specified by a pair {device_name, device_handle}.
    bool CreateDevice(VkPhysicalDevice gpu);

    // Get Vulkan device handle by target GPU name.
    // Returns NULL handle if device is not found.
    // If specified device name is empty, return handle of the 1st Vulkan device in the list.
    VkPhysicalDevice GetDeviceHandle(const std::string& device_name);

    // Build for specified device name.
    // If device name is empty, build for the 1st physical device installed on the system.
    bool BuildForDevice(const std::string& device_name);

    // Create Vulkan pipeline object.
    bool CreatePipeline();

    // Build created pipeline and shaders.
    bool BuildPipeline() const;

private:
#ifdef _ENUMERATE_LAYERS_ENABLED
    // The driver does not implement this call for RGA in ALL mode.
    // In case that this changes in the future, define _ENUMERATE_LAYERS_ENABLED.
    // Check if the required Vulkan validation layers are available.
    // Returns the list of names of required validation layers that are supported.
    std::vector<const char*> InitValidationLayers();
#endif // _ENUMERATE_LAYERS_ENABLED

    // Check if the required Vulkan extensions are available.
    // Returns the list of names of required extensions that are supported.
    std::vector<const char*> InitExtensions();

    // Set up the message reporting callback for the Vulkan validation layers.
    bool SetupValidationCallback();

    // Destroy the Vulkan validation callback.
    void DestroyValidationCallback();

    // Build pipeline to binary.
    // The "pipeline_name" specifies the name of Vulkan pipeline.
    // The pipeline itself is specified by "pipeline".
    bool BuildToBinary(const std::string& pipeline_name, VkPipeline pipeline) const;

    // Build pipeline to disassembly.
    bool BuildToDisasm(const std::string& isa_filename, VkPipeline pipeline, VkShaderStageFlagBits stage) const;

    // Build pipeline to statistics.
    bool BuildToStatistics(const std::string& stats_filename, const std::string& pipeline_name,
        VkPipeline pipeline, VkShaderStageFlagBits stage) const;

    // Load a compute pipeline state file from disk.
    bool LoadComputePipelineStateFile(const std::string& pso_file_path);

    // Load a graphics pipeline state file from disk.
    bool LoadGraphicsPipelineStateFile(const std::string& pso_file_path);

    // Create a graphics pipeline.
    bool CreateGraphicsPipeline();

    // Create a compute pipeline.
    bool CreateComputePipeline();

    // Create a shader module from the given SPIR-V file path.
    bool CreateShaderModule(const std::string& spv_file_path, VkShaderModule& shader_module_handle, std::string& error_string);

    // Read the file at the given path into the given array of bytes.
    // Return an error string if there's a problem.
    bool ReadShaderBytes(const std::string& file_path, std::vector<char>& file_bytes, std::string& error_message);

    // -- Data --

    // Configuration.
    RgConfig config_;

    // Vulkan instance.
    VkInstance instance_ = VK_NULL_HANDLE;

    // Supported AMD devices (map device_name --> Vk device handle).
    std::map<std::string, VkPhysicalDevice> gpus_;

    // Selected Vulkan device.
    VkDevice device_ = VK_NULL_HANDLE;

    // Vulkan validation report callback.
    VkDebugReportCallbackEXT debug_callback_ = VK_NULL_HANDLE;

    // Pipeline: a pair {pipeline_name, VkPipeline handle}.
    std::pair<std::string, VkPipeline>  pipeline_ = {};

    // Pointer to vkGetShaderInfoAMD function.
    PFN_vkGetShaderInfoAMD function_get_shader_info_ = nullptr;

    // Container to hold the available validation layers on this system.
    std::vector<const char*> available_validation_layers_;

    // Create Info for a graphics pipeline state object.
    rgPsoGraphicsVulkan* graphics_pipeline_create_info_ = nullptr;

    // Create Info for a compute pipeline state object.
    rgPsoComputeVulkan* compute_pipeline_create_info_ = nullptr;

    // Output file for validation info.
    std::ofstream validation_output_file_;

    // Specifies whether the Vulkan validation hint should be printed after dumping the error messages.
    bool should_print_validation_hint_ = false;
};

