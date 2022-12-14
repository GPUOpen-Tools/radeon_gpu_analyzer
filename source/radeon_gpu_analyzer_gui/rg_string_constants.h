#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_STRING_CONSTANTS_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_STRING_CONSTANTS_H_

// Application name.
static const char* kStrAppName = "Radeon GPU Analyzer";

// Global configuration file name.
static const char* kStrGlobalConfigFileNamePrefix = "RGAConfig";
static const char* kStrGlobalConfigFileExtension  = "xml";

// CLI version-info cache file name.
static const char* kStrVersionInfoFileName = "VersionInfo.xml";

// Application folder name.
static const char* kStrAppFolderName = "RadeonGPUAnalyzer";

// Application executable name.
#ifdef __linux
static const char* kStrExecutableName = "./rga";
#else
static const char* kStrExecutableName                               = "rga.exe";
#endif

// Projects folder name.
#ifdef __linux
static const char* kStrProjectsFolderName = "projects";
#else
static const char* kStrProjectsFolderName                           = "Projects";
#endif  //  __linux

// String used to build the window title text.
static const char* kStrTitleProject = "Project";

// Sub-folder of project folder for temporary files.
static const char* kStrProjectSubfolderTemp = "temp";

// Sub-folder of project folder for backup files.
static const char* kStrProjectSubfolderGenerated = "generated";

// Clone folder prefix.
static const char* kStrCloneFolderName = "Clone";

// The project build artifacts output folder.
static const char* kStrOutputFolderName = "Output";

// The filename suffix used in each session metadata file dumped for a target GPU compilation.
static const char* kStrSessionMetadataFilename = "cliInvocation.xml";

// The name used for the resource usage analysis CSV file that's dumped when a program is built.
static const char* kStrResourceUsageCsvFilename = "resourceUsage.csv";

// Project file extension.
static const char* kStrProjectFileExtension = ".rga";

// Default source filename for new files.
static const char* kStrDefaultSourceFilename = "src";

// Suffix for unsaved files.
static const char* kStrUnsavedFileSuffix = "*";

// String truncation delimeter.
static const char* kStrTruncatedStringDelimeter = "...";

// Default extensions for different Vulkan input files.
static const char  kStrVkFileExtDelimiter = ';';
static const char* kStrVkFileExtSpirvTxt  = "spvas";
static const char* kStrVkFileExtSpirvBin  = "spv";
static const char* kStrVkFileExtGlsl      = "vert;frag;tesc;tese;geom;comp";
static const char* kStrVkFileExtHlsl      = "hlsl";

// *** LOG FILE MESSAGES  - START ***
static const char* kStrLogRgaGuiStarted             = "RGA GUI started.";
static const char* kStrLogFailedLoadVersionInfoFile = "Failed to load version info file: ";
static const char* kStrLogFailedInitConfig          = "Initialization of Config Manager failed.";
static const char* kStrLogClosingRgaGui             = "Closing RGA GUI.";
static const char* kStrLogBuildingProjectClone1     = "Building Project: ";
static const char* kStrLogBuildingProjectClone2     = ", clone: ";
static const char* kStrLogLaunchingCli              = "Launching RGA CLI with command line: ";
static const char* kStrLogExtractSettingsError      = "Error reading settings file.";

// *** LOG FILE MESSAGES  - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** ERROR STRINGS  - START ***

static const char* kStrErrFatalPrefix                         = "Fatal error: ";
static const char* kStrErrCannotReadConfigFile                = "RGA is unable to read the global configuration file: ";
static const char* kStrErrDeleteFileAndRerun                  = "Please delete the file, and re-run the application.";
static const char* kStrErrRgaWillNowExit                      = "RGA will now exit.";
static const char* kStrErrCannotRestoreDefaultSettings        = "Cannot restore default settings.";
static const char* kStrErrCannotAddFileA                      = "Cannot add the file: ";
static const char* kStrErrCannotAddFileB                      = ", since a file with that name already exists.";
static const char* kStrErrCannotWriteToFile                   = "Cannot write to file: ";
static const char* kStrErrCannotRenameFile                    = "Cannot rename file to ";
static const char* kStrErrCannotRenameFileAlreadyExists       = " since a file with that name already exists.";
static const char* kStrErrCannotRenameFileIllegalFilename     = " since the given file name is illegal.";
static const char* kStrErrCannotRenameFileBlankFilename       = "File name cannot be empty.";
static const char* kStrErrCannotLoadProjectFile               = "Failed to load project";
static const char* kStrErrCannotLoadProjectFileNoValidSources = "No valid source files exist in project.";
static const char* kStrErrCannotLoadSourceFileMsg             = "Failed to find file ";
static const char* kStrErrCannotLoadBuildOutput               = "Failed to load project build output.";
static const char* kStrErrCannotLoadVersionInfoFile           = "RGA Failed to load version the info file: ";
static const char* kStrErrCannotLoadSupportedGpusListForModeA = "Failed to load list of supported GPUs for '";
static const char* kStrErrCannotLoadSupportedGpusListForModeB = "' mode.";
static const char* kStrErrCannotLoadDisassemblyCsvFile        = "Failed to load disassembly output file. ";
static const char* kStrErrCannotLoadLiveVgprFile              = "Failed to load live VGPR output file. ";
static const char* kStrErrIllegalProjectName                  = "Project name is not a legal file name or exceeds 50 characters: ";
static const char* kStrErrIllegalStringProjectName            = "Error: project file name cannot contain the string ";
static const char* kStrErrSpacesNotAllowed                    = "Spaces are not allowed.";
static const char* kStrErrFailedToOpenFileBrowser             = "Failed to open the system's file explorer.";
static const char* kStrErrCsvParsingFailedA                   = "Failed to parse ";
static const char* kStrErrCsvParsingFailedB                   = " at line ";
static const char* kStrErrFailedToGetEntrypointLineNumbers    = "Failed to query entry point names and line numbers.";
static const char* kStrErrInvalidFolder                       = "Invalid folder specified.";
static const char* kStrNewProjectAlreadyExists                = "A project with that name has already been created. Please choose a different project name.";
static const char* kStrErrFailedToGenerateBuildCommand        = "Failed to generate command line used for project build.";
static const char* kStrErrCannotOpenLogFile                   = "Failed to create a log file.";
static const char* kStrErrCannotSavePipelineStateFile         = "Failed to save pipeline state file: ";
static const char* kStrErrCannotLoadPipelineStateFile         = "Failed to load pipeline state file:";
static const char* kStrErrCannotInitializePipelineStateFile   = "Failed to initialize pipeline state file:";
static const char* kStrErrCannotDeterminePipelineType         = "The pipeline type was not graphics or compute.";
static const char* kStrErrCannotFailedToLoadPipelineStateFile = "Failed to read pipeline state from file: file might be corrupted.";
static const char* kStrErrFailedToSavePipelineStateFile       = "Failed to save pipeline state file.";
static const char* kStrErrCouldNotLocateHeaderFile            = "Could not locate header file. Check 'Additional include directories' under build settings.";
static const char* kStrErrCouldNotOpenHeaderFileViewer        = "Could not launch include file viewer:";
static const char* kStrErrFailedToValidate                    = "Invalid pipeline state.";
static const char* kStrErrorResolveAttachmentsInvalidA        = "RenderPass pSubpass[";
static const char* kStrErrorResolveAttachmentsInvalidB        = "]->pResolveAttachments is non-null, but array dimension does not match colorAttachmentCount.";
static const char* kStrErrorResolveAttachmentsInvalidC        = "Please adjust as required by the Vulkan spec.";
static const char* kStrErrorMultisamplingSampleMaskInvalidA =
    "The multisampling state 'rasterizationSamples' field is incompatible with the value of 'pSampleMask'.";
static const char* kStrErrorMultisamplingSampleMaskInvalidB =
    "Please adjust the 'rasterizationSamples' field or alter the dimension of 'pSampleMask' as required by the Vulkan spec.";

// *** ERROR STRINGS  - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** MENUS STRINGS - START ***

// File menu.
static const char* kStrAddExistingFile = "Add existing GLSL/SPIR-V file";
static const char* kStrCreateNewFile   = "Create template GLSL file";

// File menu item.
static const char* kStrMenuBarFile = "&File";

// File menu item entry point label text list.
static const char* kStrMenuBarFileItemEntrypointHeaderOpencl  = "Kernels";
static const char* kStrMenuBarFileItemEntrypointHeaderDefault = "Entrypoints";

// The tooltip shown for a file item's entry point list item.
static const char* kStrMenuItemEntrypointTooltipText = "Click to see analysis results for ";

// Find strings.
static const char* kStrMenuBarEditQuickFind        = "Find...";
static const char* kStrMenuBarEditQuickFindTooltip = "Find a string in the current source file (Ctrl+F).";
static const char* kStrEditFindNext                = "Find next.";
static const char* kStrEditFindNextTooltip         = "Find next search result (F3).";
static const char* kStrEditFindPrevious            = "Find previous";
static const char* kStrEditFindPreviousTooltip     = "Find previous search result (Shift+F3).";
static const char* kStrEditFindCloseTooltip        = "Close.";

// Open Project menu item.
static const char* kStrMenuBarOpenProject        = "&Open existing RGA project...";
static const char* kStrMenuBarOpenProjectTooltip = "Open an existing RGA project (.rga file) (Ctrl+Alt+O).";

// Edit menu item.
static const char* kStrMenuBarEdit = "&Edit";

// Go to line menu item.
static const char* kStrMenuBarGoToLine        = "&Go to...";
static const char* kStrMenuBarGoToLineTooltip = "Go to line (Ctrl+G).";

// Show max VGPR lines menu item.
static const char* kStrMenuShowMaxVgprLines        = "Go to &next maximum VGPR pressure line";
static const char* kStrMenuShowMaxVgprLinesTooltip = "Jump to the next line with maximum VGPR pressure. (Ctrl+F4)";

// Build menu items.
static const char* kStrMenuBarBuild                = "&Build";
static const char* kStrMenuBarBuildProject         = "Build project";
static const char* kStrMenuBarBuildProjectTooltip  = "Build the current project (Ctrl+Shift+B).";
static const char* kStrMenuBarBuildSettingsTooltip = "View the current project's build settings (F8).";
static const char* kStrMenuBarBuildCancelTooltip   = "Cancel the build process (Ctrl+Shift+X).";
static const char* kStrMenuBuildSettings           = "Build settings";
static const char* kStrMenuPipelineStateEditor     = "Pipeline state";
static const char* kStrMenuBarPipelineStateTooltip = "View and edit the current project's pipeline state (F9).";
static const char* kStrMenuBuildSettingsLower      = "build settings";
static const char* kStrMenuCancelBuild             = "Cancel build";

// Graphics menu item strings.
static const char* kStrGraphicsMenuPipelineStateTooltip            = "View/Edit the current pipeline's state (F9).";
static const char* kStrGraphicsMenuShaderStageAddButtonTooltipA    = "Add/Create a ";
static const char* kStrGraphicsMenuShaderStageAddButtonTooltipB    = " shader file.";
static const char* kStrGraphicsMenuShaderStageCreateButtonTooltipA = "Create a new ";
static const char* kStrGraphicsMenuShaderStageCreateButtonTooltipB = " shader file.";
static const char* kStrGraphicsMenuShaderStageCloseButtonTooltip   = "Remove this file from this stage.";
static const char* kStrGraphicsMenuShaderStageTooltip              = " stage.";

// Help menu items.
static const char* kStrMenuBarHelp                           = "&Help";
static const char* kStrMenuBarHelpAbout                      = "About";
static const char* kStrMenuBarHelpAboutTooltip               = "Display the details of this Radeon GPU Analyzer version (Ctrl+F1).";
static const char* kStrMenuBarHelpGettingStartedGuide        = "Getting started guide";
static const char* kStrMenuBarHelpGettingStartedGuideTooltip = "Open the RGA quickstart guide.";
static const char* kStrMenuBarHelpManual                     = "Help manual";
static const char* kStrMenuBarHelpManualTooltip              = "Open the RGA help manual (F1).";

// Save File menu item.
static const char* kStrMenuBarSaveFile            = "&Save file";
static const char* kStrMenuBarSaveFileTooltip     = "Save the current file (Ctrl+S).";
static const char* kStrMenuBarSaveSettings        = "&Save settings";
static const char* kStrMenuBarSaveSettingsTooltip = "Save the current project build settings (Ctrl+S).";

// Back to home menu item.
static const char* kStrMenuBarBackToHome        = "&Back to home page";
static const char* kStrMenuBarBackToHomeTooltip = "Close the existing project and go back to the home page (Ctrl+Alt+H).";

// Exit menu item.
static const char* kStrMenuBarExit        = "&Exit";
static const char* kStrMenuBarExitTooltip = "Exit the application (Alt+F4).";

// Settings view Restore default settings action.
static const char* kStrRestoreDefaultSettings = "Restore default settings.";

// *** MENUS STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** STATUS BAR STRINGS - START ***

static const char* kStrStatusBarFileModifiedOutsideEnv = "The current file has been changed outside the environment.";
static const char* kStrStatusBarBuildStarted           = "Build started...";
static const char* kStrStatusBarBuildFailed            = "Build failed";
static const char* kStrStatusBarBuildCanceled          = "Build canceled";
static const char* kStrStatusBarBuildSucceeded         = "Build succeeded";

// *** STATUS BAR STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** FILE CONTEXT MENU STRINGS - START ***

// File item context menu's "Open containing folder" action text.
#ifdef __linux
static const char* kStrFileContextMenuOpenContainingFolder = "Show in file browser";
#else
static const char* kStrFileContextMenuOpenContainingFolder          = "Show in explorer";
#endif
static const char* kStrFileContextMenuRenameFile   = "Rename";
static const char* kStrFileContextMenuRemoveFile   = "Remove";
static const char* kStrFileContextMenuCopyFileName = "Copy";
static const char* kStrFileContextMenuRestoreSpv   = "Revert to original SPIR-V binary";

// Tooltip for the remove button on the file menu item.
static const char* kStrFileMenuRemoveFileTooltipPrefix = "Remove ";
static const char* kStrFileMenuRemoveFileTooltipSuffix = " from this project.";

// Tooltip for the project title at the top of the file menu.
static const char* kStrFileMenuProjectTitleTooltipA = "<b>Project name: </b>";
static const char* kStrFileMenuProjectTitleTooltipB = " (double-click to rename).";

// *** FILE CONTEXT MENU STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** MAIN WINDOW CONTEXT MENU STRINGS - START ***

static const char* kStrMainWindowLoadProject        = "Load project";
static const char* kStrMainWindowLoadingProject     = "Loading project...";
static const char* kStrMainWindowProjectLoadSuccess = "Project loaded successfully.";

// *** MAIN WINDOW CONTEXT MENU STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** FILE DIALOG STRINGS - START ***

static const char* kStrFileDialogCaption              = "Open existing file";
static const char* kStrFileDialogRgaCaption           = "Open existing project";
static const char* kStrFileDialogRgaFilter            = "RGA files (*.rga)";
static const char* kStrFileDialogSaveNewFile          = "Save new file";
static const char* kStrFileDialogFilterOpencl         = "OpenCL files (*.cl)";
static const char* kStrFileDialogFilterSpirv          = "SPIR-V files (*.spv)";
static const char* kStrFileDialogFilterSpirvBinaryExt = "*.spv";
static const char* kStrFileDialogFilterGlsl           = "GLSL files";
static const char* kStrFileDialogFilterGlslSpirv      = "GLSL | SPIR-V files";
static const char* kStrFileDialogFilterSpvTxt         = "SPIR-V text files";
static const char* kStrFileDialogFilterAll            = "All files (*.*)";
static const char* kStrFileDialogFilterIncludeViewer  = "Select an executable to be used for opening include files";

// The title used for the unsaved file dialog.
static const char* kStrUnsavedItemsDialogTitle = "Unsaved changes";

// The title used for the browse missing source files dialog.
static const char* kStrBrowseMissingFileDialogTitle = "Failed to load project";

// Confirm that the user really wants to remove the file from the project.
static const char* kStrMenuBarConfirmRemoveFileDialogTitle   = "Remove file";
static const char* kStrMenuBarConfirmRemoveFileDialogWarning = " will be removed from the project. Are you sure?";

// Check if the user wants to reload an externally modified file.
static const char* kStrReloadFileDialogTitle = "Reload";
static const char* kStrReloadFileDialogText  = "This file has been modified by another project.\nDo you want to reload it?";

// Confirm that the user wants to revert to original SPIR-V binary shader file.
static const char* kStrMenuBarVulkanConfirmRevertToOrigSpvA = "This will revert all SPIR-V assembly edits and restore the original SPIR-V binary from: ";
static const char* kStrMenuBarVulkanConfirmRevertToOrigSpvB = "Are you sure?";

// *** FILE DIALOG STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** BUILD SETTINGS STRINGS - START ***

// General section.
static const char* kStrBuildSettingsGeneralTooltip = "General build settings.";
static const char* kStrBuildSettingsCmdLineTooltip =
    "The command line options that are going to be passed to the rga command line tool based on the values on this page.";
static const char* kStrBuildSettingsVulkanRuntimeTooltip = "Settings that impact the Vulkan runtime.";

// Target GPU dialog strings.
static const char* kStrTargetGpuArchitecture          = "Architecture";
static const char* kStrTargetGpuProductName           = "Product name";
static const char* kStrTargetGpuComputeCapability     = "Compute capability";
static const char* kStrTableTooltipColumnArchitecture = "Name of the GPU HW architecture.";
static const char* kStrTableTooltipColumnComputeCapability =
    "Name of the architecture variant. From the compiler's point of view, GPUs with the same compute capability are identical.";
static const char* kStrTableTooltipColumnProductName = "Public name of the GPU.";

// Build settings view strings.
static const char* kStrBuildSettingsDefaultTitle    = "Default";
static const char* kStrBuildSettingsProjectTitle    = "Project";
static const char* kStrBuildSettingsDefaultTitleB   = " build settings";
static const char* kStrBuildSettingsGlobalTitleEnd  = "Build Settings";
static const char* kStrBuildSettingsGlobalTooltipA  = "The default ";
static const char* kStrBuildSettingsGlobalTooltipB  = " settings that will be used for any newly created project.";
static const char* kStrBuildSettingsProjectTooltipA = "The ";
static const char* kStrBuildSettingsProjectTooltipB = " settings that will be used for this project.";
static const char* kStrBuildSettingsPredefinedMacrosTooltip =
    "Predefined macros should be separated by ';'. If a macro definition includes a space, surround it with parentheses.";
static const char* kStrBuildSettingsAdditionalIncDirectoryTooltip =
    "Additional include directories should be separated by ';'. If a path includes a space, surround it with parentheses.";
static const char* kStrBuildSettingsDefaultSettingsConfirmationTitle = "Restore default values";
static const char* kStrBuildSettingsDefaultSettingsConfirmation      = "Are you sure you want to restore default values?";
static const char* kStrBuildSettingsTargetGpusTooltip                = "The target architectures to build the code for.";
static const char* kStrBuildSettingsOptimizationLevel                = "The compiler optimization level that will be applied.";
static const char* kStrBuildSettingsAlternativeCompilerGlslangTooltip =
    "Settings for the front-end compiler (glslang), as well as tools such as disassembler and assembler.";
static const char* kStrBuildSettingsAlternativeCompilerTooltipGeneric =
    "To use an alternative compiler, provide the following paths.\nOtherwise, the default compiler, which is bundled with RGA, will be used.";
static const char* kStrBuildSettingsSettingsCmdlineTooltip =
    "The command string which will be passed to the RGA backend when it is invoked. This command is generated according to the above selected settings.";
static const char* kStrBuildSettingsClangOptionsTooltip = "Additional options for the clang compiler. For example, use -Weverything to enable all diagnostics.";

// Build settings error strings.
static const char* kStrErrInvalidPendingSetting   = "The following issues must be resolved before saving build settings:";
static const char* kStrErrTargetGpusCannotBeEmpty = "The Target GPUs field cannot be empty.";
static const char* kStrErrInvalidGpusSpecified    = "The following are not valid target GPUs: ";

// Select include directories strings.
static const char* kStrIncludeDirDialogBrowseButton       = "&Browse...";
static const char* kStrIncludeDirDialogSelectIncludeDirs  = "Add include directories";
static const char* kStrIncludeDirDialogDeleteBoxTitle     = "Delete confirmation";
static const char* kStrIncludeDirDialogDeleteBoxMessage   = "Are you sure you want to delete this entry?";
static const char* kStrIncludeDirDialogDirDoesNotExist    = "This directory does not exist.";
static const char* kStrIncludeDirDialogDirAlreadySelected = "This directory is already selected.";
static const char* kStrIncludeDirDialogSelectDirTitle     = "Select a directory";
static const char* kStrIcdLocationDialogSelectFileTitle   = "Browse for Vulkan ICD file";

// Predefined macros editor dialog strings.
static const char* kStrPreprocessorDirectivesDialogTitle                       = "Add preprocessor directives";
static const char* kStrPreprocessorDirectivesDialogDirectiveIsDuplicate        = "Definition already defined: ";
static const char* kStrPreprocessorDirectivesDialogDirectiveContainsWhitespace = "Definition contains whitespace: ";

// Default binary output file name.
static const char* kStrBuildSettingsOutputBinaryFileName = "codeobj.bin";

// *** BUILD SETTINGS STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** GLOBAL SETTINGS STRINGS - START ***

// Global settings view strings.
static const char* kStrGlobalSettingsTitle = "Global settings";
static const char* kStrGlobalSettingsCheckboxTooltip =
    "If checked, RGA will always use the auto-generated project name, without prompting for a rename when creating a new project.";
static const char* kStrGlobalSettingsLogFileLocation =
    "The directory where RGA will save log files. RGA will delete log files that are older than 3 days upon startup.";
static const char* kStrGlobalSettingsProjectFileLocation         = "The directory where RGA will save project files.";
static const char* kStrGlobalSettingsDisassemblySectionTooltip   = "Settings for the GCN ISA disassembly view.";
static const char* kStrGlobalSettingsDisassemblyColumnsTooltip   = "The columns which will be visible by default in the disassembly view.";
static const char* kStrGlobalSettingsSrcViewSectionTooltip       = "Settings for the source code editor.";
static const char* kStrGlobalSettingsSrcViewFontTypeTooltip      = "The font type which would be used in the source code editor.";
static const char* kStrGlobalSettingsSrcViewFontSizeTooltip      = "The font size which would be used in the source code editor.";
static const char* kStrGlobalSettingsSrcViewIncludeViewerDefault = "<system default>";
static const char* kStrGlobalSettingsSrcViewIncludeFilesViewerTooltip =
    "When trying to open header files from the source code viewer, RGA would use the system's default app. To override this behavior, use this setting to "
    "provide a full path to an alternative viewer.";
static const char* kStrGlobalSettnigsFileExtGlslTooltip =
    "Treat files with these extensions as GLSL source files. By default, RGA would treat files with unknown extensions as SPIR-V binary files.";
static const char* kStrGlobalSettnigsFileExtHlslTooltip =
    "Treat files with these extensions as HLSL source files. By default, RGA would treat files with unknown extensions as SPIR-V binary files.";
static const char* kStrGlobalSettnigsFileExtSpvTxtTooltip =
    "Treat files with these extensions as SPIR-V text files. By default, RGA would treat files with unknown extensions as SPIR-V binary files.";
static const char* kStrGlobalSettingsDefaultSrcLangTooltip = "Use this language when creating a new source file.";
static const char* kStrGlobalSettingsGeneralSectionTooltip = "General application-level settings.";
static const char* kStrGlobalSettingsDefaultStartupApiTooltip =
    "If this setting is set, RGA will automatically start in the specified mode, without showing the startup dialog where the desired mode can be selected.";
static const char* kStrGlobalSettingsInputFilesSectionTooltip = "Settings that configure how RGA interprets different input files.";
static const char* kStrGlobalSettingsSpvExtensionsTooltip =
    "Since RGA assumes SPIR-V binary as an input by default, use this option to specify file filters for SPIR-V binary files. When showing the Add Existing "
    "File dialog, RGA would dynamically use these file filters.";

// Default input file extensions.
static const char* kStrGlobalSettingsFileExtGlsl  = "vert;tesc;tese;frag;geom;comp;glsl";
static const char* kStrGlobalSettingsFileExtHlsl  = "hlsl;fx;hs;ds;ps";
static const char* kStrGlobalSettingsFileExtSpvas = "spvas";
static const char* kStrGlobalSettingsFileExtSpv   = "spv";

// Default binary output file name.
static const char* kStrGlobalSettingsOutputBinaryFileName = "codeobj.bin";

// *** GLOBAL SETTINGS STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** PIPELINE STATE EDITOR  - START ***

// The default name for a new pipeline.
static const char* kStrDefaultPipelineName = "Pipeline";

// The caption string used when browsing to an existing pipeline state file.
static const char* kStrPipelineStateFileDialogCaption = "Open existing pipeline state file";

// The default file extension for graphics pipeline state files.
static const char* kStrDefaultPipelineFileExtensionGraphics     = ".gpso";
static const char* kStrDefaultPipelineFileExtensionNameGraphics = "gpso";

// The default file extension for compute pipeline state files.
static const char* kStrDefaultPipelineFileExtensionCompute     = ".cpso";
static const char* kStrDefaultPipelineFileExtensionNameCompute = "cpso";

// File extension delimiter.
static const char* kStrFileExtensionDelimiter = ".";

// The file extension filter string used for graphics pipeline state files.
static const char* kStrDefaultPipelineFileExtensionFilterGraphics = "Graphics PSO files (*.gpso)";

// The file extension filter string used for compute pipeline state files.
static const char* kStrDefaultPipelineFileExtensionFilterCompute = "Compute PSO files (*.cpso)";

// The PSO TreeView's header for the column used to display the member name.
static const char* kStrPipelineStateEditorMemberName = "Member";

// The PSO TreeView's header for the column used to display the member value.
static const char* kStrPipelineStateEditorMemberValue = "Value";

// Status bar strings for the PSO editor.
static const char* kStrPipelineStateEditorFileLoadedSuccessfully = "Pipeline state file loaded successfully.";
static const char* kStrPipelineStateEditorFileFailedLoading      = "Pipeline state file failed to load.";

// Confirm that the user wants to delete the element.
static const char* kStrPipelineStateEditorDeleteElementConfirmationTitle = "Delete confirmation";

// Pipeline state editor information strings.
static const char* kStrPipelineStateEditorLabelGraphics = "The Vulkan graphics state for the current pipeline.";
static const char* kStrPipelineStateEditorLabelCompute  = "The Vulkan compute state for the current pipeline.";
static const char* kStrPipelineStateEditorLabelHelp     = " State can be edited manually or intercepted from a Vulkan app using the RGA layer.";

// *** PIPELINE STATE EDITOR STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** SOURCE EDITOR STRINGS - START ***

// Source editor titlebar text.
static const char* kStrSourceEditorTitlebarCorrelationDisabledA        = "<b>Correlation disabled: </b>";
static const char* kStrSourceEditorTitlebarCorrelationDisabledB        = "Source no longer matches disassembly. Build to re-enable correlation.";
static const char* kStrSourceEditorTitlebarSpirvDisasmSavedA           = "generated. Right-click on the left menu item and select";
static const char* kStrSourceEditorTitlebarSpirvDisasmSavedB           = "to revert all edits.";
static const char* kStrSourceEditorTitlebarDismissMessageButtonTooltip = "Click to dismiss the message.";

// Context menu.
static const char* kStrSourceEditorContextMenuOpenHeader = "Open header file...";
static const char* kStrSourceEditorContextMenuCut        = "Cut";
static const char* kStrSourceEditorContextMenuCopy       = "Copy";
static const char* kStrSourceEditorContextMenuPaste      = "Paste";
static const char* kStrSourceEditorContextMenuSelectAll  = "Select all";

// *** SOURCE EDITOR STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** DISASSEMBLY VIEW STRINGS - START ***

// Disassembly table column names.
static const char* kStrDisassemblyTableColumnAll            = "All";
static const char* kStrDisassemblyTableColumnAddress        = "Address";
static const char* kStrDisassemblyTableColumnOpcode         = "Opcode";
static const char* kStrDisassemblyTableColumnOperands       = "Operands";
static const char* kStrDisassemblyTableColumnCycles         = "Cycles";
static const char* kStrDisassemblyTableColumnFunctionalUnit = "Functional unit";
static const char* kStrDisassemblyTableColumnBinaryEncoding = "Binary encoding";
static const char* kStrDisassemblyTableLiveVgprs            = "VGPR pressure (used:%1, allocated:%2)";
static const char* kStrDisassemblyTableLiveVgprHeaderPart   = "VGPR pressure";

// Context menu strings
static const char* kStrDisassemblyTableContextMenuCopy = "Copy selected disassembly";
#ifdef __linux
static const char* kStrDisassemblyTableContextMenuOpenInFileBrowser = "Show disassembly file in file browser";
#else
static const char* kStrDisassemblyTableContextMenuOpenInFileBrowser = "Show disassembly file in explorer";
#endif
static const char* kStrDisassemblyTableContextMenuGoToMaxVgpr = "Go to next maximum VGPR pressure line (Ctrl+F4)";

// *** DISASSEMBLY VIEW STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** RESOURCE USAGE STRINGS - START ***

// Titlebar text strings.
static const char* kStrGpuResourceUsage          = "Resource usage";
static const char* kStrResourceUsageVgprs        = "VGPRs";
static const char* kStrResourceUsageSgprs        = "SGPRs";
static const char* kStrResourceUsageLds          = "LDS";
static const char* kStrResourceUsageSpills       = "spills";
static const char* kStrResourceUsageScratch      = "Scratch memory";
static const char* kStrResourceUsageIcache       = "Instruction cache";
static const char* kStrResourceUsageWavesPerSimd = "Waves/SIMD";

// *** RESOURCE USAGE STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** OUTPUT WINDOW STRINGS - START ***

static const char* kStrOutputWindowBuildingProjectHeaderA            = "Building ";
static const char* kStrOutputWindowBuildingProjectHeaderB            = " project \"";
static const char* kStrOutputWindowBuildingProjectHeaderC            = "\" for ";
static const char* kStrOutputWindowBuildingProjectFailedText         = "Build Failed ";
static const char* kStrOutputWindowBuildingProjectFailedInvalidClone = "due to invalid clone.";
static const char* kStrOutputWindowClearButtonTooltip                = "Clear the output window.";
static const char* kStrOutputWindowClearButtonStatustip              = "Clear the contents of the output window.";

// *** OUTPUT WINDOW STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** STYLESHEET STRINGS - START ***

// Stylesheet directory (relative to resource folder).
static const char* kStrStylesheetResourcePath = ":/stylesheets/";

// Stylesheet files.
static const char* kStrFileMenuStylesheetFile          = "rg_file_menu_style.qss";
static const char* kStrFileMenuStylesheetFileOpencl    = "opencl/rg_file_menu_style_opencl.qss";
static const char* kStrFileMenuStylesheetFileVulkan    = "vulkan/rg_file_menu_style_vulkan.qss";
static const char* kStrApplicationStylesheetFile       = "rg_application_style.qss";
static const char* kStrApplicationStylesheetFileOpencl = "opencl/rg_application_style_opencl.qss";
static const char* kStrApplicationStylesheetFileVulkan = "vulkan/rg_application_style_vulkan.qss";
static const char* kStrMainWindowStylesheetFile        = "rg_main_window_style.qss";
static const char* kStrMainWindowStylesheetFileOpencl  = "opencl/rg_main_window_style_opencl.qss";
static const char* kStrMainWindowStylesheetFileVulkan  = "vulkan/rg_main_window_style_vulkan.qss";

// Disassembly view frame border color.
static const char* kStrDisassemblyFrameBorderRedStylesheet   = "QFrame#frame {border: 1px solid rgb(224,30,55)}";
static const char* kStrDisassemblyFrameBorderGreenStylesheet = "QFrame#frame {border: 1px solid rgb(18, 152, 0)}";
static const char* kStrDisassemblyFrameBorderBlackStylesheet = "QFrame#frame {border: 1px solid black}";

// Disassembly view frame selected.
static const char* kStrDisassemblyFrameSelected = "selected";

// The "current" property of file menu items.
static const char* kStrFileMenuPropertyCurrent = "current";

// The "hovered" property of file menu items.
static const char* kStrFileMenuPropertyHovered = "hovered";

// The "occupied" property of file menu items.
static const char* kStrFileMenuPropertyOccupied = "occupied";

// *** STYLESHEET STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** CONFIG FILE STRINGS - START ***

static const char* kStrSplitterNameBuildOutput       = "BuildOutput";
static const char* kStrSplitterNameSourceDisassembly = "SourceAssembly";

// *** CONFIG FILE STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** VIEW CONTAINER NAME STRINGS - START ***

static const char* kStrRgFileMenuViewContainer       = "FileMenuViewContainer";
static const char* kStrRgSourceViewContainer         = "SourceViewContainer";
static const char* kStrRgIsaDisassemblyViewContainer = "DisassemblyViewContainer";
static const char* kStrRgBuildOutputViewContainer    = "BuildOutputViewContainer";

// *** VIEW CONTAINER NAME STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** GO TO LINE DIALOG STRINGS - START ***

// The title used for the go to line dialog.
static const char* kStrGoToLineDialogTitle = "Go to line";

// *** GO TO LINE DIALOG STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** SETTINGS CONFIRMATION DIALOG STRINGS - START ***

static const char* kStrSettingsConfirmationApplicationSettings = "Application settings";

// *** SETTINGS CONFIRMATION DIALOG STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** BUILD VIEW FONT FAMILY - START ***

#ifdef __linux
static const char* kStrBuildViewFontFamily = "DejaVu Sans Mono";
#else
static const char* kStrBuildViewFontFamily                          = "Consolas";
#endif
static const char* kStrBuildViewSettingsScrollarea                 = "settingsScrollArea";
static const char* kStrBuildViewSettingsScrollareaStylesheet       = "QScrollArea#settingsScrollArea { background-color: transparent; }";
static const char* kStrBuildViewBuildSettingsWidgetStylesheetGreen = "#buildSettingsWidget { border: 1px solid rgb(18, 152, 0) }";
static const char* kStrBuildViewBuildSettingsWidgetStylesheetRed   = "#buildSettingsWidget { border: 1px solid rgb(224, 30, 55); }";
static const char* kStrBuildViewBuildSettingsWidgetStylesheetBlack = "#buildSettingsWidget { border: 1px solid black; }";

// *** BUILD VIEW FONT FAMILY - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** STARTUP DIALOG STRINGS - START ***

static const char* kStrStartupDialogOpenclDescription = "Compile and analyze OpenCL source code, using AMD's LLVM-based Lightning Compiler.";
static const char* kStrStartupDialogVulkanDescription = "Compile and analyze graphics and compute Vulkan pipelines, from SPIR-V or GLSL files.";

// *** STARTUP DIALOG STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** CHECK FOR UPDATES DIALOG STRINGS - START ***

static const char* kStrUpdatesCheckingForUpdates = "Checking for updates...";
static const char* kStrUpdatesNoUpdatesAvailable = "Checking for updates... done.\n\nNo updates available.";
static const char* kStrUpdatesResultsWindowTitle = "Available updates";

// *** CHECK FOR UPDATES  DIALOG STRINGS - END ***
#endif  // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_STRING_CONSTANTS_H_
