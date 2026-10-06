//=============================================================================
/// Copyright (c) 2017-2026 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for rga shared data types.
//=============================================================================

#pragma once

// Type of Vulkan input file.
enum class RgVulkanInputType
{
    kUnknown,
    kGlsl,
    kHlsl,
    kSpirv,
    kSpirvTxt
};

// The base type for a Pipeline State file, which contains all
// configuration necessary to create a graphics or compute pipeline.
class RgPsoCreateInfo
{
public:
    RgPsoCreateInfo()          = default;
    virtual ~RgPsoCreateInfo() = default;
};

// The type of stage used for a shader module.
enum RgPipelineStage : char
{
    // The vertex shader stage.
    kVertex,

    // The Vulkan Tessellation Control / DX12 Hull shader stage.
    kTessellationControl,

    // The Vulkan Tessellation Evaluation / DX12 Domain shader stage.
    kTessellationEvaluation,

    // The geometry shader stage.
    kGeometry,

    // The Vulkan Fragment / DX12 Pixel shader stage.
    kFragment,

    // The compute shader stage.
    kCompute,

    // The mesh shader stage.
    kMesh,

    // The task shader stage.
    kTask,

    // The total count of pipeline stage types.
    kCount
};

// RGA csv file headers.
enum class RgCsvFileColumns
{
    kAddress,
    kSourceLineNumber,
    kOpcode,
    kOperands,
    kFunctionalUnit,
    kCycles,
    kBinaryEncoding,
    kSourcePath,
    kCount,
};

// CSV headers for parsed ISA output.
static const char* kStrCsvHeaderWithLineCorrelation =
    "Address, Source Line Number, Opcode, Operands, Functional Unit, Cycles, Binary Encoding, Source Line Path\n";
static const char* kStrCsvHeaderNoLineCorrelation =
    "Address, Opcode, Operands, Functional Unit, Cycles, Binary Encoding\n";

// CSV column name for source line number (used to detect line correlation in CSV headers).
static const char* kStrCsvColumnSourceLineNumber = "Source Line Number";

// Default value for the source path column when no source file is associated with an instruction.
static const char* kStrUnknownSourcePath = "UNKNOWN_SOURCE_PATH";
