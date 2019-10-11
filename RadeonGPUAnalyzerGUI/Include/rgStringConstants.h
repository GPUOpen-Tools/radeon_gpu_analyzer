#pragma once

// Application name.
static const char* STR_APP_NAME = "Radeon GPU Analyzer";

// Global configuration file name.
static const char* STR_GLOBAL_CONFIG_FILE_NAME_PREFIX = "RGAConfig";
static const char* STR_GLOBAL_CONFIG_FILE_EXTENSION = "xml";

// CLI version-info cache file name.
static const char* STR_VERSION_INFO_FILE_NAME = "VersionInfo.xml";

// Application folder name.
static const char* STR_APP_FOLDER_NAME = "RadeonGPUAnalyzer";

// Application executable name.
#ifdef __linux
static const char* STR_EXECUTABLE_NAME = "./rga";
#else
static const char* STR_EXECUTABLE_NAME = "rga.exe";
#endif

// Projects folder name.
#ifdef  __linux
static const char* STR_PROJECTS_FOLDER_NAME = "projects";
#else
static const char* STR_PROJECTS_FOLDER_NAME = "Projects";
#endif //  __linux

// String used to build the window title text.
static const char* STR_TITLE_PROJECT = "Project";

// Sub-folder of project folder for temporary files.
static const char* STR_PROJECT_SUBFOLDER_TEMP = "temp";

// Sub-folder of project folder for backup files.
static const char* STR_PROJECT_SUBFOLDER_GENERATED = "generated";

// Clone folder prefix.
static const char* STR_CLONE_FOLDER_NAME = "Clone";

// The project build artifacts output folder.
static const char* STR_OUTPUT_FOLDER_NAME = "Output";

// The filename suffix used in each session metadata file dumped for a target GPU compilation.
static const char* STR_SESSION_METADATA_FILENAME = "cliInvocation.xml";

// The name used for the resource usage analysis CSV file that's dumped when a program is built.
static const char* STR_RESOURCE_USAGE_CSV_FILENAME = "resourceUsage.csv";

// Project file extension.
static const char* STR_PROJECT_FILE_EXTENSION = ".rga";

// Default source filename for new files.
static const char* STR_DEFAULT_SOURCE_FILENAME = "src";

// Suffix for unsaved files.
static const char* STR_UNSAVED_FILE_SUFFIX = "*";

// String truncation delimeter.
static const char* STR_TRUNCATED_STRING_DELIMETER = "...";

// Default extensions for different Vulkan input files.
static const char  STR_VK_FILE_EXT_DELIMITER = ';';
static const char* STR_VK_FILE_EXT_SPIRV_TXT = "spvas";
static const char* STR_VK_FILE_EXT_SPIRV_BIN = "spv";
static const char* STR_VK_FILE_EXT_GLSL      = "vert;frag;tesc;tese;geom;comp";
static const char* STR_VK_FILE_EXT_HLSL      = "hlsl";

// *** LOG FILE MESSAGES  - START ***
static const char* STR_LOG_RGA_GUI_STARTED                               = "RGA GUI started.";
static const char* STR_LOG_FAILED_LOAD_VERSION_INFO_FILE                 = "Failed to load version info file: ";
static const char* STR_LOG_FAILED_INIT_CONFIG                            = "Initialization of Config Manager failed.";
static const char* STR_LOG_CLOSING_RGA_GUI                               = "Closing RGA GUI.";
static const char* STR_LOG_BUILDING_PROJECT_CLONE_1                      = "Building Project: ";
static const char* STR_LOG_BUILDING_PROJECT_CLONE_2                      = ", clone: ";
static const char* STR_LOG_LAUNCHING_CLI                                 = "Launching RGA CLI with command line: ";

// *** LOG FILE MESSAGES  - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** ERROR STRINGS  - START ***

static const char* STR_ERR_FATAL_PREFIX = "Fatal error: ";
static const char* STR_ERR_CANNOT_READ_CONFIG_FILE_A = "RGA is unable to read the global configuration file: ";
static const char* STR_ERR_DELETE_FILE_AND_RERUN = "Please delete the file, and re-run the application.";
static const char* STR_ERR_RGA_WILL_NOW_EXIT = "RGA will now exit.";
static const char* STR_ERR_CANNOT_RESTORE_DEFAULT_SETTINGS = "Cannot restore default settings.";
static const char* STR_ERR_CANNOT_ADD_FILE_A = "Cannot add the file: ";
static const char* STR_ERR_CANNOT_ADD_FILE_B = ", since a file with that name already exists.";
static const char* STR_ERR_CANNOT_WRITE_TO_FILE  = "Cannot write to file: ";
static const char* STR_ERR_CANNOT_RENAME_FILE_A = "Cannot rename file to ";
static const char* STR_ERR_CANNOT_RENAME_FILE_B_ALREADY_EXISTS = " since a file with that name already exists.";
static const char* STR_ERR_CANNOT_RENAME_FILE_B_ILLEGAL_FILENAME = " since the given file name is illegal.";
static const char* STR_ERR_CANNOT_RENAME_FILE_BLANK_FILENAME = "File name cannot be empty.";
static const char* STR_ERR_CANNOT_LOAD_PROJECT_FILE = "Failed to load project";
static const char* STR_ERR_CANNOT_LOAD_PROJECT_FILE_NO_VALID_SOURCES = "No valid source files exist in project.";
static const char* STR_ERR_CANNOT_LOAD_SOURCE_FILE_MSG = "Failed to find file ";
static const char* STR_ERR_CANNOT_LOAD_BUILD_OUTPUT = "Failed to load project build output.";
static const char* STR_ERR_CANNOT_LOAD_VERSION_INFO_FILE = "RGA Failed to load version the info file: ";
static const char* STR_ERR_CANNOT_LOAD_SUPPORTED_GPUS_LIST_FOR_MODE_A = "Failed to load list of supported GPUs for '";
static const char* STR_ERR_CANNOT_LOAD_SUPPORTED_GPUS_LIST_FOR_MODE_B = "' mode.";
static const char* STR_ERR_CANNOT_LOAD_DISASSEMBLY_CSV_FILE = "Failed to load disassembly output file at ";
static const char* STR_ERR_ILLEGAL_PROJECT_NAME = "Project name is not a legal file name or exceeds 50 characters: ";
static const char* STR_ERR_FAILED_TO_OPEN_FILE_BROWSER = "Failed to open the system's file explorer.";
static const char* STR_ERR_CSV_PARSING_FAILED_A = "Failed to parse ";
static const char* STR_ERR_CSV_PARSING_FAILED_B = " at line ";
static const char* STR_ERR_FAILED_TO_GET_ENTRYPOINT_LINE_NUMBERS = "Failed to query entry point names and line numbers.";
static const char* STR_ERR_INVALID_FOLDER = "Invalid folder specified.";
static const char* STR_NEW_PROJECT_ALREADY_EXISTS = "A project with that name has already been created. Please choose a different project name.";
static const char* STR_ERR_FAILED_TO_GENERATE_BUILD_COMMAND = "Failed to generate command line used for project build.";
static const char* STR_ERR_CANNOT_OPEN_LOG_FILE = "Failed to create a log file.";
static const char* STR_ERR_CANNOT_SAVE_PIPELINE_STATE_FILE = "Failed to save pipeline state file: ";
static const char* STR_ERR_CANNOT_LOAD_PIPELINE_STATE_FILE = "Failed to load pipeline state file:";
static const char* STR_ERR_CANNOT_INITIALIZE_PIPELINE_STATE_FILE = "Failed to initialize pipeline state file:";
static const char* STR_ERR_CANNOT_DETERMINE_PIPELINE_TYPE = "The pipeline type was not graphics or compute.";
static const char* STR_ERR_COULD_NOT_LOCATE_HEADER_FILE = "Could not locate header file. Check 'Additional include directories' under build settings.";
static const char* STR_ERR_COULD_NOT_OPEN_HEADER_FILE_VIEWER = "Could not launch include file viewer:";
static const char* STR_ERR_FAILED_TO_VALIDATE = "Invalid pipeline state.";
static const char* STR_ERROR_RESOLVE_ATTACHMENTS_INVALID_A = "RenderPass pSubpass[";
static const char* STR_ERROR_RESOLVE_ATTACHMENTS_INVALID_B = "]->pResolveAttachments is non-null, but array dimension does not match colorAttachmentCount.";
static const char* STR_ERROR_RESOLVE_ATTACHMENTS_INVALID_C = "Please adjust as required by the Vulkan spec.";
static const char* STR_ERROR_MULTISAMPLING_SAMPLE_MASK_INVALID_A = "The multisampling state 'rasterizationSamples' field is incompatible with the value of 'pSampleMask'.";
static const char* STR_ERROR_MULTISAMPLING_SAMPLE_MASK_INVALID_B = "Please adjust the 'rasterizationSamples' field or alter the dimension of 'pSampleMask' as required by the Vulkan spec.";

// *** ERROR STRINGS  - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** MENUS STRINGS - START ***

// File menu.
static const char* STR_ADD_EXISTING_FILE = "Add existing GLSL/SPIR-V file";
static const char* STR_CREATE_NEW_FILE = "Create template GLSL file";

// File menu item.
static const char* STR_MENU_BAR_FILE = "&File";

// File menu item entry point label text list.
static const char* STR_MENU_BAR_FILE_ITEM_ENTRYPOINT_HEADER_OPENCL = "Kernels";
static const char* STR_MENU_BAR_FILE_ITEM_ENTRYPOINT_HEADER_DEFAULT = "Entrypoints";

// The tooltip shown for a file item's entry point list item.
static const char* STR_MENU_ITEM_ENTRYPOINT_TOOLTIP_TEXT = "Click to see analysis results for ";

// Find strings.
static const char* STR_MENU_BAR_EDIT_QUICK_FIND = "Find...";
static const char* STR_MENU_BAR_EDIT_QUICK_FIND_TOOLTIP = "Find a string in the current source file (Ctrl+F).";
static const char* STR_EDIT_FIND_NEXT = "Find next.";
static const char* STR_EDIT_FIND_NEXT_TOOLTIP = "Find next search result (F3).";
static const char* STR_EDIT_FIND_PREVIOUS = "Find previous";
static const char* STR_EDIT_FIND_PREVIOUS_TOOLTIP = "Find previous search result (Shift+F3).";
static const char* STR_EDIT_FIND_CLOSE_TOOLTIP = "Close.";

// Open Project menu item.
static const char* STR_MENU_BAR_OPEN_PROJECT = "&Open existing RGA project...";
static const char* STR_MENU_BAR_OPEN_PROJECT_TOOLTIP = "Open an existing RGA project (.rga file) (Ctrl+Alt+O).";

// Edit menu item.
static const char* STR_MENU_BAR_EDIT = "&Edit";

// Go to line menu item.
static const char* STR_MENU_BAR_GO_TO_LINE = "&Go to...";
static const char* STR_MENU_BAR_GO_TO_LINE_TOOLTIP = "Go to line (Ctrl+G).";

// Build menu items.
static const char* STR_MENU_BAR_BUILD = "&Build";
static const char* STR_MENU_BAR_BUILD_PROJECT = "Build project";
static const char* STR_MENU_BAR_BUILD_PROJECT_TOOLTIP = "Build the current project (Ctrl+Shift+B).";
static const char* STR_MENU_BAR_BUILD_SETTINGS_TOOLTIP = "View the current project's build settings (F8).";
static const char* STR_MENU_BAR_BUILD_CANCEL_TOOLTIP = "Cancel the build process (Ctrl+Shift+C).";
static const char* STR_MENU_BUILD_SETTINGS = "Build settings";
static const char* STR_MENU_PIPELINE_STATE_EDITOR = "Pipeline state";
static const char* STR_MENU_BAR_PIPELINE_STATE_TOOLTIP = "View and edit the current project's pipeline state (F9).";
static const char* STR_MENU_BUILD_SETTINGS_LOWER = "build settings";
static const char* STR_MENU_CANCEL_BUILD = "Cancel build";

// Graphics menu item strings.
static const char* STR_GRAPHICS_MENU_PIPELINE_STATE_TOOLTIP = "View/Edit the current pipeline's state (F9).";
static const char* STR_GRAPHICS_MENU_SHADER_STAGE_ADD_BUTTON_TOOLTIP_A = "Add/Create a ";
static const char* STR_GRAPHICS_MENU_SHADER_STAGE_ADD_BUTTON_TOOLTIP_B = " shader file.";
static const char* STR_GRAPHICS_MENU_SHADER_STAGE_CREATE_BUTTON_TOOLTIP_A = "Create a new ";
static const char* STR_GRAPHICS_MENU_SHADER_STAGE_CREATE_BUTTON_TOOLTIP_B = " shader file.";
static const char* STR_GRAPHICS_MENU_SHADER_STAGE_CLOSE_BUTTON_TOOLTIP = "Remove this file from this stage.";
static const char* STR_GRAPHICS_MENU_SHADER_STAGE_TOOLTIP = " stage.";

// Help menu items.
static const char* STR_MENU_BAR_HELP = "&Help";
static const char* STR_MENU_BAR_HELP_ABOUT = "About";
static const char* STR_MENU_BAR_HELP_ABOUT_TOOLTIP = "Display the details of this Radeon GPU Analyzer version (Ctrl+F1).";
static const char* STR_MENU_BAR_HELP_GETTING_STARTED_GUIDE = "Getting started guide";
static const char* STR_MENU_BAR_HELP_GETTING_STARTED_GUIDE_TOOLTIP = "Open the RGA quickstart guide.";
static const char* STR_MENU_BAR_HELP_MANUAL = "Help manual";
static const char* STR_MENU_BAR_HELP_MANUAL_TOOLTIP = "Open the RGA help manual (F1).";

// Save File menu item.
static const char* STR_MENU_BAR_SAVE_FILE = "&Save file";
static const char* STR_MENU_BAR_SAVE_FILE_TOOLTIP = "Save the current file (Ctrl+S).";
static const char* STR_MENU_BAR_SAVE_SETTINGS = "&Save settings";
static const char* STR_MENU_BAR_SAVE_SETTINGS_TOOLTIP = "Save the current project build settings (Ctrl+S).";

// Back to home menu item.
static const char* STR_MENU_BAR_BACK_TO_HOME = "&Back to home page";
static const char* STR_MENU_BAR_BACK_TO_HOME_TOOLTIP = "Close the existing project and go back to the home page (Ctrl+Alt+H).";

// Exit menu item.
static const char* STR_MENU_BAR_EXIT = "&Exit";
static const char* STR_MENU_BAR_EXIT_TOOLTIP = "Exit the application (Alt+F4).";

// Settings view Restore default settings action.
static const char* STR_RESTORE_DEFAULT_SETTINGS = "Restore default settings.";

// *** MENUS STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** STATUS BAR STRINGS - START ***

static const char* STR_STATUS_BAR_FILE_MODIFIED_OUTSIDE_ENV = "The current file has been changed outside the environment.";
static const char* STR_STATUS_BAR_BUILD_STARTED = "Build started...";
static const char* STR_STATUS_BAR_BUILD_FAILED = "Build failed";
static const char* STR_STATUS_BAR_BUILD_CANCELED = "Build canceled";
static const char* STR_STATUS_BAR_BUILD_SUCCEEDED = "Build succeeded";

// *** STATUS BAR STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** FILE CONTEXT MENU STRINGS - START ***

// File item context menu's "Open containing folder" action text.
#ifdef  __linux
static const char* STR_FILE_CONTEXT_MENU_OPEN_CONTAINING_FOLDER = "Show in file browser";
#else
static const char* STR_FILE_CONTEXT_MENU_OPEN_CONTAINING_FOLDER = "Show in explorer";
#endif
static const char* STR_FILE_CONTEXT_MENU_RENAME_FILE = "Rename";
static const char* STR_FILE_CONTEXT_MENU_REMOVE_FILE = "Remove";
static const char* STR_FILE_CONTEXT_MENU_COPY_FILE_NAME = "Copy";
static const char* STR_FILE_CONTEXT_MENU_RESTORE_SPV = "Revert to original SPIR-V binary";

// Tooltip for the remove button on the file menu item.
static const char* STR_FILE_MENU_REMOVE_FILE_TOOLTIP_PREFIX = "Remove ";
static const char* STR_FILE_MENU_REMOVE_FILE_TOOLTIP_SUFFIX = " from this project.";

// Tooltip for the project title at the top of the file menu.
static const char* STR_FILE_MENU_PROJECT_TITLE_TOOLTIP_A = "<b>Project name: </b>";
static const char* STR_FILE_MENU_PROJECT_TITLE_TOOLTIP_B = " (double-click to rename).";

// *** FILE CONTEXT MENU STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** MAIN WINDOW CONTEXT MENU STRINGS - START ***

static const char* STR_MAIN_WINDOW_LOAD_PROJECT = "Load project";
static const char* STR_MAIN_WINDOW_LOADING_PROJECT = "Loading project...";
static const char* STR_MAIN_WINDOW_PROJECT_LOAD_SUCCESS = "Project loaded successfully.";

// *** MAIN WINDOW CONTEXT MENU STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** FILE DIALOG STRINGS - START ***

static const char* STR_FILE_DIALOG_CAPTION = "Open existing file";
static const char* STR_FILE_DIALOG_RGA_CAPTION = "Open existing project";
static const char* STR_FILE_DIALOG_RGA_FILTER = "RGA files (*.rga)";
static const char* STR_FILE_DIALOG_SAVE_NEW_FILE = "Save new file";
static const char* STR_FILE_DIALOG_FILTER_OPENCL = "OpenCL files (*.cl)";
static const char* STR_FILE_DIALOG_FILTER_SPIRV = "SPIR-V files (*.spv)";
static const char* STR_FILE_DIALOG_FILTER_SPIRV_BINARY_EXT = "*.spv";
static const char* STR_FILE_DIALOG_FILTER_GLSL = "GLSL files";
static const char* STR_FILE_DIALOG_FILTER_GLSL_SPIRV = "GLSL | SPIR-V files";
static const char* STR_FILE_DIALOG_FILTER_SPV_TXT = "SPIR-V text files";
static const char* STR_FILE_DIALOG_FILTER_ALL = "All files (*.*)";
static const char* STR_FILE_DIALOG_FILTER_INCLUDE_VIEWER = "Select an executable to be used for opening include files";

// The title used for the unsaved file dialog.
static const char* STR_UNSAVED_ITEMS_DIALOG_TITLE = "Unsaved changes";

// The title used for the browse missing source files dialog.
static const char* STR_BROWSE_MISSING_FILE_DIALOG_TITLE = "Failed to load project";

// Confirm that the user really wants to remove the file from the project.
static const char* STR_MENU_BAR_CONFIRM_REMOVE_FILE_DIALOG_TITLE = "Remove file";
static const char* STR_MENU_BAR_CONFIRM_REMOVE_FILE_DIALOG_WARNING = " will be removed from the project. Are you sure?";

// Check if the user wants to reload an externally modified file.
static const char* STR_RELOAD_FILE_DIALOG_TITLE = "Reload";
static const char* STR_RELOAD_FILE_DIALOG_TEXT = "This file has been modified by another project.\nDo you want to reload it?";

// Confirm that the user wants to revert to original SPIR-V binary shader file.
static const char* STR_MENU_BAR_VULKAN_CONFIRM_REVERT_TO_ORIG_SPV_A = "This will revert all SPIR-V assembly edits and restore the original SPIR-V binary from: ";
static const char* STR_MENU_BAR_VULKAN_CONFIRM_REVERT_TO_ORIG_SPV_B = "Are you sure?";

// *** FILE DIALOG STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** BUILD SETTINGS STRINGS - START ***

// General section.
static const char* STR_BUILD_SETTINGS_GENERAL_TOOLTIP = "General build settings.";
static const char* STR_BUILD_SETTINGS_CMD_LINE_TOOLTIP = "The command line options that are going to be passed to the rga command line tool based on the values on this page.";
static const char* STR_BUILD_SETTINGS_VULKAN_RUNTIME_TOOLTIP = "Settings that impact the Vulkan runtime.";

// Target GPU dialog strings.
static const char* STR_TARGET_GPU_ARCHITECTURE          = "Architecture";
static const char* STR_TARGET_GPU_PRODUCT_NAME          = "Product name";
static const char* STR_TARGET_GPU_COMPUTE_CAPABILITY    = "Compute capability";
static const char* STR_TABLE_TOOLTIP_COLUMN_ARCHITECTURE        = "Name of the GPU HW architecture.";
static const char* STR_TABLE_TOOLTIP_COLUMN_COMPUTE_CAPABILITY  = "Name of the architecture variant. From the compiler's point of view, GPUs with the same compute capability are identical.";
static const char* STR_TABLE_TOOLTIP_COLUMN_PRODUCT_NAME        = "Public name of the GPU.";

// Build settings view strings.
static const char* STR_BUILD_SETTINGS_DEFAULT_TITLE = "Default";
static const char* STR_BUILD_SETTINGS_PROJECT_TITLE = "Project";
static const char* STR_BUILD_SETTINGS_DEFAULT_TITLE_B = " build settings";
static const char* STR_BUILD_SETTINGS_GLOBAL_TITLE_END = "Build Settings";
static const char* STR_BUILD_SETTINGS_GLOBAL_TOOLTIP_A = "The default ";
static const char* STR_BUILD_SETTINGS_GLOBAL_TOOLTIP_B = " settings that will be used for any newly created project.";
static const char* STR_BUILD_SETTINGS_PROJECT_TOOLTIP_A = "The ";
static const char* STR_BUILD_SETTINGS_PROJECT_TOOLTIP_B = " settings that will be used for this project.";
static const char* STR_BUILD_SETTINGS_PREDEFINED_MACROS_TOOLTIP = "Predefined macros should be separated by ';'. If a macro definition includes a space, surround it with parentheses.";
static const char* STR_BUILD_SETTINGS_ADDITIONAL_INC_DIRECTORY_TOOLTIP = "Additional include directories should be separated by ';'. If a path includes a space, surround it with parentheses.";
static const char* STR_BUILD_SETTINGS_DEFAULT_SETTINGS_CONFIRMATION_TITLE = "Restore default values";
static const char* STR_BUILD_SETTINGS_DEFAULT_SETTINGS_CONFIRMATION = "Are you sure you want to restore default values?";
static const char* STR_BUILD_SETTINGS_TARGET_GPUS_TOOLTIP = "The target architectures to build the code for.";
static const char* STR_BUILD_SETTINGS_OPTIMIZATION_LEVEL = "The compiler optimization level that will be applied.";
static const char* STR_BUILD_SETTINGS_ALTERNATIVE_COMPILER_GLSLANG_TOOLTIP = "Settings for the front-end compiler (glslang), as well as tools such as disassembler and assembler.";
static const char* STR_BUILD_SETTINGS_ALTERNATIVE_COMPILER_TOOLTIP_GENERIC = "To use an alternative compiler, provide the following paths.\nOtherwise, the default compiler, which is bundled with RGA, will be used.";
static const char* STR_BUILD_SETTINGS_SETTINGS_CMDLINE_TOOLTIP = "The command string which will be passed to the RGA backend when it is invoked. This command is generated according to the above selected settings.";
static const char* STR_BUILD_SETTINGS_CLANG_OPTIONS_TOOLTIP = "Additional options for the clang compiler. For example, use -Weverything to enable all diagnostics.";

// Build settings error strings.
static const char* STR_ERR_INVALID_PENDING_SETTING = "The following issues must be resolved before saving build settings:";
static const char* STR_ERR_TARGET_GPUS_CANNOT_BE_EMPTY = "The Target GPUs field cannot be empty.";
static const char* STR_ERR_INVALID_GPUS_SPECIFIED = "The following are not valid target GPUs: ";

// Select include directories strings.
static const char* STR_INCLUDE_DIR_DIALOG_BROWSE_BUTTON = "&Browse...";
static const char* STR_INCLUDE_DIR_DIALOG_SELECT_INCLUDE_DIRS = "Add include directories";
static const char* STR_INCLUDE_DIR_DIALOG_DELETE_BOX_TITLE = "Delete confirmation";
static const char* STR_INCLUDE_DIR_DIALOG_DELETE_BOX_MESSAGE = "Are you sure you want to delete this entry?";
static const char* STR_INCLUDE_DIR_DIALOG_DIR_DOES_NOT_EXIST = "This directory does not exist.";
static const char* STR_INCLUDE_DIR_DIALOG_DIR_ALREADY_SELECTED = "This directory is already selected.";
static const char* STR_INCLUDE_DIR_DIALOG_SELECT_DIR_TITLE = "Select a directory";
static const char* STR_ICD_LOCATION_DIALOG_SELECT_FILE_TITLE = "Browse for Vulkan ICD file";

// Predefined macros editor dialog strings.
static const char* STR_PREPROCESSOR_DIRECTIVES_DIALOG_TITLE                         = "Add preprocessor directives";
static const char* STR_PREPROCESSOR_DIRECTIVES_DIALOG_DIRECTIVE_IS_DUPLICATE        = "Definition already defined: ";
static const char* STR_PREPROCESSOR_DIRECTIVES_DIALOG_DIRECTIVE_CONTAINS_WHITESPACE = "Definition contains whitespace: ";

// *** BUILD SETTINGS STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** GLOBAL SETTINGS STRINGS - START ***

// Global settings view strings.
static const char* STR_GLOBAL_SETTINGS_TITLE = "Global settings";
static const char* STR_GLOBAL_SETTINGS_CHECKBOX_TOOLTIP = "If checked, RGA will always use the auto-generated project name, without prompting for a rename when creating a new project.";
static const char* STR_GLOBAL_SETTINGS_LOG_FILE_LOCATION = "The directory where RGA will save log files. RGA will delete log files that are older than 3 days upon startup.";
static const char* STR_GLOBAL_SETTINGS_DISASSEMBLY_SECTION_TOOLTIP = "Settings for the GCN ISA disassembly view.";
static const char* STR_GLOBAL_SETTINGS_DISASSEMBLY_COLUMNS_TOOLTIP = "The columns which will be visible by default in the disassembly view.";
static const char* STR_GLOBAL_SETTINGS_SRC_VIEW_SECTION_TOOLTIP = "Settings for the source code editor.";
static const char* STR_GLOBAL_SETTINGS_SRC_VIEW_FONT_TYPE_TOOLTIP = "The font type which would be used in the source code editor.";
static const char* STR_GLOBAL_SETTINGS_SRC_VIEW_FONT_SIZE_TOOLTIP = "The font size which would be used in the source code editor.";
static const char* STR_GLOBAL_SETTINGS_SRC_VIEW_INCLUDE_VIEWER_DEFAULT = "<system default>";
static const char* STR_GLOBAL_SETTINGS_SRC_VIEW_INCLUDE_FILES_VIEWER_TOOLTIP = "When trying to open header files from the source code viewer, RGA would use the system's default app. To override this behavior, use this setting to provide a full path to an alternative viewer.";
static const char* STR_GLOBAL_SETTNIGS_FILE_EXT_GLSL_TOOLTIP = "Treat files with these extensions as GLSL source files. By default, RGA would treat files with unknown extensions as SPIR-V binary files.";
static const char* STR_GLOBAL_SETTNIGS_FILE_EXT_HLSL_TOOLTIP = "Treat files with these extensions as HLSL source files. By default, RGA would treat files with unknown extensions as SPIR-V binary files.";
static const char* STR_GLOBAL_SETTNIGS_FILE_EXT_SPV_TXT_TOOLTIP = "Treat files with these extensions as SPIR-V text files. By default, RGA would treat files with unknown extensions as SPIR-V binary files.";
static const char* STR_GLOBAL_SETTINGS_DEFAULT_SRC_LANG_TOOLTIP = "Use this language when creating a new source file.";
static const char* STR_GLOBAL_SETTINGS_GENERAL_SECTION_TOOLTIP = "General application-level settings.";
static const char* STR_GLOBAL_SETTINGS_DEFAULT_STARTUP_API_TOOLTIP = "If this setting is set, RGA will automatically start in the specified mode, without showing the startup dialog where the desired mode can be selected.";
static const char* STR_GLOBAL_SETTINGS_INPUT_FILES_SECTION_TOOLTIP = "Settings that configure how RGA interprets different input files.";
static const char* STR_GLOBAL_SETTINGS_SPV_EXTENSIONS_TOOLTIP = "Since RGA assumes SPIR-V binary as an input by default, use this option to specify file filters for SPIR-V binary files. When showing the Add Existing File dialog, RGA would dynamically use these file filters.";

// Default input file extensions.
static const char* STR_GLOBAL_SETTINGS_FILE_EXT_GLSL  = "vert;tesc;tese;frag;geom;comp;glsl";
static const char* STR_GLOBAL_SETTINGS_FILE_EXT_HLSL  = "hlsl;fx;hs;ds;ps";
static const char* STR_GLOBAL_SETTINGS_FILE_EXT_SPVAS = "spvas";
static const char* STR_GLOBAL_SETTINGS_FILE_EXT_SPV   = "spv";

// *** GLOBAL SETTINGS STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** PIPELINE STATE EDITOR STRINGS - START ***

// The default name for a new pipeline.
static const char* STR_DEFAULT_PIPELINE_NAME = "Pipeline";

// The caption string used when browsing to an existing pipeline state file.
static const char* STR_PIPELINE_STATE_FILE_DIALOG_CAPTION = "Open existing pipeline state file";

// The default file extension for graphics pipeline state files.
static const char* STR_DEFAULT_PIPELINE_FILE_EXTENSION_GRAPHICS      = ".gpso";
static const char* STR_DEFAULT_PIPELINE_FILE_EXTENSION_NAME_GRAPHICS = "gpso";

// The default file extension for compute pipeline state files.
static const char* STR_DEFAULT_PIPELINE_FILE_EXTENSION_COMPUTE      = ".cpso";
static const char* STR_DEFAULT_PIPELINE_FILE_EXTENSION_NAME_COMPUTE = "cpso";

// File extension delimiter.
static const char* STR_FILE_EXTENSION_DELIMITER = ".";

// The file extension filter string used for graphics pipeline state files.
static const char* STR_DEFAULT_PIPELINE_FILE_EXTENSION_FILTER_GRAPHICS = "Graphics PSO files (*.gpso)";

// The file extension filter string used for compute pipeline state files.
static const char* STR_DEFAULT_PIPELINE_FILE_EXTENSION_FILTER_COMPUTE = "Compute PSO files (*.cpso)";

// The PSO TreeView's header for the column used to display the member name.
static const char* STR_PIPELINE_STATE_EDITOR_MEMBER_NAME = "Member";

// The PSO TreeView's header for the column used to display the member value.
static const char* STR_PIPELINE_STATE_EDITOR_MEMBER_VALUE = "Value";

// Status bar strings for the PSO editor.
static const char* STR_PIPELINE_STATE_EDITOR_FILE_LOADED_SUCCESSFULLY = "Pipeline state file loaded successfully.";
static const char* STR_PIPELINE_STATE_EDITOR_FILE_FAILED_LOADING      = "Pipeline state file failed to load.";

// Confirm that the user wants to delete the element.
static const char* STR_PIPELINE_STATE_EDITOR_DELETE_ELEMENT_CONFIRMATION_TITLE = "Delete confirmation";

// Pipeline state editor information strings.
static const char* STR_PIPELINE_STATE_EDITOR_LABEL_GRAPHICS = "The Vulkan graphics state for the current pipeline.";
static const char* STR_PIPELINE_STATE_EDITOR_LABEL_COMPUTE  = "The Vulkan compute state for the current pipeline.";
static const char* STR_PIPELINE_STATE_EDITOR_LABEL_HELP      = " State can be edited manually or intercepted from a Vulkan app using the RGA layer.";

// *** PIPELINE STATE EDITOR STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** SOURCE EDITOR STRINGS - START ***

// Source editor titlebar text.
static const char* STR_SOURCE_EDITOR_TITLEBAR_CORRELATION_DISABLED_A = "<b>Correlation disabled: </b>";
static const char* STR_SOURCE_EDITOR_TITLEBAR_CORRELATION_DISABLED_B = "Source no longer matches disassembly. Build to re-enable correlation.";
static const char* STR_SOURCE_EDITOR_TITLEBAR_SPIRV_DISASM_SAVED_A = "generated. Right-click on the left menu item and select";
static const char* STR_SOURCE_EDITOR_TITLEBAR_SPIRV_DISASM_SAVED_B = "to revert all edits.";
static const char* STR_SOURCE_EDITOR_TITLEBAR_DISMISS_MESSAGE_BUTTON_TOOLTIP = "Click to dismiss the message.";

// Context menu.
static const char* STR_SOURCE_EDITOR_CONTEXT_MENU_OPEN_HEADER = "Open header file...";
static const char* STR_SOURCE_EDITOR_CONTEXT_MENU_CUT = "Cut";
static const char* STR_SOURCE_EDITOR_CONTEXT_MENU_COPY = "Copy";
static const char* STR_SOURCE_EDITOR_CONTEXT_MENU_PASTE = "Paste";
static const char* STR_SOURCE_EDITOR_CONTEXT_MENU_SELECT_ALL = "Select all";

// *** SOURCE EDITOR STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** DISASSEMBLY VIEW STRINGS - START ***

// Disassembly table column names.
static const char* STR_DISASSEMBLY_TABLE_COLUMN_ALL             = "All";
static const char* STR_DISASSEMBLY_TABLE_COLUMN_ADDRESS         = "Address";
static const char* STR_DISASSEMBLY_TABLE_COLUMN_OPCODE          = "Opcode";
static const char* STR_DISASSEMBLY_TABLE_COLUMN_OPERANDS        = "Operands";
static const char* STR_DISASSEMBLY_TABLE_COLUMN_CYCLES          = "Cycles";
static const char* STR_DISASSEMBLY_TABLE_COLUMN_FUNCTIONAL_UNIT = "Functional unit";
static const char* STR_DISASSEMBLY_TABLE_COLUMN_BINARY_ENCODING    = "Binary encoding";

// Context menu strings
static const char* STR_DISASSEMBLY_TABLE_CONTEXT_MENU_COPY = "Copy selected disassembly";
#ifdef  __linux
static const char* STR_DISASSEMBLY_TABLE_CONTEXT_MENU_OPEN_IN_FILE_BROWSER = "Show disassembly file in file browser";
#else
static const char* STR_DISASSEMBLY_TABLE_CONTEXT_MENU_OPEN_IN_FILE_BROWSER = "Show disassembly file in explorer";
#endif

// *** DISASSEMBLY VIEW STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** RESOURCE USAGE STRINGS - START ***

// Titlebar text strings.
static const char* STR_GPU_RESOURCE_USAGE               = "Resource usage";
static const char* STR_RESOURCE_USAGE_VGPRS             = "VGPRs";
static const char* STR_RESOURCE_USAGE_SGPRS             = "SGPRs";
static const char* STR_RESOURCE_USAGE_LDS               = "LDS";
static const char* STR_RESOURCE_USAGE_SPILLS            = "spills";
static const char* STR_RESOURCE_USAGE_SCRATCH           = "Scratch memory";
static const char* STR_RESOURCE_USAGE_ICACHE            = "Instruction cache";
static const char* STR_RESOURCE_USAGE_WAVES_PER_SIMD    = "Waves/SIMD";

// *** RESOURCE USAGE STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** OUTPUT WINDOW STRINGS - START ***

static const char* STR_OUTPUT_WINDOW_BUILDING_PROJECT_HEADER_A = "Building ";
static const char* STR_OUTPUT_WINDOW_BUILDING_PROJECT_HEADER_B = " project \"";
static const char* STR_OUTPUT_WINDOW_BUILDING_PROJECT_HEADER_C = "\" for ";
static const char* STR_OUTPUT_WINDOW_BUILDING_PROJECT_FAILED_TEXT = "Build Failed ";
static const char* STR_OUTPUT_WINDOW_BUILDING_PROJECT_FAILED_INVALID_CLONE = "due to invalid clone.";
static const char* STR_OUTPUT_WINDOW_CLEAR_BUTTON_TOOLTIP = "Clear the output window.";
static const char* STR_OUTPUT_WINDOW_CLEAR_BUTTON_STATUSTIP = "Clear the contents of the output window.";

// *** OUTPUT WINDOW STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** STYLESHEET STRINGS - START ***

// Stylesheet directory (relative to resource folder).
static const char* STR_STYLESHEET_RESOURCE_PATH = ":/stylesheets/";

// Stylesheet files.
static const char* STR_FILE_MENU_STYLESHEET_FILE          = "rgFileMenuStyle.qss";
static const char* STR_FILE_MENU_STYLESHEET_FILE_OPENCL   = "OpenCL/rgFileMenuStyleOpenCL.qss";
static const char* STR_FILE_MENU_STYLESHEET_FILE_VULKAN   = "Vulkan/rgFileMenuStyleVulkan.qss";
static const char* STR_APPLICATION_STYLESHEET_FILE        = "rgApplicationStyle.qss";
static const char* STR_APPLICATION_STYLESHEET_FILE_OPENCL = "OpenCL/rgApplicationStyleOpenCL.qss";
static const char* STR_APPLICATION_STYLESHEET_FILE_VULKAN = "Vulkan/rgApplicationStyleVulkan.qss";
static const char* STR_MAIN_WINDOW_STYLESHEET_FILE        = "rgMainWindowStyle.qss";
static const char* STR_MAIN_WINDOW_STYLESHEET_FILE_OPENCL = "OpenCL/rgMainWindowStyleOpenCL.qss";
static const char* STR_MAIN_WINDOW_STYLESHEET_FILE_VULKAN = "Vulkan/rgMainWindowStyleVulkan.qss";

// Disassembly view frame border color.
static const char* STR_DISASSEMBLY_FRAME_BORDER_RED_STYLESHEET = "QFrame#frame {border: 1px solid rgb(224,30,55)}";
static const char* STR_DISASSEMBLY_FRAME_BORDER_GREEN_STYLESHEET = "QFrame#frame {border: 1px solid rgb(18, 152, 0)}";
static const char* STR_DISASSEMBLY_FRAME_BORDER_BLACK_STYLESHEET = "QFrame#frame {border: 1px solid black}";

// The "current" property of file menu items.
static const char* STR_FILE_MENU_PROPERTY_CURRENT = "current";

// The "hovered" property of file menu items.
static const char* STR_FILE_MENU_PROPERTY_HOVERED = "hovered";

// The "occupied" property of file menu items.
static const char* STR_FILE_MENU_PROPERTY_OCCUPIED = "occupied";

// *** STYLESHEET STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** CONFIG FILE STRINGS - START ***

static const char* STR_SPLITTER_NAME_BUILD_OUTPUT = "BuildOutput";
static const char* STR_SPLITTER_NAME_SOURCE_DISASSEMBLY = "SourceAssembly";

// *** CONFIG FILE STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** VIEW CONTAINER NAME STRINGS - START ***

static const char* STR_RG_FILE_MENU_VIEW_CONTAINER = "FileMenuViewContainer";
static const char* STR_RG_SOURCE_VIEW_CONTAINER = "SourceViewContainer";
static const char* STR_RG_ISA_DISASSEMBLY_VIEW_CONTAINER = "DisassemblyViewContainer";
static const char* STR_RG_BUILD_OUTPUT_VIEW_CONTAINER = "BuildOutputViewContainer";

// *** VIEW CONTAINER NAME STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** GO TO LINE DIALOG STRINGS - START ***

// The title used for the go to line dialog.
static const char* STR_GO_TO_LINE_DIALOG_TITLE = "Go to line";

// *** GO TO LINE DIALOG STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** SETTINGS CONFIRMATION DIALOG STRINGS - START ***

static const char* STR_SETTINGS_CONFIRMATION_APPLICATION_SETTINGS = "Application settings";

// *** SETTINGS CONFIRMATION DIALOG STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** BUILD VIEW FONT FAMILY - START ***

#ifdef __linux
static const char* STR_BUILD_VIEW_FONT_FAMILY = "DejaVu Sans Mono";
#else
static const char* STR_BUILD_VIEW_FONT_FAMILY = "Consolas";
#endif
static const char* STR_BUILD_VIEW_SETTINGS_SCROLLAREA = "settingsScrollArea";
static const char* STR_BUILD_VIEW_SETTINGS_SCROLLAREA_STYLESHEET = "QScrollArea#settingsScrollArea { background-color: transparent; }";
static const char* STR_BUILD_VIEW_BUILD_SETTINGS_WIDGET_STYLESHEET_GREEN = "#buildSettingsWidget { border: 1px solid rgb(18, 152, 0) }";
static const char* STR_BUILD_VIEW_BUILD_SETTINGS_WIDGET_STYLESHEET_RED = "#buildSettingsWidget { border: 1px solid rgb(224, 30, 55); }";
static const char* STR_BUILD_VIEW_BUILD_SETTINGS_WIDGET_STYLESHEET_BLACK = "#buildSettingsWidget { border: 1px solid black; }";

// *** BUILD VIEW FONT FAMILY - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** STARTUP DIALOG STRINGS - START ***

static const char* STR_STARTUP_DIALOG_OPENCL_DESCRIPTION = "Compile and analyze OpenCL source code, using AMD's LLVM-based Lightning Compiler.";
static const char* STR_STARTUP_DIALOG_VULKAN_DESCRIPTION = "Compile and analyze graphics and compute Vulkan pipelines, from SPIR-V or GLSL files.";

// *** STARTUP DIALOG STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** CHECK FOR UPDATES DIALOG STRINGS - START ***

static const char* STR_UPDATES_CHECKING_FOR_UPDATES = "Checking for updates...";
static const char* STR_UPDATES_NO_UPDATES_AVAILABLE = "Checking for updates... done.\n\nNo updates available.";
static const char* STR_UPDATES_RESULTS_WINDOW_TITLE = "Available updates";

// *** CHECK FOR UPDATES  DIALOG STRINGS - END ***