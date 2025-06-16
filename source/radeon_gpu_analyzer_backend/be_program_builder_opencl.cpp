//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for rga backend progam builder opencl class.
//=============================================================================
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

beKA::beStatus BeProgramBuilderOpencl::GetKernels(const std::string&, std::vector<std::string>&)
{
    return beKA::beStatus::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpencl::GetBinary(const std::string&, const beKA::BinaryOptions&, std::vector<char>&)
{
    return beKA::beStatus::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpencl::GetBinaryFromFile(const std::string&, const beKA::BinaryOptions&, std::vector<char>&)
{
    return beKA::beStatus::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpencl::GetStatistics(const std::string&, const std::string&, beKA::AnalysisData&)
{
    return beKA::beStatus::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpencl::GetKernelMetaDataText(const std::string&, const std::string&, std::string&)
{
    return beKA::beStatus::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpencl::GetKernelIlText(const std::string&, const std::string&, std::string&)
{
    return beKA::beStatus::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpencl::GetKernelSectionText(const std::string&, const std::string&, std::string&)
{
    return beKA::beStatus::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpencl::GetKernelIsaText(const std::string&, const std::string&, std::string&)
{
    return beKA::beStatus::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpencl::Compile(const std::string&, const OpenCLOptions&,
    const std::string&, const std::vector<std::string>*, int&)
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

bool BeProgramBuilderOpencl::ExtractStatisticsCodeObject(const std::string&,
    std::map<std::string, beKA::AnalysisData>&)
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
