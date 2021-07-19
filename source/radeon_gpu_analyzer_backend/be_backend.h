//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================
#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_BACKEND_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_BACKEND_H_

// C++.
#include <map>
#include <ostream>
#include <set>
#include <string>
#include <vector>

// We need the lazy version of TSingleton.
// Global ctor/dtor objects in shared libraries have problems.
#define USE_POINTER_SINGLETON 1
#include "TSingleton.h"

// Local.
#include "radeon_gpu_analyzer_backend/be_program_builder.h"

#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4251)
#endif

struct GDT_GfxCardInfo;
class BeProgramBuilderOpencl;
class beProgramBuilderGL;

#ifdef _WIN32
class BeProgramBuilderDx11;
#endif

using namespace std;
using namespace beKA;

class Backend : public TSingleton<Backend>
{
    // TSingleton needs to be able to use our constructor.
    friend class TSingleton<Backend>;

public:
    ~Backend();

    // Initialize the backend.
    beStatus Initialize(BuiltProgramKind program_kind, LoggingCallBackFuncP callback);

    // Retrieve device information.
    static beKA::beStatus GetDeviceInfo(const std::string& device_name, GDT_DeviceInfo& gdt_device_info);
    static beKA::beStatus GetDeviceInfo(const std::string& device_name, const std::vector<GDT_GfxCardInfo>** info);
    static beKA::beStatus GetDeviceInfo(size_t device_id, GDT_GfxCardInfo& info);
    static beStatus GetDeviceInfo(const std::string& device_name, GDT_GfxCardInfo& info);

    // Get device marketing name.
    static beKA::beStatus GetDeviceInfoMarketingName(const std::string& device_name, std::vector<GDT_GfxCardInfo>& info);

    // Get device family and chip revision.
    static beKA::beStatus GetDeviceChipFamilyRevision(const GDT_GfxCardInfo& table_entry, unsigned int& chip_family, unsigned int& chip_revision);

    // Get the per-mode builder.
    BeProgramBuilderOpencl*    theOpenCLBuilder() { return builder_opencl_; }
    beProgramBuilderGL*        theOpenGLBuilder() { return builder_opengl_; }

#ifdef _WIN32
    BeProgramBuilderDx11*      theOpenDXBuilder() { return builder_dx11_; }
#endif

    // Extract the list of supported, public, devices
    // \param devices a set to be populated with the supported device names
    // \returns true for success, false otherwise
    bool GetSupportedPublicDevices(std::set<std::string>& devices);

private:
    // Utility for logging messages.
    bool LogCallBack(const std::string& msg);

    // Private constructor to adhere to singleton pattern.
    Backend();

    // No assignment.
    Backend operator=(const Backend& backend);

    // Builders.
    BeProgramBuilderOpencl* builder_opencl_ = nullptr;
    beProgramBuilderGL* builder_opengl_ = nullptr;
#ifdef _WIN32
    BeProgramBuilderDx11* builder_dx11_ = nullptr;
#endif

    // Additional directories where DX binaries should be searched (e.g. D3D default compiler).
    static std::vector<std::string> custom_dx11_load_paths_;

    std::set<std::string> supported_public_devices_;

    LoggingCallBackFuncP log_callback_ = nullptr;
};

#ifdef _WIN32
    #pragma warning(pop)
#endif

#endif // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_BACKEND_H_
