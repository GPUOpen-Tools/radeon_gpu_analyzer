//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for rga backend progam builder dx11 class.
//=============================================================================

#ifdef _WIN32

#ifndef RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_DX11_H_
#define RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_DX11_H_



// C++.
#include <vector>
#include <string>
#include <utility>

// GSA.
#include "external/dx11/GSA/AmdDxGsaCompile.h"

// Local.
#include "radeon_gpu_analyzer_backend/be_program_builder.h"
#include "radeon_gpu_analyzer_backend\be_amddxxmodule_wrapper.h"

using namespace beKA;
class CElf;
class CElfSection;

// Options specific to DX11.
struct Dx11Options : public CompileOptions
{
    // The name of the source file.
    std::string filename;

    /// The entry point for the shader.
    std::string entrypoint;

    // A string that specifies the shader target or set of shader
    // features to compile against. The shader target can be
    // shader model 2, shader model 3, shader model 4, or shader
    // model 5. In DX, the target can also be an effect type (for
    // example, fx_4_1).  So we expect to see ps_5_0, vs_5_0 &c.
    std::string target;

    // Device name.
    std::string device_name;

    // Which chip family?  See ...\drivers\inc\asic_reg\atiid.h for the list.
    // Currently only SI is supported.
    UINT chip_family;

    // Which device?  See ...\drivers\inc\asic_reg\si_id.h &c. for the lists.
    UINT chip_revision;

    // Compilation flags to pass to D3DCompile.
    // See D3DCompiler.h in the DX SDK.
    union Dx11Flags
    {
        Dx11Flags() : flags_as_int(0) {}
        Dx11Flags(unsigned val) : flags_as_int(val) {}

        unsigned int flags_as_int;
        struct
        {
            int debug : 1; // 0
            int skip_validation : 1; // 1
            int skip_optimization : 1; // 2
            int pack_matrix_row_major : 1; // 3
            int pack_matrix_column_major : 1; // 4
            int partial_precision : 1; // 5
            int force_vs_software_no_opt : 1; // 6
            int force_ps_software_no_opt : 1; // 7
            int no_preshader : 1; // 8
            int avoid_flow_control : 1; // 9
            int prefer_flow_control : 1; // 10
            int enable_strictness : 1; // 11
            int enable_backwards_compatiblity : 1; // 12
            int ieee_strictness : 1; // 13
            int optimization_level : 2; // 14:15 (level,bits): (0,01)(1,00)(2,11)(3,10)
            int reserved16 : 1; // 16
            int reserved17 : 1; // 17
            int warnings_are_errors : 1; // 18
        } flags_as_bit_field;
    } dx_flags;

    // Defines to pass to D3Dompile.
    // The pairs are (as you might expect): Symbol, Value
    // So -DDEBUG=1 would be "DEBUG", "1"
    std::vector<std::pair<std::string, std::string> > defines;

    // Additional include directories for searching included headers.
    std::vector<std::string> include_directories;

    // User-defined UAV Slot value for AMD D3D11 Shader Intrinsics.
    // This value should be in the range of [0,63].
    int uav_slot = -1;

    // True to enable AMD D3D11 Shader Intrinsics extension.
    bool is_shader_intrinsics_enabled = false;

    // Tue to save the MS Blob as text
    bool should_dump_ms_intermediate;
};

class BeProgramBuilderDx11 : public BeProgramBuilder
{
public:
    BeProgramBuilderDx11();
    ~BeProgramBuilderDx11();

    beKA::beStatus GetKernelIlText(const std::string& device, const std::string& kernel, std::string& il) override;
    beKA::beStatus GetKernelIsaText(const std::string& device, const std::string& kernel, std::string& isa) override;
    beKA::beStatus GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis) override;
    beKA::beStatus GetDeviceTable(std::vector<GDT_GfxCardInfo>& table) override;
    beKA::beStatus GetISABinary(const std::string& device, std::vector<char>& binary);
    void ReleaseProgram();

    // Compile the specified source file.
    virtual beKA::beStatus Compile(RgaMode mode, const std::string& source_code, const Dx11Options& dx_options, bool is_dxbc);

    // Retrieve the bytecode blob.
    virtual beKA::beStatus GetIntermediateMsBlob(std::string& intermediate_ms_blob);

    // Used when the DXASM code is generated using the FXC tool, and therefore needs to be injected to the DX builder.
    void SetIntermediateMsBlob(const std::string& intermediate_ms_blob);

    // Add a search directory for include files.
    void AddDxSearchDir(const std::string& dir);

    // Extract the ISA disassembly for a given target device.
    beKA::beStatus GetDxShaderIsaText(const std::string& device, std::string& isa_buffer);

    /// Extract the IL disassembly for a given target device.
    beKA::beStatus GetDxShaderIl(const std::string& device, std::string& il_buffer);

    // Extracts the textual contents of an ELF section that contains text in its data.
    void ExtractTextFromElfSection(const CElfSection* elf_section, std::string& content);

    // Extract the size in bytes of ISA code.
    // Returns  true for success, false otherwise.
    bool GetIsaSize(const std::string& isa_as_text, size_t& size_in_bytes) const;

    // Extract the number of threads per wavefront for a given device.
    // Returns    true for success, false otherwise.
    bool GetWavefrontSize(const std::string& device_name, size_t& wavefront_size) const;

    // Returns the CELF* for the given device, or nullptr if no such CELF* exists.
    CElf* GetDeviceElf(const std::string& device_name) const;

    std::vector<char> GetDeviceBinaryElf(const std::string& device_name) const;

    // Sets the set of public device names.
    void SetPublicDeviceNames(const std::set<std::string>& public_device_names);

    // Retrieves the list of names of AMD display adapters installed on the system.
    static bool GetSupportedDisplayAdapterNames(bool should_print_cmd, std::vector<std::string>& adapter_names);

    // Gets the full path to AMD DXX library for GPU adapter specified by "adapterID".
    // Returns the adapter name in "adapterName" and DXX lib path in "dxxModulePath".
    static bool GetDxxModulePathForAdapter(int adapter_id, bool should_print_cmd, std::string& adapter_name, std::string& dxx_module_path);

    beKA::beStatus Initialize(const std::string& dxx_module_name, const std::string& compiler_module_name, bool print_process_cmd_line);

private:
    // (Wrapper) Interface with atidxx{32,64}.dll
    AMDDXXModuleWrapper amd_dxx_module_;

    // Interface with d3dcompiler_xx.dll
    D3DCompileModule d3d_compile_module_;

    // Stream for diagnostic output.
    LoggingCallBackFuncP log_callback_ = nullptr;

    std::vector<GDT_GfxCardInfo> dx_device_table_;
    std::set<std::string> public_device_names_;

    // Holds additional directories where DX binaries should be searched at.
    std::vector<std::string> loader_search_directories_;

    // Alias for ELF and its binary representation
    using CelfBinaryPair = std::pair<CElf*, std::vector<char>>;

    // Maps between a device name and its corresponding ELF pointer and Elf binary.
    std::map<std::string, CelfBinaryPair> compiled_elf_;

    // Holds the D3D compiler's output.
    std::string ms_intermediate_text_;

    // True if initialized.
    bool is_initialized_ = false;

#ifdef DXASM_T_ENABLED
public:
    static std::string* translated_program_;
    static int* translated_program_size_;
#endif

    // Wrapper to deal with crashes in the driver.
    HRESULT AmdDxGsaCompileShaderWrapper(const struct _AmdDxGsaCompileShaderInput* input_config, struct _AmdDxGsaCompileShaderOutput* output);

    beKA::beStatus CompileAMDIL(const std::string& program_source, const Dx11Options& dx_options);
    beKA::beStatus CompileHLSL(const std::string& program_source, const Dx11Options& dx_options, bool is_dxbc);
    beKA::beStatus CompileDXAsm(const std::string& program_source, const Dx11Options& dx_options);
    const CElfSection* GetISATextSection(const std::string& device_name) const;
    const CElfSection* GetILDisassemblySection(const std::string& device_name) const;
    const CElfSection* GetElfSection(const std::string& device_name, const std::string& section_name) const;
    std::string ToLower(const std::string& str) const;
    void SetDeviceElf(const std::string& device_name, const AmdDxGsaCompileShaderOutput& output);
    bool GetDeviceElfBinPair(const std::string& device_name, CelfBinaryPair& elf_bin_pair) const;

    /// Clears the member variables which hold the build outputs.
    void ClearFormerBuildOutputs();
};

#endif // RGA_RADEONGPUANALYZERBACKEND_SRC_BE_PROGRAM_BUILDER_DX11_H_
#endif
