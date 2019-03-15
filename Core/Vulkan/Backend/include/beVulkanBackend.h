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
struct rgConfig
{
    // Requested action.
    enum class Action
    {
        ListTargets,
        ListAdapters,
        Build,

        Invalid
    };

    // Requested action.
    Action  m_action   = Action::Invalid;

    // Selected target GPU.
    std::string  m_targetGPU;

    // Path to the Pipeline State file.
    std::string m_pipelineStateFile;

    // Path to the ICD file to load instead of the loader (optional).
    std::string m_icdPath;

    // Value of the VK_LOADER_DEBUG environment variable (optional).
    std::string m_loaderDebug;

    // Stage-specific shader SPV file inputs.
    std::array<std::string, rgPipelineStage::Count>  m_spvFiles;

    // Entry points.
    std::array<std::string, rgPipelineStage::Count>  m_entries;

    // Stage-specific output ISA file paths.
    std::array<std::string, rgPipelineStage::Count>  m_isaFiles;

    // Stage-specific output statistics file paths.
    std::array<std::string, rgPipelineStage::Count>  m_statsFiles;

    // Shader binary output file path.
    std::string   m_binaryFile;

    // Validation output file path.
    std::string   m_validationFile;

    // Enable Vulkan validation layers.
    bool  m_enableLayers   = false;
};

//
// The Vulkan Backend class responsible for interaction with Vulkan interface of AMD Graphics Driver.
//
class rgVulkanBackend
{
public:
    // CTOR
    rgVulkanBackend() = default;

    // DTOR
    ~rgVulkanBackend() = default;

    // Parse the command line options and fill the configuration.
    // Returns requested action. If parsing command line failed, returns "Invalid" action.
    rgConfig::Action ParseCmdLine(int argc, char* argv[]);

    // List supported Vulkan targets.
    bool ListTargets();

    // List physical graphic adapters installed on the system.
    // If adapter #0 is an AMD GPU, isAmdGpu is set to true.
    // If shouldPrint is set to true, the list of adapters would be printed to stdout.
    bool ListAdapters(bool& isAmdGpu, bool shouldPrint = false);

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
    // The target names are returned in "targetNames" vector.
    bool GetTargetGPUNames(std::vector<std::string>& targetNames);

    // Verifies that the device supports required extensions.
    bool VerifyDevice(VkPhysicalDevice gpu);

    // Create Vulkan device object.
    // The device is specified by a pair {device_name, device_handle}.
    bool CreateDevice(VkPhysicalDevice gpu);

    // Get Vulkan device handle by target GPU name.
    // Returns NULL handle if device is not found.
    // If specified device name is empty, return handle of the 1st Vulkan device in the list.
    VkPhysicalDevice GetDeviceHandle(const std::string& deviceName);

    // Build for specified device name.
    // If device name is empty, build for the 1st physical device installed on the system.
    bool BuildForDevice(const std::string& deviceName);

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
    // The "pipelineName" specifies the name of Vulkan pipeline.
    // The pipeline itself is specified by "pipeline".
    bool BuildToBinary(const std::string& pipelineName, VkPipeline pipeline) const;

    // Build pipeline to disassembly.
    bool BuildToDisasm(const std::string& isaFileName, VkPipeline pipeline, VkShaderStageFlagBits stage) const;

    // Build pipeline to statistics.
    bool BuildToStatistics(const std::string& statsFileName, const std::string& pipelineName,
                           VkPipeline pipeline, VkShaderStageFlagBits stage) const;

    // Load a compute pipeline state file from disk.
    bool LoadComputePipelineStateFile(const std::string& psoFilePath);

    // Load a graphics pipeline state file from disk.
    bool LoadGraphicsPipelineStateFile(const std::string& psoFilePath);

    // Create a graphics pipeline.
    bool CreateGraphicsPipeline();

    // Create a compute pipeline.
    bool CreateComputePipeline();

    // Create a shader module from the given SPIR-V file path.
    bool CreateShaderModule(const std::string& spvFilePath, VkShaderModule& shaderModuleHandle, std::string& errorString);

    // Read the file at the given path into the given array of bytes.
    // Return an error string if there's a problem.
    bool ReadShaderBytes(const std::string& filePath, std::vector<char>& fileBytes, std::string& errorMessage);

    // -- Data --

    // Configuration.
    rgConfig                            m_config;

    // Vulkan instance.
    VkInstance                          m_instance = VK_NULL_HANDLE;

    // Supported AMD devices (map device_name --> Vk device handle).
    std::map<std::string,
             VkPhysicalDevice>          m_gpus;

    // Selected Vulkan device.
    VkDevice                            m_device = VK_NULL_HANDLE;

    // Vulkan validation report callback.
    VkDebugReportCallbackEXT            m_debugCallback = VK_NULL_HANDLE;

    // Pipeline: a pair {pipeline_name, VkPipeline handle}.
    std::pair<std::string, VkPipeline>  m_pipeline = {};

    // Pointer to vkGetShaderInfoAMD function.
    PFN_vkGetShaderInfoAMD              m_pFuncGetShaderInfo = nullptr;

    // Container to hold the available validation layers on this system.
    std::vector<const char*> m_availableValidationLayers;

    // Create Info for a graphics pipeline state object.
    rgPsoGraphicsVulkan*                m_pGraphicsPipelineCreateInfo = nullptr;

    // Create Info for a compute pipeline state object.
    rgPsoComputeVulkan*                 m_pComputePipelineCreateInfo = nullptr;

    // Output file for validation info.
    std::ofstream                       m_validationOutFile;

    // Specifies whether the Vulkan validation hint should be printed after dumping the error messages.
    bool                                m_printValidationHint = false;
};

