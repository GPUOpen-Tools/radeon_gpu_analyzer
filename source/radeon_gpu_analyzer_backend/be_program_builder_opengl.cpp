//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================

// C++.
#include <sstream>

// Infra.
#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable:4309)
#endif
#include "external/amdt_base_tools/Include/gtAssert.h"
#include "external/amdt_os_wrappers/Include/osDirectory.h"
#include "external/amdt_os_wrappers/Include/osProcess.h"
#include "external/amdt_os_wrappers/Include/osThread.h"
#ifdef _WIN32
    #pragma warning(pop)
#endif

// Local.
#include "radeon_gpu_analyzer_backend/be_program_builder_opengl.h"
#include "radeon_gpu_analyzer_backend/be_utils.h"
#include "radeon_gpu_analyzer_backend/be_backend.h"

// Device info.
#include "DeviceInfoUtils.h"

// *****************************************
// *** INTERNALLY LINKED SYMBOLS - START ***
// *****************************************

// The list of devices not supported by glc.
static const std::set<std::string> kOpenglDisabledDevices = {"Bristol Ridge", "Carrizo", "Iceland", "Tonga", "Stoney", "gfx804"};

static const std::string kStrVkOfflineSpirvOutputFilenameVertex                 = "vert.spv";
static const std::string kStrVkOfflineSpirvOutputFilenameTessellationControl    = "tesc.spv";
static const std::string kStrVkOfflineSpirvOutputFilenameTessellationEvaluation = "tese.spv";
static const std::string kStrVkOfflineSpirvOutputFilenameGeometry               = "geom.spv";
static const std::string kStrVkOfflineSpirvOutputFilenameFragment               = "frag.spv";
static const std::string kStrVkOfflineSpirvOutputFilenameCompute                = "comp.spv";

static const std::string kStrPalIlOutputFilenameVert                   = "vert.palIl";
static const std::string kStrPalIlOutputFilenameTessellationControl    = "tesc.palIl";
static const std::string kStrPalIlOutputFilenameTessellationEvaluation = "tese.palIl";
static const std::string kStrPalIlOutputFilenameGeometry               = "geom.palIl";
static const std::string kStrPalIlOutputFilenameFragment               = "frag.palIl";
static const std::string kStrPalIlOutputFilenameCompute                = "comp.palIl";

// ***************************************
// *** INTERNALLY LINKED SYMBOLS - END ***
// ***************************************

// Internally-linked utilities.
static bool GetGlcPath(std::string& glc_path)
{
#ifdef __linux
    glc_path = "glc";
#elif _WIN64
    glc_path = "utils\\glc.exe";
#else
    glc_path = "x86\\glc.exe";
#endif
    return true;
}

static beKA::beStatus AddInputFileNames(const OpenglOptions& options, std::stringstream& cmd)
{
    beKA::beStatus status = beKA::kBeStatusSuccess;

    // Indicates that some of stage-specific file names was provided (--frag, --vert, etc.).
    bool is_stage_input = false;

    // You cannot mix compute and non-compute shaders in opengl,
    // so this has to be mutually exclusive.
    if (options.pipeline_shaders.compute_shader.isEmpty())
    {
        // Vertex shader.
        if (!options.pipeline_shaders.vertex_shader.isEmpty())
        {
            cmd << "in.vert.glsl=\"" << options.pipeline_shaders.vertex_shader.asASCIICharArray() << "\" ";
            is_stage_input = true;
        }

        // Tessellation control shader.
        if (!options.pipeline_shaders.tessellation_control_shader.isEmpty())
        {
            cmd << "in.tesc.glsl=\"" << options.pipeline_shaders.tessellation_control_shader.asASCIICharArray() << "\" ";
            is_stage_input = true;
        }

        // Tessellation evaluation shader.
        if (!options.pipeline_shaders.tessellation_evaluation_shader.isEmpty())
        {
            cmd << "in.tese.glsl=\"" << options.pipeline_shaders.tessellation_evaluation_shader.asASCIICharArray() << "\" ";
            is_stage_input = true;
        }

        // Geometry shader.
        if (!options.pipeline_shaders.geometry_shader.isEmpty())
        {
            cmd << "in.geom.glsl=\"" << options.pipeline_shaders.geometry_shader.asASCIICharArray() << "\" ";
            is_stage_input = true;
        }

        // Fragment shader.
        if (!options.pipeline_shaders.fragment_shader.isEmpty())
        {
            cmd << "in.frag.glsl=\"" << options.pipeline_shaders.fragment_shader.asASCIICharArray() << "\" ";
            is_stage_input = true;
        }
    }
    else
    {
        // Compute shader.
        cmd << "in.comp.glsl=\"" << options.pipeline_shaders.compute_shader.asASCIICharArray() << "\" ";
        is_stage_input = true;
    }

    return status;
}

static void AddOutputFileNames(const OpenglOptions& options, std::stringstream& cmd)
{
    auto add_output_file = [&](bool flag, const std::string& option, const std::string& fileName) {
        if (flag)
        {
            cmd << option << "\"" << fileName << "\""
                << " ";
        }
    };

    // AMD ISA binary generation.
    if (options.is_amd_isa_binaries_required)
    {
        // Compute.
        add_output_file(
            !options.pipeline_shaders.compute_shader.isEmpty(), "out.comp.isa=", options.il_disassembly_output_files.compute_shader.asASCIICharArray());

        if (options.pipeline_shaders.compute_shader.isEmpty())
        {
            // Vertex.
            add_output_file(
                !options.pipeline_shaders.vertex_shader.isEmpty(), "out.vert.isa=", options.il_disassembly_output_files.vertex_shader.asASCIICharArray());
            // Tessellation control.
            add_output_file(!options.pipeline_shaders.tessellation_control_shader.isEmpty(),
                            "out.tesc.isa=",
                            options.il_disassembly_output_files.tessellation_control_shader.asASCIICharArray());
            // Tessellation evaluation.
            add_output_file(!options.pipeline_shaders.tessellation_evaluation_shader.isEmpty(),
                            "out.tese.isa=",
                            options.il_disassembly_output_files.tessellation_evaluation_shader.asASCIICharArray());
            // Geometry.
            add_output_file(
                !options.pipeline_shaders.geometry_shader.isEmpty(), "out.geom.isa=", options.il_disassembly_output_files.geometry_shader.asASCIICharArray());
            // Fragment.
            add_output_file(
                !options.pipeline_shaders.fragment_shader.isEmpty(), "out.frag.isa=", options.il_disassembly_output_files.fragment_shader.asASCIICharArray());
        }
    }

    // Pipeline ELF binary generation.
    if (options.is_pipeline_binary_required)
    {
        add_output_file(!options.program_binary_filename.empty(), "out.pipeBin=", options.program_binary_filename);
    }

    // AMD ISA disassembly generation.
    if (options.is_amd_isa_disassembly_required)
    {
        // Compute.
        add_output_file(
            !options.pipeline_shaders.compute_shader.isEmpty(), "out.comp.isaText=", options.isa_disassembly_output_files.compute_shader.asASCIICharArray());

        if (options.pipeline_shaders.compute_shader.isEmpty())
        {
            // Vertex.
            add_output_file(
                !options.pipeline_shaders.vertex_shader.isEmpty(), "out.vert.isaText=", options.isa_disassembly_output_files.vertex_shader.asASCIICharArray());
            // Tessellation control.
            add_output_file(!options.pipeline_shaders.tessellation_control_shader.isEmpty(),
                            "out.tesc.isaText=",
                            options.isa_disassembly_output_files.tessellation_control_shader.asASCIICharArray());
            // Tessellation evaluation.
            add_output_file(!options.pipeline_shaders.tessellation_evaluation_shader.isEmpty(),
                            "out.tese.isaText=",
                            options.isa_disassembly_output_files.tessellation_evaluation_shader.asASCIICharArray());
            // Geometry.
            add_output_file(!options.pipeline_shaders.geometry_shader.isEmpty(),
                            "out.geom.isaText=",
                            options.isa_disassembly_output_files.geometry_shader.asASCIICharArray());
            // Fragment.
            add_output_file(!options.pipeline_shaders.fragment_shader.isEmpty(),
                            "out.frag.isaText=",
                            options.isa_disassembly_output_files.fragment_shader.asASCIICharArray());
        }
    }

    // Shader compiler statistics disassembly generation.
    if (options.is_stats_required)
    {
        // Compute.
        add_output_file(!options.pipeline_shaders.compute_shader.isEmpty(), "out.comp.isaInfo=", options.stats_output_files.compute_shader.asASCIICharArray());

        if (options.pipeline_shaders.compute_shader.isEmpty())
        {
            // Vertex.
            add_output_file(
                !options.pipeline_shaders.vertex_shader.isEmpty(), "out.vert.isaInfo=", options.stats_output_files.vertex_shader.asASCIICharArray());
            // Tessellation control.
            add_output_file(!options.pipeline_shaders.tessellation_control_shader.isEmpty(),
                            "out.tesc.isaInfo=",
                            options.stats_output_files.tessellation_control_shader.asASCIICharArray());
            // Tessellation evaluation.
            add_output_file(!options.pipeline_shaders.tessellation_evaluation_shader.isEmpty(),
                            "out.tese.isaInfo=",
                            options.stats_output_files.tessellation_evaluation_shader.asASCIICharArray());
            // Geometry.
            add_output_file(
                !options.pipeline_shaders.geometry_shader.isEmpty(), "out.geom.isaInfo=", options.stats_output_files.geometry_shader.asASCIICharArray());
            // Fragment.
            add_output_file(
                !options.pipeline_shaders.fragment_shader.isEmpty(), "out.frag.isaInfo=", options.stats_output_files.fragment_shader.asASCIICharArray());
        }
    }

    // Shader compiler il disassembly generation.
    if (options.is_il_disassembly_required)
    {
        // Compute.
        add_output_file(!options.pipeline_shaders.compute_shader.isEmpty(), "out.comp.ilText=", options.il_disassembly_output_files.compute_shader.asASCIICharArray());

        if (options.pipeline_shaders.compute_shader.isEmpty())
        {
            // Vertex.
            add_output_file(
                !options.pipeline_shaders.vertex_shader.isEmpty(), "out.vert.ilText=", options.il_disassembly_output_files.vertex_shader.asASCIICharArray());
            // Tessellation control.
            add_output_file(!options.pipeline_shaders.tessellation_control_shader.isEmpty(),
                            "out.tesc.ilText=",
                            options.il_disassembly_output_files.tessellation_control_shader.asASCIICharArray());
            // Tessellation evaluation.
            add_output_file(!options.pipeline_shaders.tessellation_evaluation_shader.isEmpty(),
                            "out.tese.ilText=",
                            options.il_disassembly_output_files.tessellation_evaluation_shader.asASCIICharArray());
            // Geometry.
            add_output_file(
                !options.pipeline_shaders.geometry_shader.isEmpty(), "out.geom.ilText=", options.il_disassembly_output_files.geometry_shader.asASCIICharArray());
            // Fragment.
            add_output_file(
                !options.pipeline_shaders.fragment_shader.isEmpty(), "out.frag.ilText=", options.il_disassembly_output_files.fragment_shader.asASCIICharArray());
        }
    }
}

// Checks if the required output files are generated by the glc.
// Only verifies the files requested in the "options.m_pipelineShaders" name list.
static bool VerifyGlcOutput(const OpenglOptions& options, const std::string& glc_gfx_ip)
{
    bool ret = true;

    // For now, only perform input validation for pre Vega targets.
    // This should be updated to take into consideration shader merging
    // which may happen in Vega subsequent generations.
    if (!glc_gfx_ip.empty())
    {
        if (options.is_amd_isa_disassembly_required)
        {
            ret &= (options.pipeline_shaders.compute_shader.isEmpty() ||
                    BeUtils::IsFilePresent(options.isa_disassembly_output_files.compute_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.fragment_shader.isEmpty() ||
                    BeUtils::IsFilePresent(options.isa_disassembly_output_files.fragment_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.geometry_shader.isEmpty() ||
                    BeUtils::IsFilePresent(options.isa_disassembly_output_files.geometry_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.tessellation_control_shader.isEmpty() ||
                    BeUtils::IsFilePresent(options.isa_disassembly_output_files.tessellation_control_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.tessellation_evaluation_shader.isEmpty() ||
                    BeUtils::IsFilePresent(options.isa_disassembly_output_files.tessellation_evaluation_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.vertex_shader.isEmpty() ||
                    BeUtils::IsFilePresent(options.isa_disassembly_output_files.vertex_shader.asASCIICharArray()));
        }

        if (ret && options.is_stats_required)
        {
            ret &= (options.pipeline_shaders.compute_shader.isEmpty() || BeUtils::IsFilePresent(options.stats_output_files.compute_shader.asASCIICharArray()));
            ret &=
                (options.pipeline_shaders.fragment_shader.isEmpty() || BeUtils::IsFilePresent(options.stats_output_files.fragment_shader.asASCIICharArray()));
            ret &=
                (options.pipeline_shaders.geometry_shader.isEmpty() || BeUtils::IsFilePresent(options.stats_output_files.geometry_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.tessellation_control_shader.isEmpty() ||
                    BeUtils::IsFilePresent(options.stats_output_files.tessellation_control_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.tessellation_evaluation_shader.isEmpty() ||
                    BeUtils::IsFilePresent(options.stats_output_files.tessellation_evaluation_shader.asASCIICharArray()));
            ret &= (options.pipeline_shaders.vertex_shader.isEmpty() || BeUtils::IsFilePresent(options.stats_output_files.vertex_shader.asASCIICharArray()));
        }
    }

    return ret;
}

BeProgramBuilderOpengl::BeProgramBuilderOpengl()
{
}

BeProgramBuilderOpengl::~BeProgramBuilderOpengl()
{
}

beKA::beStatus BeProgramBuilderOpengl::GetKernelIlText(const std::string& device, const std::string& kernel, std::string& il)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(il);

    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpengl::GetKernelIsaText(const std::string& device, const std::string& kernel, std::string& isa)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(isa);

    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpengl::GetStatistics(const std::string& device, const std::string& kernel, beKA::AnalysisData& analysis)
{
    GT_UNREFERENCED_PARAMETER(device);
    GT_UNREFERENCED_PARAMETER(kernel);
    GT_UNREFERENCED_PARAMETER(analysis);

    // In the executable-oriented architecture, this operation is no longer meaningful.
    return beKA::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpengl::GetDeviceTable(std::vector<GDT_GfxCardInfo>& table)
{
    (void)table;
    return beKA::kBeStatusInvalid;
}

beKA::beStatus BeProgramBuilderOpengl::Compile(const OpenglOptions& gl_options,
                                               bool&                cancel_signal,
                                               bool                 should_print_cmd,
                                               gtString&            glc_output,
                                               gtString&            build_log)
{
    GT_UNREFERENCED_PARAMETER(cancel_signal);
    beKA::beStatus ret = beKA::kBeStatusSuccess;

    // Clear the output buffer if needed.
    if (!glc_output.isEmpty())
    {
        glc_output.makeEmpty();
    }

    // Get glc's path.
    std::string glc_path;
    GetGlcPath(glc_path);

    AMDTDeviceInfoUtils* device_info = AMDTDeviceInfoUtils::Instance();
    if (device_info != nullptr)
    {
        // Numerical representation of the HW generation.
        std::string device_gfx_ip;

        // Convert the HW generation to the glc string.
        bool is_device_hw_gen_extracted = GetDeviceGLName(gl_options.device_name, device_gfx_ip);
        if (is_device_hw_gen_extracted && !gl_options.device_name.empty())
        {
            // Build the command for invoking glc.
            std::stringstream cmd;
            cmd << glc_path;

            if (gl_options.optimization_level != -1)
            {
                cmd << " -O" << std::to_string(gl_options.optimization_level) << " ";
            }

           cmd << " -gfxip " << device_gfx_ip << " -set ";

            if ((ret = AddInputFileNames(gl_options, cmd)) == beKA::kBeStatusSuccess)
            {
                AddOutputFileNames(gl_options, cmd);

                // Redirect build log to a temporary file.
                const gtString kGlcTmpOutputFile = L"glcTempFile.txt";
                osFilePath     tmp_file_path(osFilePath::OS_TEMP_DIRECTORY);
                tmp_file_path.setFileName(kGlcTmpOutputFile);

                // Delete the log file if it already exists.
                if (tmp_file_path.exists())
                {
                    osFile tmp_log_file(tmp_file_path);
                    tmp_log_file.deleteFile();
                }

                cmd << "out.glslLog=\"" << tmp_file_path.asString().asASCIICharArray() << "\" ";

                // No default output (only generate the output files that we explicitly specified).
                cmd << "defaultOutput=0";

                // Launch glc.
                BeUtils::PrintCmdLine(cmd.str(), should_print_cmd);
                bool is_launch_success = osExecAndGrabOutput(cmd.str().c_str(), cancel_signal, glc_output);
                if (is_launch_success)
                {
                    // This is how glc signals success.
                    const gtString kGlcTokenSuccess = L"SUCCESS!";

                    // Check if the output files were generated and glc returned "success".
                    if (glc_output.find(kGlcTokenSuccess) == std::string::npos)
                    {
                        ret = beKA::kBeStatusGlcCompilationFailure;

                        // Read the build log.
                        if (tmp_file_path.exists())
                        {
                            // Read the build log.
                            gtString      compiler_output;
                            std::ifstream file(tmp_file_path.asString().asASCIICharArray());
                            std::string   tmp_cmd_output((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                            build_log << tmp_cmd_output.c_str();

                            // Delete the temporary file.
                            osFile file_to_delete(tmp_file_path);
                            file_to_delete.deleteFile();
                        }

                        // Let's end the build log with the error that was provided by the backend.
                        if (!glc_output.isEmpty())
                        {
                            build_log << "Error: " << glc_output << L"\n";
                        }
                    }
                    else if (!VerifyGlcOutput(gl_options, device_gfx_ip))
                    {
                        ret = beKA::kBeStatusFailedOutputVerification;
                    }
                    else
                    {
                        ret = beKA::kBeStatusSuccess;
                    }
                }
                else
                {
                    ret = beKA::kBeStatusGlcLaunchFailure;
                }
            }
        }
        else
        {
            ret = beKA::kBeStatusOpenglUnknownHwFamily;
        }
    }

    return ret;
}

bool BeProgramBuilderOpengl::GetOpenGLVersion(bool should_print_cmd, gtString& opengl_version) const
{
    // Get glc's path.
    std::string glc_path;
    GetGlcPath(glc_path);

    // Build the command for invoking glc.
    std::stringstream cmd;
    cmd << glc_path << " \";;;;;;;;;;;;;;;;;;;;;version;;;;;;;\"";

    // A flag for canceling the operation, we will not use it.
    bool dummy_cancel_flag = false;
    BeUtils::PrintCmdLine(cmd.str(), should_print_cmd);
    bool isLaunchSuccess = osExecAndGrabOutput(cmd.str().c_str(), dummy_cancel_flag, opengl_version);

    return isLaunchSuccess;
}

bool BeProgramBuilderOpengl::GetDeviceGLName(const std::string& device_name, std::string& valid_device_name) const
{
    bool ret = false;

    // This map will hold the device values as expected by the OpenGL backend.
    static std::map<std::string, std::string> gl_backend_values;
    if (gl_backend_values.empty())
    {
        // Fill in the values if that's the first time.
        gl_backend_values["gfx803"]        = "803";
        gl_backend_values["Ellesmere"]     = "803";
        gl_backend_values["Baffin"]        = "803";
        gl_backend_values["Fiji"]          = "803";
        gl_backend_values["gfx900"]        = "900";
        gl_backend_values["gfx902"]        = "902";
        gl_backend_values["gfx904"]        = "904";
        gl_backend_values["gfx906"]        = "906";
        gl_backend_values["gfx90c"]        = "90c";
        gl_backend_values["gfx1010"]       = "1010";
        gl_backend_values["gfx1011"]       = "1011";
        gl_backend_values["gfx1012"]       = "1012";
        gl_backend_values["gfx1030"]       = "1030";
        gl_backend_values["gfx1031"]       = "1031";
        gl_backend_values["gfx1032"]       = "1032";
        gl_backend_values["gfx1033"]       = "1033";
        gl_backend_values["gfx1034"]       = "1034";
        gl_backend_values["gfx1035"]       = "1035";
        gl_backend_values["gfx1036"]       = "1036";
        gl_backend_values["gfx1100"]       = "1100";
        gl_backend_values["gfx1101"]       = "1101";
        gl_backend_values["gfx1102"]       = "1102";
        gl_backend_values["gfx1103"]       = "1103";
    }

    // Fetch the relevant value.
    auto device_iter = gl_backend_values.find(device_name);
    if (device_iter != gl_backend_values.end())
    {
        valid_device_name = device_iter->second;
        ret               = true;
    }

    return ret;
}

bool BeProgramBuilderOpengl::GetSupportedDevices(std::set<std::string>& device_list)
{
    std::vector<GDT_GfxCardInfo> tmp_card_list;
    bool ret = BeUtils::GetAllGraphicsCards(tmp_card_list, device_list);

    // Remove unsupported devices.
    if (ret)
    {
        for (const std::string& device : kOpenglDisabledDevices)
        {
            device_list.erase(device);
        }
    }
    return ret;
}

const std::set<std::string>& BeProgramBuilderOpengl::GetDisabledDevices()
{
    return kOpenglDisabledDevices;
}
