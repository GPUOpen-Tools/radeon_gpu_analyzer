#pragma once

// C++.
#include <string>
#include <vector>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>

// Forward declarations.
class QApplication;
class QFont;
class QObject;
class QString;
class QStringList;
class QWidget;
class ArrowIconWidget;
class ListWidget;

class rgUtils
{
public:

    // *******************
    // Data Types - BEGIN.
    // *******************

    // Generate a unique project filename based on the time.
    static std::string GenerateDefaultProjectName();

    // Generate a clone name based on index.
    static std::string GenerateCloneName(int cloneIndex);

    // Retrieve the API-specific text to use in an input file's "entrypoints" list label.
    static std::string GetEntrypointsNameString(rgProjectAPI api);

    // Translates the project API enum value to a string.
    static bool ProjectAPIToString(rgProjectAPI api, std::string& str);

    // Translates the project API string to an enum.
    static rgProjectAPI ProjectAPIToEnum(const std::string& str);

    // Translates the project API enum value to the file extension string.
    static bool ProjectAPIToSourceFileExtension(rgProjectAPI api, std::string& extension);

    // Check that the incoming project clone has valid source file paths. If it doesn't, prompt the user to fix it.
    static bool IsProjectSourcePathsValid(std::shared_ptr<rgProject> pProject, int cloneIndex, QWidget* pParent);

    // Check whether a file is of a valid type to be opened as a source file.
    static bool IsSourceFileTypeValid(const std::string& str);

    // *****************
    // Data Types - END.
    // *****************


    // ****************
    // Dialogs - BEGIN.
    // ****************

    // Show the user a dialog that displays each missing source file.
    static bool ShowBrowseMissingFilesDialog(std::shared_ptr<rgProject> pProject, const std::vector<std::string>& missingFilesList, std::map<std::string, std::string>& updatedFilePaths, QWidget* pParent);

    // Show an error message box with the given error message.
    static void ShowErrorMessageBox(const char* errMsg, QWidget* pWidget = nullptr);

    // Show a confirmation dialog with the given title, message, and Yes/No buttons.
    static bool ShowConfirmationMessageBox(const char* pDialogTitle, const char* pConfirmationMessage, QWidget* pParent = nullptr);

    // Open a customized, per-API, file dialog for selecting source file.
    static bool OpenFileDialog(QWidget* pParent, rgProjectAPI api, std::string& selectedFilePath);

    // Open a file dialog with the given caption and file filter.
    static bool OpenFileDialog(QWidget* pParent, std::string& selectedFilePath, const std::string& caption, const std::string& filter);

    // Open a customized per-API, file dialog for selecting multiple source files.
    static bool OpenFileDialogForMultipleFiles(QWidget* pParent, rgProjectAPI api, QStringList& selectedFilePaths);

    // Open the system file browser to the given folder path.
    static bool OpenFolderInFileBrowser(const std::string& folderPath);

    // Open the project selection dialog.
    static bool OpenProjectDialog(QWidget* pParent, std::string& selectedFilePath);

    // **************
    // Dialogs - END.
    // **************


    // ***********
    // Qt - BEGIN.
    // ***********

    // Load and apply the widget's style from the given stylesheet file.
    static bool LoadAndApplyStyle(const std::vector<std::string>& stylesheetFileNames, QWidget* pWidget);

    // Load and apply the application's style from the given stylesheet file.
    static bool LoadAndApplyStyle(const std::vector<std::string>& stylesheetFileNames, QApplication* pApplication);

    // Set both tool and status tip for a widget.
    static void SetToolAndStatusTip(const std::string& tip, QWidget* pWidget);

    // Set the status tip.
    static void SetStatusTip(const std::string& tip, QWidget* pWidget);

    // Center a widget on another widget (mostly useful for centering QDialogs on the window).
    static void CenterOnWidget(QWidget* pWidget, QWidget* pCenterOn);

    // Focus on the first valid ancestor widget of the given QWidget.
    static void FocusOnFirstValidAncestor(QWidget* pWidget);

    // Re-polish the widget from it's style (this is useful for applying style changes after modifying a dynamic property).
    static void StyleRepolish(QWidget* pWidget, bool repolishChildren = false);

    // Set the background color for the given widget to the given color.
    static void SetBackgroundColor(QWidget* pWidget, const QColor& color);

    // ***********
    // Qt - END.
    // ***********


    // **************
    // Files - BEGIN.
    // **************

    // Extract file name from a file path.
    static bool ExtractFileName(const std::string& filePath, std::string& fileName, bool includeFileExtension = true);

    // Return the path to the directory when given a full path to a file.
    static bool ExtractFileDirectory(const std::string& fileFullPath, std::string& pathToFileDirectory);

    // Return the file extension when given a file path.
    static bool ExtractFileExtension(const std::string& filePathString, std::string& fileExtension);

    // Return the truncated file name to display.
    static void GetDisplayText(const std::string& fileName, std::string& displayText, const int availableSpace, QWidget* pWidget, const int numBackChars);

    // Read a text file into a string.
    static bool ReadTextFile(const std::string& fileFullPath, QString& txt);

    // Rename a file on disk.
    static bool RenameFile(const std::string& oldFilepath, const std::string& newFilepath);

    // Writes a text file from a string.
    static bool WriteTextFile(const std::string& targetFilePath, const std::string& txt);

    // Appends a folder name to a given path.
    static bool AppendFolderToPath(const std::string& basePath, const std::string& folderName, std::string& updatedPath);

    // Appends a file name to a given path.
    static bool AppendFileNameToPath(const std::string& basePath, const std::string& fileName, std::string& updatedPath);

    // Append a path separator to the given base path.
    static bool AppendPathSeparator(const std::string& basePath, std::string& updatedPath);

    // Makes path separators consistent (a single forward slash '/').
    static void StandardizePathSeparator(std::string& path);

    // Returns true if the given file exists.
    static bool IsFileExists(const std::string& fileFullPath);

    // Returns true if the given directory exists.
    static bool IsDirExists(const std::string& dirFullPath);

    // Creates a directory at the given path, if it does not exists already.
    static bool CreateFolder(const std::string& dirPath);

    // Returns true if the given file name does not contain illegal characters.
    static bool IsValidFileName(const std::string& fileName);

    // Check if the provided file is a SPIR-V binary file.
    static bool IsSpvBinFile(const std::string& filePath);

    // Construct a name (full path) for SPIR-V disassembly file.
    // The constructed name is returned in "spvDisasmFileName".
    static bool ConstructSpvDisasmFileName(const std::string& projFolder,
                                           const std::string& spvFileName,
                                           std::string&       spvDisasmFileName);

    // Detects the type of Vulkan input file.
    // Returns detected file type and corresponding Code Editor language for syntax highlighting.
    static std::pair<rgVulkanInputType, rgSrcLanguage>
    DetectInputFileType(const std::string& filePath);

    // ************
    // Files - END.
    // ************


    // ****************
    // Strings - BEGIN.
    // ****************

    // String truncation type.
    enum TruncateType
    {
        EXPAND_NONE,
        EXPAND_FRONT,
        EXPAND_BACK
    };

    // Truncate a string with behavior based on the TruncateType.
    // Truncation will always respect numFrontChars and numBackChars as minimum lengths for the front and back of the truncated string.
    // EXPAND_NONE will simply truncate the string in the middle, with the front and back at length numFrontChars and numBackChars respectively.
    // EXPAND_FRONT will expand the front of the string to fill the available space.
    // EXPAND_BACK will expand the back of the string to fill the available space.
    static std::string TruncateString(const std::string& text, unsigned int numFrontChars, unsigned int numBackChars,
        unsigned int availableWidth, const QFont& textFont, TruncateType truncateType);

    // Splits the given string according to the given delimiter, and inserts the substrings into dst.
    static void splitString(const std::string& str, char delim, std::vector<std::string>& dst);

    // Creates a comma-separated list out of the given bools.
    static std::string BuildSemicolonSeparatedBoolList(const std::vector<bool>& boolList);

    // Creates a comma-separated list out of the given strings.
    static std::string BuildSemicolonSeparatedStringList(const std::vector<std::string>& strList);

    // Creates a comma-separated list out of the given ints.
    static std::string BuildSemicolonSeparatedIntList(const std::vector<int>& intList);

    // Return true when the incoming string contains whitespace, and false if it doesn't.
    static bool IsContainsWhitespace(const std::string& text);
    // Trim the whitespace off the left side of the incoming string.
    static void LeftTrim(const std::string& text, std::string& trimmedText);

    // Trim the whitespace off the right side of the incoming string.
    static void RightTrim(const std::string& text, std::string& trimmedText);

    // Trim the leading and trailing whitespace off both sides of the incoming string.
    static void TrimLeadingAndTrailingWhitespace(const std::string& text, std::string& trimmedText);

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

    // **************
    // Strings - END.
    // **************

    // ****************
    // Content - BEGIN.
    // ****************

    // Generate template code for a newly created source file.
    // If entryPointPrefix is not empty, it is being used as the prefix for the name of the entry point
    // in the auto-generated code.
    static std::string GenerateTemplateCode(rgProjectAPI apiName, const std::string& entryPointPrefix);

    // Gets the name of the project according to the given API.
    static const char* GenerateProjectName(rgProjectAPI apiName);

    // Generates the title prefix for the project name label.
    static std::string GetProjectTitlePrefix(rgProjectAPI currentApi);

    // Retrieve the file path to a shader file for the given pipeline stage.
    static bool GetStageShaderPath(const rgPipelineShaders& pipeline, rgPipelineStage stage, std::string& shaderPath);

    // Set the input file path for a given stage within the provided pipeline.
    static bool SetStageShaderPath(rgPipelineStage stage, const std::string& shaderPath, rgPipelineShaders& pipeline);

    // Retrieves a map that maps between GPU compute capabilities and their architecture.
    // For example: "gfx900" --> "Vega".
    // Returns true if one or more mapping were found.
    static bool GetComputeCapabilityToArchMapping(std::map<std::string, std::string>& deviceNameMapping);

    // Retrieves the gfx123 notation for a compute capability.
    // For example: "Tonga" --> "gfx802".
    // Returns true if notation found, otherwise false.
    static bool GetGfxNotation(const std::string& codeName, std::string& gfxNotation);

    // Removes the gfx notation from the family name if it is required (i.e.
    // it is of the form <codename>/<gfx notation>. For example, for "Tonga/gfx802"
    // this would return "Tonga". If the family name is not of the relevant format,
    // the same family name would be returned.
    static std::string RemoveGfxNotation(const std::string& familyName);

    // **************
    // Content - END.
    // **************

    // *********************
    // Build output - BEGIN.
    // *********************
    static bool GetFirstValidOutputGpu(const rgBuildOutputsMap& buildOutputs, std::string& firstValidGpu, std::shared_ptr<rgCliBuildOutput>& pOutput);

    // *******************
    // Build output - END.
    // *******************

    // *********************
    // List widgets - BEGIN.
    // *********************

    static void SetupComboList(QWidget* pParent, ListWidget* &pListWidget, ArrowIconWidget* &pButton, QObject* &pEventFilter, bool hide);

    // *******************
    // List widgets - END.
    // *******************

private:
    rgUtils() = default;
    ~rgUtils() = default;
};