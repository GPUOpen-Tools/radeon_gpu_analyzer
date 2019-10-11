//=================================================================
// Copyright 2017 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

#ifdef _WIN32

#include <CElf.h>

// C++.
#include <algorithm>
#include <iterator>

// Local.
#include <RadeonGPUAnalyzerBackend/Include/beD3DIncludeManager.h>
#include <RadeonGPUAnalyzerBackend/Include/beProgramBuilderDX11.h>
#include <RadeonGPUAnalyzerBackend/Include/beUtils.h>
#include <RadeonGPUAnalyzerBackend/Include/beStringConstants.h>
#include <DeviceInfoUtils.h>

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#ifdef _WIN32
    #pragma warning(pop)
#endif

using namespace std;

#define D3D_COMPILE_STANDARD_FILE_INCLUDE ((ID3DInclude*)(UINT_PTR)1)

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

// These string constants will be relocated to  a unified DB once the
// device data and meta-data handling mechanism is revised.
static const char* DEVICE_NAME_TONGA    = "Tonga";
static const char* DEVICE_NAME_ICELAND  = "Iceland";

static const char* AMD_ADAPTER_TOKEN                = "AMD";
static const char* RGA_DX11_DRIVER_EXECUTABLE_PATH  = "x64\\RGADX11.exe";
static const char* RGA_DX11_DRIVER_GET_ADAPTERS_ARG = "--list-adapters";
static const char* RGA_DX11_DRIVER_GET_ADAPTER_INFO = "--get-adapter-info";
static const char* RGA_DX11_DRIVER_ERROR_TOKEN      = "Error";

// Error messages.
static const char* STR_ERROR_DXBC_BLOB_PARSE_FAILURE = "Error: failed to parse DXBC blob.";
static const char* STR_ERROR_DXBC_BLOB_INVALID = "Error: invalid DXBC blob.";
static const char* STR_ERROR_DXBC_BLOB_DISASSEMBLE_FAILURE = "Error: failed to disassemble the DXBC blob.";
static const char* STR_ERROR_D3D_COMPILER_EXCEPTION = "Error: exception occurred in D3DCompiler.";

// This function returns true if the given device is
// affected by the HW issue which forces an allocation of
// a fixed number of SGPRs.
static bool IsFixedSgprAlloc(const string& deviceName)
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


HRESULT beProgramBuilderDX::AmdDxGsaCompileShaderWrapper(const struct _AmdDxGsaCompileShaderInput* shaderInput, struct _AmdDxGsaCompileShaderOutput* shaderOutput)
{
    HRESULT hr = S_OK;

    __try
    {
        hr = m_TheAMDDXXModule.AmdDxGsaCompileShader(shaderInput, shaderOutput);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return ~hr;
    }

    return hr;
}


beKA::beStatus beProgramBuilderDX::CompileAMDIL(const std::string& programSource, const DXOptions& dxOptions)
{
    AmdDxGsaCompileShaderInput shaderInput;
    memset(&shaderInput, 0, sizeof(AmdDxGsaCompileShaderInput));
    AmdDxGsaCompileOption compileOptions[1];
    shaderInput.numCompileOptions = 0;
    shaderInput.pCompileOptions = compileOptions;
    shaderInput.chipFamily = dxOptions.m_ChipFamily;
    shaderInput.chipRevision = dxOptions.m_ChipRevision;
    shaderInput.inputType = AmdDxGsaInputType::GsaInputIlText;

    // The code directly follows the header.
    shaderInput.pShaderByteCode = (char*)programSource.c_str();
    shaderInput.byteCodeLength = programSource.length();

    AmdDxGsaCompileShaderOutput shaderOutput;
    memset(&shaderOutput, 0, sizeof(AmdDxGsaCompileShaderOutput));

    // The size field of this out parameter is the sizeof the struct.
    // This allows for some future expansion (perhaps starting with a version number).
    shaderOutput.size = sizeof(AmdDxGsaCompileShaderOutput);
    shaderOutput.pShaderBinary = NULL;

    // Inside a wrapper to handle exceptions like acvio.
    HRESULT result = AmdDxGsaCompileShaderWrapper(&shaderInput, &shaderOutput);

    if (result != S_OK)
    {
        stringstream ss;
        ss << AMDDXXModule::s_DefaultModuleName << " AmdDxGsaCompileShader failed." << endl;
        LogCallBack(ss.str());

        if (shaderOutput.pShaderBinary != NULL)
        {
            m_TheAMDDXXModule.AmdDxGsaFreeCompiledShader(shaderOutput.pShaderBinary);
        }

        return beStatus_AmdDxGsaCompileShader_FAILED;
    }

    // Open the binaries as CElf objects.
    vector<char> elfBinary((char*)shaderOutput.pShaderBinary, (char*)shaderOutput.pShaderBinary + shaderOutput.shaderBinarySize);
    CElf* pCurrElf = new CElf(elfBinary);

    if (pCurrElf->good())
    {
        SetDeviceElf(dxOptions.m_deviceName, shaderOutput);
    }
    else
    {
        // Report about the failure.
        stringstream ss;
        ss << "Unable to parse ELF binary.\n";
        LogCallBack(ss.str());

        // Release the resources.
        m_TheAMDDXXModule.AmdDxGsaFreeCompiledShader(shaderOutput.pShaderBinary);

        return beStatus_NO_BINARY_FOR_DEVICE;
    }

    // Free stuff.
    m_TheAMDDXXModule.AmdDxGsaFreeCompiledShader(shaderOutput.pShaderBinary);

    return beStatus_SUCCESS;
}

beProgramBuilderDX::beProgramBuilderDX(void) : m_TheAMDDXXModule(AMDDXXModule::s_DefaultModuleName), m_TheD3DCompileModule(D3DCompileModule::s_DefaultModuleName), m_compiledElf()
{
    m_bIsInitialized = false;
}


beProgramBuilderDX::~beProgramBuilderDX(void)
{
    m_TheAMDDXXModule.UnloadModule();
}

beKA::beStatus beProgramBuilderDX::Initialize(const std::string& dxxModuleName, const std::string& compilerModuleName)
{
    beStatus beRet = beStatus_SUCCESS;
    std::string  compilerDllName = compilerModuleName;

    if (compilerDllName.empty())
    {
        // This solves the VS extension issue where devenv.exe looked for the D3D compiler
        // at its own directory, instead of looking for it at CodeXL's binaries directory.
        osFilePath defaultCompilerFilePath;

        // Get CodeXL's binaries directory. Both the 32 and 64-bit versions of d3dcompiler are bundled with CodeXL.
        // We use the 32-bit version by default
        bool isOk = osGetCurrentApplicationDllsPath(defaultCompilerFilePath, OS_I386_ARCHITECTURE);

        if (isOk)
        {
            // Create the full path to the default D3D compiler.
            defaultCompilerFilePath.setFileName(SA_BE_STR_HLSL_optionsDefaultCompilerFileName);
            defaultCompilerFilePath.setFileExtension(SA_BE_STR_HLSL_optionsDefaultCompilerFileExtension);
            compilerDllName = defaultCompilerFilePath.asString().asASCIICharArray();
        }
    }

    // Clear the outputs of former builds (if there are any).
    ClearFormerBuildOutputs();

    // load AMD DXX module
    if (!dxxModuleName.empty())
    {
        if (m_TheAMDDXXModule.IsLoaded())
        {
            m_TheAMDDXXModule.UnloadModule();
        }
        m_TheAMDDXXModule.LoadModule(dxxModuleName);
    }

    if (!m_TheAMDDXXModule.IsLoaded())
    {
        // Notice: This message receives an extra "\n", since later in the call chain, one is removed. We do want to remove them for
        // the rest of the messages, so we only remove it for initialization messages:
        stringstream ss;
        ss << "Error: " << AMDDXXModule::s_DefaultModuleName << " module not loaded.\n\n" << endl;
        LogCallBack(ss.str());
        beRet = beStatus_AMDDXX_MODULE_NOT_LOADED;
    }

    // load D3D compiler module
    if (beRet == beStatus_SUCCESS)
    {
        bool isDllLoad = false;
        int errorCode = 0;

        // If the user did not specify a default D3D compiler, use the one in the sub-folder.
        std::string fixedMsD3DModuleName = compilerDllName;
        if (fixedMsD3DModuleName.empty())
        {
#if _WIN64
            fixedMsD3DModuleName = "x64\\d3dcompiler_47.dll";
#elif _WIN32
            fixedMsD3DModuleName = "x86\\d3dcompiler_47.dll";
#endif
        }

        if (fixedMsD3DModuleName.length() > 0)
        {
            isDllLoad = m_TheD3DCompileModule.LoadModule(fixedMsD3DModuleName, &errorCode);
        }
        else
        {
            isDllLoad = m_TheD3DCompileModule.LoadModule();
        }

        if (!m_TheD3DCompileModule.IsLoaded())
        {
            // Check if the additional search paths should be searched.
            bool shouldSearchAdditionalPaths = !isDllLoad &&
                                               (fixedMsD3DModuleName.find(D3DCompileModule::s_DefaultModuleName) != string::npos);

            if (shouldSearchAdditionalPaths)
            {
                gtString moduleNameAsGtStr;
                moduleNameAsGtStr << D3DCompileModule::s_DefaultModuleName;

                // Try searching in the additional directories (if any).
                for (const string& path : m_loaderSearchDirectories)
                {
                    // Build the full path to the module.
                    gtString pathAsGtStr;
                    pathAsGtStr << path.c_str();

                    osFilePath searchPath;
                    searchPath.setFileDirectory(pathAsGtStr);
                    searchPath.setFileName(moduleNameAsGtStr);

                    // Try to load the module.
                    isDllLoad = m_TheD3DCompileModule.LoadModule(searchPath.asString().asASCIICharArray(), &errorCode);

                    if (isDllLoad)
                    {
                        // We are done.
                        break;
                    }
                }
            }

            if (!isDllLoad)
            {
                // Take the relevant module's name.
                const char* pModuleName = (compilerDllName.length() > 0) ?
                                           compilerDllName.c_str() : D3DCompileModule::s_DefaultModuleName;

                // This flag will be true if the given D3D module's bitness is 64-bit, while
                // this process' bitness is 32-bit.
                bool is64from32Error = false;
#ifndef _WIN64
                // If we are in 32-bit mode, check if the module's bitness is 64-bit.
                osFilePath modulePath;
                gtString moduleFilaNameAsGtStr;
                moduleFilaNameAsGtStr << pModuleName;
                modulePath.setFileName(moduleFilaNameAsGtStr);
                osIs64BitModule(moduleFilaNameAsGtStr, is64from32Error);
#endif // !_WIN64

                // Generate the error message.
                stringstream ss;
                if (is64from32Error)
                {
                    ss << "Error: " << pModuleName << " is a 64-bit module, which cannot be loaded during a 32-bit build." << std::endl;
                }
                else
                {
                    // We failed to load the module.
                    // Notice: This message receives an extra "\n", since later in the call chain, one is removed. We do want to remove them for
                    // the rest of the messages, so we only remove it for initialization messages:
                    ss << "Error: " << pModuleName << " module not loaded. Error = " << errorCode << ". Please use D3D compiler version 43 or above." << std::endl;
                }

                // Print the error message, and abort the build process.
                LogCallBack(ss.str());
                exit(-1);
            }
        }
    }

    if (beRet == beStatus_SUCCESS)
    {
        std::set<std::string> uniqueNamesOfPublishedDevices;
        m_bIsInitialized = beUtils::GetAllGraphicsCards(m_DXDeviceTable, uniqueNamesOfPublishedDevices);
        if (!m_bIsInitialized)
        {
            beRet = beStatus_NO_DEVICE_FOUND;
        }
    }

    if (beRet == beStatus_SUCCESS)
    {
        m_bIsInitialized = true;
    }

    return beRet;
}

bool static WriteBinaryFile(const std::string& fileName, const std::vector<char>& content)
{
    bool ret = false;
    ofstream output;
    output.open(fileName.c_str(), ios::binary);

    if (output.is_open() && !content.empty())
    {
        output.write(&content[0], content.size());
        output.close();
        ret = true;
    }
    else
    {
        std::stringstream log;
        log << "Error: Unable to open " << fileName << " for write.\n";
    }

    return ret;
}

beKA::beStatus beProgramBuilderDX::CompileHLSL(const std::string& programSource,
    const DXOptions& dxOptions, bool isDxbcInput)
{
    beKA::beStatus ret = beStatus::beStatus_General_FAILED;

    // This blob would hold the DXBC blob (either
    // compiled from HLSL or given as an input by the user).
    ID3DBlob* pShader    = nullptr;

    // In the case that the input is a DXBC blob, we would read it into this vector.
    std::vector<char> dxbcBlob;
    char* pShaderBytes = nullptr;
    size_t shaderByteCount = 0;

    if (!isDxbcInput)
    {
        ID3DBlob* pErrorMsgs = nullptr;

        // Turn options.m_Defines into what D3DCompile expects.
        D3D_SHADER_MACRO* pMacros = new D3D_SHADER_MACRO[dxOptions.m_defines.size() + 1];
        pMacros[dxOptions.m_defines.size()].Name = NULL;
        pMacros[dxOptions.m_defines.size()].Definition = NULL;

        // Handle the defines.
        if (!dxOptions.m_defines.empty())
        {
            int i = 0;
            for (vector<pair<string, string> >::const_iterator it = dxOptions.m_defines.begin();
                it != dxOptions.m_defines.end();
                ++it, i++)
            {
                pMacros[i].Name = it->first.c_str();
                pMacros[i].Definition = it->second.c_str();
            }
        }

        // Handle custom includes.
        ID3DInclude* pIncludeMechanism = D3D_COMPILE_STANDARD_FILE_INCLUDE;

        if (!dxOptions.m_includeDirectories.empty())
        {
            // Extract the source file's directory.
            gtString shaderFileName;
            shaderFileName << dxOptions.m_FileName.c_str();
            osFilePath tmpFilePath(shaderFileName);

            gtString shaderDirStr;
            osDirectory shaderDir;
            bool isDirExtracted = tmpFilePath.getFileDirectory(shaderDir);

            if (isDirExtracted)
            {
                // Create an include manager.
                shaderDirStr = shaderDir.directoryPath().asString();
                pIncludeMechanism = new D3DIncludeManager(shaderDirStr.asASCIICharArray(), dxOptions.m_includeDirectories);
            }
        }

        // Invoke the offline compiler.
        HRESULT result = E_FAIL;
        try
        {
            result = m_TheD3DCompileModule.D3DCompile(
                programSource.c_str(),
                programSource.length(),
                LPCSTR(dxOptions.m_FileName.c_str()),
                pMacros,
                pIncludeMechanism,
                dxOptions.m_Entrypoint.c_str(),
                dxOptions.m_Target.c_str(),
                dxOptions.m_DXFlags.flagsAsInt,
                0,
                &pShader,
                &pErrorMsgs);
        }
        catch (...)
        {
            LogCallBack(STR_ERROR_D3D_COMPILER_EXCEPTION);
            return beStatus_D3DCompile_FAILED;
        }

        // Release resources.
        delete[] pMacros;

        if (pErrorMsgs != NULL)
        {
            char* errorString = (char*)pErrorMsgs->GetBufferPointer();
            size_t error_size = pErrorMsgs->GetBufferSize();
            stringstream ss;
            ss << string(errorString, error_size);
            LogCallBack(ss.str());
            pErrorMsgs->Release();
        }

        if (result != S_OK)
        {
            if (pShader != NULL)
            {
                pShader->Release();
            }
            return beStatus_D3DCompile_FAILED;
        }
    }
    else
    {
        bool isBlobRead = beUtils::ReadBinaryFile(dxOptions.m_FileName, dxbcBlob);
        assert(isBlobRead);
        pShaderBytes = dxbcBlob.data();
        shaderByteCount = dxbcBlob.size();
    }

    // For DX10 & DX11, we need to peel off the Microsoft headers.
    // This will get us to the executable bit that the RTL usually passes
    // down to the driver.  That's what the code below wants.
    // For DX9, the object has no header (it's all byte code).
    // But for DX9, we need to talk to a different driver and that's NYI.
    if (pShaderBytes == nullptr)
    {
        pShaderBytes = (char*)pShader->GetBufferPointer();
    }
    if (shaderByteCount == 0)
    {
        shaderByteCount = pShader->GetBufferSize();
    }
    assert(pShaderBytes != nullptr);
    assert(shaderByteCount != 0);

    if (pShaderBytes != nullptr && shaderByteCount != 0)
    {
        string sShader(pShaderBytes, shaderByteCount);

        // If requested by the user, disassemble D3D offline compiler's output.
        if (dxOptions.m_bDumpMSIntermediate && m_msIntermediateText.empty())
        {
            ID3DBlob* pDisassembly = NULL;
            HRESULT result = m_TheD3DCompileModule.D3DDisassemble(pShaderBytes,
                shaderByteCount, 0, "", &pDisassembly);

            if (result == S_OK)
            {
                pShaderBytes = (char*)pDisassembly->GetBufferPointer();
                shaderByteCount = pDisassembly->GetBufferSize();
                m_msIntermediateText = string(pShaderBytes, shaderByteCount);
            }
            else
            {
                // If failed, report and continue.
                LogCallBack(STR_ERROR_DXBC_BLOB_DISASSEMBLE_FAILURE);
            }
        }

        // Perform the actual compilation.
        ret = CompileDXAsm(sShader, dxOptions);
    }

    return ret;
}

static bool ExtractShaderCode(const std::string& programSource, uint32_t& codeSize, const char*& pCode)
{
    bool ret = false;

    // Flag set to true if we need to abort the parsing process.
    bool shouldAbort = false;

    // Validate the binary.
    size_t dxbcOffset = programSource.find("DXBC");
    assert(dxbcOffset == 0);
    if (dxbcOffset == 0)
    {
        // Pointer to the current section in the DXBC blob.
        const uint32_t* pDxbc = (const uint32_t*)programSource.data();

        // Pointer to the end of the blob.
        const uint32_t* pEnd = (const uint32_t*)(pDxbc + programSource.size());

        // Skip the "DXBC" section.
        pDxbc++;

        // Skip the hash.
        pDxbc += 4;

        // Skip unknown and file length.
        pDxbc++;
        pDxbc++;

        assert(pDxbc < pEnd);
        if (pDxbc < pEnd)
        {
            const uint32_t numChunks = *pDxbc;
            pDxbc++;

            // Extract the information about all chunks within this blob.
            std::vector<uint32_t> chunkOffsets;
            for (uint32_t i = 0; !shouldAbort && i < numChunks; i++)
            {
                assert(pDxbc < pEnd);
                if (pDxbc < pEnd)
                {
                    chunkOffsets.push_back(*pDxbc);
                    pDxbc++;
                }
                else
                {
                    shouldAbort = true;
                    std::cout << STR_ERROR_DXBC_BLOB_PARSE_FAILURE << std::endl;
                }
            }

            if (!shouldAbort)
            {
                const char* pStart = programSource.data();

                // Look for the relevant chunk (SHEX or SHDR), extract the code pointer and its size.
                for (uint32_t offset : chunkOffsets)
                {
                    if (ret)
                    {
                        // We are done.
                        break;
                    }

                    pDxbc = (const uint32_t *)(pStart + offset);
                    assert(pDxbc + 2 < pEnd);
                    if (pDxbc + 2 < pEnd)
                    {
                        const char* pDxbcChar = (char*)pDxbc;
                        if ((pDxbcChar[0] == 'S' && pDxbcChar[1] == 'H' && pDxbcChar[2] == 'E' && pDxbcChar[3] == 'X') ||
                            (pDxbcChar[0] == 'S' && pDxbcChar[1] == 'H' && pDxbcChar[2] == 'D' && pDxbcChar[3] == 'R'))
                        {
                            pDxbc++;
                            codeSize = *pDxbc;
                            pCode = (char*)(++pDxbc);
                            assert(pDxbc + (codeSize / 4) <= pEnd);
                            ret = (pDxbc + (codeSize / 4) <= pEnd);
                        }
                    }
                    else
                    {
                        std::cout << STR_ERROR_DXBC_BLOB_PARSE_FAILURE << std::endl;
                        shouldAbort = true;
                        break;
                    }
                }
            }
        }
        else
        {
            shouldAbort = true;
            std::cout << STR_ERROR_DXBC_BLOB_PARSE_FAILURE << std::endl;
        }
    }
    else
    {
        std::cout << STR_ERROR_DXBC_BLOB_INVALID << std::endl;
    }

    ret = ret && !shouldAbort;
    return ret;
}

beKA::beStatus beProgramBuilderDX::CompileDXAsm(const string& programSource, const DXOptions& dxOptions)
{
    uint32_t shaderCodeSize = 0;
    const char* pShaderCode = nullptr;
    bool isOk = ExtractShaderCode(programSource, shaderCodeSize, pShaderCode);
    assert(isOk);

    AmdDxGsaCompileShaderInput shaderInput;
    memset(&shaderInput, 0, sizeof(AmdDxGsaCompileShaderInput));
    AmdDxGsaCompileOption compileOptions[2];
    memset(compileOptions, 0, 2*sizeof(AmdDxGsaCompileOption));
    if (dxOptions.m_isShaderIntrinsicsEnabled)
    {
        // Enable D3D11 shader intrinsics. The value does not matter, only the setting does.
        compileOptions[0].setting = AmdDxGsaCompileOptionEnum::AmdDxGsaEnableShaderIntrinsics;
        shaderInput.numCompileOptions = 1;

        if (dxOptions.m_UAVSlot > -1)
        {
            // Handle the case where the user defined a UAV slot for shader intrinsics.
            compileOptions[1].setting = AmdDxGsaCompileOptionEnum::AmdDxGsaShaderIntrinsicsUAVSlot;
            compileOptions[1].value = dxOptions.m_UAVSlot;
            shaderInput.numCompileOptions = 2;
        }
    }
    else
    {
        shaderInput.numCompileOptions = 0;
    }
    shaderInput.pCompileOptions = compileOptions;

    shaderInput.chipFamily = dxOptions.m_ChipFamily;
    shaderInput.chipRevision = dxOptions.m_ChipRevision;
    // The code directly follows the header.
    shaderInput.pShaderByteCode = pShaderCode;
    shaderInput.byteCodeLength = shaderCodeSize;

    AmdDxGsaCompileShaderOutput shaderOutput;
    memset(&shaderOutput, 0, sizeof(AmdDxGsaCompileShaderOutput));

    // The size field of this out parameter is the sizeof the struct.
    // This allows for some future expansion (perhaps starting with a version number).
    shaderOutput.size = sizeof(AmdDxGsaCompileShaderOutput);
    // For good measure, zero the output pointer.
    shaderOutput.pShaderBinary = NULL;

    // Inside a wrapper to handle exceptions like acvio.
    HRESULT result = AmdDxGsaCompileShaderWrapper(&shaderInput, &shaderOutput);

    if (result != S_OK)
    {
        stringstream ss;
        ss << AMDDXXModule::s_DefaultModuleName << " AmdDxGsaCompileShader failed." << endl;
        LogCallBack(ss.str());

        if (shaderOutput.pShaderBinary != NULL)
        {
            m_TheAMDDXXModule.AmdDxGsaFreeCompiledShader(shaderOutput.pShaderBinary);
        }

        return beStatus_AmdDxGsaCompileShader_FAILED;
    }

    // Open the binaries as CElf objects.
    vector<char> elfBinary((char*)shaderOutput.pShaderBinary, (char*)shaderOutput.pShaderBinary + shaderOutput.shaderBinarySize);
    CElf* pCurrElf = new CElf(elfBinary);

    if (pCurrElf->good())
    {
        SetDeviceElf(dxOptions.m_deviceName, shaderOutput);
    }
    else
    {
        // Report about the failure.
        stringstream ss;
        ss << "Unable to parse ELF binary.\n";
        LogCallBack(ss.str());

        // Release the resources.
        m_TheAMDDXXModule.AmdDxGsaFreeCompiledShader(shaderOutput.pShaderBinary);

        return beStatus_NO_BINARY_FOR_DEVICE;
    }

    // Free stuff.
    m_TheAMDDXXModule.AmdDxGsaFreeCompiledShader(shaderOutput.pShaderBinary);

    return beStatus_SUCCESS;
}

beKA::beStatus beProgramBuilderDX::Compile(RgaMode mode, const std::string& programSource,
    const DXOptions& dxOptions, bool isDxbcInput)
{
    if (!m_bIsInitialized)
    {
        stringstream ss;
        ss << "DX Module not initialized";
        LogCallBack(ss.str());
        return beStatus_D3DCompile_MODULE_NOT_LOADED;
    }

    beStatus beRet = beStatus_General_FAILED;

    if (mode == Mode_DX11)
    {
        beRet = CompileHLSL(programSource, dxOptions, isDxbcInput);
    }
    else if (mode == Mode_AMDIL)
    {
        beRet = CompileAMDIL(programSource, dxOptions);
    }
    else
    {
        stringstream ss;
        ss << "Source language not supported";
        LogCallBack(ss.str());
    }

    return beRet;
}

beKA::beStatus beProgramBuilderDX::GetBinary(const string& device, const beKA::BinaryOptions& binopts, vector<char>& binary)
{

    GT_UNREFERENCED_PARAMETER(binopts);
    binary = GetDeviceBinaryElf(device);

    return beKA::beStatus_SUCCESS;
}


beKA::beStatus beProgramBuilderDX::GetISABinary(const string& device, vector<char>& binary)
{
    const CElfSection* pTextSection = GetISATextSection(device);
    beStatus result = beStatus_Invalid;

    if (nullptr != pTextSection)
    {
        result = beStatus_SUCCESS;
        binary = pTextSection->GetData();
    }

    return result;

}

beKA::beStatus beProgramBuilderDX::GetKernelILText(const string& device, const string& kernel, string& il)
{
    // For DX shaders, we currently cannot disassemble the IL binary section.
    // For now, we extract D3D ASM code.
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    il.clear();
    beStatus beRet = GetIntermediateMSBlob(il);
    return beRet;
}

beKA::beStatus beProgramBuilderDX::GetKernelISAText(const string& device, const string& shaderName, string& isa)
{
    // Not implemented: see beProgramBuilderDX::GetDxShaderD3DASM.
    // The current inheritance architecture where beProgramBuilderDX and
    // beProgramBuilderCL share the same interface for ISA extraction does not hold.
    // This mechanism will be refactored.
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(shaderName);
    GT_UNREFERENCED_PARAMETER(isa);
    return beStatus_Invalid;
}

beKA::beStatus beProgramBuilderDX::GetStatistics(const string& device, const string& kernel, beKA::AnalysisData& analysis)
{
    GT_UNREFERENCED_PARAMETER(kernel);

    beStatus ret = beKA::beStatus_General_FAILED;

    // There is no symbol table.  We just get the .stats  section.
    CElf* pElf = GetDeviceElf(device);

    if (pElf != nullptr)
    {
        // Get the .stats section.
        CElfSection* pStatsSection = pElf->GetSection(".stats");

        if (pStatsSection != nullptr)
        {
            // This is the binary image.
            const vector<char>& sectionData = pStatsSection->GetData();

            const AmdDxGsaCompileStats* pStats = reinterpret_cast<const AmdDxGsaCompileStats*>(sectionData.data());

            if (pStats != nullptr)
            {
                (void)memset(&analysis, 0, sizeof(analysis));

                if (!IsFixedSgprAlloc(device))
                {
                    analysis.numSGPRsUsed = pStats->numSgprsUsed;
                }
                else
                {
                    // Assign the fixed value. This is due to a HW bug. The fixed values are: 94 (excluding
                    // the 2 VCC registers) until driver 15.10 (including), and 96 afterwards (to be handled).
                    const unsigned SGPRs_USED_TONGA_ICELAND = 94;
                    analysis.numSGPRsUsed = SGPRs_USED_TONGA_ICELAND;
                }

                analysis.numSGPRsAvailable = pStats->availableSgprs;
                analysis.numVGPRsUsed = pStats->numVgprsUsed;
                analysis.numVGPRsAvailable = pStats->availableVgprs;
                analysis.LDSSizeUsed = pStats->usedLdsBytes;
                analysis.LDSSizeAvailable = pStats->availableLdsBytes;
                analysis.scratchMemoryUsed = pStats->usedScratchBytes;
                analysis.numAluInst = pStats->numAluInst;
                analysis.numControlFlowInst = pStats->numControlFlowInst;
                analysis.numTfetchInst = pStats->numTfetchInst;

                // We are done.
                ret = beKA::beStatus_SUCCESS;
            }
        }
        else
        {
            ret = beStatus_NoStatSectionInElfPossibleOldDxDriver;
        }
    }

    return ret;
}

void beProgramBuilderDX::ClearFormerBuildOutputs()
{
    // Clear the ELF sections.
    for (auto& devElfPair : m_compiledElf)
    {
        CelfBinaryPair& celfCelfBinaryPair = devElfPair.second;
        delete celfCelfBinaryPair.first;
        celfCelfBinaryPair.first = nullptr;
        celfCelfBinaryPair.second.clear();
    }

    m_compiledElf.clear();

    // Clear the DSASM text.
    m_msIntermediateText.clear();
}

void beProgramBuilderDX::ReleaseProgram()
{
    ClearFormerBuildOutputs();
}

beKA::beStatus beProgramBuilderDX::GetDeviceTable(vector<GDT_GfxCardInfo>& table)
{
    table = m_DXDeviceTable;
    return beStatus_SUCCESS;
}

#ifdef DXASM_T_ENABLED
/////////////////////////////////////////////////////
// File scoped global declarations
/////////////////////////////////////////////////////
static bool g_bWroteFiles = false;
// flag to denote whether "SHDR" token needs to be added
// to the output binary dump file.
bool g_bSHDR = false;
unsigned long dwSHDRToken = 0x52444853; /* "SHDR" */
bool g_bBinIsHex = false;
static string g_szTxtFile;
static string g_szBinFile;
static string g_szFullBinFile;
static string g_szRegFile;
static string g_szHexFile;
static string g_szFullHexFile;
static string g_szConversion;
static string test_asm;
static string test_hlsl;
static int g_hlsl_flag = 0;
bool g_bNoDebug = false;

static FILE* g_fp = NULL;
/////////////////////////////////////////////////////
// Callback functions
/////////////////////////////////////////////////////
void* XLT_STDCALL allocSysMem(void* pHandle, unsigned int dwSizeInBytes)
{
    (void)(pHandle); // Unreferenced parameter

    return malloc(dwSizeInBytes);
}

void XLT_STDCALL freeSysMem(void* pHandle, void* lpAddress)
{
    (void)(pHandle); // Unreferenced parameter

    free(lpAddress);
}

int XLT_STDCALL outputString(XLT_PVOID pHandle, const char* pTranslatedString, ...)
{
    (void)(pHandle); // Unreferenced parameter

    char pszFormattedBuffer[1024];
    va_list va;

    va_start(va, pTranslatedString);
    vsprintf(pszFormattedBuffer, pTranslatedString, va);
    va_end(va);

    //g_szResultTxtFile += pszFormattedBuffer;

    return 0;
}

string* beProgramBuilderDX::s_pTranslatedProgram;
int* beProgramBuilderDX::s_pipTranslatedProgramSize;

beKA::beStatus beProgramBuilderDX::CompileDXAsmT(const string& programSource, const DXOptions& dxOptions)
{
    // lets convert the source, which is DX ASM as text to DX ASM binary which is what DX driver expects.
    XLT_PROGINFO xltInput;
    XLT_CALLBACKS xltCallbacks;

    memset(&xltCallbacks, 0, sizeof(XLT_CALLBACKS));

    xltCallbacks.eXltMode = E_XLT_NORMAL;
    xltCallbacks.pHandle = NULL;
    xltCallbacks.AllocateSysMem = allocSysMem;
    xltCallbacks.FreeSysMem = freeSysMem;
    xltCallbacks.OutputString = outputString;
    xltCallbacks.Assert = NULL;
    xltCallbacks.flags = g_hlsl_flag;
    xltInput.pBuffer = (char*)programSource.c_str();
    xltInput.nBufferSize = (int)programSource.length() + 1;

    char*  pTheBinBuffer;
    string stemp;
    int nBufferSize = 0;
    unsigned int stempsize;

    s_pipTranslatedProgramSize = &nBufferSize;
    s_pTranslatedProgram = &stemp;

    DX10AsmText2Stream(&xltInput, &xltCallbacks, pTheBinBuffer, stempsize);

    // const D3D10_ChunkHeader* byteCodeHeader = ( D3D10_ChunkHeader* ) pTheBinBuffer;

    AmdDxGsaCompileShaderInput shaderInput;
    memset(&shaderInput, 0, sizeof(AmdDxGsaCompileShaderInput));
    AmdDxGsaCompileOption compileOptions[1];
    memset(compileOptions, 0, sizeof(AmdDxGsaCompileOption));

    shaderInput.chipFamily = dxOptions.m_ChipFamily;
    shaderInput.chipRevision = dxOptions.m_ChipRevision;
    shaderInput.pShaderByteCode = (char*)(pTheBinBuffer);  // The code directly follows the header.
    shaderInput.byteCodeLength = stempsize;
    shaderInput.pCompileOptions = compileOptions;
    shaderInput.numCompileOptions = 0;

    AmdDxGsaCompileShaderOutput shaderOutput;
    memset(&shaderOutput, 0, sizeof(AmdDxGsaCompileShaderOutput));
    // The size field of this out parameter is the sizeof the struct.
    // This allows for some future expansion (perhaps starting with a version number).
    shaderOutput.size = sizeof(AmdDxGsaCompileShaderOutput);
    // For good measure, zero the output pointer.
    shaderOutput.pShaderBinary = NULL;

    // Inside a wrapper to handle exceptions like acvio.
    HRESULT result = AmdDxGsaCompileShaderWrapper(&shaderInput, &shaderOutput);

    if (result != S_OK)
    {
        stringstream ss;
        ss << AMDDXXModule::s_DefaultModuleName << " AmdDxGsaCompileShader failed." << endl;
        LogCallBack(ss.str());

        if (shaderOutput.pShaderBinary != NULL)
        {
            m_TheAMDDXXModule.AmdDxGsaFreeCompiledShader(shaderOutput.pShaderBinary);
        }

        return beStatus_AmdDxGsaCompileShader_FAILED;
    }

    // Crack open the binaries as CElf objects.
    vector<char> elfBinary((char*)shaderOutput.pShaderBinary, (char*)shaderOutput.pShaderBinary + shaderOutput.shaderBinarySize);
    CElf* pElf = new CElf(elfBinary);

    if (pElf->good())
    {
        // Store the ELF section in the relevant container.
        SetDeviceElf(dxOptions.m_deviceName, shaderOutput);
    }
    else
    {
        // Inform the user about the failure.
        stringstream ss;
        ss << "Unable to parse ELF binary.\n";
        LogCallBack(ss.str());

        // Release the resources.
        m_TheAMDDXXModule.AmdDxGsaFreeCompiledShader(shaderOutput.pShaderBinary);

        return beStatus_NO_BINARY_FOR_DEVICE;
    }

    // Free stuff.
    m_TheAMDDXXModule.AmdDxGsaFreeCompiledShader(shaderOutput.pShaderBinary);

    return beStatus_SUCCESS;
}
#endif DXASM_T_ENABLED

beKA::beStatus beProgramBuilderDX::GetIntermediateMSBlob(string& IntermediateMDBlob)
{
    IntermediateMDBlob = m_msIntermediateText;
    return beStatus_SUCCESS;
}

void beProgramBuilderDX::SetIntermediateMSBlob(const string& intermediateMSCode)
{
    m_msIntermediateText = intermediateMSCode;
}

void beProgramBuilderDX::AddDxSearchDir(const string& dir)
{
    if (find(m_loaderSearchDirectories.begin(),
             m_loaderSearchDirectories.end(), dir) == m_loaderSearchDirectories.end())
    {
        m_loaderSearchDirectories.push_back(dir);
    }
}

const CElfSection* beProgramBuilderDX::GetISATextSection(const string& deviceName) const
{
    // Get the relevant ELF section for the required device.
    const CElfSection* result = nullptr;
    CElf* pElf = GetDeviceElf(deviceName);

    if (pElf != nullptr)
    {
        // There is no symbol table.  We just need the .text section.
        const string CODE_SECTION_NAME(".text");
        result = pElf->GetSection(CODE_SECTION_NAME);
    }

    return result;
}

const CElfSection* beProgramBuilderDX::GetElfSection(const std::string& deviceName, const std::string& sectionName) const
{
    // Get the relevant ELF section for the required device.
    const CElfSection* result = nullptr;
    CElf* pElf = GetDeviceElf(deviceName);

    if (pElf != nullptr)
    {
        // There is no symbol table.  We just need the ELF section.
        result = pElf->GetSection(sectionName);
    }

    return result;
}

const CElfSection* beProgramBuilderDX::GetILDisassemblySection(const std::string& deviceName) const
{
    const std::string IL_DISASSEMBLY_SECTION_NAME(".amdil_disassembly");
    return GetElfSection(deviceName, IL_DISASSEMBLY_SECTION_NAME);
}

beKA::beStatus beProgramBuilderDX::GetDxShaderISAText(const std::string& device, std::string& isaBuffer)
{
    beKA::beStatus ret = beStatus_NO_ISA_FOR_DEVICE;

    // Get the pointer to the ISA disassembly section.
    const std::string ISA_DISASSEMBLY_SECTION_NAME(".disassembly");
    const CElfSection* pElfSection = GetElfSection(device, ISA_DISASSEMBLY_SECTION_NAME);

    // Extract the disassembly from the ELF section's data.
    isaBuffer.clear();
    ExtractTextFromElfSection(pElfSection, isaBuffer);
    if (!isaBuffer.empty())
    {
        // We are done.
        ret = beKA::beStatus_SUCCESS;
    }

    return ret;
}

beKA::beStatus beProgramBuilderDX::GetDxShaderIL(const std::string& device, std::string& isaBuffer)
{
    beKA::beStatus ret = beStatus_NO_IL_FOR_DEVICE;
    isaBuffer.clear();

    // Get the relevant ELF section for the required device.
    const CElfSection* pAmdilDiassemblySection = GetILDisassemblySection(device);

    // Extract the disassembly from the ELF section.
    ExtractTextFromElfSection(pAmdilDiassemblySection, isaBuffer);
    if (!isaBuffer.empty())
    {
        // We are done.
        ret = beKA::beStatus_SUCCESS;
    }

    return ret;
}

void beProgramBuilderDX::ExtractTextFromElfSection(const CElfSection* pSection, std::string& content)
{
    if (pSection != nullptr)
    {
        // This is the disassembly section (ASCII representation).
        const vector<char>& sectionData = pSection->GetData();
        content = std::string(sectionData.begin(), sectionData.end());
    }
}

bool beProgramBuilderDX::GetIsaSize(const string& isaAsText, size_t& sizeInBytes) const
{
    // The length in characters of a 32-bit instruction in text format.
    const size_t INSTRUCTION_32_LENGTH = 8;
    const size_t INSTRUCTION_32_SIZE_IN_BYTES = 4;

    // The length in characters of a 64-bit instruction in text format.
    const size_t INSTRUCTION_64_LENGTH = 16;
    const size_t INSTRUCTION_64_SIZE_IN_BYTES = 8;

    bool ret = false;
    sizeInBytes = 0;

    size_t posBegin = isaAsText.rfind("//");

    if (posBegin != string::npos)
    {
        size_t posFirst32BitEnd = isaAsText.find(':', posBegin);

        // Get past the "// " prefix.
        posBegin += 3;

        // Determine the length of the PC string.
        size_t pcLength = (posFirst32BitEnd - posBegin);

        if (posFirst32BitEnd != string::npos &&  pcLength > 0)
        {
            GT_IF_WITH_ASSERT(posBegin + pcLength < isaAsText.size())
            {
                // Get the first 32 bit of the final instruction.
                const string& instrFirst32bit = isaAsText.substr(posBegin, pcLength);

                // Convert the PC of the final instruction. This will indicate
                // how many bytes we used until the final instruction (excluding
                // the final instruction).
                sizeInBytes = stoul(instrFirst32bit, nullptr, 16);

                // Get past the prefix.
                posFirst32BitEnd += 2;

                // Find the end of the final instruction line.
                size_t posEnd = isaAsText.find("\n", posBegin);

                if (posEnd != string::npos && posFirst32BitEnd < posEnd)
                {
                    // Extract the final instruction as text.
                    const string& instructionAsText = isaAsText.substr(posFirst32BitEnd, posEnd - posFirst32BitEnd);
                    const size_t numOfCharactersInInstr = instructionAsText.size();

                    if (numOfCharactersInInstr == INSTRUCTION_32_LENGTH)
                    {
                        // The final instruction is a 32-bit instruction.
                        sizeInBytes += INSTRUCTION_32_SIZE_IN_BYTES;
                        ret = true;
                    }
                    else if (numOfCharactersInInstr == (INSTRUCTION_64_LENGTH + 1))
                    {
                        // The final instruction is a 64-bit instruction.
                        sizeInBytes += INSTRUCTION_64_SIZE_IN_BYTES;
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

bool beProgramBuilderDX::GetWavefrontSize(const string& deviceName, size_t& wavefrontSize) const
{
    bool ret = false;
    wavefrontSize = 0;

    // Extract the device info.
    GDT_DeviceInfo s_deviceInfo;

    if (AMDTDeviceInfoUtils::Instance()->GetDeviceInfo(deviceName.c_str(), s_deviceInfo))
    {
        wavefrontSize = s_deviceInfo.m_nWaveSize;
        ret = true;
    }

    return ret;
}


string beProgramBuilderDX::ToLower(const string& str) const
{
    string result;
    transform(str.begin(), str.end(),
              inserter(result, result.begin()), ::tolower);
    return result;
}

void beProgramBuilderDX::SetDeviceElf(const string& deviceName, const AmdDxGsaCompileShaderOutput& shaderOutput)
{
    if (!deviceName.empty())
    {
        // First, convert the device name to lower case.
        string deviceNameLowerCase = ToLower(deviceName);

        // Update the container.
        CelfBinaryPair& celfpair = m_compiledElf[deviceNameLowerCase];

        celfpair.second.assign((char*)shaderOutput.pShaderBinary, (char*)shaderOutput.pShaderBinary + shaderOutput.shaderBinarySize);
        // Open the binaries as CElf objects.
        celfpair.first = new CElf(celfpair.second);
    }
}

bool beProgramBuilderDX::GetDeviceElfBinPair(const string& deviceName, CelfBinaryPair& elfBinPair) const
{
    bool result =  false;


    if (!deviceName.empty())
    {
        // First, convert the device name to lower case.
        string deviceNameLowerCase = ToLower(deviceName);

        // Look for the relevant element.
        auto iter = m_compiledElf.find(deviceNameLowerCase);

        if (iter != m_compiledElf.end())
        {
            elfBinPair = iter->second;
            result = true;
        }
    }

    return result;
}

static beKA::beStatus  InvokeDX11Driver(const std::string& args, bool printCmd, std::string& output)
{
    beKA::beStatus  status = beStatus_dxDriverLaunchFailure;
    std::stringstream  cmdLine;
    bool  cancelSignal = false;
    gtString  gtOutput;
    cmdLine << RGA_DX11_DRIVER_EXECUTABLE_PATH << " " << args;

    if (printCmd)
    {
        beUtils::PrintCmdLine(cmdLine.str(), printCmd);
    }

    bool  result = osExecAndGrabOutput(cmdLine.str().c_str(), cancelSignal, gtOutput);

    if (result)
    {
        status = beStatus_SUCCESS;
        output = gtOutput.asASCIICharArray();
    }

    return status;
}

CElf* beProgramBuilderDX::GetDeviceElf(const string& deviceName) const
{
    CElf* pRet = nullptr;
    CelfBinaryPair elfBinPair;

    if (GetDeviceElfBinPair(deviceName, elfBinPair))
    {
        pRet = elfBinPair.first;
    }

    return pRet;
}

vector<char> beProgramBuilderDX::GetDeviceBinaryElf(const string& deviceName) const
{
    vector<char> result;
    CelfBinaryPair elfBinPair;

    if (GetDeviceElfBinPair(deviceName, elfBinPair))
    {
        result = elfBinPair.second;
    }

    return result;
}

void beProgramBuilderDX::SetPublicDeviceNames(const std::set<std::string>& publicDeviceNames)
{
    m_publicDeviceNames = publicDeviceNames;
}

bool beProgramBuilderDX::GetSupportedDisplayAdapterNames(bool printCmd, std::vector<std::string>& adapterNames)
{
    bool  result = false;
    std::string  driverOut;

    beKA::beStatus  status = InvokeDX11Driver(RGA_DX11_DRIVER_GET_ADAPTERS_ARG, printCmd, driverOut);

    result = (status == beKA::beStatus_SUCCESS && !driverOut.empty());

    if (result)
    {
        if (driverOut.find(RGA_DX11_DRIVER_ERROR_TOKEN) == std::string::npos)
        {
            std::stringstream  outStream;
            std::string  adapterName;
            outStream << driverOut;
            while (std::getline(outStream, adapterName, '\n'))
            {
                adapterNames.push_back(adapterName);
            }
        }
        else
        {
            result = false;
        }
    }

    return result;
}

bool beProgramBuilderDX::GetDXXModulePathForAdapter(int adapterID, bool printCmd, std::string& adapterName, std::string& dxxModulePath)
{
    bool  result = false;
    std::string  driverOut;
    std::stringstream  args;
    args << RGA_DX11_DRIVER_GET_ADAPTER_INFO << " " << adapterID;

    beKA::beStatus  status = InvokeDX11Driver(args.str().c_str(), printCmd, driverOut);

    result = (status == beKA::beStatus_SUCCESS && !driverOut.empty());

    if (result)
    {
        if (driverOut.find(RGA_DX11_DRIVER_ERROR_TOKEN) == std::string::npos)
        {
            std::stringstream  outStream;
            outStream << driverOut;

            result = (std::getline(outStream, adapterName, '\n') && std::getline(outStream, dxxModulePath));
        }
        else
        {
            result = false;
        }
    }

    return result;
}
#endif
