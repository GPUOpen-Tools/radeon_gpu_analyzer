//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifdef _WIN32

// C++.
#include <algorithm>
#include <iterator>

// Infra.
#include "CElf.h"
#include "DeviceInfoUtils.h"
#include "amdt_base_tools/Include/gtString.h"
#include "amdt_base_tools/Include/gtAssert.h"
#include "amdt_os_wrappers/Include/osFilePath.h"
#include "amdt_os_wrappers/Include/osDirectory.h"
#include "amdt_os_wrappers/Include/osModule.h"
#include "amdt_os_wrappers/Include/osApplication.h"
#include "amdt_os_wrappers/Include/osProcess.h"

// Shared.
#include "common/rga_shared_utils.h"

// Local.
#include "radeon_gpu_analyzer_backend/be_d3d_include_manager.h"
#include "radeon_gpu_analyzer_backend/be_program_builder_dx11.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"
#include "radeon_gpu_analyzer_backend/be_string_constants.h"

// CLI.
#include "source/radeon_gpu_analyzer_cli/kc_utils.h"

#define D3D_COMPILE_STANDARD_FILE_INCLUDE ((ID3DInclude*)(UINT_PTR)1)

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

static const wchar_t* kStrHlslOptionsDefaultCompilerFilename = L"D3DCompiler_47";
static const wchar_t* kStrHlslOptionsDefaultCompilerExtension = L"dll";

// These string constants will be relocated to  a unified DB once the
// device data and meta-data handling mechanism is revised.
static const char* DEVICE_NAME_TONGA = "Tonga";
static const char* DEVICE_NAME_ICELAND = "Iceland";

static const char* AMD_ADAPTER_TOKEN = "AMD";
static const char* kRgaDx11BackendExePath  = "utils\\dx11_adapter.exe";
static const char* kRgaDx11BackendGetAdaptersOption = "--list-adapters";
static const char* kRgaDx11BackendGetAdapterInfoOption = "--get-adapter-info";
static const char* kRgaDx11BackendErrorToken = "Error";

// Error messages.
static const char* STR_ERROR_DXBC_BLOB_PARSE_FAILURE = "Error: failed to parse DXBC blob.";
static const char* STR_ERROR_DXBC_BLOB_INVALID = "Error: invalid DXBC blob.";
static const char* STR_ERROR_DXBC_BLOB_DISASSEMBLE_FAILURE = "Error: failed to disassemble the DXBC blob.";
static const char* STR_ERROR_D3D_COMPILER_EXCEPTION = "Error: exception occurred in D3DCompiler.";

// This function returns true if the given device is
// affected by the HW issue which forces an allocation of
// a fixed number of SGPRs.
static bool IsFixedSgprAlloc(const std::string& deviceName)
{
    // DX only: due to a HW bug, SGPR allocation for
    // Tonga and Iceland devices should be fixed (94 prior to
    // SC interface update, which is until driver 15.10 including,
    // and 96 after SC interface update, which is after driver 15.10)
    bool ret = (deviceName.compare(DEVICE_NAME_TONGA) == 0 ||
                deviceName.compare(DEVICE_NAME_ICELAND) == 0);
    return ret;
}

// ***************************************
// *** INTERNALLY LINKED SYMBOLS - END ***
// ***************************************

HRESULT BeProgramBuilderDx11::AmdDxGsaCompileShaderWrapper(const struct _AmdDxGsaCompileShaderInput* shader_input, struct _AmdDxGsaCompileShaderOutput* shader_output)
{
    HRESULT hr = S_OK;

    __try
    {
        hr = amd_dxx_module_.AmdDxGsaCompileShader(shader_input, shader_output);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return ~hr;
    }

    return hr;
}

beKA::beStatus BeProgramBuilderDx11::CompileAMDIL(const std::string& program_source, const Dx11Options& dx_options)
{
    AmdDxGsaCompileShaderInput shader_input;
    memset(&shader_input, 0, sizeof(AmdDxGsaCompileShaderInput));
    AmdDxGsaCompileOption compile_options[1];
    shader_input.numCompileOptions = 0;
    shader_input.pCompileOptions = compile_options;
    shader_input.chipFamily = dx_options.chip_family;
    shader_input.chipRevision = dx_options.chip_revision;
    shader_input.inputType = AmdDxGsaInputType::GsaInputIlText;

    // The code directly follows the header.
    shader_input.pShaderByteCode = (char*)program_source.c_str();
    shader_input.byteCodeLength = program_source.length();

    AmdDxGsaCompileShaderOutput shader_output;
    memset(&shader_output, 0, sizeof(AmdDxGsaCompileShaderOutput));

    // The size field of this out parameter is the sizeof the struct.
    // This allows for some future expansion (perhaps starting with a version number).
    shader_output.size = sizeof(AmdDxGsaCompileShaderOutput);
    shader_output.pShaderBinary = NULL;

    // Inside a wrapper to handle exceptions like acvio.
    HRESULT result = AmdDxGsaCompileShaderWrapper(&shader_input, &shader_output);

    if (result != S_OK)
    {
        std::stringstream ss;
        ss << AMDDXXModule::s_DefaultModuleName << " AmdDxGsaCompileShader failed." << std::endl;
        LogCallback(ss.str());

        if (shader_output.pShaderBinary != NULL)
        {
            amd_dxx_module_.AmdDxGsaFreeCompiledShader(shader_output.pShaderBinary);
        }

        return kBeStatusAmdDx11GsaCompileShaderFailed;
    }

    // Open the binaries as CElf objects.
    std::vector<char> elfBinary((char*)shader_output.pShaderBinary, (char*)shader_output.pShaderBinary + shader_output.shaderBinarySize);
    CElf* curr_elf = new CElf(elfBinary);

    if (curr_elf->good())
    {
        SetDeviceElf(dx_options.device_name, shader_output);
    }
    else
    {
        // Report about the failure.
        std::stringstream ss;
        ss << "Unable to parse ELF binary.\n";
        LogCallback(ss.str());

        // Release the resources.
        amd_dxx_module_.AmdDxGsaFreeCompiledShader(shader_output.pShaderBinary);

        return kBeStatusNoBinaryForDevice;
    }

    // Free stuff.
    amd_dxx_module_.AmdDxGsaFreeCompiledShader(shader_output.pShaderBinary);

    return kBeStatusSuccess;
}

BeProgramBuilderDx11::BeProgramBuilderDx11(void) : amd_dxx_module_(),
    d3d_compile_module_(D3DCompileModule::s_DefaultModuleName), compiled_elf_()
{
}

BeProgramBuilderDx11::~BeProgramBuilderDx11(void)
{
    amd_dxx_module_.UnloadModule();
}

beKA::beStatus BeProgramBuilderDx11::Initialize(const std::string& dxx_module_name, const std::string& compiler_module_name, bool print_process_cmd_line)
{
    beStatus be_rc = kBeStatusSuccess;
    std::string  compiler_dll_name = compiler_module_name;

    if (compiler_dll_name.empty())
    {
        // This solves the VS extension issue where devenv.exe looked for the D3D compiler
        // at its own directory, instead of looking for it at CodeXL's binaries directory.
        osFilePath default_compiler_file_path;
        bool is_ok = osGetCurrentApplicationDllsPath(default_compiler_file_path, OS_I386_ARCHITECTURE);

        if (is_ok)
        {
            // Create the full path to the default D3D compiler.
            default_compiler_file_path.setFileName(kStrHlslOptionsDefaultCompilerFilename);
            default_compiler_file_path.setFileExtension(kStrHlslOptionsDefaultCompilerExtension);
            compiler_dll_name = default_compiler_file_path.asString().asASCIICharArray();
        }
    }

    // Clear the outputs of former builds (if there are any).
    ClearFormerBuildOutputs();

    // load AMD DXX module
    if (!dxx_module_name.empty())
    {
        if (amd_dxx_module_.IsLoaded())
        {
            amd_dxx_module_.UnloadModule();
        }
        amd_dxx_module_.LoadModule(dxx_module_name);
    }

    if (amd_dxx_module_.IsLoaded() && !amd_dxx_module_.GetModuleName().empty())
    {
        if (print_process_cmd_line)
        {
            std::stringstream ss;
            ss << "Info: " << amd_dxx_module_.GetModuleName() << " module loaded.\n" << std::endl;
            LogCallback(ss.str());
        }
    }
    else
    {
        // Notice: This message receives an extra "\n", since later in the call chain, one is removed. We do want to remove them for
        // the rest of the messages, so we only remove it for initialization messages:
        std::stringstream ss;
        ss << "Error: " << AMDDXXModule::s_DefaultModuleName << " module not loaded.\n\n" << std::endl;
        LogCallback(ss.str());
        be_rc = kBeStatusAmdxxModuleNotLoaded;
    }

    // load D3D compiler module
    if (be_rc == kBeStatusSuccess)
    {
        bool is_dll_load = false;
        int error_code = 0;

        // If the user did not specify a default D3D compiler, use the one in the sub-folder.
        std::string fixed_ms_d3d_module_name = compiler_dll_name;
        if (fixed_ms_d3d_module_name.empty())
        {
#if _WIN64
            fixed_ms_d3d_module_name = "utils\\d3dcompiler_47.dll";
#elif _WIN32
            fixed_ms_d3d_module_name = "x86\\d3dcompiler_47.dll";
#endif
        }

        if (fixed_ms_d3d_module_name.length() > 0)
        {
            is_dll_load = d3d_compile_module_.LoadModule(fixed_ms_d3d_module_name, &error_code);
        }
        else
        {
            is_dll_load = d3d_compile_module_.LoadModule();
        }

        if (!d3d_compile_module_.IsLoaded())
        {
            // Check if the additional search paths should be searched.
            bool should_search_additional_paths = !is_dll_load &&
                                               (fixed_ms_d3d_module_name.find(D3DCompileModule::s_DefaultModuleName) != std::string::npos);

            if (should_search_additional_paths)
            {
                gtString module_name_as_gtstr;
                module_name_as_gtstr << D3DCompileModule::s_DefaultModuleName;

                // Try searching in the additional directories (if any).
                for (const std::string& path : loader_search_directories_)
                {
                    // Build the full path to the module.
                    gtString path_as_gtstr;
                    path_as_gtstr << path.c_str();

                    osFilePath search_path;
                    search_path.setFileDirectory(path_as_gtstr);
                    search_path.setFileName(module_name_as_gtstr);

                    // Try to load the module.
                    is_dll_load = d3d_compile_module_.LoadModule(search_path.asString().asASCIICharArray(), &error_code);

                    if (is_dll_load)
                    {
                        // We are done.
                        break;
                    }
                }
            }

            if (!is_dll_load)
            {
                // Take the relevant module's name.
                const char* module_name = (compiler_dll_name.length() > 0) ?
                                           compiler_dll_name.c_str() : D3DCompileModule::s_DefaultModuleName;

                // This flag will be true if the given D3D module's bitness is 64-bit, while
                // this process' bitness is 32-bit.
                bool is_64_from_32_error = false;
#ifndef _WIN64
                // If we are in 32-bit mode, check if the module's bitness is 64-bit.
                osFilePath modulePath;
                gtString moduleFilaNameAsGtStr;
                moduleFilaNameAsGtStr << module_name;
                modulePath.setFileName(moduleFilaNameAsGtStr);
                osIs64BitModule(moduleFilaNameAsGtStr, is_64_from_32_error);
#endif // !_WIN64

                // Generate the error message.
                std::stringstream ss;
                if (is_64_from_32_error)
                {
                    ss << "Error: " << module_name << " is a 64-bit module, which cannot be loaded during a 32-bit build." << std::endl;
                }
                else
                {
                    // We failed to load the module.
                    // Notice: This message receives an extra "\n", since later in the call chain, one is removed. We do want to remove them for
                    // the rest of the messages, so we only remove it for initialization messages:
                    ss << "Error: " << module_name << " module not loaded. Error = " << error_code << ". Please use D3D compiler version 43 or above." << std::endl;
                }

                // Print the error message, and abort the build process.
                LogCallback(ss.str());
                exit(-1);
            }
        }
    }

    if (be_rc == kBeStatusSuccess)
    {
        std::set<std::string> unique_names_of_published_devices;
        is_initialized_ = BeUtils::GetPreRdna3GraphicsCards(dx_device_table_, unique_names_of_published_devices);
        if (!is_initialized_)
        {
            be_rc = kBeStatusNoDeviceFound;
        }
    }

    if (be_rc == kBeStatusSuccess)
    {
        is_initialized_ = true;
    }

    return be_rc;
}

beKA::beStatus BeProgramBuilderDx11::CompileHLSL(const std::string& program_source, const Dx11Options& dx_options, bool is_dxbc_input)
{
    beKA::beStatus ret = beStatus::kBeStatusGeneralFailed;

    // This blob would hold the DXBC blob (either
    // compiled from HLSL or given as an input by the user).
    ID3DBlob* shader = nullptr;

    // In the case that the input is a DXBC blob, we would read it into this vector.
    std::vector<char> dxbc_blob;
    char*             shader_bytes      = nullptr;
    size_t            shader_byte_count = 0;

    if (!is_dxbc_input)
    {
        ID3DBlob* error_messages = nullptr;

        // Turn options.m_Defines into what D3DCompile expects.
        D3D_SHADER_MACRO* macros                     = new D3D_SHADER_MACRO[dx_options.defines.size() + 1];
        macros[dx_options.defines.size()].Name       = NULL;
        macros[dx_options.defines.size()].Definition = NULL;

        // Handle the defines.
        if (!dx_options.defines.empty())
        {
            int i = 0;
            for (std::vector<std::pair<std::string, std::string> >::const_iterator it = dx_options.defines.begin(); it != dx_options.defines.end(); ++it, i++)
            {
                macros[i].Name       = it->first.c_str();
                macros[i].Definition = it->second.c_str();
            }
        }

        // Handle custom includes.
        ID3DInclude* include_mechanism = D3D_COMPILE_STANDARD_FILE_INCLUDE;

        if (!dx_options.include_directories.empty())
        {
            // Extract the source file's directory.
            gtString shader_filename;
            shader_filename << dx_options.filename.c_str();
            osFilePath tmpFilePath(shader_filename);

            gtString    shader_dir_str;
            osDirectory shader_dir;
            bool        is_dir_extracted = tmpFilePath.getFileDirectory(shader_dir);

            if (is_dir_extracted)
            {
                // Create an include manager.
                shader_dir_str    = shader_dir.directoryPath().asString();
                include_mechanism = new D3dIncludeManager(shader_dir_str.asASCIICharArray(), dx_options.include_directories);
            }
        }

        // Invoke the offline compiler.
        HRESULT result = E_FAIL;
        try
        {
            result = d3d_compile_module_.D3DCompile(program_source.c_str(),
                                                    program_source.length(),
                                                    LPCSTR(dx_options.filename.c_str()),
                                                    macros,
                                                    include_mechanism,
                                                    dx_options.entrypoint.c_str(),
                                                    dx_options.target.c_str(),
                                                    dx_options.dx_flags.flags_as_int,
                                                    0,
                                                    &shader,
                                                    &error_messages);
        }
        catch (...)
        {
            LogCallback(STR_ERROR_D3D_COMPILER_EXCEPTION);
            return kBeStatusD3dCompileFailed;
        }

        // Release resources.
        delete[] macros;

        if (error_messages != NULL)
        {
            char*             error_string = (char*)error_messages->GetBufferPointer();
            size_t            error_size   = error_messages->GetBufferSize();
            std::stringstream ss;
            ss << std::string(error_string, error_size);
            LogCallback(ss.str());
            error_messages->Release();
        }

        if (result != S_OK)
        {
            if (shader != NULL)
            {
                shader->Release();
            }
            return kBeStatusD3dCompileFailed;
        }
    }
    else
    {
        [[maybe_unused]] bool is_blob_read = BeUtils::ReadBinaryFile(dx_options.filename, dxbc_blob);
        assert(is_blob_read);
        shader_bytes      = dxbc_blob.data();
        shader_byte_count = dxbc_blob.size();
    }

    // For DX10 & DX11, we need to peel off the Microsoft headers.
    // This will get us to the executable bit that the RTL usually passes
    // down to the driver.  That's what the code below wants.
    // For DX9, the object has no header (it's all byte code).
    // But for DX9, we need to talk to a different driver and that's NYI.
    if (shader_bytes == nullptr)
    {
        shader_bytes = (char*)shader->GetBufferPointer();
    }
    if (shader_byte_count == 0)
    {
        shader_byte_count = shader->GetBufferSize();
    }
    assert(shader_bytes != nullptr);
    assert(shader_byte_count != 0);

    if (shader_bytes != nullptr && shader_byte_count != 0)
    {
        std::string sShader(shader_bytes, shader_byte_count);

        // If requested by the user, disassemble D3D offline compiler's output.
        if (dx_options.should_dump_ms_intermediate && ms_intermediate_text_.empty())
        {
            ID3DBlob* disassembly = NULL;
            HRESULT   result      = d3d_compile_module_.D3DDisassemble(shader_bytes, shader_byte_count, 0, "", &disassembly);

            if (result == S_OK)
            {
                shader_bytes          = (char*)disassembly->GetBufferPointer();
                shader_byte_count     = disassembly->GetBufferSize();
                ms_intermediate_text_ = std::string(shader_bytes, shader_byte_count);
            }
            else
            {
                // If failed, report and continue.
                LogCallback(STR_ERROR_DXBC_BLOB_DISASSEMBLE_FAILURE);
            }
        }

        // Perform the actual compilation.
        ret = CompileDXAsm(sShader, dx_options);
    }

    return ret;
}

static bool ExtractShaderCode(const std::string& program_source, uint32_t& code_size, const char*& code)
{
    bool ret = false;

    // Flag set to true if we need to abort the parsing process.
    bool should_abort = false;

    // Validate the binary.
    size_t dxbc_offset = program_source.find("DXBC");
    assert(dxbc_offset == 0);
    if (dxbc_offset == 0)
    {
        // Pointer to the current section in the DXBC blob.
        const uint32_t* dxbc = (const uint32_t*)program_source.data();

        // Pointer to the end of the blob.
        const uint32_t* end = (const uint32_t*)(dxbc + program_source.size());

        // Skip the "DXBC" section.
        dxbc++;

        // Skip the hash.
        dxbc += 4;

        // Skip unknown and file length.
        dxbc++;
        dxbc++;

        assert(dxbc < end);
        if (dxbc < end)
        {
            const uint32_t num_chunks = *dxbc;
            dxbc++;

            // Extract the information about all chunks within this blob.
            std::vector<uint32_t> chunk_offsets;
            for (uint32_t i = 0; !should_abort && i < num_chunks; i++)
            {
                assert(dxbc < end);
                if (dxbc < end)
                {
                    chunk_offsets.push_back(*dxbc);
                    dxbc++;
                }
                else
                {
                    should_abort = true;
                    std::cout << STR_ERROR_DXBC_BLOB_PARSE_FAILURE << std::endl;
                }
            }

            if (!should_abort)
            {
                const char* start = program_source.data();

                // Look for the relevant chunk (SHEX or SHDR), extract the code pointer and its size.
                for (uint32_t offset : chunk_offsets)
                {
                    if (ret)
                    {
                        // We are done.
                        break;
                    }

                    dxbc = (const uint32_t *)(start + offset);
                    assert(dxbc + 2 < end);
                    if (dxbc + 2 < end)
                    {
                        const char* dxbc_char = (char*)dxbc;
                        if ((dxbc_char[0] == 'S' && dxbc_char[1] == 'H' && dxbc_char[2] == 'E' && dxbc_char[3] == 'X') ||
                            (dxbc_char[0] == 'S' && dxbc_char[1] == 'H' && dxbc_char[2] == 'D' && dxbc_char[3] == 'R'))
                        {
                            dxbc++;
                            code_size = *dxbc;
                            code = (char*)(++dxbc);
                            assert(dxbc + (code_size / 4) <= end);
                            ret = (dxbc + (code_size / 4) <= end);
                        }
                    }
                    else
                    {
                        std::cout << STR_ERROR_DXBC_BLOB_PARSE_FAILURE << std::endl;
                        should_abort = true;
                        break;
                    }
                }
            }
        }
        else
        {
            should_abort = true;
            std::cout << STR_ERROR_DXBC_BLOB_PARSE_FAILURE << std::endl;
        }
    }
    else
    {
        std::cout << STR_ERROR_DXBC_BLOB_INVALID << std::endl;
    }

    ret = ret && !should_abort;
    return ret;
}

beKA::beStatus BeProgramBuilderDx11::CompileDXAsm(const std::string& program_source, const Dx11Options& dx_options)
{
    uint32_t shader_code_size = 0;
    const char* shader_code = nullptr;
    [[maybe_unused]] bool is_ok            = ExtractShaderCode(program_source, shader_code_size, shader_code);
    assert(is_ok);

    AmdDxGsaCompileShaderInput shader_input;
    memset(&shader_input, 0, sizeof(AmdDxGsaCompileShaderInput));
    AmdDxGsaCompileOption compile_options[2];
    memset(compile_options, 0, 2*sizeof(AmdDxGsaCompileOption));
    if (dx_options.is_shader_intrinsics_enabled)
    {
        // Enable D3D11 shader intrinsics. The value does not matter, only the setting does.
        compile_options[0].setting = AmdDxGsaCompileOptionEnum::AmdDxGsaEnableShaderIntrinsics;
        shader_input.numCompileOptions = 1;

        if (dx_options.uav_slot > -1)
        {
            // Handle the case where the user defined a UAV slot for shader intrinsics.
            compile_options[1].setting = AmdDxGsaCompileOptionEnum::AmdDxGsaShaderIntrinsicsUAVSlot;
            compile_options[1].value = dx_options.uav_slot;
            shader_input.numCompileOptions = 2;
        }
    }
    else
    {
        shader_input.numCompileOptions = 0;
    }
    shader_input.pCompileOptions = compile_options;

    shader_input.chipFamily = dx_options.chip_family;
    shader_input.chipRevision = dx_options.chip_revision;

    // The code directly follows the header.
    shader_input.pShaderByteCode = shader_code;
    shader_input.byteCodeLength = shader_code_size;

    AmdDxGsaCompileShaderOutput shader_output;
    memset(&shader_output, 0, sizeof(AmdDxGsaCompileShaderOutput));

    // The size field of this out parameter is the sizeof the struct.
    // This allows for some future expansion (perhaps starting with a version number).
    shader_output.size = sizeof(AmdDxGsaCompileShaderOutput);
    // For good measure, zero the output pointer.
    shader_output.pShaderBinary = NULL;

    // Inside a wrapper to handle exceptions like acvio.
    HRESULT result = AmdDxGsaCompileShaderWrapper(&shader_input, &shader_output);

    if (result != S_OK)
    {
        std::stringstream ss;
        ss << AMDDXXModule::s_DefaultModuleName << " AmdDxGsaCompileShader failed." << std::endl;
        LogCallback(ss.str());

        if (shader_output.pShaderBinary != NULL)
        {
            amd_dxx_module_.AmdDxGsaFreeCompiledShader(shader_output.pShaderBinary);
        }

        return kBeStatusAmdDx11GsaCompileShaderFailed;
    }

    // Open the binaries as CElf objects.
    std::vector<char> elf_binary((char*)shader_output.pShaderBinary, (char*)shader_output.pShaderBinary + shader_output.shaderBinarySize);
    CElf* curr_elf = new CElf(elf_binary);

    if (curr_elf->good())
    {
        SetDeviceElf(dx_options.device_name, shader_output);
    }
    else
    {
        // Report about the failure.
        std::stringstream ss;
        ss << "Unable to parse ELF binary.\n";
        LogCallback(ss.str());

        // Release the resources.
        amd_dxx_module_.AmdDxGsaFreeCompiledShader(shader_output.pShaderBinary);

        return kBeStatusNoBinaryForDevice;
    }

    // Free stuff.
    amd_dxx_module_.AmdDxGsaFreeCompiledShader(shader_output.pShaderBinary);

    return kBeStatusSuccess;
}

beKA::beStatus BeProgramBuilderDx11::Compile(RgaMode mode, const std::string& program_source,
    const Dx11Options& dx_options, bool is_dxbc_input)
{
    if (!is_initialized_)
    {
        std::stringstream ss;
        ss << "DX Module not initialized";
        LogCallback(ss.str());
        return kBeStatusD3dCompilerModuleNotLoaded;
    }

    beStatus be_ret = kBeStatusGeneralFailed;

    if (mode == RgaMode::kModeDx11)
    {
        be_ret = CompileHLSL(program_source, dx_options, is_dxbc_input);
    }
    else if (mode == RgaMode::kModeAmdil)
    {
        be_ret = CompileAMDIL(program_source, dx_options);
    }
    else
    {
        std::stringstream ss;
        ss << "Source language not supported";
        LogCallback(ss.str());
    }

    return be_ret;
}

beKA::beStatus BeProgramBuilderDx11::GetISABinary(const std::string& device, std::vector<char>& binary)
{
    const CElfSection* text_section = GetISATextSection(device);
    beStatus result = kBeStatusInvalid;

    if (nullptr != text_section)
    {
        result = kBeStatusSuccess;
        binary = text_section->GetData();
    }

    return result;

}

beKA::beStatus BeProgramBuilderDx11::GetKernelIlText(const std::string& device, const std::string& kernel, std::string& il)
{
    // For DX shaders, we currently cannot disassemble the IL binary section.
    // For now, we extract D3D ASM code.
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    il.clear();
    beStatus be_ret = GetIntermediateMsBlob(il);
    return be_ret;
}

beKA::beStatus BeProgramBuilderDx11::GetKernelIsaText(const std::string& device, const std::string& shader_name, std::string& isa)
{
    // Not implemented: see BeProgramBuilderDx11::GetDxShaderD3DASM.
    // The current inheritance architecture where BeProgramBuilderDx11 and
    // beProgramBuilderCL share the same interface for ISA extraction does not hold.
    // This mechanism will be refactored.
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(shader_name);
    GT_UNREFERENCED_PARAMETER(isa);
    return kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderDx11::GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis)
{
    GT_UNREFERENCED_PARAMETER(kernel);

    beStatus ret = beKA::kBeStatusGeneralFailed;

    // There is no symbol table.  We just get the .stats  section.
    CElf* pElf = GetDeviceElf(device);

    if (pElf != nullptr)
    {
        // Get the .stats section.
        CElfSection* stats_section = pElf->GetSection(".stats");

        if (stats_section != nullptr)
        {
            // This is the binary image.
            const std::vector<char>& section_data = stats_section->GetData();

            const AmdDxGsaCompileStats* pStats = reinterpret_cast<const AmdDxGsaCompileStats*>(section_data.data());

            if (pStats != nullptr)
            {
                (void)memset(&analysis, 0, sizeof(analysis));

                if (!IsFixedSgprAlloc(device))
                {
                    analysis.num_sgprs_used = pStats->numSgprsUsed;
                }
                else
                {
                    // Assign the fixed value. This is due to a HW bug. The fixed values are: 94 (excluding
                    // the 2 VCC registers) until driver 15.10 (including), and 96 afterwards (to be handled).
                    const unsigned SGPRs_USED_TONGA_ICELAND = 94;
                    analysis.num_sgprs_used = SGPRs_USED_TONGA_ICELAND;
                }

                analysis.num_sgprs_available = pStats->availableSgprs;
                analysis.num_vgprs_used = pStats->numVgprsUsed;
                analysis.num_vgprs_available = pStats->availableVgprs;
                analysis.lds_size_used = pStats->usedLdsBytes;
                analysis.lds_size_available = pStats->availableLdsBytes;
                analysis.scratch_memory_used = pStats->usedScratchBytes;
                analysis.num_alu_instructions = pStats->numAluInst;
                analysis.num_instructions_control_flow = pStats->numControlFlowInst;
                analysis.num_instructions_fetch = pStats->numTfetchInst;

                // We are done.
                ret = beKA::kBeStatusSuccess;
            }
        }
        else
        {
            ret = kBeStatusElfStatisticsSectionMissing;
        }
    }

    return ret;
}

void BeProgramBuilderDx11::ClearFormerBuildOutputs()
{
    // Clear the ELF sections.
    for (auto& dev_elf_pair : compiled_elf_)
    {
        CelfBinaryPair& elf_elf_binary_pair = dev_elf_pair.second;
        delete elf_elf_binary_pair.first;
        elf_elf_binary_pair.first = nullptr;
        elf_elf_binary_pair.second.clear();
    }

    compiled_elf_.clear();

    // Clear the DSASM text.
    ms_intermediate_text_.clear();
}

void BeProgramBuilderDx11::ReleaseProgram()
{
    ClearFormerBuildOutputs();
}

beKA::beStatus BeProgramBuilderDx11::GetDeviceTable(std::vector<GDT_GfxCardInfo>& table)
{
    table = dx_device_table_;
    return kBeStatusSuccess;
}

#ifdef DXASM_T_ENABLED
/////////////////////////////////////////////////////
// Callback functions
/////////////////////////////////////////////////////
void* XLT_STDCALL allocSysMem(void* handle, unsigned int dw_size_in_bytes)
{
    (void)(handle); // Unreferenced parameter

    return malloc(dw_size_in_bytes);
}

void XLT_STDCALL freeSysMem(void* handle, void* address)
{
    (void)(handle); // Unreferenced parameter

    free(address);
}

int XLT_STDCALL outputString(XLT_PVOID handle, const char* translated_str, ...)
{
    (void)(handle);

    char formatted_buffer[1024];
    va_list va;

    va_start(va, translated_str);
    vsprintf(formatted_buffer, translated_str, va);
    va_end(va);

    return 0;
}

string* BeProgramBuilderDx11::translated_program_;
int* BeProgramBuilderDx11::translated_program_size_;

beKA::beStatus BeProgramBuilderDx11::CompileDXAsmT(const string& program_source, const Dx11Options& dx_options)
{
    // lets convert the source, which is DX ASM as text to DX ASM binary which is what DX driver expects.
    XLT_PROGINFO xlt_input;
    XLT_CALLBACKS xlt_callbacks;

    memset(&xlt_callbacks, 0, sizeof(XLT_CALLBACKS));

    xlt_callbacks.eXltMode = E_XLT_NORMAL;
    xlt_callbacks.pHandle = NULL;
    xlt_callbacks.AllocateSysMem = allocSysMem;
    xlt_callbacks.FreeSysMem = freeSysMem;
    xlt_callbacks.OutputString = outputString;
    xlt_callbacks.Assert = NULL;
    xlt_callbacks.flags = 0;
    xlt_input.pBuffer = (char*)program_source.c_str();
    xlt_input.nBufferSize = (int)program_source.length() + 1;

    char*  bin_buffer;
    string stemp;
    int buffer_size = 0;
    unsigned int stemp_size;

    translated_program_size_ = &buffer_size;
    translated_program_ = &stemp;

    DX10AsmText2Stream(&xlt_input, &xlt_callbacks, bin_buffer, stemp_size);

    // const D3D10_ChunkHeader* byteCodeHeader = ( D3D10_ChunkHeader* ) pTheBinBuffer;

    AmdDxGsaCompileShaderInput shader_Input;
    memset(&shader_Input, 0, sizeof(AmdDxGsaCompileShaderInput));
    AmdDxGsaCompileOption compile_options[1];
    memset(compile_options, 0, sizeof(AmdDxGsaCompileOption));

    shader_Input.chipFamily = dx_options.chip_family;
    shader_Input.chipRevision = dx_options.chip_revision;
    shader_Input.pShaderByteCode = (char*)(bin_buffer);  // The code directly follows the header.
    shader_Input.byteCodeLength = stemp_size;
    shader_Input.pCompileOptions = compile_options;
    shader_Input.numCompileOptions = 0;

    AmdDxGsaCompileShaderOutput shader_output;
    memset(&shader_output, 0, sizeof(AmdDxGsaCompileShaderOutput));
    // The size field of this out parameter is the sizeof the struct.
    // This allows for some future expansion (perhaps starting with a version number).
    shader_output.size = sizeof(AmdDxGsaCompileShaderOutput);
    // For good measure, zero the output pointer.
    shader_output.pShaderBinary = NULL;

    // Inside a wrapper to handle exceptions like acvio.
    HRESULT result = AmdDxGsaCompileShaderWrapper(&shader_Input, &shader_output);

    if (result != S_OK)
    {
        stringstream ss;
        ss << AMDDXXModule::s_DefaultModuleName << " AmdDxGsaCompileShader failed." << endl;
        LogCallback(ss.str());

        if (shader_output.pShaderBinary != NULL)
        {
            amd_dxx_module_.AmdDxGsaFreeCompiledShader(shader_output.pShaderBinary);
        }

        return kBeStatusAmdDx11GsaCompileShaderFailed;
    }

    // Crack open the binaries as CElf objects.
    vector<char> elf_binary((char*)shader_output.pShaderBinary, (char*)shader_output.pShaderBinary + shader_output.shaderBinarySize);
    CElf* pElf = new CElf(elf_binary);

    if (pElf->good())
    {
        // Store the ELF section in the relevant container.
        SetDeviceElf(dx_options.device_name, shader_output);
    }
    else
    {
        // Inform the user about the failure.
        stringstream ss;
        ss << "Unable to parse ELF binary.\n";
        LogCallback(ss.str());

        // Release the resources.
        amd_dxx_module_.AmdDxGsaFreeCompiledShader(shader_output.pShaderBinary);

        return kBeStatusNoBinaryForDevice;
    }

    // Free stuff.
    amd_dxx_module_.AmdDxGsaFreeCompiledShader(shader_output.pShaderBinary);

    return kBeStatusSuccess;
}
#endif DXASM_T_ENABLED

beKA::beStatus BeProgramBuilderDx11::GetIntermediateMsBlob(std::string& intermediate_ms_blob)
{
    intermediate_ms_blob = ms_intermediate_text_;
    return kBeStatusSuccess;
}

void BeProgramBuilderDx11::SetIntermediateMsBlob(const std::string& intermediate_ms_code)
{
    ms_intermediate_text_ = intermediate_ms_code;
}

void BeProgramBuilderDx11::AddDxSearchDir(const std::string& dir)
{
    if (find(loader_search_directories_.begin(),
             loader_search_directories_.end(), dir) == loader_search_directories_.end())
    {
        loader_search_directories_.push_back(dir);
    }
}

const CElfSection* BeProgramBuilderDx11::GetISATextSection(const std::string& device_name) const
{
    // Get the relevant ELF section for the required device.
    const CElfSection* result = nullptr;
    CElf* elf = GetDeviceElf(device_name);
    if (elf != nullptr)
    {
        // There is no symbol table. We just need the .text section.
        const std::string CODE_SECTION_NAME(".text");
        result = elf->GetSection(CODE_SECTION_NAME);
    }

    return result;
}

const CElfSection* BeProgramBuilderDx11::GetElfSection(const std::string& device_name, const std::string& section_name) const
{
    // Get the relevant ELF section for the required device.
    const CElfSection* result = nullptr;
    CElf* pElf = GetDeviceElf(device_name);

    if (pElf != nullptr)
    {
        // There is no symbol table.  We just need the ELF section.
        result = pElf->GetSection(section_name);
    }

    return result;
}

const CElfSection* BeProgramBuilderDx11::GetILDisassemblySection(const std::string& device_name) const
{
    const std::string kIlDisassemblySectionName(".amdil_disassembly");
    return GetElfSection(device_name, kIlDisassemblySectionName);
}

beKA::beStatus BeProgramBuilderDx11::GetDxShaderIsaText(const std::string& device, std::string& isa_buffer)
{
    beKA::beStatus ret = kBeStatusNoIsaForDevice;

    // Get the pointer to the ISA disassembly section.
    const std::string kIsaDisassemblySectionName(".disassembly");
    const CElfSection* elf_section = GetElfSection(device, kIsaDisassemblySectionName);

    // Extract the disassembly from the ELF section's data.
    isa_buffer.clear();
    ExtractTextFromElfSection(elf_section, isa_buffer);
    if (!isa_buffer.empty())
    {
        // We are done.
        ret = beKA::kBeStatusSuccess;
    }

    return ret;
}

beKA::beStatus BeProgramBuilderDx11::GetDxShaderIl(const std::string& device, std::string& isa_buffer)
{
    beKA::beStatus ret = kBeStatusNoIlForDevice;
    isa_buffer.clear();

    // Get the relevant ELF section for the required device.
    const CElfSection* amdil_disassembly_section = GetILDisassemblySection(device);

    // Extract the disassembly from the ELF section.
    ExtractTextFromElfSection(amdil_disassembly_section, isa_buffer);
    if (!isa_buffer.empty())
    {
        // We are done.
        ret = beKA::kBeStatusSuccess;
    }

    return ret;
}

void BeProgramBuilderDx11::ExtractTextFromElfSection(const CElfSection* section, std::string& content)
{
    if (section != nullptr)
    {
        // This is the disassembly section (ASCII representation).
        const std::vector<char>& section_data = section->GetData();
        content = std::string(section_data.begin(), section_data.end());
    }
}

bool BeProgramBuilderDx11::GetIsaSize(const std::string& isa_as_text, size_t& size_in_bytes) const
{
    // The length in characters of a 32-bit instruction in text format.
    const size_t kInstruction32Length = 8;
    const size_t kInstruction32SizeInBytes = 4;

    // The length in characters of a 64-bit instruction in text format.
    const size_t kInstruction64Length = 16;
    const size_t kInstruction64SizeInBytes = 8;

    bool ret = false;
    size_in_bytes = 0;

    size_t pos_begin = isa_as_text.rfind("//");
    if (pos_begin != std::string::npos)
    {
        size_t pos_first_32_bit_end = isa_as_text.find(':', pos_begin);

        // Get past the "// " prefix.
        pos_begin += 3;

        // Determine the length of the PC string.
        size_t pc_length = (pos_first_32_bit_end - pos_begin);
        if (pos_first_32_bit_end != std::string::npos && pc_length > 0)
        {
            GT_IF_WITH_ASSERT(pos_begin + pc_length < isa_as_text.size())
            {
                // Get the first 32 bit of the final instruction.
                const std::string& instruction_first_32_bit = isa_as_text.substr(pos_begin, pc_length);

                // Convert the PC of the final instruction. This will indicate
                // how many bytes we used until the final instruction (excluding
                // the final instruction).
                size_in_bytes = std::stoul(instruction_first_32_bit, nullptr, 16);

                // Get past the prefix.
                pos_first_32_bit_end += 2;

                // Find the end of the final instruction line.
                size_t pos_end = isa_as_text.find("\n", pos_begin);

                if (pos_end != std::string::npos && pos_first_32_bit_end < pos_end)
                {
                    // Extract the final instruction as text.
                    const std::string& instructionAsText = isa_as_text.substr(pos_first_32_bit_end, pos_end - pos_first_32_bit_end);
                    const size_t instruction_character_count = instructionAsText.size();
                    if (instruction_character_count == kInstruction32Length)
                    {
                        // The final instruction is a 32-bit instruction.
                        size_in_bytes += kInstruction32SizeInBytes;
                        ret = true;
                    }
                    else if (instruction_character_count == (kInstruction64Length + 1))
                    {
                        // The final instruction is a 64-bit instruction.
                        size_in_bytes += kInstruction64SizeInBytes;
                        ret = true;
                    }
                    else
                    {
                        // We shouldn't get here.
                        GT_ASSERT_EX(false, L"Unknown instruction size");
                    }
                }
            }
        }
    }

    return ret;
}

bool BeProgramBuilderDx11::GetWavefrontSize(const std::string& device_name, size_t& wavefront_size) const
{
    bool ret = false;
    wavefront_size = 0;

    // For Navi targets, the wave size is being determined in runtime - keep it at zero.
    if (!RgaSharedUtils::IsNaviTarget(device_name))
    {
        // Extract the device info.
        GDT_DeviceInfo device_info;
        if (AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(device_name.c_str(), device_info))
        {
            wavefront_size = device_info.m_nWaveSize;
            ret = true;
        }
    }
    else
    {
        ret = true;
    }

    return ret;
}

std::string BeProgramBuilderDx11::ToLower(const std::string& str) const
{
    std::string result;
    std::transform(str.begin(), str.end(), inserter(result, result.begin()), [](const char& c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

void BeProgramBuilderDx11::SetDeviceElf(const std::string& device_name, const AmdDxGsaCompileShaderOutput& shader_output)
{
    if (!device_name.empty())
    {
        // First, convert the device name to lower case.
        std::string device_name_lower_case = ToLower(device_name);

        // Update the container.
        CelfBinaryPair& elf_pair = compiled_elf_[device_name_lower_case];
        elf_pair.second.assign((char*)shader_output.pShaderBinary, (char*)shader_output.pShaderBinary + shader_output.shaderBinarySize);

        // Open the binaries as CElf objects.
        elf_pair.first = new CElf(elf_pair.second);
    }
}

bool BeProgramBuilderDx11::GetDeviceElfBinPair(const std::string& device_name, CelfBinaryPair& elf_bin_pair) const
{
    bool result =  false;
    if (!device_name.empty())
    {
        // First, convert the device name to lower case.
        std::string device_name_lower_case = ToLower(device_name);

        // Look for the relevant element.
        auto iter = compiled_elf_.find(device_name_lower_case);
        if (iter != compiled_elf_.end())
        {
            elf_bin_pair = iter->second;
            result = true;
        }
    }

    return result;
}

static beKA::beStatus  InvokeDx11Backend(const std::string& args, bool should_print_cmd, std::string& output)
{
    beKA::beStatus status = kBeStatusdxDriverLaunchFailure;
    std::stringstream  cmd_line;
    bool cancel_signal = false;
    gtString output_gtstr;
    cmd_line << kRgaDx11BackendExePath << " " << args;

    if (should_print_cmd)
    {
        BeUtils::PrintCmdLine(cmd_line.str(), should_print_cmd);
    }

    bool result = osExecAndGrabOutput(cmd_line.str().c_str(), cancel_signal, output_gtstr);

    if (result)
    {
        status = kBeStatusSuccess;
        output = output_gtstr.asASCIICharArray();
    }

    return status;
}

CElf* BeProgramBuilderDx11::GetDeviceElf(const std::string& device_name) const
{
    CElf* ret = nullptr;
    CelfBinaryPair elf_bin_pair;

    if (GetDeviceElfBinPair(device_name, elf_bin_pair))
    {
        ret = elf_bin_pair.first;
    }

    return ret;
}

std::vector<char> BeProgramBuilderDx11::GetDeviceBinaryElf(const std::string& device_name) const
{
    std::vector<char> result;
    CelfBinaryPair elf_bin_pair;

    if (GetDeviceElfBinPair(device_name, elf_bin_pair))
    {
        result = elf_bin_pair.second;
    }

    return result;
}

void BeProgramBuilderDx11::SetPublicDeviceNames(const std::set<std::string>& public_device_name)
{
    public_device_names_ = public_device_name;
}

bool BeProgramBuilderDx11::GetSupportedDisplayAdapterNames(bool should_print_cmd, std::vector<std::string>& adapter_names)
{
    bool result = false;
    std::string backend_output;
    beKA::beStatus status = InvokeDx11Backend(kRgaDx11BackendGetAdaptersOption, should_print_cmd, backend_output);

    result = (status == beKA::kBeStatusSuccess && !backend_output.empty());

    if (result)
    {
        if (backend_output.find(kRgaDx11BackendErrorToken) == std::string::npos)
        {
            std::stringstream  output_stream;
            std::string  adapter_name;
            output_stream << backend_output;
            while (std::getline(output_stream, adapter_name, '\n'))
            {
                adapter_names.push_back(adapter_name);
            }
        }
        else
        {
            result = false;
        }
    }

    return result;
}

bool BeProgramBuilderDx11::GetDxxModulePathForAdapter(int adapter_id, bool should_print_cmd, std::string& adapter_name, std::string& dxx_module_path)
{
    bool result = false;
    std::string  backend_output;
    std::stringstream  args;
    args << kRgaDx11BackendGetAdapterInfoOption << " " << adapter_id;

    // Launch the DX11 backend.
    beKA::beStatus  status = InvokeDx11Backend(args.str().c_str(), should_print_cmd, backend_output);
    result = (status == beKA::kBeStatusSuccess && !backend_output.empty());

    if (result)
    {
        if (backend_output.find(kRgaDx11BackendErrorToken) == std::string::npos)
        {
            std::stringstream  output_stream;
            output_stream << backend_output;
            result = (std::getline(output_stream, adapter_name, '\n') && std::getline(output_stream, dxx_module_path));
        }
        else
        {
            result = false;
        }
    }

    return result;
}
#endif
