//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_OPENCL_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_OPENCL_H_

// C++.
#include <string>
#include <sstream>

// Infra.
#ifdef _WIN32
    #include "DXXModule.h"
#endif
#include "CALModule.h"
#include "CL/cl.h"
#include "OpenCLModule.h"
#include "ACLModule.h"

// Local.
#include "RadeonGPUAnalyzerBackend/Src/be_program_builder.h"
#include "RadeonGPUAnalyzerBackend/Src/be_opencl_definitions.h"

#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable : 4127)
    #pragma warning(disable : 4251)
#endif

using namespace std;
using namespace beKA;

class CElf;

class BeProgramBuilderOpencl : public BeProgramBuilder
{
public:
    virtual ~BeProgramBuilderOpencl(void);

    // Get list of Kernels for devices.
    beKA::beStatus GetKernels(const std::string& device, std::vector<std::string>& kernels);

    // Get a binary representation of the program. The binopts parameter lets you customize the output object.
    // If empty, the complete object is returned.
    virtual beKA::beStatus GetBinary(const std::string& device, const beKA::BinaryOptions& binopts, std::vector<char>& binary);

    // Get a binary representation of the program encoded in the given file. The binopts parameter lets you customize the output object.
    // If empty, the complete object is returned.
    virtual beKA::beStatus GetBinaryFromFile(const std::string& path_to_binary, const beKA::BinaryOptions& binopts, std::vector<char>& output_path);

    // Retrieve statistics for a given kernel for the given device.
    virtual beKA::beStatus GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis);

    // Get version information of the OpenCL runtime.
    std::string GetOpenclVersionInfo() const;

    // Get IL disassembly for the given kernel for the given device.
    virtual beKA::beStatus GetKernelIlText(const std::string& device, const std::string& kernel, std::string& il) override;

    // Get ISA disassembly for the given kernel for the given device.
    virtual beKA::beStatus GetKernelIsaText(const std::string& device, const std::string& kernel, std::string& isa) override;

#ifdef _RGA_DEBUG_IL_ENABLED
    // Get ISA DebugIL for the given kernel for the given device.
    virtual beKA::beStatus GetKernelDebugIlText(const std::string& device, const std::string& kernel, std::string& debugil);
#endif // _RGA_DEBUG_IL_ENABLED

    // Get kernel metadata for the given kernel for the given device.
    virtual beKA::beStatus GetKernelMetaDataText(const std::string& device, const std::string& kernel, std::string& metadata);

    // Compile the specified source file.
    beKA::beStatus Compile(const std::string& program_source, const OpenCLOptions& ocl_options, const std::string& source_code_full_path_name,
                           const std::vector<std::string>* source_path, int& num_of_successful_builds);

    // Get a set of available devices.
    virtual beKA::beStatus GetDevices(std::set<string>& devices);

    // Get a sorted table of devices.
    // The entries are arranged by Hardware Generation, CAL Name, Marketing Name and Device ID.
    // Because the table is compiled into the tool, old versions of the tool may not have have an incomplete table
    // with respect to the list of CAL names generated by GetDevices. Users of this table will want to be careful
    // to make any additional, new, devices available to the user.
    beKA::beStatus GetDeviceTable(std::vector<GDT_GfxCardInfo>& table) override;

    // Free the resources associated with a previous Compile.
    void ReleaseProgram();

    // Force ending of the thread in a safe way.
    void ForceEnd();

    // Retrieves the names of the supported public devices, as exposed by the OpenCL runtime.
    void GetSupportedPublicDevices(std::set<std::string>& devices) const;

    // Retrieves a mapping between device name and the Code Object disassembly.
    // This is only applicable for devices for whom the runtime compiler actually
    // generates a Code Object style binary.
    void GetDeviceToCodeObjectDisassemblyMapping(std::map<std::string, std::string>& mapping);

    // Returns true if a Code Object binary has been generated for the device.
    // The compiler may generate a Code Object binary for certain targets. During the compilation
    // process, if such binary is indeed generated, it is being stored.
    bool HasCodeObjectBinary(const std::string& device) const;

    // Extracts the statistics from a Code Object binary that has been generated for the device.
    // Returns true on success and false otherwise.
    bool ExtractStatisticsCodeObject(const std::string& device, std::map<std::string, beKA::AnalysisData>& stats);

protected:
    BeProgramBuilderOpencl();

    beKA::beStatus Initialize(const string& dll_module = "");

private:
    friend class Backend;

    // Set up the OpenCL part.
    beKA::beStatus InitializeOpencl();

    // Free resources used to set up OpenCL.
    beKA::beStatus DeinitializeOpenCL();

    // returns if the OpenCL Module was loaded
    bool isOpenClModuleLoaded();

    beKA::beStatus GetAnalysisInternal(cl_program& program, const std::string& device, const std::string& kernel, beKA::AnalysisData* analysis);

    // Get statistics parameter value for the kernel and device.
    // parameter_val is the pointer to memory where the appropriate result being queried is returned.
    // parameter_val_size is the size in bytes of memory pointed to by pParamVal.
    // parameter_name is the name of the parameter to query.
    beKA::beStatus  Inquire(void* parameter_val, size_t parameter_val_size, KernelInfoAMD parameter_name, cl_kernel kernel, cl_device_id device_id);

    // Get the text for the given kernel for the given device from the compiled binary.
    beKA::beStatus GetKernelSectionText(const std::string& device, const std::string& kernel_name, std::string& kernel_text);

    // An internal version of CompileOpenCL(). Here we actually do the compilation for a specific device.
    beKA::beStatus CompileOpenCLInternal(const std::string& source_code_full_path_name, const std::string& program_source, const OpenCLOptions& ocl_options,
                                         cl_device_id requested_device_id, cl_program& program, std::string& defines_and_options, int compilation_number, std::string& error_string);

    // Get a binary for an OpenCL kernel using for the given program.
    beKA::beStatus GetProgramBinary(cl_program& program, cl_device_id& device, std::vector<char>* binary_data);

    // Callback to get the ISA text line by line from CAL image binary object.
    ///
    // implicit param[out] s_ISAString Where the ISA is assembled.
    // \param   line                   The ISA text created by the runtime.
    static void CalLoggerFunc(const CALchar* line);

    // Disassembler callback function for ACLModule
    // \param msg disassembly text
    // \param size size of the message
    static void disassembleLogFunction(const char* msg, size_t size);

    bool BuildOpenCLProgramWrapper(
        cl_int&             status,
        cl_program          program,
        cl_uint             num_devices,
        const cl_device_id* device_list,
        const char*         options,
        void (CL_CALLBACK* pfn_notify)(cl_program program, void* user_data),
        void*               user_data);

    // Utility  functions to extract the OpenCL driver version out of the big string
    double GetOpenCLPlatformVersion();

    // Iterate through the device names that the OpenCL driver reported, and remove the names of devices that have not been published yet.
    // This is done only in the CodeXL public version. In CodeXL NDA and INTERNAL versions this function is no-op.
    void RemoveNamesOfUnpublishedDevices(const set<string>& unique_name_published_devices);

    // Internal auxiliary function that determines if for a given device the HSAIL
    // path would be used.
    bool DoesUseHsailPath(const std::string& device_name) const;

    // Force the string to use native line endings.
    // Linux will get LF.  Windows gets CR LF.
    // This makes notepad and other Windows tools happier.
    virtual void use_platform_native_line_endings(std::string& text);

    // The program binaries.
    std::vector<CElf*> elfs_;

    // Map from device name to cracked binary.
    std::map<std::string, CElf*> elf_map_;

    // Map from device to its compiled program binary.
    std::map<std::string, std::vector<char> > device_to_binary_;

    // Map Device and kernel and it's statistics data
    std::map<std::string, std::map<std::string, beKA::AnalysisData> > kernel_analysis_;

    // Interface with OpenCL.dll/libOpenCL.so
    OpenCLModule opencl_module_;

    // Handle to the ACL module.
    ACLModule* acl_module_ = nullptr;

    // Handle to the ACL compiler.
    aclCompiler* acl_compiler_ = nullptr;

    // Interface with aticalcl.dll/libaticalcl.so
    CALCLModule cal_cl_module_;

    // The OpenCL context used by this Backend.
    cl_context opencl_context_;

    // The number of OpenCL devices.
    size_t opencl_device_count_ = 0;

    // The OpenCL device IDs.
    std::vector<cl_device_id> opencl_device_ids_;

    // The sorted device table for OpenCL.
    std::vector<GDT_GfxCardInfo> opencl_device_table_;

    // A string to be used to report OpenCL version information.
    std::string opencl_version_info_;

    // Temporary used to construct ISA string from OpenCL/CAL callback.
    // This member shouldn't be static. To be handled in a future cleanup.
    static std::string* isa_string_;

    // Temporary used to construct IL string from OpenCL/CAL callback.
    // This member shouldn't be static. To be handled in a future cleanup.
    static std::string hsa_il_disassembly_;

    // Counter used for the disassemble callback. It is being used by the callback
    // to differentiate between ISA and HSAIL disassembly.
    static size_t disassemble_callback_counter_;

    // Stream for diagnostic output.
    LoggingCallBackFuncP log_callback_ = nullptr;

    // The device names.
    std::set<string> device_names_;

    // Map from OpenCL device ID to OpenCL device name.
    std::map<cl_device_id, string>   device_id_to_name_;

    // Map from OpenCL device name to OpenCL device ID.
    std::map<string, cl_device_id>   name_to_device_id_;

    // Map from OpenCL device name to device type.
    std::map<string, cl_device_type> name_device_type_map_;

    // Map from device name to ISA Code Object disassembly.
    std::map<string, std::string> device_to_code_object_disassembly_isa_;

    // Map from device name to whole Code Object disassembly.
    std::map<string, std::string> device_to_code_object_disassembly_whole_;

    // Driver version string.
    std::string driver_version_;

    // Flag set to true if the object is initialized, otherwise false.
    bool is_initialized_ = false;

    // Flag set to if a fatal error happened which requires us to
    // stop further processing, otherwise false.
    bool should_force_ending_ = false;

    // Flag set to true if we are in legacy mode, otherwise set to false.
    bool is_legacy_mode_ = false;
};

#ifdef _WIN32
    #pragma warning(pop)
#endif

#endif // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_OPENCL_H_
