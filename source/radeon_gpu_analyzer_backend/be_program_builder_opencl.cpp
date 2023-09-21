//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <locale>

// Infra.
#include "DeviceInfoUtils.h"

// Local.
#include "radeon_gpu_analyzer_backend/be_program_builder_opencl.h"
#include "emulator/parser/be_isa_parser.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"

// CLI.
#include "source/radeon_gpu_analyzer_cli/kc_utils.h"

// Constants: error messages.
static const char* kStrErrorKernelSymbolNotFound = "Error: failed to locate kernel symbol in ELF container for device: ";
static const char* kStrErrorFailedToExtractCodeObjectStats = "Error: failed to extract statistics from Code Object for ";

beKA::beStatus BeProgramBuilderOpencl::Initialize(const std::string& dll_module /* = ""*/)
{
    (void)(dll_module);

    // Populate the sorted device (card) info table.
    std::set<std::string> unique_published_devices_names;
    BeUtils::GetAllGraphicsCards(opencl_device_table_, unique_published_devices_names);

    return beKA::kBeStatusSuccess;
}

beKA::beStatus BeProgramBuilderOpencl::GetKernels(const std::string& device, std::vector<std::string>& kernels)
{
    return beKA::beStatus::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpencl::GetBinary(const std::string& device, const beKA::BinaryOptions& binopts, std::vector<char>& binary)
{
    return beKA::beStatus::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpencl::GetBinaryFromFile(const std::string& path_to_binary, const beKA::BinaryOptions& binopts, std::vector<char>& output_path)
{
    return beKA::beStatus::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpencl::GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis)
{
    return beKA::beStatus::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpencl::GetKernelMetaDataText(const std::string& device, const std::string& kernel, std::string& metadata)
{
    return beKA::beStatus::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpencl::GetKernelIlText(const std::string& device, const std::string& kernel_name, std::string& kernel_text)
{
    return beKA::beStatus::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpencl::GetKernelSectionText(const std::string& device, const std::string& kernel_name, std::string& kernel_text)
{
    return beKA::beStatus::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpencl::GetKernelIsaText(const std::string& device, const std::string& kernel, std::string& isa)
{
    return beKA::beStatus::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpencl::Compile(const std::string& program_source, const OpenCLOptions& opencl_options,
    const std::string& source_code_full_path, const std::vector<std::string>* source_path, int& successful_builds_count)
{
    return beKA::beStatus::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpencl::GetDevices(std::set<std::string>& devices)
{
    beKA::beStatus retVal = beKA::beStatus::kBeStatusBackendNotInitialized;
    if (is_initialized_)
    {
        devices = device_names_;
        retVal  = beKA::beStatus::kBeStatusSuccess;
    }
    return retVal;
}

beKA::beStatus BeProgramBuilderOpencl::GetDeviceTable(std::vector<GDT_GfxCardInfo>& table)
{
    table = opencl_device_table_;
    return beKA::beStatus::kBeStatusSuccess;
}

BeProgramBuilderOpencl::~BeProgramBuilderOpencl()
{
}

void BeProgramBuilderOpencl::GetSupportedPublicDevices(std::set<std::string>& devices) const
{
    devices = device_names_;
}

void BeProgramBuilderOpencl::GetDeviceToCodeObjectDisassemblyMapping(std::map<std::string, std::string>& mapping)
{
    mapping = device_to_code_object_disassembly_isa_;
}

bool BeProgramBuilderOpencl::ExtractStatisticsCodeObject(const std::string& device,
    std::map<std::string, beKA::AnalysisData>& stats)
{
    return false;
}

double BeProgramBuilderOpencl::GetOpenCLPlatformVersion()
{
    return 0.0;
}

void BeProgramBuilderOpencl::RemoveNamesOfUnpublishedDevices(const std::set<std::string>& unique_published_device_names)
{
    // Take advantage of the fact that the m_OpenCLDeviceTable collection contains only published devices,
    // so we look for each name that the OpenCL driver provided in the table, and remove it if it is not found
    for (std::set<std::string>::iterator iter = device_names_.begin(); iter != device_names_.end();)
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
        else
        {
            iter++;
        }
    }
}
