//=================================================================
// Copyright 2020 Advanced Micro Devices, Inc. All rights reserved.
//=================================================================
#ifndef RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_STRING_CONSTANTS_H_
#define RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_STRING_CONSTANTS_H_

// Constants - general.
const char* const kStrRgaProductName = "Radeon GPU Analyzer";
const char* const kStrRgaVersionPrefix = "Version: ";
static const char* kStrAllDevicesToken = "all";

// Errors.
static const char* kStrErrorError = "Error: ";
static const char* kStrErrorNoValidCommandDetected = "Error: no valid command. Please run -h for available commands.";
static const char* kStrErrorCommandNotSupported = "Error: the command is not supported for this mode.";
static const char* kStrErrorCannotExtractSupportedDeviceList = "Error: unable to extract the list of supported devices.";
static const char* kStrErrorNoDeviceFound = "Error: unable to find supported devices.";
static const char* kStrErrorUnknownDevice = "Error: code object compiled for unknown device.";
static const char* kStrErrorMemoryAllocationFailure = "Error: memory allocation failure.";
static const char* kStrErrorGraphicsComputeMix = "Error: cannot mix compute and non-compute shaders.";
static const char* kStrErrorCannotFindOutputDir = "Error: output directory does not exist: ";
static const char* kStrErrorCannotInvokeCompiler = "Error: unable to invoke the compiler.";
static const char* kStrErrorCannotReadFile = "Error: unable to read: ";
static const char* kStrErrorCannotCopyFileA = "Error: unable to copy file: ";
static const char* kStrErrorCannotCopyFileB = "to file: ";
static const char* kStrErrorCannotOpenFileForWriteA = "Error: unable to open ";
static const char* kStrErrorCannotOpenFileForWriteB = " for write.";
static const char* kStrErrorLiveregWithoutIsa = "Error: cannot perform live register analysis without generating ISA disassembly (use --isa option).";
static const char* kStrErrorLiveregSgprWithoutIsa = "Error: cannot perform live register sgpr analysis without generating ISA disassembly (use --isa option).";
static const char* kStrErrorCfgWithoutIsa = "Error: cannot generate control flow graph without generating ISA disassembly (use --isa option).";
static const char* kStrErrorCannotPerformLiveregAnalysis = "Error: failed to perform live register analysis for ";
static const char* kStrErrorCannotPerformLiveregAnalysisSgpr = "Error: failed to perform live sgpr analysis for ";
static const char* kStrErrorCannotGenerateCfg = "Error: failed to generate control-flow graph for ";
static const char* kStrErrorCannotExtractBinaries = "Error: failed to extract binaries";
static const char* kStrErrorCannotExtractMetadata = "Error: failed to extract meta-data";
static const char* kStrErrorUnknownCompilationStatus = "Error: unknown compilation status returned.";
static const char* kStrErrorUnknownAmdGpuDisStatus = "Error: unknown disassembly status returned.";
static const char* kStrErrorFailedAmdGpuDisStatus = "Error: failed to disassemble CodeObject. ";
static const char* kErrCannotWriteDisassemblyFile = "Error: failed to write shader disassembly for ";
static const char* kStrErrorFailedToAdjustFileName = "Error: failed to construct some of the output file names.";
static const char* kStrErrorNoInputFile = "Error: no input file provided.";
static const char* kStrErrorSingleInputFileExpected = "Error: exactly one input file must be specified.";
static const char* kStrErrorOutputFileVerificationFailed = "Error: failed to generate one or more output files.";
static const char* kStrErrorFailedCreateOutputFilename = "Error: failed to construct output file name.";
static const char* kStrErrorFailedToGenerateSessionMetdata = "Error: failed to generate Session Metadata file.";
static const char* kStrErrorFailedToWriteIsaFile = "Error: failed to write ISA file: ";
static const char* kStrErrorFailedToConvertToCsvFormat = "Error: CSV conversion failed in file: ";

// Warnings.
static const char* kStrWarningDx11MinSupportedVersion = "Warning: AMD DirectX driver supports DX10 and above.";
static const char* kStrWarningOpenclSupressWithoutBinary = "Warning: --suppress option is valid only with output binary.";
static const char* kStrWarningVkOfflineIncorrectOptLevel = "Warning: The optimization level is not supported; ignoring.";
static const char* kStrWarningOpenclMetadataNotSupported1 = "Warning: Extracting metadata for ";
static const char* kStrWarningOpenclMetadataNotSupported2 = " is not supported.";

static const char* kStrWarningLiveregNotSupported = "Warning: live register analysis is disabled in this mode ";
static const char* kStrWarningCfgNotSupported = "Warning: control-flow graph generation is disabled in this mode ";
static const char* kStrWarningSkipping = "- skipping.";
static const char* kStrWarningPso1 = "Warning: no pipeline state file provided. Compilation may fail or produce inaccurate results.";
static const char* kStrWarningPso2 = " Use --pso option to provide full path to ";
static const char* kStrWarningPsoGraphics = ".gpso (graphics)";
static const char* kStrWarningPsoCompute = ".cpso (compute)";
static const char* kStrWarningPso3 = " pipeline state input file.\n";

// Info.
static const char* kStrInfoVulkanUsingCustomIcdFile = "Info: forcing the Vulkan runtime to load a custom ICD: ";
static const char* kStrInfoGeneratingVersionInfoFile = "Generating version info header in file: ";
static const char* kStrInfoPerformingLiveregAnalysisVgpr = "Performing live vgpr analysis for ";
static const char* kStrInfoPerformingLiveregAnalysisSgpr    = "Performing live sgpr analysis for ";
static const char* kStrInfoPerformingStallAnalysis1      = "Performing stall analysis for ";
static const char* kStrInfoPerformingAnalysis2 = " shader...";
static const char* kStrInfoContructingPerBlockCfg1 = "Generating per-block control-flow graph for ";
static const char* kStrInfoContructingPerBlockCfg2 = " shader...";
static const char* kStrInfoContructingPerInstructionCfg1 = "Generating per-instruction control-flow graph for ";
static const char* kStrInfoContructingPerInstructionCfg2 = " shader...";
static const char* kStrInfoValidateBinFile = "Info: Validating input code object: ";
static const char* kStrInfoDetectBinWorkflowType  = "Info: Identifying workflow type for input code object: ";
static const char* kStrInfoDetectBinTargetDevice  = "Info: extracting device information from: ";


// Build output.
static const char* kStrInfoSuccess = "succeeded.";
static const char* kStrInfoFailed = "failed.";
static const char* kStrInfoAborting = "Aborting.";
static const char* kStrInfoCompiling = "Building for ";
static const char* kStrInfoExtractingIsaForDevice = "Extracting ISA for ";
static const char* kStrInfoExtractingStats = "Extracting statistics";

// Shaders and pipeline stages.
static const char* kStrVertexStage = "vertex";
static const char* kStrTessellationControlStageName = "tessellation control";
static const char* kStrTessellationEvaluationStageName = "tessellation evaluation";
static const char* kStrGeometryStageName = "geometry";
static const char* kStrFragmentStageName = "fragment";
static const char* kStrComputeStageName = "compute";

// Default file extensions.
static const char* kStrDefaultExtensionIsa = "amdisa";
static const char* kStrDefaultExtensionAmdil = "amdil";
static const char* kStrDefaultExtensionLlvmir = "llvmir";
static const char* kStrDefaultExtensionLivereg = "livereg";
static const char* kStrDefaultExtensionLiveregSgpr  = "livereg_sgpr";
static const char* kStrDefaultExtensionImage    = "png";
static const char* kStrDefaultExtensionText = "txt";
static const char* KC_STR_DEFAULT_CFG_SUFFIX = "cfg";
static const char* kStrDefaultExtensionDot = "dot";
static const char* kStrDefaultExtensionBin = "bin";
static const char* kStrDefaultExtensionMetadata = "amdMetadata";
static const char* kStrDefaultExtensionDxasm = "dxasm";
static const char* kStrDefaultExtensionStats = "stats";
static const char* kStrDefaultExtensionCsv = "csv";

// Default file names.
static const char* kStrDefaultFilenameLivereg = "livereg";
static const char* kStrDefaultFilenameLiveregSgpr = "livereg_sgpr";
static const char* kStrDefaultFilenameIl = "il";
static const char* kStrDefaultFilenameIsa = "isa";
static const char* kStrDefaultFilenameBin = "bin";
static const char* kStrDefaultFilenamePreprocessedIsa = "preprocessed_isa";
static const char* kStrDefaultFilenameOutputBinaryFileName = "codeobj.bin";

// RGA Mode API Name Strings.
static const char* kStrRgaModeOpenclOffline    = "OpenCL";
static const char* kStrRgaModeOpengl           = "OpenGL";
static const char* kStrRgaModeVulkan           = "Vulkan";
static const char* kStrRgaModeVkOffline        = "Vulkan Offline";
static const char* kStrRgaModeDx11             = "DX11";
static const char* kStrRgaModeDx12             = "DX12";
static const char* kStrRgaModeDxr              = "DXR";
static const char* kStrRgaModeErrorUnknownMode = "Unknown mode.";

#endif // RGA_RADEONGPUANALYZERCLI_SRC_KC_CLI_STRING_CONSTANTS_H_
