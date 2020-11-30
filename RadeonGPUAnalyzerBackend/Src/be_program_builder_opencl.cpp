//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <locale>

// Infra.
#include "CElf.h"
#include "ACLModuleManager.h"
#include "DeviceInfoUtils.h"

// Local.
#include "Src/be_program_builder_opencl.h"
#include "Emulator/Parser/be_isa_parser.h"
#include "RadeonGPUAnalyzerBackend/Src/be_utils.h"

// CLI.
#include "RadeonGPUAnalyzerCLI/Src/kc_utils.h"

#define CL_STATUS_TABLE \
    X(CL_SUCCESS) \
    X(CL_DEVICE_NOT_FOUND) \
    X(CL_DEVICE_NOT_AVAILABLE) \
    X(CL_COMPILER_NOT_AVAILABLE) \
    X(CL_MEM_OBJECT_ALLOCATION_FAILURE) \
    X(CL_OUT_OF_RESOURCES) \
    X(CL_OUT_OF_HOST_MEMORY) \
    X(CL_PROFILING_INFO_NOT_AVAILABLE) \
    X(CL_MEM_COPY_OVERLAP) \
    X(CL_IMAGE_FORMAT_MISMATCH) \
    X(CL_IMAGE_FORMAT_NOT_SUPPORTED) \
    X(CL_BUILD_PROGRAM_FAILURE) \
    X(CL_MAP_FAILURE) \
    X(CL_MISALIGNED_SUB_BUFFER_OFFSET) \
    X(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST) \
    X(CL_COMPILE_PROGRAM_FAILURE) \
    X(CL_LINKER_NOT_AVAILABLE) \
    X(CL_LINK_PROGRAM_FAILURE) \
    X(CL_DEVICE_PARTITION_FAILED) \
    X(CL_KERNEL_ARG_INFO_NOT_AVAILABLE) \
    X(CL_INVALID_VALUE) \
    X(CL_INVALID_DEVICE_TYPE) \
    X(CL_INVALID_PLATFORM) \
    X(CL_INVALID_DEVICE) \
    X(CL_INVALID_CONTEXT) \
    X(CL_INVALID_QUEUE_PROPERTIES) \
    X(CL_INVALID_COMMAND_QUEUE) \
    X(CL_INVALID_HOST_PTR) \
    X(CL_INVALID_MEM_OBJECT) \
    X(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR) \
    X(CL_INVALID_IMAGE_SIZE) \
    X(CL_INVALID_SAMPLER) \
    X(CL_INVALID_BINARY) \
    X(CL_INVALID_BUILD_OPTIONS) \
    X(CL_INVALID_PROGRAM) \
    X(CL_INVALID_PROGRAM_EXECUTABLE) \
    X(CL_INVALID_KERNEL_NAME) \
    X(CL_INVALID_KERNEL_DEFINITION) \
    X(CL_INVALID_KERNEL) \
    X(CL_INVALID_ARG_INDEX) \
    X(CL_INVALID_ARG_VALUE) \
    X(CL_INVALID_ARG_SIZE) \
    X(CL_INVALID_KERNEL_ARGS) \
    X(CL_INVALID_WORK_DIMENSION) \
    X(CL_INVALID_WORK_GROUP_SIZE) \
    X(CL_INVALID_WORK_ITEM_SIZE) \
    X(CL_INVALID_GLOBAL_OFFSET) \
    X(CL_INVALID_EVENT_WAIT_LIST) \
    X(CL_INVALID_EVENT) \
    X(CL_INVALID_OPERATION) \
    X(CL_INVALID_GL_OBJECT) \
    X(CL_INVALID_BUFFER_SIZE) \
    X(CL_INVALID_MIP_LEVEL) \
    X(CL_INVALID_GLOBAL_WORK_SIZE) \
    X(CL_INVALID_PROPERTY) \
    X(CL_INVALID_IMAGE_DESCRIPTOR) \
    X(CL_INVALID_COMPILER_OPTIONS) \
    X(CL_INVALID_LINKER_OPTIONS) \
    X(CL_INVALID_DEVICE_PARTITION_COUNT) \
    X(CL_PLATFORM_NOT_FOUND_KHR) \
    X(CL_DEVICE_PARTITION_FAILED_EXT) \
    X(CL_INVALID_PARTITION_COUNT_EXT) \
    X(CL_INVALID_PARTITION_NAME_EXT)

// Constants: error messages.
static const char* kStrErrorKernelSymbolNotFound = "Error: failed to locate kernel symbol in ELF container for device: ";
static const char* kStrErrorFailedToExtractCodeObjectStats = "Error: failed to extract statistics from Code Object for ";

static std::string GetCLErrorString(cl_int error)
{
    switch (error)
    {
#define X(S) case S: return #S;
            CL_STATUS_TABLE;
#undef X

        default: return ("UNKNOWN");
    }
}

// Predicate for characters we want to allow through.
static bool IsNotNormalChar(char c)
{
    std::locale loc;
    return !std::isprint(c, loc) && !std::isspace(c, loc);
}

std::string* BeProgramBuilderOpencl::isa_string_ = NULL;
std::string BeProgramBuilderOpencl::hsa_il_disassembly_;
size_t BeProgramBuilderOpencl::disassemble_callback_counter_ = 0;

// Filter buildLog of temporary OpenCL file names.
static std::string PostProcessOpenclBuildLog(const std::string& build_log, const std::string& source_code_full_path)
{
    std::istringstream in(build_log);
    std::string out;

    // So that we don't go nuts with reallocations...
    out.reserve(build_log.size());

    do
    {
        std::string line;
        getline(in, line);

        // Adjust lines of the form:
        //   PATH_TO_FILE\OCL123.tmp.cl(123):message.
        // to:
        //   PATH_TO_FILE\file.cl, line 123:message.
        const char suffix[] = ".tmp.cl(";
        std::string::size_type lpos = line.find(suffix);
        std::string::size_type rpos = line.find("):", lpos);

        if (lpos != std::string::npos &&
            rpos != std::string::npos)
        {
            std::string lineNum = line.substr(lpos + sizeof(suffix) - 1, rpos - lpos - sizeof(suffix) + 1);
            line.replace(line.begin(), line.begin() + rpos + 2, ", line " + lineNum + ":");
            line = source_code_full_path + line;
        }

        // OpenCL started doing this about the time of Catalyst 12.4.
        // Adjust lines of the form:
        //   "X:\Users\name\AppData\Local\Temp\OCL456.tmp.cl", line 123:message
        // Also adjusting lines of the form:
        //   "/tmp/OCL456.cl", line 123:message.
        // to:
        //   PATH_TO_FILE\file.cl, line 123:message

        const char suffix2[] = ", line";
        std::string::size_type pos = line.find(suffix2);

        if (pos != std::string::npos)
        {
            line = line.substr(pos);
            line = source_code_full_path + line;
        }

        // If for some reason sourceCodeFullPathName is empty, prevent error message from starting with ", line 123:..."
        // change it to "Line 123:.."
        pos = line.find(suffix2);

        if (pos == 0)
        {
            line.replace(pos, sizeof(suffix2) / sizeof(char), "Line");
        }

        // Chop-off summary message at the end.
        // It also contains the temporary file name.
        pos = line.find(" detected in the compilation of ");

        if (pos != std::string::npos)
        {
            line.replace(pos, std::string::npos, " detected.");
        }

        // Get rid of nasty unprintable characters.
        // Replace them with spaces.
        replace_if(line.begin(), line.end(), IsNotNormalChar, ' ');

        out += line + '\n';
    }
    while (!in.eof());

    // We might have added an extra \n above.
    // This happens when the buildLog already ends in \n.
    // If we find two \n's in a row, assume this happened and remove the extra.
    if (out.size() > 1 &&
        out[out.size() - 1] == '\n' &&
        out[out.size() - 2] == '\n')
    {
        out.resize(out.size() - 1);
    }

    return out;
}

BeProgramBuilderOpencl::BeProgramBuilderOpencl() :
    opencl_module_(OpenCLModule::s_DefaultModuleName),
    cal_cl_module_(CALCLModule::s_DefaultModuleName)
{
}

beKA::beStatus BeProgramBuilderOpencl::Initialize(const string& dll_module/* = ""*/)
{
    (void)(dll_module);

    beKA::beStatus retVal = beKA::kBeStatusGeneralFailed;

    retVal = InitializeOpencl();
    std::set<string> unique_published_devices_names;

    if (retVal == beKA::kBeStatusSuccess)
    {
        // Populate the sorted device (card) info table.
        BeUtils::GetAllGraphicsCards(opencl_device_table_,unique_published_devices_names);
    }

    return retVal;
}

beKA::beStatus BeProgramBuilderOpencl::InitializeOpencl()
{
    beKA::beStatus ret_val = beKA::kBeStatusSuccess;

    // Check that the DLLs that we use are present.
    // Also check that the function that we use are there. this check is done here since LoadModule do not verify this.
    if (!isOpenClModuleLoaded())
    {
        return beKA::kBeStatusOpenclModuleNotLoaded;
    }

    if (opencl_module_.OpenCLLoaded() == OpenCLModule::OpenCL_1_0)
    {
        std::stringstream ss;
        ss << "OpenCL Error: OpenCL V1.1 or later required." << endl;
        LogCallback(ss.str());

        return beKA::kBeStatusOpenclModuleNotSupported;
    }

    // Continue with initialization.
    ret_val = beKA::kBeStatusSuccess;
    cl_uint num_platforms = 0;
    cl_int status;

    if (ret_val == beKA::kBeStatusSuccess)
    {
        // Get all the platforms and then pick the AMD one.
        status = opencl_module_.GetPlatformIDs(0, NULL, &num_platforms);

        if (CL_SUCCESS != status || num_platforms == 0)
        {
            std::stringstream ss;
            ss << "Offline compilation of OpenCL code is currently not supported on this platform." << endl;
            LogCallback(ss.str());
            ret_val = beKA::kBeStatusOpenclGetPlatformIDsFailed;
        }
        else
        {
            ret_val = beKA::kBeStatusSuccess;
        }
    }

    if (ret_val == beKA::kBeStatusSuccess)
    {
        // Find the AMD platform.
        cl_platform_id platform = NULL;

        if (num_platforms != 0)
        {
            std::vector<cl_platform_id>platforms;
            platforms.resize(num_platforms);

            status = opencl_module_.GetPlatformIDs(num_platforms, &platforms[0], NULL);

            if (CL_SUCCESS != status)
            {
                std::stringstream ss;
                ss << "OpenCL Error: clGetPlatformIDs failed (" + GetCLErrorString(status) + ")." << endl;
                LogCallback(ss.str());
                ret_val = beKA::kBeStatusOpenclGetPlatformIDsFailed;
            }

            char platform_name[256];
            for (unsigned int i = 0; i < num_platforms; ++i)
            {
                status = opencl_module_.GetPlatformInfo(
                             platforms[i],
                             CL_PLATFORM_VENDOR,
                             sizeof(platform_name),
                             platform_name,
                             NULL);

                if (CL_SUCCESS != status)
                {
                    std::stringstream ss;
                    ss << "OpenCL Error: clGetPlatformIDs failed (" + GetCLErrorString(status) + ")." << endl;
                    LogCallback(ss.str());

                    ret_val = beKA::kBeStatusOpenclGetPlatformInfoFailed;
                }

                if (0 == strcmp(platform_name, "Advanced Micro Devices, Inc."))
                {
                    // found AMD platform
                    platform = platforms[i];
                    break;
                }
            }
        }

        if (NULL == platform)
        {
            std::stringstream ss;
            ss << "OpenCL Error: Can't find AMD platform.\n";
            LogCallback(ss.str());

            ret_val = beKA::kBeStatusNoOpenclAmdPlatform;
        }

        if (ret_val == beKA::kBeStatusSuccess)
        {
            // Set up the extension functions now that we have a platform.
            opencl_module_.LoadOpenCLExtensions(platform);

            // Create a context including Offline Devices.
            cl_context_properties properties[] =
            {
                CL_CONTEXT_PLATFORM,
                (cl_context_properties)platform,
                CL_CONTEXT_OFFLINE_DEVICES_AMD,
                (cl_context_properties)1,
                (cl_context_properties)NULL,
                (cl_context_properties)NULL
            };

            opencl_context_ = opencl_module_.CreateContextFromType(properties, CL_DEVICE_TYPE_ALL, NULL, NULL, &status);

            if (CL_SUCCESS != status)
            {
                std::stringstream ss;
                ss << "OpenCL Error: clCreateContextFromType failed (" + GetCLErrorString(status) + ")." << endl;
                LogCallback(ss.str());

                ret_val = beKA::kBeStatusOpenclCreateContextFromTypeFailed;
            }
        }

        size_t context_info_device_size = 0;
        if (ret_val == beKA::kBeStatusSuccess)
        {
            // Compute the number of devices
            status = opencl_module_.GetContextInfo(opencl_context_, CL_CONTEXT_DEVICES, 0, NULL, &context_info_device_size);
            if (CL_SUCCESS != status)
            {
                std::stringstream ss;
                ss << "OpenCL Error: clGetContextInfo CL_CONTEXT_DEVICES failed (" + GetCLErrorString(status) + ")." << endl;
                LogCallback(ss.str());
                ret_val = beKA::kBeStatusOpenclGetContextInfoFailed;
            }
        }

        if (ret_val == beKA::kBeStatusSuccess)
        {
            opencl_device_count_ = context_info_device_size / sizeof(cl_device_id);

            // Get the devices
            opencl_device_ids_.resize(opencl_device_count_);
            status = opencl_module_.GetContextInfo(
                         opencl_context_,
                         CL_CONTEXT_DEVICES,
                         opencl_device_count_ * sizeof(cl_device_id),
                         &opencl_device_ids_[0],
                         NULL);

            if (CL_SUCCESS != status)
            {
                std::stringstream ss;
                ss << "OpenCL Error: clGetContextInfo CL_CONTEXT_DEVICES failed (" + GetCLErrorString(status) + ")." << endl;
                LogCallback(ss.str());
                ret_val = beKA::kBeStatusOpenclGetContextInfoFailed;
            }
        }

        if (ret_val == beKA::kBeStatusSuccess)
        {
            // Get the list of official OpenCL/CAL device names.
            for (size_t i = 0; i < opencl_device_count_; ++i)
            {
                char asic_name[256];
                status = opencl_module_.GetDeviceInfo(opencl_device_ids_[i], CL_DEVICE_NAME, sizeof(asic_name), asic_name, NULL);
                if (CL_SUCCESS != status)
                {
                    std::stringstream ss;
                    ss << "OpenCL Error: clGetContextInfo CL_DEVICE_NAME failed (" + GetCLErrorString(status) + ")." << endl;
                    LogCallback(ss.str());
                    ret_val = beKA::kBeStatusOpenclGetDeviceInfoFailed;
                    break;
                }

                cl_device_type device_type;
                status = opencl_module_.GetDeviceInfo(opencl_device_ids_[i], CL_DEVICE_TYPE, sizeof(cl_device_type), &device_type, NULL);

                if (CL_SUCCESS != status)
                {
                    std::stringstream ss;
                    ss << "OpenCL Error: clGetContextInfo CL_DEVICE_TYPE failed (" + GetCLErrorString(status) + ")." << endl;
                    LogCallback(ss.str());
                    ret_val = beKA::kBeStatusOpenclGetDeviceInfoFailed;
                    break;
                }

                if (device_type != CL_DEVICE_TYPE_CPU)
                {
                    device_names_.insert(string(asic_name));
                    device_id_to_name_[opencl_device_ids_[i]] = string(asic_name);
                    name_to_device_id_[string(asic_name)] = opencl_device_ids_[i];
                    name_device_type_map_[string(asic_name)] = device_type;
                }
            }
        }

        // Create a string for m_OpenCLVersionInfo.
        if (ret_val == beKA::kBeStatusSuccess)
        {
            size_t version_legnth = 0;
            status = opencl_module_.GetPlatformInfo(
                         platform,
                         CL_PLATFORM_VERSION,
                         0,
                         NULL,
                         &version_legnth);

            if (CL_SUCCESS != status)
            {
                std::stringstream ss;
                ss << "OpenCL Error: clGetPlatformInfo failed (" + GetCLErrorString(status) + ")." << endl;
                LogCallback(ss.str());
            }
            else
            {
                char* version_size = new char[version_legnth];
                status = opencl_module_.GetPlatformInfo(
                             platform,
                             CL_PLATFORM_VERSION,
                             version_legnth,
                             version_size,
                             NULL);

                if (CL_SUCCESS != status)
                {
                    std::stringstream ss;
                    ss << "OpenCL Error: clGetPlatformInfo failed (" + GetCLErrorString(status) + ")." << endl;
                    LogCallback(ss.str());
                }
                else
                {
                    // OpenCL 1.1 AMD-APP (898.1)
                    opencl_version_info_ = string(version_size);
                }
            }
        }
    }

    if (ret_val == beKA::kBeStatusSuccess)
    {
        is_initialized_ = true;
        opencl_version_info_ += string("\nGraphics Driver Version: ") + driver_version_;
    }

    return ret_val;
}

bool BeProgramBuilderOpencl::isOpenClModuleLoaded()
{
    bool retVal = true;

    if ((opencl_module_.OpenCLLoaded() == OpenCLModule::OpenCL_None) ||
        ((opencl_module_.GetPlatformIDs == NULL) || (opencl_module_.GetPlatformInfo == NULL) || (opencl_module_.CreateContextFromType == NULL) ||
         (opencl_module_.GetContextInfo == NULL) || (opencl_module_.GetDeviceInfo == NULL) || (opencl_module_.ReleaseContext == NULL) || (opencl_module_.CreateKernel == NULL) ||
         (opencl_module_.GetKernelWorkGroupInfo == NULL) || (opencl_module_.ReleaseKernel == NULL) ||(opencl_module_.CreateProgramWithSource == NULL) ||
         (opencl_module_.GetProgramBuildInfo == NULL) || (opencl_module_.GetProgramInfo == NULL) || (opencl_module_.GetProgramInfo == NULL)))
    {
        retVal = false;
    }

    return retVal;
}

beKA::beStatus BeProgramBuilderOpencl::GetKernels(const std::string& device, std::vector<std::string>& kernels)
{
    beKA::beStatus retVal = beKA::kBeStatusSuccess;
    if (!is_initialized_)
    {
        return kBeStatusOpenclModuleNotLoaded;
    }

    if (elf_map_.count(device) == 0)
    {
        std::stringstream ss;
        ss << "Error: No binary for device \'" << device << "\'.\n";
        LogCallback(ss.str());

        retVal = beKA::kBeStatusNoBinaryForDevice;
    }

    if (retVal == beKA::kBeStatusSuccess)
    {
        const CElf& elf = *elf_map_[device];
        if (elf_map_[device] == nullptr)
        {
            retVal = beKA::kBeStatusNoBinaryForDevice;
        }

        if (nullptr != elf.GetSymbolTable())
        {
            const CElfSymbolTable& symtab = *elf.GetSymbolTable();
            kernels.clear();

            // Look for symbols of the form <kernelPrefix><name><kernelSuffix>.
            std::string kernel_suffix("_kernel");
            const string kernel_prefix("__OpenCL_");
            const string kernel_prefix_hsail("__OpenCL_&__OpenCL_");
            const string kernel_suffix_hsail("_kernel_metadata");

            // Apparently there is a new variant for the format of the ELF symbol name,
            // when AMDIL compilation path is used.
            const std::string kKernelSuffixAmdilAlternative("_binary");
            const std::string kKernelPrefixAmdilAlternative("__ISA_");

            if (elf.GetSection(".text") == nullptr)
            {
                if (elf.GetSection(".amdil") != nullptr)
                {
                    kernel_suffix = "_amdil";
                }
                else if (elf.GetSection(".rodata") != nullptr)
                {
                    kernel_suffix = "_metadata";
                }
            }

            for (CElfSymbolTable::const_SymbolIterator sym = symtab.SymbolsBegin(); sym != symtab.SymbolsEnd(); ++sym)
            {
                std::string name;
                unsigned char bind;
                unsigned char type;
                unsigned char other;
                CElfSection* section;
                Elf64_Addr value;
                Elf64_Xword  size;

                symtab.GetInfo(sym, &name, &bind, &type, &other, &section, &value, &size);

                if (name.substr(0, kernel_prefix.size()) == kernel_prefix &&
                    name.substr(name.length() - kernel_suffix.size(), kernel_suffix.size()) == kernel_suffix)
                {
                    // Pick out the meaty goodness that is the kernel name.
                    kernels.push_back(name.substr(kernel_prefix.size(), name.length() - kernel_prefix.size() - kernel_suffix.size()));
                }
                else if (name.substr(0, kernel_prefix_hsail.size()) == kernel_prefix_hsail &&
                         name.substr(name.length() - kernel_suffix_hsail.size(), kernel_suffix_hsail.size()) == kernel_suffix_hsail)
                {
                    // Fallback to the HSAIL (BIF) scenario.
                    kernels.push_back(name.substr(kernel_prefix_hsail.size(), name.length() - kernel_prefix_hsail.size() - kernel_suffix_hsail.size()));
                }
                else if (name.substr(0, kKernelPrefixAmdilAlternative.size()) == kKernelPrefixAmdilAlternative &&
                    name.substr(name.length() - kKernelSuffixAmdilAlternative.size(), kKernelSuffixAmdilAlternative.size()) == kKernelSuffixAmdilAlternative)
                {
                    // Fallback to using the alternative symbol names for AMDIL.
                    kernels.push_back(name.substr(kKernelPrefixAmdilAlternative.size(), name.length() - kKernelPrefixAmdilAlternative.size() - kKernelSuffixAmdilAlternative.size()));
                }
            }
        }
        else
        {
            retVal = beKA::kBeStatusNoBinaryForDevice;
        }

        if (retVal == beKA::kBeStatusSuccess && kernels.empty())
        {
            retVal = kBeStatusGeneralFailed;
        }
    }

    return retVal;
}

beKA::beStatus BeProgramBuilderOpencl::GetBinary(const std::string& device, const beKA::BinaryOptions& binopts, std::vector<char>& binary)
{
    beKA::beStatus retVal = beKA::kBeStatusSuccess;
    if (!is_initialized_)
    {
        return kBeStatusOpenclModuleNotLoaded;
    }

    if (elf_map_.count(device) == 0)
    {
        std::stringstream ss;
        ss << "No binary for device \'" << device << "\'.\n";
        LogCallback(ss.str());

        retVal = beKA::kBeStatusNoBinaryForDevice;
    }
    else
    {
        CElf& elf = *elf_map_[device];
        if (binopts.suppress_section.size() == 0)
        {
            elf.Store(&binary);
            return beKA::kBeStatusSuccess;
        }

        // Make a copy so that we can abuse it.
        if (retVal == beKA::kBeStatusSuccess)
        {
            CElf elfCopy(elf);

            for (std::vector<string>::const_iterator it = binopts.suppress_section.begin();
                 it != binopts.suppress_section.end();
                 ++it)
            {
                elfCopy.RemoveSection(*it);
            }

            elfCopy.Store(&binary);
        }
    }

    return retVal;

}

beKA::beStatus BeProgramBuilderOpencl::GetBinaryFromFile(const std::string& path_to_binary, const beKA::BinaryOptions& binopts, std::vector<char>& output_path)
{
    beKA::beStatus ret_val = beKA::kBeStatusSuccess;
    CElf elf(path_to_binary);
    if (elf.good())
    {
        if (binopts.suppress_section.size() == 0)
        {
            elf.Store(&output_path);
        }
        else
        {
            for (std::vector<string>::const_iterator it = binopts.suppress_section.begin();
                 it != binopts.suppress_section.end();
                 ++it)
            {
                elf.RemoveSection(*it);
            }

            elf.Store(&output_path);
        }
    }
    else
    {
        ret_val = beKA::kBeStatusNoBinaryForDevice;
    }

    return ret_val;
}

beKA::beStatus BeProgramBuilderOpencl::GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis)
{
    beKA::beStatus ret = kBeStatusGeneralFailed;
    auto iter = device_to_code_object_disassembly_whole_.find(device);
    bool is_code_object_target = iter != device_to_code_object_disassembly_whole_.end();
    if (!is_code_object_target)
    {
        if (!is_initialized_)
        {
            return kBeStatusOpenclModuleNotLoaded;
        }

        if (kernel_analysis_.find(device) != kernel_analysis_.end())
        {
            std::map<std::string, beKA::AnalysisData> kernel_analysis;
            kernel_analysis = kernel_analysis_[device];
            std::map<std::string, beKA::AnalysisData>::const_iterator iter = kernel_analysis.find(kernel);
            if (iter != kernel_analysis.end())
            {
                analysis = kernel_analysis[kernel];
                if (analysis.isa_size == 0)
                {
                    std::string isa_text;
                    ret = GetKernelIsaText(device, kernel, isa_text);
                    if (ret == kBeStatusSuccess)
                    {
                        ParserIsa isa_parser;
                        bool is_prase_success = isa_parser.ParseForSize(isa_text);
                        if (is_prase_success)
                        {
                            analysis.isa_size = isa_parser.GetCodeLength();
                        }
                    }
                }
                ret = beKA::kBeStatusSuccess;
            }
            else
            {
                ret = beKA::kBeStatusWrongKernelName;
            }
        }
        else
        {
            ret = beKA::kBeStatusNoDeviceFound;
        }
    }
    else
    {
        // Assume success if not relevant.
        ret = kBeStatusSuccess;
    }
    return ret;
}

void BeProgramBuilderOpencl::ReleaseProgram()
{
    for (size_t i = 0; i < elfs_.size(); ++i)
    {
        delete elfs_[i];
    }

    elf_map_.clear();

    for (size_t i = 0; i < device_to_binary_.size(); ++i)
    {
        device_to_binary_.clear();
    }

    device_to_binary_.clear();

    kernel_analysis_.clear();
    elfs_.clear();
}

std::string BeProgramBuilderOpencl::GetOpenclVersionInfo() const
{
    return opencl_version_info_;
}

beKA::beStatus BeProgramBuilderOpencl::DeinitializeOpenCL()
{
    // If TSingleton does lazy loading (and DeleteInstance actually calls a dtor),
    // we want to be tidy and release the context.
#if defined(_WIN32)
    if (NULL != opencl_module_.ReleaseContext)
    {
        opencl_module_.ReleaseContext(opencl_context_);
    }

#else
    // If our singleton is a non-lazy global,
    // then the memory for m_OpenCLContext might be deallocated before this dtor is called.
    // In that case we cannot cleanup.
#endif

    opencl_context_ = NULL;

    return beKA::kBeStatusSuccess;
}

beKA::beStatus BeProgramBuilderOpencl::GetAnalysisInternal(cl_program& program, const std::string& device,
    const std::string& kernel, beKA::AnalysisData* analysis)
{
    beKA::beStatus ret = beKA::kBeStatusSuccess;
    std::string kernel_name;
    std::string kernel_name_alternative;

    bool does_use_hasil = !is_legacy_mode_ && DoesUseHsailPath(device);
    const size_t kNumOfAttempts = does_use_hasil ? 2 : 1;
    for (size_t i = 0; i < kNumOfAttempts; i++)
    {
        if (does_use_hasil && i > 0)
        {
            // If we haven't succeeded with the HSAIL path, fallback to the AMDIL path.
            does_use_hasil = false;
        }

        if (does_use_hasil)
        {
            kernel_name = "&__OpenCL_" + kernel + "_kernel";
            kernel_name_alternative = kernel;
        }
        else
        {
            kernel_name = kernel;
            kernel_name_alternative = "&__OpenCL_" + kernel + "_kernel";
        }

        cl_device_id device_id = name_to_device_id_[device];
        cl_int status;
        cl_kernel kernel_cl = opencl_module_.CreateKernel(program, kernel_name.c_str(), &status);

        if (status == CL_INVALID_KERNEL_NAME)
        {
            // Retry with the alternative kernel name.
            kernel_cl = opencl_module_.CreateKernel(program, kernel_name_alternative.c_str(), &status);
        }

        if (status == CL_SUCCESS)
        {

#define X(FIELD,PARAM) Inquire(&FIELD,sizeof(FIELD),PARAM,kernel_cl,device_id);
            X(analysis->scratch_memory_used, CL_KERNELINFO_SCRATCH_REGS)
            X(analysis->wavefront_count_per_simd, CL_KERNELINFO_WAVEFRONT_PER_SIMD)
            X(analysis->wavefront_size, CL_KERNELINFO_WAVEFRONT_SIZE)
            X(analysis->num_gprs_available, CL_KERNELINFO_AVAILABLE_GPRS)
            X(analysis->num_gprs_used, CL_KERNELINFO_USED_GPRS)
            X(analysis->lds_size_available, CL_KERNELINFO_AVAILABLE_LDS_SIZE)
            X(analysis->lds_size_used, CL_KERNELINFO_USED_LDS_SIZE)
            X(analysis->stack_size_available, CL_KERNELINFO_AVAILABLE_STACK_SIZE)
            X(analysis->stack_size_used, CL_KERNELINFO_USED_STACK_SIZE)
            X(analysis->num_sgprs_available, CL_KERNELINFO_AVAILABLE_SGPRS)
            X(analysis->num_sgprs_used, CL_KERNELINFO_USED_SGPRS)
            X(analysis->num_vgprs_available, CL_KERNELINFO_AVAILABLE_VGPRS)
            X(analysis->num_vgprs_used, CL_KERNELINFO_USED_VGPRS)
#undef X

            // Get the clGetKernelWorkGroupInfo
            size_t compiler_workgroup_size[3];
            status = opencl_module_.GetKernelWorkGroupInfo(kernel_cl, device_id, CL_KERNEL_COMPILE_WORK_GROUP_SIZE, sizeof(size_t) * 3, compiler_workgroup_size, NULL);

            if (CL_SUCCESS == status)
            {
                analysis->num_threads_per_group_x = compiler_workgroup_size[0];
                analysis->num_threads_per_group_y = compiler_workgroup_size[1];
                analysis->num_threads_per_group_z = compiler_workgroup_size[2];
            }

            // TODO: Add code to gather the other info clGetKernelWorkGroupInfo provides.
            opencl_module_.ReleaseKernel(kernel_cl);

            // SI reports no numGPRs.
            // pre-SI reports no numVGPRs nor numSGPRs.
            // To avoid confusing the user, copy the pre-SI GPR numbers to VGPR.
            // They are vector registers ...  the scalars are new with SI.
            if (analysis->num_vgprs_available == 0 && analysis->num_sgprs_available == 0)
            {
                analysis->num_vgprs_available = analysis->num_gprs_available;
                analysis->num_vgprs_used = analysis->num_gprs_used;

                analysis->num_sgprs_available = beKA::kCalValue64Na;
                analysis->num_sgprs_used = beKA::kCalValue64Na;
            }

            // No further attempts are required.
            break;
        }
        else if (i == (kNumOfAttempts - 1))
        {
            // Try to extract the statistics from the textual ISA.
            // This should at least give us basic info for until the HSAIL-path
            // statistics generation issue is fixed.
            std::string isa;
            beKA::beStatus rc =  GetKernelIsaText(device, kernel, isa);
            bool were_stats_extracted = false;
            if (rc == kBeStatusSuccess)
            {
                were_stats_extracted = ParserIsa::ParseHsailStatistics(isa, *analysis);
            }

            if (!were_stats_extracted)
            {
                ret = beKA::kBeStatusNoStatisticsForDevice;
                stringstream ss;
                ss << "Error: No statistics for \'" << device << "\'.\n";
                LogCallback(ss.str());
            }
        }
    }

    return ret;
}

#ifdef _RGA_DEBUG_IL_ENABLED
beKA::beStatus BeProgramBuilderOpencl::GetKernelDebugIlText(const std::string& device, const std::string& kernel, std::string& debugil)
{
    (void)(&kernel);
    beKA::beStatus ret_val = beKA::kBeStatusSuccess;

    if (!is_initialized_)
    {
        return kBeStatusOpenclModuleNotLoaded;
    }

    if (elf_map_.count(device) == 0)
    {
        stringstream ss;
        ss << "Error: No DebugIL for device \'" << device << "\'.\n";
        LogCallback(ss.str());

        ret_val = beKA::kBeStatusNoDebugIlForDevice;
    }

    if (name_device_type_map_[device] == CL_DEVICE_TYPE_CPU)
    {
        stringstream ss;
        ss << "Warning: No DebugIL for CPU device \'" << device << "\'.\n";
        LogCallback(ss.str());
        ret_val = beKA::kBeStatusNoDebugIlForDevice;
    }

    if (ret_val == beKA::kBeStatusSuccess)
    {
        const CElf& elf = *elf_map_[device];
        const CElfSection* debug_il_section = elf.GetSection(".debugil");
        if (debug_il_section == NULL)
        {
            stringstream ss;
            ss << "Error: no .debugil section found.  Did you compile with '-g'?\n";
            LogCallback(ss.str());
            ret_val = kBeStatusNoDebugIlForDevice;
        }
        else
        {
            const vector<char> data(debug_il_section->GetData());
            debugil = string(data.begin(), data.end());
        }
    }

    return ret_val;
}
#endif // _RGA_DEBUG_IL_ENABLED

beKA::beStatus BeProgramBuilderOpencl::GetKernelMetaDataText(const std::string& device, const std::string& kernel, std::string& metadata)
{
    beKA::beStatus ret = kBeStatusNoMetadataForDevice;
    std::string amdil_name;
    amdil_name = "__OpenCL_" + kernel + "_metadata";

    // Basically, for HSAIL we should use the following symbol name
    // for extracting the metadata section: "__OpenCL_&__OpenCL_" + kernel + "_kernel_metadata",
    // but it seems like we are unable to parse it.
    bool is_hsail = !is_legacy_mode_ && DoesUseHsailPath(device);

    if (!is_hsail)
    {
        ret = GetKernelSectionText(device, amdil_name, metadata);
    }

    return ret;
}


beKA::beStatus BeProgramBuilderOpencl::GetKernelIlText(const std::string& device, const std::string& kernel_name, std::string& kernel_text)
{
    beKA::beStatus ret = kBeStatusGeneralFailed;
    kernel_text.clear();

    bool is_hsail = !is_legacy_mode_ && DoesUseHsailPath(device);
    const size_t kNumAttempts = is_hsail ? 2 : 1;
    for (size_t i = 0; i < kNumAttempts; i++)
    {
        if (is_hsail && i > 0)
        {
            // If we haven't succeeded with the HSAIL path, fallback to the AMDIL path.
            is_hsail = false;
        }

        if (is_hsail && !hsa_il_disassembly_.empty())
        {
            kernel_text = hsa_il_disassembly_;
            ret = beKA::kBeStatusSuccess;

            // No further attempts are required.
            break;
        }
        else
        {
            std::string amdil_name;
            amdil_name = "__OpenCL_" + kernel_name + "_amdil";
            ret = GetKernelSectionText(device, amdil_name, kernel_text);

            if (ret != beKA::kBeStatusSuccess)
            {
                // Retry with an alternative name.
                amdil_name = "__AMDIL_" + kernel_name + "_text";
                ret = GetKernelSectionText(device, amdil_name, kernel_text);

                if (ret != beKA::kBeStatusSuccess && name_device_type_map_[device] == CL_DEVICE_TYPE_GPU)
                {
                    // Inform the user about the failure. CPUs are ignored.
                    const char* kStrLegacyClNoIlForDevice1 = "Error: No IL for device \'";
                    const char* kStrLegacyClNoIlForDevice2 = "\'.\nNote that AMD IL disassembly cannot be extracted for certain targets (depending on what compiler is invoked at runtime). This is not controlled by the tool.";

                    std::stringstream msg;
                    msg << kStrLegacyClNoIlForDevice1 << device << kStrLegacyClNoIlForDevice2 << std::endl;
                    LogCallback(msg.str());
                }
            }
        }
    }

    return ret;
}

beKA::beStatus BeProgramBuilderOpencl::GetKernelSectionText(const std::string& device, const std::string& kernel_name, std::string& kernel_text)
{
    beKA::beStatus ret_val = beKA::kBeStatusSuccess;
    if (!is_initialized_)
    {
        return kBeStatusOpenclModuleNotLoaded;
    }

    if (elf_map_.count(device) == 0)
    {
        stringstream ss;
        ss << "Error: No binary for device \'" << device << "\'.\n";
        LogCallback(ss.str());
        ret_val = beKA::kBeStatusNoBinaryForDevice;
    }

    if (ret_val == beKA::kBeStatusSuccess)
    {
        const CElf& kElf = *elf_map_[device];
        if (NULL != kElf.GetSymbolTable())
        {
            const CElfSymbolTable& symtab = *kElf.GetSymbolTable();
            CElfSymbolTable::const_SymbolIterator symIt = symtab.GetSymbol(kernel_name);
            if (symIt == symtab.SymbolsEnd())
            {
                ret_val = beKA::kBeStatusNoIlForDevice;
            }

            if (ret_val == beKA::kBeStatusSuccess)
            {
                std::string name;
                unsigned char bind;
                unsigned char type;
                unsigned char other;
                CElfSection* section = nullptr;
                Elf64_Addr value;
                Elf64_Xword size;

                symtab.GetInfo(symIt, &name, &bind, &type, &other, &section, &value, &size);

                if (section != nullptr)
                {
                    // Get the part that we care about.
                    std::vector<char> section_data(section->GetData());
                    kernel_text = std::string(section_data.begin() + size_t(value), section_data.begin() + size_t(value + size));
                    use_platform_native_line_endings(kernel_text);
                }
            }
        }
        else
        {
            ret_val = beKA::kBeStatusNoIlForDevice;
        }
    }

    return ret_val;
}

beKA::beStatus BeProgramBuilderOpencl::GetKernelIsaText(const std::string& device, const std::string& kernel, std::string& isa)
{
    // interface guard
    if (!is_initialized_)
    {
        return kBeStatusOpenclModuleNotLoaded;
    }

    //end guard

    beKA::beStatus ret_val = beKA::kBeStatusSuccess;

    // Because of EPR 378198 specifically check for 2 different devices and not get their ISA if we have the ACL module.
    // Getting the ISA will cause a crash. the EPR is fixed in catalyst 13.8.
    if (device.compare("Devastator") == 0 || device.compare("Scrapper") == 0)
    {
        // extract the version by the format, it should be in the (ver) in the end.
        double opencl_platform_version = GetOpenCLPlatformVersion();

        // 1306 is the version where the fix exist. checking also if 0 to avoid crash if something was wrong with the version
        if ((opencl_platform_version < 1306) || (opencl_platform_version == 0))
        {
            stringstream ss;
            ss << "Warning: No ISA for " << device << " for OpenCL version prior 1306, Current version is: " << opencl_platform_version << "\n";
            LogCallback(ss.str());
            ret_val = beKA::kBeStatusNoIsaForDevice;
        }
    }

    // CPU ISA is arguably x86 assembly.
    // Since the symbols in .text don't have good sizes,
    // and disassembly would be an extra step, use .astext.
    // It has no symbols, so return it all.
    if (name_device_type_map_[device] == CL_DEVICE_TYPE_CPU)
    {
        const CElf& elf = *elf_map_[device];
        const CElfSection* astext_section = elf.GetSection(".astext");
        if (astext_section == NULL)
        {
            stringstream ss;
            ss << "Error: no .astext section found.\n";
            LogCallback(ss.str());
            ret_val = beKA::kBeStatusNoIsaForDevice;
        }
        else
        {
            const std::vector<char> data(astext_section->GetData());
            isa = std::string(data.begin(), data.end());
            ret_val = beKA::kBeStatusSuccess;
        }
    }
    else
    {
        // Handle GPU.
        std::string name;
        bool is_hsail = !is_legacy_mode_ && DoesUseHsailPath(device);
        const size_t kNumAttempts = is_hsail ? 2 : 1;
        for (size_t i = 0; i < kNumAttempts; i++)
        {
            disassemble_callback_counter_ = 0;
            hsa_il_disassembly_.clear();
            isa_string_ = new std::string;

            if (is_hsail && i > 0)
            {
                // If we haven't succeeded with the HSAIL path, fallback to the AMDIL path.
                is_hsail = false;
            }

            ACLModuleManager::Instance()->GetACLModule(is_hsail, acl_module_, acl_compiler_);
            if (acl_module_ != nullptr && acl_module_->IsLoaded())
            {
                acl_error acl_err = ACL_SUCCESS;

                // aclCompiler object
                aclCompiler* pCom = acl_module_->CompilerInit(NULL, &acl_err);

                if (acl_err != ACL_SUCCESS)
                {
                    std::stringstream ss;
                    ss << "Error: Compiler init failed (ACLModule) " << acl_err << ".\n";
                    LogCallback(ss.str());
                    ret_val = beKA::kBeStatusAclCompileFailed;
                }

                aclBinary* acl_bin = NULL;
                std::map<std::string, std::vector<char> >::iterator iter = device_to_binary_.find(device);
                if (ret_val == beKA::kBeStatusSuccess)
                {
                    if (iter == device_to_binary_.end())
                    {
                        std::stringstream ss;
                        ss << "Error: No binary for device \'" << device << "\'.\n";
                        LogCallback(ss.str());

                        ret_val = beKA::kBeStatusNoBinaryForDevice;
                    }
                    else
                    {
                        std::vector<char>& binary_data = iter->second;
                        if (binary_data.size() < 1)
                        {
                            std::stringstream ss;
                            ss << "Error: No binary for device \'" << device << "\'.\n";
                            LogCallback(ss.str());
                            ret_val = beKA::kBeStatusNoBinaryForDevice;
                        }
                    }
                }

                if (ret_val == beKA::kBeStatusSuccess)
                {
                    std::vector<char>& binary_data = iter->second;
                    char* binary_data_ptr = reinterpret_cast<char*>(&binary_data[0]);
                    acl_err = ACL_SUCCESS;
                    acl_bin = acl_module_->ReadFromMem(binary_data_ptr, binary_data.size(), &acl_err);

                    if (acl_err != ACL_SUCCESS)
                    {
                        ret_val = beKA::kBeStatusAclBinaryFailed;
                    }
                    else
                    {
                        try
                        {
                            disassemble_callback_counter_ = 0;
                            isa_string_ = &isa;
                            isa_string_->clear();
                            hsa_il_disassembly_.clear();

                            std::string kernel_name;
                            std::string kernel_name_alternative;

                            // For HSAIL kernels, try the "&__OpenCL..." kernel name first, as that is the most-likely kernel symbol name
                            // For non-HSAIL kernels, try the undecorated kernel name first, as that is the most-likely kernel symbol name
                            // In both cases, fall back to the other name if the most-likely name fails
                            if (is_hsail)
                            {
                                kernel_name = "&__OpenCL_" + kernel + "_kernel";
                                kernel_name_alternative = kernel;
                            }
                            else
                            {
                                kernel_name = kernel;
                                kernel_name_alternative = "&__OpenCL_" + kernel + "_kernel";
                            }

                            acl_err = acl_module_->Disassemble(acl_compiler_, acl_bin, kernel_name.c_str(), disassembleLogFunction);
                            bool disassemble_failed = (acl_err != ACL_SUCCESS);
                            if (disassemble_failed)
                            {
                                isa_string_->clear();
                                hsa_il_disassembly_.clear();
                                disassemble_callback_counter_ = 0;
                                acl_err = acl_module_->Disassemble(acl_compiler_, acl_bin, kernel_name_alternative.c_str(), disassembleLogFunction);
                            }

                            // Cleanup.
                            acl_module_->BinaryFini(acl_bin);
                            acl_module_->CompilerFini(pCom);

                            if (!disassemble_failed && !isa_string_->empty())
                            {
                                // No need for further attempts.
                                break;
                            }

                            if (!is_hsail)
                            {
                                if (ACL_SUCCESS != acl_err)
                                {
                                    ret_val = kBeStatusNoIsaForDevice;
                                }

                                // No need for further attempts.
                                break;
                            }
                        }
                        catch (...)
                        {
                            ret_val = beKA::kBeStatusNoIsaForDevice;
                        }
                    }
                }


                if (!is_hsail && ret_val != beKA::kBeStatusSuccess)
                {
                    std::stringstream ss;
                    ss << "Error: Failed getting the disassembly for kernel: " << kernel << " on device: " << device << ".\n";
                    LogCallback(ss.str());
                    ret_val = beKA::kBeStatusNoIsaForDevice;
                }
            }
        }
    }

    return ret_val;
}

beKA::beStatus  BeProgramBuilderOpencl::Inquire(void* parameter_value, size_t parameter_value_size, KernelInfoAMD parameter_name, cl_kernel kernel, cl_device_id device_id)
{
    beKA::beStatus ret_val = beKA::kBeStatusSuccess;
    size_t analysis_field_size = 0;
    const CALuint64 na = beKA::kCalValue64Na;
    cl_int status = opencl_module_.GetKernelInfoAMD(kernel, device_id, parameter_name, 0, NULL, &analysis_field_size);

    if (status != CL_SUCCESS)
    {
        memcpy(parameter_value, &na, parameter_value_size);
        ret_val = beKA::kBeStatusGeneralFailed;
    }

    if ((ret_val == beKA::kBeStatusSuccess) && (parameter_value_size < analysis_field_size))
    {
        const CALuint64 kErr = beKA::kCalValue64Error;
        memcpy(parameter_value, &kErr, parameter_value_size);
        std::stringstream ss;
        ss << "Error: AnalysisData field`s size is too small.\n";
        LogCallback(ss.str());
        ret_val = beKA::kBeStatusGeneralFailed;
    }

    if (ret_val == beKA::kBeStatusSuccess)
    {
        status = opencl_module_.GetKernelInfoAMD(kernel, device_id, parameter_name, parameter_value_size, parameter_value, NULL);
        if (status != CL_SUCCESS)
        {
            memcpy(parameter_value, &na, parameter_value_size);
            std::stringstream ss;
            ss << "Error: Couldn't get ANalysis info.\n";
            LogCallback(ss.str());
            ret_val = beKA::kBeStatusGeneralFailed;
        }
    }

    return ret_val;
}

beKA::beStatus BeProgramBuilderOpencl::Compile(const std::string& program_source, const OpenCLOptions& opencl_options,
    const std::string& source_code_full_path, const std::vector<std::string>* source_path, int& successful_builds_count)
{
    // Init the output variables.
    successful_builds_count = 0;

    if (!is_initialized_)
    {
        return kBeStatusOpenclModuleNotLoaded;
    }

    // Check if we are in legacy mode.
    const char* kStrLegacyFlag = "-legacy";
    for (const std::string& opt : opencl_options.opencl_compile_options)
    {
        if (opt.compare(kStrLegacyFlag) == 0)
        {
            is_legacy_mode_ = true;
            break;
        }
    }

    // clear the force ending flag
    should_force_ending_ = false;

    // Clear and get ready for a new compilation session.
    ReleaseProgram();

    // Get set up to build the program.
    const char* source_content = program_source.c_str();
    size_t source_size[] = { program_source.size() };

    // Create CL program from the CL source kernel.
    beKA::beStatus ret_status = beKA::kBeStatusSuccess;
    cl_int status = CL_SUCCESS;
    cl_program program = opencl_module_.CreateProgramWithSource(opencl_context_, 1, &source_content, source_size, &status);

    if (CL_SUCCESS != status)
    {
        std::stringstream ss;
        ss << "OpenCL Compile Error: clCreateProgramWithSource failed (" + GetCLErrorString(status) + ")." << endl;
        LogCallback(ss.str());
        ret_status = beKA::kBeStatusOpenclCreateProgramWithSourceFailed;
    }
    else
    {
        // Collect the -D options.
        string defines_and_options;

        for (std::vector<string>::const_iterator it = opencl_options.defines.begin();
            it != opencl_options.defines.end();
            ++it)
        {
            defines_and_options += "-D" + *it + " ";
        }

        // Prepare the include path, Source file directory path name is put in quotes in order to use
        // path name with spaces (e.g. C:\Program Files (x86)) and not encounter CL_INVALID_BUILD_OPTIONS error, while clBuildProgram.
        if (source_path != NULL)
        {
            for (std::vector<string>::const_iterator it = source_path->begin(); it < source_path->end(); ++it)
            {
                defines_and_options += " -I \"" + *it + "\" ";
            }
        }

        // Collect the other compiler options.
        bool is_option_requested = false;
        for (std::vector<string>::const_iterator it = opencl_options.opencl_compile_options.begin();
            it != opencl_options.opencl_compile_options.end();
            ++it)
        {
            if (((*it).compare("-h")) || ((*it).compare("-H")))
            {
                is_option_requested = true;
            }

            defines_and_options += *it + " ";
        }

        // We want these so that we can present them to the user.
        defines_and_options += "-fbin-as -fbin-amdil -fbin-source";

        // Which devices do we care about?
        std::vector<cl_device_id> requested_devices;
        if (opencl_options.selected_devices_sorted.empty())
        {
            // None were specified by the user, so do them all.
            requested_devices = opencl_device_ids_;
        }
        else
        {
            // Make a vector of device IDs from the requested device list.
            for (auto it = opencl_options.selected_devices_sorted.begin();
                it != opencl_options.selected_devices_sorted.end(); ++it)
            {
                if (name_to_device_id_.count(*it) > 0)
                {
                    requested_devices.push_back(name_to_device_id_[*it]);
                }
                else
                {
                    std::stringstream ss;
                    ss << "Error: Unknown device " << *it << std::endl;
                    LogCallback(ss.str());
                }
            }
        }

        elfs_.resize(opencl_device_count_);
        std::vector<cl_device_id>::iterator iter_deviceid = requested_devices.begin();
        for (int compilation_number = 0; iter_deviceid < requested_devices.end(); iter_deviceid++, compilation_number++)
        {
            if (should_force_ending_)
            {
                ret_status = beKA::kBeStatusInvalid;
                break;
            }

            stringstream ss;
            ss << "Building for ";
            ss << device_id_to_name_[*iter_deviceid];
            ss << "... ";

            LogCallback(ss.str());
            std::string error_string;
            ret_status = CompileOpenCLInternal(source_code_full_path, program_source, opencl_options, *iter_deviceid, program, defines_and_options, compilation_number, error_string);

            if (ret_status == beKA::kBeStatusSuccess)
            {
                ret_status = GetProgramBinary(program, *iter_deviceid, &device_to_binary_[device_id_to_name_[*iter_deviceid]]);
                if (ret_status == beKA::kBeStatusSuccess)
                {
                    elf_map_[device_id_to_name_[*iter_deviceid]] = elfs_[compilation_number];
                }

                if (ret_status == beKA::kBeStatusSuccess)
                {
                    std::stringstream stream_inner;
                    stream_inner << "succeeded.\n";

                    if (is_option_requested)
                    {
                        stream_inner << error_string;

                        // Print this only once.
                        is_option_requested = false;
                    }

                    LogCallback(stream_inner.str());

                    // Increment the successful builds counter.
                    ++successful_builds_count;
                }
            }

            // Log and try another device.
            if (ret_status != beKA::kBeStatusSuccess)
            {
                std::stringstream msg;
                msg << "failed.\n";
                msg << error_string;
                LogCallback(msg.str());

                // Stop here.
                continue;
            }

            // Get the kernel list.
            std::vector<string> kernels;
            beStatus ret_status_analysis = kBeStatusInvalid;
            ret_status = GetKernels(device_id_to_name_[*iter_deviceid], kernels);

            // Go over the kernels and get the statistics.
            if (!should_force_ending_)
            {
                // If we succeeded in extracting the symbol from the ELF file, it means
                // that the ELF file is in the older format and we can continue.
                if (ret_status == beKA::kBeStatusSuccess)
                {
                    // Pass through all the kernels:
                    size_t numKernels = kernels.size();
                    std::map<std::string, beKA::AnalysisData> kernel_analysys;

                    for (size_t nKernel = 0; nKernel < numKernels; nKernel++)
                    {
                        string kernel = (kernels.at(nKernel));
                        beKA::AnalysisData ad;
                        ret_status_analysis = GetAnalysisInternal(program, device_id_to_name_[*iter_deviceid], kernel, &ad);

                        if (should_force_ending_)
                        {
                            ret_status = beKA::kBeStatusInvalid;
                        }

                        if (ret_status_analysis != beKA::kBeStatusSuccess)
                        {
                            break;
                        }

                        kernel_analysys[kernel] = ad;
                    }

                    if (ret_status_analysis == beKA::kBeStatusSuccess)
                    {
                        kernel_analysis_[device_id_to_name_[*iter_deviceid]] = kernel_analysys;
                    }
                }
                else
                {
                    // Write the binary to a file.
                    std::vector<char> binary_data;
                    elfs_[compilation_number]->Store(&binary_data);

                    // Create a temporary file with the binary data for the disassembler to work on.
                    std::string temp_file = KcUtils::ConstructTempFileName("lc_co", "bin");
                    KcUtils::WriteBinaryFile(temp_file, binary_data, nullptr);

                    // Disassemble the binary.
                    std::string disassembly_whole;
                    std::string disassembly_text;
                    std::string error_msg;
                    BeUtils::DisassembleCodeObject(temp_file, false,
                        disassembly_whole, disassembly_text, error_msg);

                    if (!disassembly_text.empty())
                    {
                        // Set the disassembly, we would retrieve it later when writing the output files.
                        device_to_code_object_disassembly_isa_[device_id_to_name_[*iter_deviceid]] = disassembly_text;
                        device_to_code_object_disassembly_whole_[device_id_to_name_[*iter_deviceid]] = disassembly_whole;
                    }

                    // Delete the temporary file.
                    BeUtils::DeleteFileFromDisk(temp_file);

                    // Try to extract the disassembly and statistics using amdgpu-dis, since we probably
                    // are dealing with a new format Code Object ELF binary.
                    ret_status = disassembly_text.empty() ? beKA::kBeStatusInvalid : beKA::beStatus::kBeStatusSuccess;
                    assert(ret_status == beKA::kBeStatusSuccess);
                    if (ret_status != beKA::kBeStatusSuccess)
                    {
                        std::cout << kStrErrorKernelSymbolNotFound << device_id_to_name_[*iter_deviceid] << std::endl;
                    }
                }
            }
        }
    }

    return ret_status;
}

beKA::beStatus BeProgramBuilderOpencl::CompileOpenCLInternal(const std::string& source_code_full_path, const std::string& program_source,
    const OpenCLOptions& opencl_options, cl_device_id requested_deviceid, cl_program& program, std::string& defines_and_options, int compilation_number, std::string& error_string)
{
    // Unused parameters.
    (void)(opencl_options);
#ifndef SAVE_ELF_OBJECTS
    (void)(program_source);
#endif

    beKA::beStatus ret = beKA::kBeStatusSuccess;
    cl_int status = CL_SUCCESS;
    std::vector<cl_device_id> requested_devices;
    requested_devices.push_back(requested_deviceid);

    bool is_build_succeeded = true;
    if (!BuildOpenCLProgramWrapper(
            status,
            program,
            (cl_uint)1,
            &requested_devices[0],
            defines_and_options.c_str(),
            NULL,
            NULL))
    {
        error_string = "OpenCL Compile Error: clBuildProgram raised an unhandled exception.\n";
        return beKA::kBeStatusOpenclBuildProgramException;
    }

    if (CL_SUCCESS != status)
    {
        error_string = "OpenCL Compile Error: clBuildProgram failed (";
        error_string += GetCLErrorString(status);
        error_string += ").\n";
        is_build_succeeded = false;
    }

    for (size_t i = 0; i < requested_devices.size(); i++)
    {
        // Show the build log (error and warnings)
        size_t build_log_size = 0;
        cl_int log_status = opencl_module_.GetProgramBuildInfo(
            program,
            requested_devices[i],
            CL_PROGRAM_BUILD_LOG,
            0,
            NULL,
            &build_log_size);

        if (log_status == CL_SUCCESS && build_log_size > 1)
        {
            char* build_log = new char[build_log_size];
            log_status = opencl_module_.GetProgramBuildInfo(
                program,
                requested_devices[i],
                CL_PROGRAM_BUILD_LOG,
                build_log_size,
                build_log,
                NULL);

            std::string build_log_str(build_log);
            delete[] build_log;

            // Remove references to OpenCL temporary files.
            std::string build_log_filtered = PostProcessOpenclBuildLog(build_log_str, source_code_full_path);
            error_string += build_log_filtered;
        }
    }

    if (is_build_succeeded == false)
    {
        return beKA::kBeStatusOpenclBuildProgramWrapperFailed;
    }

    // Get the CL_PROGRAM_DEVICES.
    // These may be in a different order than the CL_CONTEXT_DEVICES.
    // This happens when we specify a subset --
    // which is at the least an ugly misfeature and is arguably a bug.
    int opencl_device_count = 1;
    std::vector<cl_device_id> program_devices;
    program_devices.resize(opencl_device_count);
    status = opencl_module_.GetProgramInfo(
        program,
        CL_PROGRAM_DEVICES,
        opencl_device_count * sizeof(cl_device_id),
        &program_devices[0],
        NULL);

    if (CL_SUCCESS != status)
    {
        error_string = "OpenCL Compile Error: clGetProgramInfo CL_PROGRAM_DEVICES failed (" + GetCLErrorString(status) + ").\n";
        return beKA::kBeStatusOpenclGetProgramInfoFailed;
    }

    // Get the binaries.
    vector<size_t> binary_sizes;
    binary_sizes.resize(opencl_device_count);

    // Get the sizes of the CL binaries
    status = opencl_module_.GetProgramInfo(
        program,
        CL_PROGRAM_BINARY_SIZES,
        opencl_device_count * sizeof(size_t),
        &binary_sizes[0],
        NULL);

    if (CL_SUCCESS != status)
    {
        error_string = "OpenCL Compile Error: clGetProgramInfo CL_PROGRAM_BINARY_SIZES failed (" + GetCLErrorString(status) + ").\n";
        return beKA::kBeStatusOpenclGetProgramInfoFailed;
    }

    // Get the CL binaries.
    std::vector<char*> binaries;
    binaries.resize(opencl_device_count);
    size_t size = binary_sizes[0];
    binaries[0] = size ? new char[binary_sizes[0]] : NULL;

    status = opencl_module_.GetProgramInfo(
        program,
        CL_PROGRAM_BINARIES,
        opencl_device_count * sizeof(char*),
        &binaries[0],
        NULL);

    if (CL_SUCCESS != status)
    {
        error_string = "OpenCL Compile Error: clGetProgramInfo CL_PROGRAM_BINARIES failed (" + GetCLErrorString(status) + ").\n";
    }

    if (binaries[0] != NULL)
    {
        std::vector<char> binary_data(binaries[0], binaries[0] + binary_sizes[0]);
#if SAVE_ELF_OBJECTS
        // debugging code.
        ofstream output;
        output.open("debug.elf", ios::binary);
        output.write(binaries[i], binary_sizes[i]);
        output.close();

        output.open("debug.source");
        output.write(program_source.c_str(), program_source.size());
        output.close();
#endif
        elfs_[compilation_number] = new CElf(binary_data);
        if (!elfs_[compilation_number]->good())
        {
            ret = beKA::kBeStatusAclCompileFailed;
        }
        elf_map_[device_id_to_name_[requested_deviceid]] = elfs_[compilation_number];
    }

    // Clean up.
    delete[] binaries[0];

    return ret;
}

beKA::beStatus BeProgramBuilderOpencl::GetProgramBinary(cl_program& program, cl_device_id& device, std::vector<char>* binary_data)
{
    cl_int err = CL_SUCCESS;
    beKA::beStatus ret_val = beKA::kBeStatusSuccess;

    // Get a device count for this program.
    size_t device_count = 0;
    err = opencl_module_.GetProgramInfo(program,
        CL_PROGRAM_NUM_DEVICES,
        sizeof(device_count),
        &device_count,
        NULL);

    // Grab the handles to all of the devices in the program.
    vector<cl_device_id> devices(device_count);
    err = opencl_module_.GetProgramInfo(program,
        CL_PROGRAM_DEVICES,
        sizeof(cl_device_id) * device_count,
        &devices[0],
        NULL);

    // Set the device index to match the relevant device.
    bool is_device_found = false;
    size_t device_index = 0;

    for (size_t i = 0; i < device_count && !is_device_found; i++)
    {
        if (devices[i] == device)
        {
            device_index = i;
            is_device_found = true;
        }
    }

    // If this fails, we've done something very wrong.
    if (!is_device_found)
    {
        ret_val = beKA::kBeStatusOpenclGetProgramInfoFailed;
    }

    if (ret_val == beKA::kBeStatusSuccess)
    {
        // Figure out the sizes of each of the binaries.
        size_t* binary_sizes = new size_t[device_count];
        err = opencl_module_.GetProgramInfo(program,
            CL_PROGRAM_BINARY_SIZES,
            sizeof(size_t) * device_count,
            binary_sizes,
            NULL);

        // Retrieve all of the generated binaries.
        binary_data->resize(binary_sizes[device_index]);
        char* pTemp = &(*(binary_data->begin()));
        if (ret_val == beKA::kBeStatusSuccess)
        {
            err = opencl_module_.GetProgramInfo(program,
                CL_PROGRAM_BINARIES,
                sizeof(char*) * device_count,
                &pTemp,
                NULL);
        }
    }

    if (err != CL_SUCCESS)
    {
        std::stringstream msg;
        msg << "GetProgramBinary failed with err = " << err;
        LogCallback(msg.str());
    }

    return ret_val;
}

void BeProgramBuilderOpencl::CalLoggerFunc(const CALchar* line)
{
    // For SI, deal with a bug where the ISA instructions all get passed together.
    // This really shouldn't be necessary, but it's easy enough to deal with.
    string line_string(line);

    // Remove all carriage returns.
    line_string.erase(remove(line_string.begin(), line_string.end(), '\r'),
        line_string.end());

    // Add a linefeed at the end if there's not one there already.
    if (line_string[line_string.length() - 1] != '\n')
    {
        line_string += '\n';
    }

    // And capture the string up to here.
    (*isa_string_) += line_string;
}

void BeProgramBuilderOpencl::disassembleLogFunction(const char* msg, size_t size)
{
    // For SI, deal with a bug where the ISA instructions all get passed together.
    // This really shouldn't be necessary, but it's easy enough to deal with.
    string line_string(msg, size);

    // Remove all carriage returns.
    line_string.erase(remove(line_string.begin(), line_string.end(), '\r'), line_string.end());

    // Add a linefeed at the end if there's not one there already.
    if (line_string[line_string.length() - 1] != '\n')
    {
        line_string += '\n';
    }

    // And capture the string up to here.
    if (disassemble_callback_counter_ == 0)
    {
        (*isa_string_) = line_string;
    }
    else if (disassemble_callback_counter_ == 1)
    {
        hsa_il_disassembly_ = line_string;
    }

    disassemble_callback_counter_++;
}

bool BeProgramBuilderOpencl::BuildOpenCLProgramWrapper(
    cl_int& status,
    cl_program program,
    cl_uint num_devices,
    const cl_device_id* device_list,
    const char* options,
    void (CL_CALLBACK* pfn_notify)(cl_program program, void* user_data),
    void* user_data)
{
#ifdef _WIN32
    __try
    {
#endif
        status = opencl_module_.BuildProgram(
            program,
            num_devices,
            device_list,
            options,
            pfn_notify,
            user_data);
        return true;
#ifdef _WIN32
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }

#endif
}

beKA::beStatus BeProgramBuilderOpencl::GetDevices(std::set<string>& devices)
{
    beKA::beStatus retVal = kBeStatusBackendNotInitialized;
    if (is_initialized_)
    {
        devices = device_names_;
        retVal = kBeStatusSuccess;
    }
    return retVal;
}

beStatus BeProgramBuilderOpencl::GetDeviceTable(std::vector<GDT_GfxCardInfo>& table)
{
    table = opencl_device_table_;
    return kBeStatusSuccess;
}

BeProgramBuilderOpencl::~BeProgramBuilderOpencl()
{
}

void BeProgramBuilderOpencl::ForceEnd()
{
    should_force_ending_ = true;
}

void BeProgramBuilderOpencl::GetSupportedPublicDevices(std::set<std::string>& devices) const
{
    devices = device_names_;
}

void BeProgramBuilderOpencl::GetDeviceToCodeObjectDisassemblyMapping(std::map<std::string, std::string>& mapping)
{
    mapping = device_to_code_object_disassembly_isa_;
}

bool BeProgramBuilderOpencl::HasCodeObjectBinary(const std::string& device) const
{
    auto iter = device_to_code_object_disassembly_isa_.find(device);
    return iter != device_to_code_object_disassembly_isa_.end();
}

bool BeProgramBuilderOpencl::ExtractStatisticsCodeObject(const std::string& device,
    std::map<std::string, beKA::AnalysisData>& stats)
{
    bool ret = false;
    auto iter = device_to_code_object_disassembly_whole_.find(device);
    assert(iter != device_to_code_object_disassembly_whole_.end());
    if (iter != device_to_code_object_disassembly_whole_.end())
    {
        ret = BeUtils::ExtractCodeObjectStatistics(iter->second, stats);
        assert(ret);
        if (!ret)
        {
            std::cout << kStrErrorFailedToExtractCodeObjectStats << device << std::endl;
        }
    }
    return ret;
}

double BeProgramBuilderOpencl::GetOpenCLPlatformVersion()
{
    // Extract the version by the format.
    double opencl_platform_version = 0;
    char opencl_version[256];
    memset(&opencl_version[0], 0, sizeof(char) * 256);
    int start = (int)opencl_version_info_.find("(");
    int end = (int)opencl_version_info_.find(")");
    int length = end - start;

    if (length > 0)
    {
        int i = 0;
        for (start += 1; start < end; start++, i++)
        {
            opencl_version[i] = opencl_version_info_[start];
        }
        opencl_platform_version = atof((char*)opencl_version);
    }

    return opencl_platform_version;

}

void BeProgramBuilderOpencl::RemoveNamesOfUnpublishedDevices(const std::set<string>& unique_published_device_names)
{
    // Take advantage of the fact that the m_OpenCLDeviceTable collection contains only published devices,
    // so we look for each name that the OpenCL driver provided in the table, and remove it if it is not found
    for (std::set<string>::iterator iter = device_names_.begin(); iter != device_names_.end();)
    {
        bool is_device_published = false;
        const std::string& kDeviceName = *iter;
        if (unique_published_device_names.find(kDeviceName) != unique_published_device_names.end())
        {
            // The device name exists in the OpenCL device table, therefore it is a published device.
            // Nothing more to do with this device name.
            is_device_published = true;
        }

        if (is_device_published)
        {
            // Keep the published device in the names collection - just iterate to the next name
            iter++;
        }
        else if (name_device_type_map_[kDeviceName] != CL_DEVICE_TYPE_CPU)
        {
            // Remove name of unpublished device from the names collections
            set<string>::iterator iter_to_remove = iter;
            iter++;
            name_device_type_map_.erase(kDeviceName);
            cl_device_id deviceId = name_to_device_id_[kDeviceName];
            name_to_device_id_.erase(kDeviceName);
            device_id_to_name_.erase(deviceId);
            device_names_.erase(iter_to_remove);

            // Remove the device ID from the vector of OpenCL devices
            size_t opencl_device_count = opencl_device_ids_.size();
            for (size_t i = 0; i < opencl_device_count; ++i)
            {
                if (opencl_device_ids_[i] == deviceId)
                {
                    opencl_device_ids_.erase(opencl_device_ids_.begin() + i);
                    break;
                }
            }
        }
        else
        {
            iter++;
        }
    }
}

bool BeProgramBuilderOpencl::DoesUseHsailPath(const std::string& device_name) const
{
    bool ret_val = false;
    // HSAIL path is not used on Linux 32-bit regardless of driver version, hardware family
#ifdef _LINUX
#ifdef X86
    ret_val = false;
    return ret_val;
#endif
#endif

    bool is_si_family = false;

    // HSAIL path is not used on SI hardware.
    AMDTDeviceInfoUtils::Instance()->IsSIFamily(device_name.c_str(), is_si_family);

    if (!is_si_family)
    {
        ret_val = true;
    }

    return ret_val;
}

void BeProgramBuilderOpencl::use_platform_native_line_endings(std::string& text)
{
    // If text is empty
    if (text.length() <= 0)
    {
        return;
    }

    // Remove all carriage returns.
    // This will put us into Linux format (from either Mac or Windows).
    // [With the AMD OpenCL stack as of April 2012, this does nothing.]
    text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());

    // Add a linefeed at the end if there's not one there already.
    if (text[text.length() - 1] != '\n')
    {
        text += '\n';
    }

#ifdef _WIN32
    // Now convert all of the \n to \r\n.
    size_t pos = 0;

    while ((pos = text.find('\n', pos)) != string::npos)
    {
        text.replace(pos, 1, "\r\n");
        pos += 2;
    }

#endif
}
