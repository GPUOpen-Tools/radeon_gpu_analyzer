//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <locale>
#ifdef _WIN32
    #include <codecvt>
#endif
#include <string>
#include <cassert>

#define KA_BACKEND_DLL_EXPORT

// Local.
#include "radeon_gpu_analyzer_backend/be_backend.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_opencl.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_lightning.h"
#include "radeon_gpu_analyzer_backend/be_string_constants.h"
#ifdef _WIN32
    #include "radeon_gpu_analyzer_backend/be_program_builder_dx11.h"
#endif

// Infra.
#include <DeviceInfoUtils.h>
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include "external/amdt_os_wrappers/Include/osFilePath.h"
#include "external/amdt_os_wrappers/Include/osDebugLog.h"
#ifdef _WIN32
    #pragma warning(pop)
#endif

std::vector<std::string> Backend::custom_dx11_load_paths_;

// We must declare an explicit implementation for the CTOR
// due to the TSingleton<> pattern.
Backend::Backend() : supported_public_devices_()
{
}

bool Backend::LogCallBack(const std::string& msg)
{
    bool ret = false;
    if (log_callback_)
    {
        log_callback_(msg);
        ret = true;
    }
    return ret;
}

beKA::beStatus Backend::Initialize(BuiltProgramKind program_kind, LoggingCallBackFuncP callback)
{
    beKA::beStatus ret = kBeStatusGeneralFailed;
    log_callback_ = callback;

    if (builder_opencl_ == nullptr && program_kind == kProgramTypeOpencl)
    {
        builder_opencl_ = new BeProgramBuilderOpencl();
    }

    if (builder_opencl_ != nullptr)
    {
        builder_opencl_->SetLog(callback);

        ret = builder_opencl_->Initialize();

        if (supported_public_devices_.empty() && ret == kBeStatusSuccess)
        {
            builder_opencl_->GetSupportedPublicDevices(supported_public_devices_);
        }
    }

#ifdef _WIN32
    if (program_kind == kProgramTypeDx11)
    {
        // Initialize the DX backend.
        // Release the old DX driver since we can initialize each run with a different dx dll.
        if (builder_dx11_ != nullptr)
        {
            delete builder_dx11_;
            builder_dx11_ = nullptr;
        }

        if (builder_dx11_ == nullptr)
        {
            builder_dx11_ = new BeProgramBuilderDx11();
            builder_dx11_->SetPublicDeviceNames(supported_public_devices_);

            for (const std::string& dir : custom_dx11_load_paths_)
            {
                builder_dx11_->AddDxSearchDir(dir);
            }
        }
    }
#endif

    return ret;
}

Backend::~Backend()
{
    AMDTDeviceInfoUtils::DeleteInstance();
}

beKA::beStatus Backend::GetDeviceInfo(const std::string& device_name, GDT_DeviceInfo& gdt_device_info)
{
    bool ret = AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(device_name.c_str(), gdt_device_info);
    return ret ? kBeStatusSuccess : kBeStatusNoDeviceFound;
}

beStatus Backend::GetDeviceInfo(const std::string& device_name, const std::vector<GDT_GfxCardInfo>** info)
{
    std::vector<GDT_GfxCardInfo> device_info;
    bool ret = AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(device_name.c_str(), device_info);

    if (info != nullptr)
    {
        *info = ret ? &device_info : nullptr;
    }

    return ret ? kBeStatusSuccess : kBeStatusNoDeviceFound;
}

// This is a temporary implementation until the following function:
// beStatus Backend::GetDeviceInfo(const std::string& deviceName, const std::vector<GDT_GfxCardInfo>** info)
// is being fixed to accept GDT_GfxCardInfo& instead of the meaningless std::vector that it currently accepts.
// In the end we should have only a single implementation of that function, accepting a GDT_GfxCardInfo&, but
// without the copy which is being made in this implementation.
beStatus Backend::GetDeviceInfo(const std::string& device_name, GDT_GfxCardInfo& info)
{
    beStatus ret = kBeStatusNoDeviceFound;
    std::vector<GDT_GfxCardInfo> temp_device_info;
    bool rc = AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(device_name.c_str(), temp_device_info);

    if (rc && (temp_device_info.empty() == false))
    {
        info = temp_device_info[0];
        ret = kBeStatusSuccess;
    }

    return ret;
}

beStatus Backend::GetDeviceInfo(size_t device_id, GDT_GfxCardInfo& info)
{
    return AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(device_id, 0, info) ? kBeStatusSuccess : kBeStatusNoDeviceFound;
}

beStatus Backend::GetDeviceInfoMarketingName(const std::string& device_name, std::vector<GDT_GfxCardInfo>& info)
{
    bool ret = AMDTDeviceInfoUtils::Instance()->GetDeviceInfoMarketingName(device_name.c_str(), info);
    return ret ? kBeStatusSuccess : kBeStatusNoDeviceFound;
}

beStatus Backend::GetDeviceChipFamilyRevision(const GDT_GfxCardInfo& table_entry, unsigned int& chip_family, unsigned int& chip_revision)
{
    beStatus ret = kBeStatusSuccess;
    chip_family = (unsigned int)-1;
    chip_revision = (unsigned int)-1;

    switch (table_entry.m_asicType)
    {
    case GDT_GFX10_3_4:
        chip_family   = FAMILY_NV;
        chip_revision = NV_NAVI24_P_A0;
        ret           = kBeStatusSuccess;
        break;

    case GDT_GFX10_3_2:
        chip_family = FAMILY_NV;
        chip_revision = NV_NAVI23_P_A0;
        ret = kBeStatusSuccess;
        break;

    case GDT_GFX10_3_1:
        chip_family = FAMILY_NV;
        chip_revision = NV_NAVI22_P_A0;
        ret = kBeStatusSuccess;
        break;

    case GDT_GFX10_3_0:
        chip_family = FAMILY_NV;
        chip_revision = NV_NAVI21_P_A0;
        ret = kBeStatusSuccess;
        break;

    case GDT_GFX10_1_2:
        chip_family = FAMILY_NV;
        chip_revision = NV_NAVI14_M_A0;
        ret = kBeStatusSuccess;
        break;

    case GDT_GFX10_1_1:
        chip_family = FAMILY_NV;
        chip_revision = NV_NAVI12_P_A0;
        ret = kBeStatusSuccess;
        break;

    case GDT_GFX10_1_0:
        chip_family = FAMILY_NV;
        chip_revision = NV_NAVI10_P_A0;
        ret = kBeStatusSuccess;
        break;

    case GDT_GFX9_0_6:
        chip_family = FAMILY_AI;
        chip_revision = AI_VEGA20_P_A0;
        ret = kBeStatusSuccess;
        break;

    case GDT_GFX9_0_0:
    case GDT_GFX9_0_2:
    case GDT_GFX9_0_9:
    case GDT_GFX9_0_C:
        chip_family = FAMILY_AI;
        chip_revision = AI_GD_P0;
        ret = kBeStatusSuccess;
        break;

    case GDT_TAHITI_PRO:
    case GDT_TAHITI_XT:
        chip_family = FAMILY_SI;
        chip_revision = SI_TAHITI_P_B1;
        ret = kBeStatusSuccess;
        break;

    case GDT_PITCAIRN_PRO:
    case GDT_PITCAIRN_XT:
        chip_family = FAMILY_SI;
        chip_revision = SI_PITCAIRN_PM_A1;
        ret = kBeStatusSuccess;
        break;

    case GDT_CAPEVERDE_PRO:
    case GDT_CAPEVERDE_XT:
        chip_family = FAMILY_SI;
        chip_revision = SI_CAPEVERDE_M_A1;
        ret = kBeStatusSuccess;
        break;

    case GDT_OLAND:
        chip_family = FAMILY_SI;
        chip_revision = SI_OLAND_M_A0;
        ret = kBeStatusSuccess;
        break;

    case GDT_HAINAN:
        chip_family = FAMILY_SI;
        chip_revision = SI_HAINAN_V_A0;
        ret = kBeStatusSuccess;
        break;

    case GDT_BONAIRE:
        chip_family = FAMILY_CI;
        chip_revision = CI_BONAIRE_M_A0;
        break;

    case GDT_HAWAII:
        chip_family = FAMILY_CI;
        chip_revision = CI_HAWAII_P_A0;
        break;

    case GDT_KALINDI:
        chip_family = FAMILY_CI;
        chip_revision = CI_BONAIRE_M_A0;
        break;

    case GDT_SPECTRE:
    case GDT_SPECTRE_SL:
    case GDT_SPECTRE_LITE:
        chip_family = FAMILY_CI;
        chip_revision = KV_SPECTRE_A0;
        break;

    case GDT_SPOOKY:
        chip_family = FAMILY_CI;
        chip_revision = KV_SPOOKY_A0;
        break;

    case GDT_ICELAND:
        chip_family = FAMILY_VI;
        chip_revision = VI_ICELAND_M_A0;
        break;

    case GDT_TONGA:
        chip_family = FAMILY_VI;
        chip_revision = VI_TONGA_P_A0;
        break;

    case GDT_CARRIZO_EMB:
    case GDT_CARRIZO:
        chip_family = FAMILY_VI;
        chip_revision = CARRIZO_A0;
        break;

    case GDT_STONEY:
        chip_family = FAMILY_VI;
        chip_revision = STONEY_A0;
        break;

    case GDT_FIJI:
        chip_family = FAMILY_VI;
        chip_revision = VI_FIJI_P_A0;
        break;

    case GDT_BAFFIN:
        chip_family = FAMILY_VI;
        chip_revision = VI_BAFFIN_M_A0;
        break;

    case GDT_ELLESMERE:
        chip_family = FAMILY_VI;
        chip_revision = VI_ELLESMERE_P_A0;
        break;

    case GDT_GFX8_0_4:
    case GDT_VEGAM2:
        chip_family = FAMILY_VI;
        chip_revision = VI_LEXA_V_A0;
        break;

    default:
        // The 600's EG, and NI are no longer supported by OpenCL.
        // For GSA the earliest DX/GL support is SI.
        ret = kBeStatusNoDeviceFound;
        assert(false);
        break;
    }

    return ret;
}

#ifdef _WIN32

#endif

bool Backend::GetSupportedPublicDevices(std::set<std::string>& devices)
{
    bool ret = false;

    if (!supported_public_devices_.empty())
    {
        ret = true;
        devices = supported_public_devices_;
    }
    else
    {
        // Get the supported public devices from the OpenCL backend.
        if (builder_opencl_ == nullptr)
        {
            builder_opencl_ = new BeProgramBuilderOpencl;
        }

        beKA::beStatus rc = builder_opencl_->Initialize();

        if (rc == kBeStatusSuccess)
        {
            // Retrieve the supported public devices from the OpenCL runtime.
            builder_opencl_->GetSupportedPublicDevices(devices);
            ret = true;
        }
    }

    return ret;
}
