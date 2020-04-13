#pragma once
#include <string>
#include <vector>
#include <map>

namespace rga
{
    // Per-stage configuration (based on user's invocation command).
    struct rgDx12ShaderConfig
    {
        // Full path to HLSL file containing the shader.
        std::string hlsl;

        // Full path to compiled DXBC of the shader as an input.
        std::string dxbc;

        // Full path to ISA disassembly output file.
        std::string isa;

        // Full path to resource usage output file.
        std::string stats;

        // Shader model for the shader (also known as Target).
        std::string shaderModel;

        // The target entry point.
        std::string entryPoint;

        // The compiled shader DXBC binary output file.
        std::string dxbcOut;

        // The compiled shader DXBC disassembly output file.
        std::string dxbcDisassembly;
    };

    // Global configuration (based on user's invocation command).
    struct rgDx12Config
    {
        // Per-stage configuration.
        rgDx12ShaderConfig comp;
        rgDx12ShaderConfig vert;
        rgDx12ShaderConfig hull;
        rgDx12ShaderConfig domain;
        rgDx12ShaderConfig geom;
        rgDx12ShaderConfig pixel;

        // Additional include directories.
        std::vector<std::string> includeDirs;

        // Additional include directories.
        std::vector<std::string> defines;

        // The name of the macro which defines the root signature in the HLSL code.
        std::string rsMacro;

        // The full path to the HLSL file where the root signature macro is defined in.
        std::string rsMacroFile;

        // Root signature version, by default "rootsig_1_1" is assumed.
        std::string rsVersion = "rootsig_1_1";

        // Full path to the serialized root signature file.
        std::string rsSerialized;

        // Full path to the file that describes the pipeline state.
        std::string rsPso;

        // Full path to the pipeline binary output file.
        std::string pipelineBinary;

        // The ID of the GPU for which to compile the pipeline. If not specified,
        // the pipeline would be compiled for the physically installed GPU.
        int targetGpu = -1;

        // True if we should list all supported targets.
        bool shouldListTargets = false;
    };

    // Per-stage boolean.
    struct rgPipelineBool
    {
        bool vert = false;
        bool hull = false;
        bool domain = false;
        bool geom = false;
        bool pixel = false;
    };

    // Per-stage bytecode.
    struct rgDx12PipelineByteCode
    {
        D3D12_SHADER_BYTECODE vert;
        D3D12_SHADER_BYTECODE hull;
        D3D12_SHADER_BYTECODE domain;
        D3D12_SHADER_BYTECODE geom;
        D3D12_SHADER_BYTECODE pixel;
    };
}
