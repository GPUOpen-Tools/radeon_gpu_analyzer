//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#include <locale>
#ifdef _WIN32
    #include <codecvt>
#endif
#include <string>


#ifdef _WIN32
    // DX is Windows only.
    #include <D3D10ShaderObject.h>
#endif

#define KA_BACKEND_DLL_EXPORT

// This is from ADL's include directory.
#include <DeviceInfoUtils.h>

// Local.
#include <RadeonGPUAnalyzerBackend/include/beBackend.h>
#include <RadeonGPUAnalyzerBackend/include/beProgramBuilderOpenCL.h>
#include <RadeonGPUAnalyzerBackend/include/beProgramBuilderLightning.h>
#include <RadeonGPUAnalyzerBackend/include/beStringConstants.h>
#ifdef _WIN32
    #include <include/beProgramBuilderDX.h>
#endif

// Infra.
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

std::vector<std::string> Backend::m_customDxLoadPaths;

Backend::Backend() : m_supportedPublicDevices()
{
    m_beOpenCL = NULL;
#ifdef _WIN32
    m_beDX = NULL;
#endif
}

beKA::beStatus Backend::Initialize(BuiltProgramKind ProgramKind, LoggingCallBackFuncP callback)
{
    beKA::beStatus retVal = beStatus_General_FAILED;
    m_LogCallback = callback;

    if (m_beOpenCL == NULL && ProgramKind == BuiltProgramKind_OpenCL)
    {
        m_beOpenCL = new beProgramBuilderOpenCL();
    }

    if (m_beOpenCL != NULL)
    {
        m_beOpenCL->SetLog(callback);

        retVal = m_beOpenCL->Initialize();

        if (m_supportedPublicDevices.empty() && retVal == beStatus_SUCCESS)
        {
            m_beOpenCL->GetSupportedPublicDevices(m_supportedPublicDevices);
        }
    }

#ifdef _WIN32
    if (ProgramKind == BuiltProgramKind_DX)
    {
        // Initialize the DX backend.
        // Release the old DX driver since we can initialize each run with a different dx dll.
        if (m_beDX != NULL)
        {
            delete m_beDX;
            m_beDX = NULL;
        }

        if (m_beDX == NULL)
        {
            m_beDX = new beProgramBuilderDX();
            m_beDX->SetPublicDeviceNames(m_supportedPublicDevices);

            for (const std::string& dir : m_customDxLoadPaths)
            {
                m_beDX->AddDxSearchDir(dir);
            }
        }
    }
#endif

    return retVal;
}

//
// Backend member functions.
//

Backend::~Backend()
{
    if (m_beOpenCL != NULL)
    {
        m_beOpenCL->DeinitializeOpenCL();
    }

    AMDTDeviceInfoUtils::DeleteInstance();
}

beKA::beStatus Backend::GetDeviceInfo(const std::string& deviceName, GDT_DeviceInfo& Gdtdi)
{
    bool ret = AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(deviceName.c_str(), Gdtdi);
    return ret ? beStatus_SUCCESS : beStatus_NO_DEVICE_FOUND;
}

beStatus Backend::GetDeviceInfo(const std::string& deviceName, const std::vector<GDT_GfxCardInfo>** info)
{
    std::vector<GDT_GfxCardInfo> deviceInfo;
    bool ret = AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(deviceName.c_str(), deviceInfo);

    if (info != NULL)
    {
        *info = ret ? &deviceInfo : NULL;
    }

    return ret ? beStatus_SUCCESS : beStatus_NO_DEVICE_FOUND;
}


//TODO: Note: the following comment doesn't make sense. For each CAL device name, there can be many different GDT_GfxCardInfo instances

// This is a temporary implementation until the following function:
// beStatus Backend::GetDeviceInfo(const std::string& deviceName, const std::vector<GDT_GfxCardInfo>** info)
// is being fixed to accept GDT_GfxCardInfo& instead of the meaningless std::vector that it currently accepts.
// In the end we should have only a single implementation of that function, accepting a GDT_GfxCardInfo&, but
// without the copy which is being made in this implementation.
beStatus Backend::GetDeviceInfo(const std::string& deviceName, GDT_GfxCardInfo& info)
{
    beStatus ret = beStatus_NO_DEVICE_FOUND;
    std::vector<GDT_GfxCardInfo> tmpInfoVector;
    bool rc = AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(deviceName.c_str(), tmpInfoVector);

    if (rc && (tmpInfoVector.empty() == false))
    {
        info = tmpInfoVector[0];
        ret = beStatus_SUCCESS;
    }

    return ret;
}


beStatus Backend::GetDeviceInfo(size_t deviceID, GDT_GfxCardInfo& info)
{
    // TODO check to see if we need to pass rev id here
    return AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(deviceID, 0, info) ? beStatus_SUCCESS : beStatus_NO_DEVICE_FOUND;
}


beStatus Backend::GetDeviceInfoMarketingName(const std::string& deviceName, std::vector<GDT_GfxCardInfo>& info)
{
    bool ret = AMDTDeviceInfoUtils::Instance()->GetDeviceInfoMarketingName(deviceName.c_str(), info);
    return ret ? beStatus_SUCCESS : beStatus_NO_DEVICE_FOUND;
}


beStatus Backend::GetDeviceChipFamilyRevision(
    const GDT_GfxCardInfo& tableEntry,
    unsigned int&          chipFamily,
    unsigned int&          chipRevision)
{
    beStatus retVal = beStatus_SUCCESS;
    chipFamily = (unsigned int) - 1;
    chipRevision = (unsigned int) - 1;

    switch (tableEntry.m_asicType)
    {
        default:
            // The 600's EG, and NI are no longer supported by OpenCL.
            // For GSA the earliest DX/GL support is SI.
            retVal = beStatus_NO_DEVICE_FOUND;
            break;

        case GDT_GFX9_0_0:
        case GDT_GFX9_0_2:
            chipFamily = FAMILY_AI;
            chipRevision = AI_GD_P0;
            retVal = beStatus_SUCCESS;
            break;

        case GDT_TAHITI_PRO:
        case GDT_TAHITI_XT:
            chipFamily = FAMILY_SI;
            chipRevision = SI_TAHITI_P_B1;
            retVal = beStatus_SUCCESS;
            break;

        case GDT_PITCAIRN_PRO:
        case GDT_PITCAIRN_XT:
            chipFamily = FAMILY_SI;
            chipRevision = SI_PITCAIRN_PM_A1;
            retVal = beStatus_SUCCESS;
            break;

        case GDT_CAPEVERDE_PRO:
        case GDT_CAPEVERDE_XT:
            chipFamily = FAMILY_SI;
            chipRevision = SI_CAPEVERDE_M_A1;
            retVal = beStatus_SUCCESS;
            break;

        case GDT_OLAND:
            chipFamily = FAMILY_SI;
            chipRevision = SI_OLAND_M_A0;
            retVal = beStatus_SUCCESS;
            break;

        case GDT_HAINAN:
            chipFamily = FAMILY_SI;
            chipRevision = SI_HAINAN_V_A0;
            retVal = beStatus_SUCCESS;
            break;

        case GDT_BONAIRE:
            chipFamily = FAMILY_CI;
            chipRevision = CI_BONAIRE_M_A0;
            break;

        case GDT_HAWAII:
            chipFamily = FAMILY_CI;
            chipRevision = CI_HAWAII_P_A0;
            break;

        case GDT_KALINDI:
            chipFamily = FAMILY_CI;
            chipRevision = CI_BONAIRE_M_A0;
            break;

        case GDT_SPECTRE:
        case GDT_SPECTRE_SL:
        case GDT_SPECTRE_LITE:
            chipFamily = FAMILY_CI;
            chipRevision = KV_SPECTRE_A0;
            break;

        case GDT_SPOOKY:
            chipFamily = FAMILY_CI;
            chipRevision = KV_SPOOKY_A0;
            break;

        case GDT_ICELAND:
            chipFamily = FAMILY_VI;
            chipRevision = VI_ICELAND_M_A0;
            break;

        case GDT_TONGA:
            chipFamily = FAMILY_VI;
            chipRevision = VI_TONGA_P_A0;
            break;

        case GDT_CARRIZO_EMB:
        case GDT_CARRIZO:
            chipFamily = FAMILY_VI;
            chipRevision = CARRIZO_A0;
            break;

        case GDT_STONEY:
            chipFamily = FAMILY_VI;
            chipRevision = STONEY_A0;
            break;

        case GDT_FIJI:
            chipFamily = FAMILY_VI;
            chipRevision = VI_FIJI_P_A0;
            break;

        case GDT_BAFFIN:
            chipFamily = FAMILY_VI;
            chipRevision = VI_BAFFIN_M_A0;
            break;

        case GDT_ELLESMERE:
            chipFamily = FAMILY_VI;
            chipRevision = VI_ELLESMERE_P_A0;
            break;

        case GDT_GFX8_0_4:
        case GDT_VEGAM2:
            chipFamily = FAMILY_VI;
            chipRevision = VI_LEXA_V_A0;
            break;
    }

    return retVal;
}

#ifdef _WIN32

void Backend::AddDxSearchDir(const std::string& dir)
{
    if (std::find(m_customDxLoadPaths.begin(), m_customDxLoadPaths.end(), dir) == m_customDxLoadPaths.end())
    {
        m_customDxLoadPaths.push_back(dir);
    }
}

#endif

bool Backend::GetSupportedPublicDevices(std::set<std::string>& devices)
{
    bool ret = false;

    if (!m_supportedPublicDevices.empty())
    {
        ret = true;
        devices = m_supportedPublicDevices;
    }
    else
    {
        // Get the supported public devices from the OpenCL backend.
        if (m_beOpenCL == nullptr)
        {
            m_beOpenCL = new beProgramBuilderOpenCL;
        }

        beKA::beStatus rc = m_beOpenCL->Initialize();

        if (rc == beStatus_SUCCESS)
        {
            // Retrieve the supported public devices from the OpenCL runtime.
            m_beOpenCL->GetSupportedPublicDevices(devices);
            ret = true;
        }
    }

    return ret;
}

