#pragma once

// Application name.
static const char* STR_APP_NAME = "Radeon GPU Analyzer";

// Global configuration file name.
static const char* STR_GLOBAL_CONFIG_FILE_NAME = "RGAConfig.xml";

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

// Shader source file extensions.
static const char* STR_CL_SOURCE_FILE_EXTENSION = ".cl";

// OpenCL API Name.
static const char* STR_API_NAME_OPENCL = "OpenCL";
static const char* STR_API_ABBREVIATION_OPENCL = "CL";

// Default OpenCL Build Settings string.
static const char* STR_DEFAULT_OPENCL_BUILD_SETTINGS = "Default OpenCL build settings";

// Suffix for unsaved files.
static const char* STR_UNSAVED_FILE_SUFFIX = "*";

// String truncation delimeter.
static const char* STR_TRUNCATED_STRING_DELIMETER = "...";

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
static const char* STR_ERR_CANNOT_LOAD_RESOURCE_USAGE_CSV_FILE = "Failed to load resource usage data for ";
static const char* STR_ERR_ILLEGAL_PROJECT_NAME = "Project name is not a legal file name: ";
static const char* STR_ERR_FAILED_TO_OPEN_FILE_BROWSER = "Failed to open the system's file explorer.";
static const char* STR_ERR_CSV_PARSING_FAILED_A = "Failed to parse ";
static const char* STR_ERR_CSV_PARSING_FAILED_B = " at line ";
static const char* STR_ERR_FAILED_TO_GET_ENTRYPOINT_LINE_NUMBERS = "Failed to query entrypoint names and line numbers.";
static const char* STR_ERR_INVALID_FOLDER = "Invalid folder specified.";

// *** ERROR STRINGS  - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** MENUS STRINGS - START ***

// File menu item.
static const char* STR_MENU_BAR_FILE = "&File";

// File menu item entrypoint label text list.
static const char* STR_MENU_BAR_FILE_ITEM_ENTRYPOINT_HEADER_OPENCL = "Kernels";
static const char* STR_MENU_BAR_FILE_ITEM_ENTRYPOINT_HEADER_DEFAULT = "Entrypoints";

// The tooltip shown for a file item's entrypoint list item.
static const char* STR_MENU_ITEM_ENTRYPOINT_TOOLTIP_TEXT = "Click to see analysis results for ";

// Create New File menu item.
static const char* STR_MENU_BAR_CREATE_NEW_FILE = "&Create new .cl file";
static const char* STR_MENU_BAR_CREATE_NEW_FILE_TOOLTIP = "Create a new OpenCL (.cl) source file (Ctrl+N).";

// Open Existing File menu item.
static const char* STR_MENU_BAR_OPEN_EXISTING_FILE = "&Open existing .cl file";
static const char* STR_MENU_BAR_OPEN_EXISTING_FILE_TOOLTIP = "Open an existing OpenCL (.cl) file (Ctrl+O).";

// Find strings.
static const char* STR_MENU_BAR_EDIT_QUICK_FIND = "Find...";
static const char* STR_MENU_BAR_EDIT_QUICK_FIND_TOOLTIP = "Find a string in the current source file (Ctrl+F).";

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
static const char* STR_MENU_BUILD_SETTINGS_LOWER = "build settings";
static const char* STR_MENU_CANCEL_BUILD = "Cancel build";

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
static const char* STR_MENU_BAR_SAVE_SETTINGS_TOOLTIP = "Save the current project build settings (Ctrl+S)";

// Back to home menu item.
static const char* STR_MENU_BAR_BACK_TO_HOME = "&Back to home page";
static const char* STR_MENU_BAR_BACK_TO_HOME_TOOLTIP = "Close the existing project and go back to the home page (Ctrl+Alt+H).";

// Exit menu item.
static const char* STR_MENU_BAR_EXIT = "&Exit";
static const char* STR_MENU_BAR_EXIT_TOOLTIP = "Exit the application (Alt+F4).";

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

// Tooltip for the remove button on the file menu item.
static const char* STR_FILE_MENU_REMOVE_FILE_TOOLTIP_PREFIX = "Remove ";
static const char* STR_FILE_MENU_REMOVE_FILE_TOOLTIP_SUFFIX = " from this project.";

// Tooltip for the project title at the top of the file menu.
static const char* STR_FILE_MENU_PROJECT_TITLE_TOOLTIP_A = "<b>Project name: </b>";
static const char* STR_FILE_MENU_PROJECT_TITLE_TOOLTIP_B = "\n(double-click to rename).";

// *** FILE CONTEXT MENU STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** MAIN WINDOW CONTEXT MENU STRINGS - START ***

static const char* STR_MAIN_WINDOW_LOAD_PROJECT = "Load project";
static const char* STR_MAIN_WINDOW_LOADING_PROJECT = "Loading project...";
static const char* STR_MAIN_WINDOW_PROJECT_LOAD_SUCCESS = "Project loaded successfully.";

// *** MAIN WINDOW CONTEXT MENU STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** FILE DIALOG STRINGS - START ***

static const char* STR_FILE_DIALOG_CL_CAPTION = "Open existing file";
static const char* STR_FILE_DIALOG_CL_FILTER = "OpenCL files (*.cl);; All files (*.*)";
static const char* STR_FILE_DIALOG_RGA_CAPTION = "Open existing project";
static const char* STR_FILE_DIALOG_RGA_FILTER = "RGA files (*.rga)";
static const char* STR_FILE_DIALOG_SAVE_NEW_FILE = "Save new file";

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

// *** FILE DIALOG STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** BUILD SETTINGS STRINGS - START ***

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
static const char* STR_BUILD_SETTINGS_GLOBAL_TITLE_END = "Build Settings";
static const char* STR_BUILD_SETTINGS_GLOBAL_TOOLTIP_A = "The default ";
static const char* STR_BUILD_SETTINGS_GLOBAL_TOOLTIP_B = " settings that will be used for any newly created project.";
static const char* STR_BUILD_SETTINGS_PROJECT_TOOLTIP_A = "The ";
static const char* STR_BUILD_SETTINGS_PROJECT_TOOLTIP_B = " settings that will be used for this project.";
static const char* STR_BUILD_SETTINGS_PREDEFINED_MACROS_TOOLTIP = "Predefined macros should be separated by ';'. If a macro definition includes a space, surround it with parentheses";
static const char* STR_BUILD_SETTINGS_ADDITIONAL_INC_DIRECTORY_TOOLTIP = "Additional include directories should be separated by ';'. If a path includes a space, surround it with parentheses";
static const char* STR_BUILD_SETTINGS_APPLICATION = "Application";
static const char* STR_BUILD_SETTINGS_DEFAULT_SETTINGS_CONFIRMATION_TITLE = "Restore default values";
static const char* STR_BUILD_SETTINGS_DEFAULT_SETTINGS_CONFIRMATION = "Are you sure you want to restore default values?";
static const char* STR_BUILD_SETTINGS_TARGET_GPUS_TOOLTIP = "The target architectures to build the code for";
static const char* STR_BUILD_SETTINGS_OPTIMIZATION_LEVEL = "The compiler optimization level that will be applied.";
static const char* STR_BUILD_SETTINGS_ALTERNATIVE_COMPILER_TOOLTIP = "To use an alternative compiler, provide the following paths.\nOtherwise, the default compiler, which is bundled with RGA, will be used";
static const char* STR_BUILD_SETTINGS_SETTINGS_CMDLINE_TOOLTIP = "The command string which will be passed to the RGA backend when it is invoked. This command is generated according to the above selected settings";
static const char* STR_BUILD_SETTINGS_CLANG_OPTIONS_TOOLTIP = "Additional options for the clang compiler. For example, use -Weverything to enable all diagnostics";

// Build settings error strings.
static const char* STR_ERR_INVALID_PENDING_SETTING = "The following issues must be resolved before saving build settings:";
static const char* STR_ERR_TARGET_GPUS_CANNOT_BE_EMPTY = "The Target GPUs field cannot be empty.";
static const char* STR_ERR_INVALID_GPUS_SPECIFIED = "The following are not valid target GPUs: ";

// Select include directories strings.
static const char* STR_INCLUDE_DIR_DIALOG_BROWSE_BUTTON = "Browse...";
static const char* STR_INCLUDE_DIR_DIALOG_SELECT_INCLUDE_DIRS = "Add include directories";
static const char* STR_INCLUDE_DIR_DIALOG_DELETE_BOX_TITLE = "Delete confirmation";
static const char* STR_INCLUDE_DIR_DIALOG_DELETE_BOX_MESSAGE = "Are you sure you want to delete this entry?";
static const char* STR_INCLUDE_DIR_DIALOG_DIR_DOES_NOT_EXIST = "This directory does not exist.";
static const char* STR_INCLUDE_DIR_DIALOG_DIR_ALREADY_SELECTED = "This directory is already selected.";
static const char* STR_INCLUDE_DIR_DIALOG_SELECT_DIR_TITLE = "Select a directory";

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
static const char* STR_GLOBAL_SETTINGS_DISASSEMBLY_COLUMNS = "The columns which will be visible by default in the disassembly view.";

// *** GLOBAL SETTINGS STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** SOURCE EDITOR STRINGS - START ***

// Source editor titlebar text.
static const char* STR_SOURCE_EDITOR_TITLEBAR_CORRELATION_DISABLED_A = "<b>Correlation disabled: </b>";
static const char* STR_SOURCE_EDITOR_TITLEBAR_CORRELATION_DISABLED_B = "Source no longer matches disassembly. Build to re-enable correlation.";

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
static const char* STR_FILE_MENU_STYLESHEET_FILE = "rgFileMenuStyle.qss";
static const char* STR_MAIN_WINDOW_STYLESHEET_FILE = "rgMainWindowStyle.qss";
static const char* STR_APPLICATION_STYLESHEET_FILE = "rgApplicationStyle.qss";

// *** STYLESHEET STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** APPLICATION CONTENT STRINGS - START ***

static const char* STR_NEW_FILE_TEMPLATE_CODE_OPENCL_A = "/* Generated by Radeon GPU Analyzer */\n__kernel void ";
static const char* STR_NEW_FILE_TEMPLATE_CODE_OPENCL_B = "MyKernel()\n{\n}";

// *** APPLICATION CONTENT STRINGS - END ***

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// *** CONFIG FILE STRINGS - START ***

static const char* STR_SPLITTER_NAME_BUILD_OUTPUT = "BuildOutput";
static const char* STR_SPLITTER_NAME_SOURCE_DISASSEMBLY = "SourceAssembly";

// *** CONFIG FILE STRINGS - END ***

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
