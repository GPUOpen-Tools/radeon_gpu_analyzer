#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_UTILS_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_UTILS_H_

// C++.
#include <string>
#include <vector>

// Local.
#include "radeon_gpu_analyzer_gui/rg_data_types.h"

// Forward declarations.
class QApplication;
class QFont;
class QObject;
class QString;
class QStringList;
class QWidget;
class ArrowIconWidget;
class ListWidget;

class RgUtils
{
public:

    // *******************
    // Data Types - BEGIN.
    // *******************

    // Generate a unique project filename based on the time.
    static std::string GenerateDefaultProjectName();

    // Generate a clone name based on index.
    static std::string GenerateCloneName(int clone_index);

    // Retrieve the API-specific text to use in an input file's "entrypoints" list label.
    static std::string GetEntrypointsNameString(RgProjectAPI api);

    // Translates the project API enum value to a string.
    static bool ProjectAPIToString(RgProjectAPI api, std::string& str);

    // Translates the project API string to an enum.
    static RgProjectAPI ProjectAPIToEnum(const std::string& str);

    // Translates the project API enum value to the file extension string.
    static bool ProjectAPIToSourceFileExtension(RgProjectAPI api, std::string& extension);

    // Check that the incoming project clone has valid source file paths. If it doesn't, prompt the user to fix it.
    static bool IsProjectSourcePathsValid(std::shared_ptr<RgProject> project, int clone_index, QWidget* parent);

    // Check whether a file is of a valid type to be opened as a source file.
    static bool IsSourceFileTypeValid(const std::string& str);

    // *****************
    // Data Types - END.
    // *****************

    // ****************
    // Dialogs - BEGIN.
    // ****************

    // Show the user a dialog that displays each missing source file.
    static bool ShowBrowseMissingFilesDialog(std::shared_ptr<RgProject> project, const std::vector<std::string>& missing_files_list, std::map<std::string, std::string>& updated_file_paths, QWidget* parent);

    // Show an error message box with the given error message.
    static void ShowErrorMessageBox(const char* errMsg, QWidget* widget = nullptr);

    // Show a confirmation dialog with the given title, message, and Yes/No buttons.
    static bool ShowConfirmationMessageBox(const char* dialog_title, const char* confirmation_message, QWidget* parent = nullptr);

    // Open a customized, per-API, file dialog for selecting source file.
    static bool OpenFileDialog(QWidget* parent, RgProjectAPI api, std::string& selected_file_path);

    // Open a file dialog with the given caption and file filter.
    static bool OpenFileDialog(QWidget* parent, std::string& selected_file_path, const std::string& caption, const std::string& filter);

    // Open a customized per-API, file dialog for selecting multiple source files.
    static bool OpenFileDialogForMultipleFiles(QWidget* parent, RgProjectAPI api, QStringList& selected_file_paths);

    // Open the system file browser to the given folder path.
    static bool OpenFolderInFileBrowser(const std::string& folder_path);

    // Open the project selection dialog.
    static bool OpenProjectDialog(QWidget* parent, std::string& selected_file_path);

    // **************
    // Dialogs - END.
    // **************

    // ***********
    // Qt - BEGIN.
    // ***********

    // Load and apply the widget's style from the given stylesheet file.
    static bool LoadAndApplyStyle(const std::vector<std::string>& stylesheet_file_names, QWidget* widget);

    // Load and apply the application's style from the given stylesheet file.
    static bool LoadAndApplyStyle(const std::vector<std::string>& stylesheet_file_names, QApplication* application);

    // Set both tool and status tip for a widget.
    static void SetToolAndStatusTip(const std::string& tip, QWidget* widget);

    // Set the status tip.
    static void SetStatusTip(const std::string& tip, QWidget* widget);

    // Center a widget on another widget (mostly useful for centering QDialogs on the window).
    static void CenterOnWidget(QWidget* widget, QWidget* center_on);

    // Focus on the first valid ancestor widget of the given QWidget.
    static void FocusOnFirstValidAncestor(QWidget* widget);

    // Re-polish the widget from it's style (this is useful for applying style changes after modifying a dynamic property).
    static void StyleRepolish(QWidget* widget, bool repolish_children = false);

    // Set the background color for the given widget to the given color.
    static void SetBackgroundColor(QWidget* widget, const QColor& color);

    // ***********
    // Qt - END.
    // ***********

    // **************
    // Files - BEGIN.
    // **************

    // Extract file name from a file path.
    static bool ExtractFileName(const std::string& file_path, std::string& filename, bool include_file_extension = true);

    // Return the path to the directory when given a full path to a file.
    static bool ExtractFileDirectory(const std::string& file_full_path, std::string& path_to_file_directory);

    // Return the file extension when given a file path.
    static bool ExtractFileExtension(const std::string& file_path_string, std::string& file_extension);

    // Return the truncated file name to display.
    static void GetDisplayText(const std::string& filename, std::string& display_text, const int available_space, QWidget* widget, const int num_back_chars);

    // Read a text file into a string.
    static bool ReadTextFile(const std::string& file_full_path, QString& txt);

    // Rename a file on disk.
    static bool RenameFile(const std::string& old_file_path, const std::string& new_file_path);

    // Writes a text file from a string.
    static bool WriteTextFile(const std::string& target_file_path, const std::string& txt);

    // Appends a folder name to a given path.
    static bool AppendFolderToPath(const std::string& base_path, const std::string& folder_name, std::string& updated_path);

    // Appends a file name to a given path.
    static bool AppendFileNameToPath(const std::string& base_path, const std::string& filename, std::string& updated_path);

    // Append a path separator to the given base path.
    static bool AppendPathSeparator(const std::string& base_path, std::string& updated_path);

    // Makes path separators consistent (a single forward slash '/').
    static void StandardizePathSeparator(std::string& path);

    // Returns true if the given file exists.
    static bool IsFileExists(const std::string& file_full_path);

    // Returns true if the given directory exists.
    static bool IsDirExists(const std::string& dir_full_path);

    // Creates a directory at the given path, if it does not exists already.
    static bool CreateFolder(const std::string& dir_path);

    // Returns true if the given file name does not contain illegal characters.
    static bool IsValidFileName(const std::string& filename);

    // Returns true if the given file name is a valid project name.
    static bool IsValidProjectName(const std::string& filename, std::string& error_message);

    // Check if the provided file is a SPIR-V binary file.
    static bool IsSpvBinFile(const std::string& file_path);

    // Construct a name (full path) for SPIR-V disassembly file.
    // The constructed name is returned in "spvDisasmFileName".
    static bool ConstructSpvDisasmFileName(const std::string& proj_folder,
                                           const std::string& spv_file_name,
                                           std::string&       spv_disasm_file_name);

    // Detects the type of Vulkan input file.
    // Returns detected file type and corresponding Code Editor language for syntax highlighting.
    static std::pair<RgVulkanInputType, RgSrcLanguage>
    DetectInputFileType(const std::string& file_path);

    // Update CliLauncher option if spvas file is a text file.
    static bool IsSpvasTextFile(const std::string& stage_input_file, std::string& stage_abbreviation);

    // ************
    // Files - END.
    // ************

    // ****************
    // Strings - BEGIN.
    // ****************

    // String truncation type.
    enum TruncateType
    {
        kExpandNone,
        kExpandFront,
        kExpandBack
    };

    // Truncate a string with behavior based on the TruncateType.
    // Truncation will always respect numFrontChars and numBackChars as minimum lengths for the front and back of the truncated string.
    // EXPAND_NONE will simply truncate the string in the middle, with the front and back at length numFrontChars and numBackChars respectively.
    // EXPAND_FRONT will expand the front of the string to fill the available space.
    // EXPAND_BACK will expand the back of the string to fill the available space.
    static std::string TruncateString(const std::string& text, unsigned int num_front_chars, unsigned int num_back_chars,
        unsigned int available_width, const QFont& text_font, TruncateType truncate_type);

    // Splits the given string according to the given delimiter, and inserts the substrings into dst.
    static void splitString(const std::string& str, char delim, std::vector<std::string>& dst);

    // Creates a comma-separated list out of the given bools.
    static std::string BuildSemicolonSeparatedBoolList(const std::vector<bool>& bool_list);

    // Creates a comma-separated list out of the given strings.
    static std::string BuildSemicolonSeparatedStringList(const std::vector<std::string>& str_list);

    // Creates a comma-separated list out of the given ints.
    static std::string BuildSemicolonSeparatedIntList(const std::vector<int>& int_list);

    // Return true when the incoming string contains whitespace, and false if it doesn't.
    static bool IsContainsWhitespace(const std::string& text);
    // Trim the whitespace off the left side of the incoming string.
    static void LeftTrim(const std::string& text, std::string& trimmed_text);

    // Trim the whitespace off the right side of the incoming string.
    static void RightTrim(const std::string& text, std::string& trimmed_text);

    // Trim the leading and trailing whitespace off both sides of the incoming string.
    static void TrimLeadingAndTrailingWhitespace(const std::string& text, std::string& trimmed_text);

    // Replace instances of a target string with a replacement string within the given text.
    static void Replace(std::string& text, const std::string& target, const std::string& replacement);

    // Remove instances of markup substrings (like "<b>") from a given text.
    static std::string GetPlainText(const std::string& text);

    // Checks if "token" is present in the list of tokens specified by "list" string.
    // The tokens in the list must be divided by the "delim" symbol.
    // (The comparison is case-sensitive).
    // Example:
    //    list  = "red,black,white"
    //    token = "black"
    //    delim = ','
    //    result --> true.
    static bool IsInList(const std::string& list, const std::string& token, char delim);

    // Find and returns indices of test_to_find in text and return in search_result_indices.
    static void FindSearchResultIndices(const QString& text, const QString& text_to_find, std::vector<size_t>& search_result_indices);

    // **************
    // Strings - END.
    // **************

    // ****************
    // Content - BEGIN.
    // ****************

    // Generate template code for a newly created source file.
    // If entryPointPrefix is not empty, it is being used as the prefix for the name of the entry point
    // in the auto-generated code.
    static std::string GenerateTemplateCode(RgProjectAPI api_name, const std::string& entry_point_prefix);

    // Gets the name of the project according to the given API.
    static const char* GenerateProjectName(RgProjectAPI api_name);

    // Generates the title prefix for the project name label.
    static std::string GetProjectTitlePrefix(RgProjectAPI current_api);

    // Retrieve the file path to a shader file for the given pipeline stage.
    static bool GetStageShaderPath(const RgPipelineShaders& pipeline, RgPipelineStage stage, std::string& shader_path);

    // Set the input file path for a given stage within the provided pipeline.
    static bool SetStageShaderPath(RgPipelineStage stage, const std::string& shader_path, RgPipelineShaders& pipeline);

    // Retrieves a map that maps between GPU compute capabilities and their architecture.
    // For example: "gfx900" --> "Vega".
    // Returns true if one or more mapping were found.
    static bool GetComputeCapabilityToArchMapping(std::map<std::string, std::string>& device_name_mapping);

    // Retrieves the gfx123 notation for a compute capability.
    // For example: "Tonga" --> "gfx802".
    // Returns true if notation found, otherwise false.
    static bool GetGfxNotation(const std::string& code_name, std::string& gfx_notation);

    // Removes the gfx notation from the family name if it is required (i.e.
    // it is of the form <codename>/<gfx notation>. For example, for "Tonga/gfx802"
    // this would return "Tonga". If the family name is not of the relevant format,
    // the same family name would be returned.
    static std::string RemoveGfxNotation(const std::string& family_name);

    // **************
    // Content - END.
    // **************

    // *********************
    // Build output - BEGIN.
    // *********************
    static bool GetFirstValidOutputGpu(const RgBuildOutputsMap& build_outputs, std::string& first_valid_gpu, std::shared_ptr<RgCliBuildOutput>& output);

    // *******************
    // Build output - END.
    // *******************

    // *********************
    // List widgets - BEGIN.
    // *********************

    static void SetupComboList(QWidget* parent, ListWidget* &list_widget, ArrowIconWidget* &bustton, QObject* &pEventFilter, bool hide);

    // *******************
    // List widgets - END.
    // *******************

private:
    RgUtils() = default;
    ~RgUtils() = default;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_UTILS_H_
