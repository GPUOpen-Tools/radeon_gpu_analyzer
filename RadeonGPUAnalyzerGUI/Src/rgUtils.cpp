// C++.
#include <cassert>
#include <fstream>
#include <iterator>
#include <locale>
#include <memory>
#include <sstream>

// Qt.
#include <QApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFile>
#include <QFont>
#include <QMessageBox>
#include <QString>
#include <QTextStream>
#include <QWidget>

// Infra.
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4309)
#endif
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTBaseTools/Include/gtString.h>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include <QtCommon/CustomWidgets/ArrowIconWidget.h>
#include <QtCommon/CustomWidgets/ListWidget.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBrowseMissingFileDialog.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgHideListWidgetEventFilter.h>
#include <RadeonGPUAnalyzerGUI/Include/rgConfigManager.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesVulkan.h>

// Static constants

static const uint32_t  SPV_BINARY_MAGIC_NUMBER = 0x07230203;


// *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - START ***

static bool OpenFileDialogHelper(QWidget* pParent, const QString& caption, const std::string& initialFolder,
    const QString& filter, std::string& selectedFilePath)
{
    bool ret = false;
    selectedFilePath.clear();

    QString filename = QFileDialog::getOpenFileName(pParent, caption, initialFolder.c_str(), filter);

    if (!filename.isNull())
    {
        selectedFilePath = filename.toStdString();
        ret = !selectedFilePath.empty();
    }

    return ret;
}

static bool OpenMultipleFileDialogHelper(QWidget* pParent, const QString& caption,
    const QString& filter, QStringList& selectedFilePaths)
{
    bool ret = false;
    selectedFilePaths.clear();

    QStringList filenames = QFileDialog::getOpenFileNames(pParent, caption, rgConfigManager::Instance().GetLastSelectedFolder().c_str(), filter);

    if (!filenames.isEmpty())
    {
        selectedFilePaths = filenames;
        ret = !selectedFilePaths.empty();
    }

    return ret;
}

// Build the file filter for Open File dialog in Vulkan mode.
static QString ConstructVulkanOpenFileFilter()
{
    QString filter;
    auto settings = rgConfigManager::Instance().GetGlobalConfig();
    assert(settings != nullptr);
    if (settings != nullptr)
    {
        // Convert the extensions stored in the Global Settings to the Qt file filter format.
        QStringList glslExts   = QString(settings->m_inputFileExtGlsl.c_str()).split(';');
        QStringList spvTxtExts = QString(settings->m_inputFileExtSpvTxt.c_str()).split(';');
        QStringList spvBinExts = QString(settings->m_inputFileExtSpvBin.c_str()).split(';');

        QString glslExtList, spvTxtExtList, spvBinExtList;

        for (QString& ext : glslExts)   { glslExtList += ("*." + ext + " "); }
        for (QString& ext : spvTxtExts) { spvTxtExtList += ("*." + ext + " "); }
        for (QString& ext : spvBinExts) { spvBinExtList += ("*." + ext + " "); }

        glslExtList.chop(1);
        spvTxtExtList.chop(1);
        spvBinExtList.chop(1);

        filter += (QString(STR_FILE_DIALOG_FILTER_GLSL_SPIRV) + " (" + glslExtList + " " + spvTxtExtList + " " +
            STR_FILE_DIALOG_FILTER_SPIRV_BINARY_EXT + " " + ");;");
        filter += (QString(STR_FILE_DIALOG_FILTER_SPIRV) + ";;");
        filter += (QString(STR_FILE_DIALOG_FILTER_GLSL) + " (" + glslExtList + ");;");
        filter += (QString(STR_FILE_DIALOG_FILTER_SPV_TXT) + " (" + spvTxtExtList + ");;");
        filter += (QString(STR_FILE_DIALOG_FILTER_SPIRV_BINARY_EXT) + " (" + spvBinExtList + ");;");

        filter += STR_FILE_DIALOG_FILTER_ALL;
    }

    return filter;
}

// *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - END ***

void rgUtils::splitString(const std::string& str, char delim, std::vector<std::string>& dst)
{
    std::stringstream ss;
    ss.str(str);
    std::string substr;
    while (std::getline(ss, substr, delim))
    {
        dst.push_back(substr);
    }
}

std::string rgUtils::BuildSemicolonSeparatedBoolList(const std::vector<bool>& boolList)
{
    std::stringstream stream;
    for (size_t i = 0; i < boolList.size(); i++)
    {
        stream << boolList[i] ? 1 : 0;

        // If this is not the last item, add the delimiter.
        if (i < (boolList.size() - 1))
        {
            stream << rgConfigManager::RGA_LIST_DELIMITER;
        }
    }
    return stream.str();
}

std::string rgUtils::BuildSemicolonSeparatedStringList(const std::vector<std::string>& strList)
{
    std::stringstream stream;
    for (size_t i = 0; i < strList.size(); i++)
    {
        stream << strList[i];

        // If this is not the last item, add the delimiter.
        if (i < (strList.size() - 1))
        {
            stream << rgConfigManager::RGA_LIST_DELIMITER;
        }
    }
    return stream.str();
}

std::string rgUtils::BuildSemicolonSeparatedIntList(const std::vector<int>& intList)
{
    std::stringstream stream;
    for (size_t i = 0; i < intList.size(); i++)
    {
        stream << intList[i];

        // If this is not the last item, add the delimiter.
        if (i < (intList.size() - 1))
        {
            stream << rgConfigManager::RGA_LIST_DELIMITER;
        }
    }
    return stream.str();
}

bool rgUtils::IsContainsWhitespace(const std::string& text)
{
    bool isContainsWhitespace = false;

    // Step through each character in the string.
    for (size_t characterIndex = 0; characterIndex < text.length(); ++characterIndex)
    {
        // Check if the current character is whitespace.
        if (isspace(text[characterIndex]))
        {
            isContainsWhitespace = true;
            break;
        }
    }

    return isContainsWhitespace;
}

void rgUtils::LeftTrim(const std::string& text, std::string& trimmedText)
{
    trimmedText = text;
    auto spaceIter = std::find_if(trimmedText.begin(), trimmedText.end(), [](char ch) { return !std::isspace<char>(ch, std::locale::classic()); });
    trimmedText.erase(trimmedText.begin(), spaceIter);
}

void rgUtils::RightTrim(const std::string& text, std::string& trimmedText)
{
    trimmedText = text;
    auto spaceIter = std::find_if(trimmedText.rbegin(), trimmedText.rend(), [](char ch) { return !std::isspace<char>(ch, std::locale::classic()); });
    trimmedText.erase(spaceIter.base(), trimmedText.end());
}

void rgUtils::TrimLeadingAndTrailingWhitespace(const std::string& text, std::string& trimmedText)
{
    // Trim the whitespace off the left and right sides of the incoming text.
    std::string leftTrimmed;
    LeftTrim(text, leftTrimmed);
    RightTrim(leftTrimmed, trimmedText);
}

void rgUtils::Replace(std::string& text, const std::string& target, const std::string& replacement)
{
    if (!target.empty())
    {
        // Search for instances of the given target text in the source string.
        size_t startPosition = 0;
        while ((startPosition = text.find(target, startPosition)) != std::string::npos)
        {
            // Replace the target text in the string with the replacement text.
            text.replace(startPosition, target.length(), replacement);

            // Update the start position for the next search.
            startPosition += replacement.length();
        }
    }
}

std::string rgUtils::GenerateTemplateCode(rgProjectAPI apiName, const std::string& entryPointPrefix)
{
    std::stringstream strBuilder;
    switch (apiName)
    {
    case rgProjectAPI::OpenCL:
    {
        strBuilder << STR_NEW_FILE_TEMPLATE_CODE_OPENCL_A;
        if (!entryPointPrefix.empty())
        {
            strBuilder << entryPointPrefix << "_";
        }
        strBuilder << STR_NEW_FILE_TEMPLATE_CODE_OPENCL_B;
        break;
    }
    case rgProjectAPI::Unknown:
    default:
        // We shouldn't get here.
        assert(false);
        break;
    }
    return strBuilder.str();
}

std::string rgUtils::GenerateDefaultProjectName()
{
    rgProjectAPI currentAPI = rgConfigManager::Instance().GetCurrentAPI();

    // Generate a timestamp to append to the base filename.
    QDateTime rightNow = QDateTime::currentDateTime();
    QString localTime = rightNow.toString("yyMMdd-HHmmss");
    std::stringstream projectName(localTime.toStdString());
    return projectName.str();
}

const char* rgUtils::GenerateProjectName(rgProjectAPI apiName)
{
    static const char* STR_FILE_MENU_PROJECT_NAME = "Project";
    const char* pRet = nullptr;
    switch (apiName)
    {
    case rgProjectAPI::OpenCL:
    case rgProjectAPI::Vulkan:
        pRet = STR_FILE_MENU_PROJECT_NAME;
        break;
    default:
        // We shouldn't get here.
        assert(false);
        break;
    }
    return pRet;
}

std::string rgUtils::GetProjectTitlePrefix(rgProjectAPI currentApi)
{
    std::stringstream str;
    str << "<b>" << rgUtils::GenerateProjectName(currentApi) << " name: </b>";
    return str.str();
}

bool rgUtils::GetStageShaderPath(const rgPipelineShaders& pipeline, rgPipelineStage stage, std::string& shaderPath)
{
    bool res = false;

    size_t stageIndex = static_cast<size_t>(stage);
    const auto& stageInputFilePath = pipeline.m_shaderStages[stageIndex];
    if (!stageInputFilePath.empty())
    {
        // Extract the shader's file path.
        shaderPath = stageInputFilePath;
        res = true;
    }

    return res;
}

bool rgUtils::SetStageShaderPath(rgPipelineStage stage, const std::string& shaderPath, rgPipelineShaders& pipeline)
{
    bool res = false;

    assert(!shaderPath.empty());
    if (!shaderPath.empty())
    {
        // Set the shader's file path within the pipeline.
        size_t stageIndex = static_cast<size_t>(stage);
        pipeline.m_shaderStages[stageIndex] = shaderPath;
        res = true;
    }

    return res;
}

bool rgUtils::GetComputeCapabilityToArchMapping(std::map<std::string, std::string>& deviceNameMapping)
{
    deviceNameMapping.clear();
    bool ret = false;

    // Get the current app mode.
    const std::string& currentMode = rgConfigManager::Instance().GetCurrentModeString();

    // Get the version info.
    std::shared_ptr<rgCliVersionInfo> pVersionInfo = rgConfigManager::Instance().GetVersionInfo();

    // Find the architecture node for our current mode.
    auto currentModeArchitecturesIter = pVersionInfo->m_gpuArchitectures.find(currentMode);
    bool isModeFound = (currentModeArchitecturesIter != pVersionInfo->m_gpuArchitectures.end());
    assert(isModeFound);
    if (isModeFound)
    {
        // Step through each GPU hardware architecture.
        std::vector<rgGpuArchitecture> architectures = currentModeArchitecturesIter->second;
        for (const rgGpuArchitecture& hardwareArchitecture : architectures)
        {
            const std::string& currentArchitecture = hardwareArchitecture.m_architectureName;

            // Determine how many families are found within the architecture.
            std::vector<rgGpuFamily> gpuFamilies = hardwareArchitecture.m_gpuFamilies;
            int numFamiliesInArchitecture = static_cast<int>(gpuFamilies.size());

            // Step through each family within the architecture.
            for (int familyIndex = 0; familyIndex < numFamiliesInArchitecture; familyIndex++)
            {
                // Create a copy of the family info and sort by product name.
                rgGpuFamily currentFamily = gpuFamilies[familyIndex];
                deviceNameMapping[currentFamily.m_familyName] = currentArchitecture;
            }
        }
    }

    ret = !deviceNameMapping.empty();
    return ret;
}

bool rgUtils::GetGfxNotation(const std::string& codeName, std::string& gfxNotation)
{
    bool ret = false;

    // Mapping between codename and gfx compute capability notation.
    static const std::map<std::string, std::string> gfxNotationMapping
    {
        std::make_pair<std::string, std::string>("Tahiti",              "gfx600"),
        std::make_pair<std::string, std::string>("Hainan",              "gfx601"),
        std::make_pair<std::string, std::string>("Oland",               "gfx601"),
        std::make_pair<std::string, std::string>("Capeverde",           "gfx601"),
        std::make_pair<std::string, std::string>("Pitcairn",            "gfx601"),
        std::make_pair<std::string, std::string>("Kaveri",              "gfx700"),
        std::make_pair<std::string, std::string>("Spectre",             "gfx700"),
        std::make_pair<std::string, std::string>("Spooky",              "gfx700"),
        std::make_pair<std::string, std::string>("Hawaii",              "gfx701"),
        std::make_pair<std::string, std::string>("Kabini",              "gfx703"),
        std::make_pair<std::string, std::string>("Kalindi",             "gfx703"),
        std::make_pair<std::string, std::string>("Godavari",            "gfx703"),
        std::make_pair<std::string, std::string>("Mullins",             "gfx703"),
        std::make_pair<std::string, std::string>("Bonaire",             "gfx704"),
        std::make_pair<std::string, std::string>("Carrizo",             "gfx801"),
        std::make_pair<std::string, std::string>("Bristol Ridge",       "gfx801"),
        std::make_pair<std::string, std::string>("Iceland",             "gfx802"),
        std::make_pair<std::string, std::string>("Tonga",               "gfx802"),
        std::make_pair<std::string, std::string>("Fiji",                "gfx803"),
        std::make_pair<std::string, std::string>("Ellesmere",           "gfx803"),
        std::make_pair<std::string, std::string>("Baffin",              "gfx803"),
        std::make_pair<std::string, std::string>("Lexa",                "gfx803"),
        std::make_pair<std::string, std::string>("Stoney",              "gfx810"),
    };

    auto iter = gfxNotationMapping.find(codeName);
    if (iter != gfxNotationMapping.end())
    {
        gfxNotation = iter->second;
        ret = true;
    }

    return ret;
}

std::string rgUtils::RemoveGfxNotation(const std::string& familyName)
{
    std::string fixedGroupName;
    size_t bracketPos = familyName.find("/");
    if (bracketPos != std::string::npos)
    {
        fixedGroupName = familyName.substr(bracketPos + 1);
    }
    else
    {
        fixedGroupName = familyName;
    }
    return fixedGroupName;
}

bool rgUtils::GetFirstValidOutputGpu(const rgBuildOutputsMap& buildOutputs, std::string& firstValidGpu, std::shared_ptr<rgCliBuildOutput>& pOutput)
{
    bool ret = false;

    // Find the first target ASIC that appears to have valid compilation results.
    auto asicOutputsIter = buildOutputs.begin();
    for (; asicOutputsIter != buildOutputs.end(); ++asicOutputsIter)
    {
        if (asicOutputsIter->second != nullptr)
        {
            // The current ASIC appears to have valid output, so return the output's info.
            firstValidGpu = asicOutputsIter->first;
            pOutput = asicOutputsIter->second;
            ret = true;
            break;
        }
    }

    return ret;
}

void rgUtils::SetupComboList(QWidget* pParent, ListWidget* &pListWidget, ArrowIconWidget* &pButton, QObject* &pEventFilter, bool hide)
{
    assert(pButton != nullptr);
    if (pButton != nullptr)
    {
        // Create list widget for the combo box.
        pListWidget = new ListWidget(pParent, pButton, hide);

        assert(pListWidget != nullptr);
        if (pListWidget != nullptr)
        {
            pListWidget->hide();

            // Also disable scrollbars on this list widget.
            pListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            pListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

            // Install a custom event filter on this list widget
            // so it can be hidden when something else is clicked on.
            pEventFilter = new rgHideListWidgetEventFilter(pListWidget, pButton);
            pListWidget->installEventFilter(pEventFilter);
            qApp->installEventFilter(pEventFilter);

            // Start out the combo box with the first entry.
            pListWidget->setCurrentRow(0);
        }
    }
}

std::string rgUtils::GenerateCloneName(int cloneIndex)
{
    // Generate a name based on the default clone name and the incoming index.
    std::stringstream cloneName;
    cloneName << STR_CLONE_FOLDER_NAME;
    cloneName << cloneIndex;
    return cloneName.str();
}

std::string rgUtils::GetEntrypointsNameString(rgProjectAPI api)
{
    std::string resultString;

    switch (api)
    {
    case rgProjectAPI::OpenCL:
    {
        resultString = STR_MENU_BAR_FILE_ITEM_ENTRYPOINT_HEADER_OPENCL;
    }
    break;
    case rgProjectAPI::Unknown:
    default:
    {
        // All other cases use API-agnostic label text.
        resultString = STR_MENU_BAR_FILE_ITEM_ENTRYPOINT_HEADER_DEFAULT;
    }
    break;
    }

    return resultString;
}

bool rgUtils::ProjectAPIToString(rgProjectAPI api, std::string& str)
{
    bool ret = true;
    switch (api)
    {
    case rgProjectAPI::OpenCL:
        str = STR_API_NAME_OPENCL;
        break;
    case rgProjectAPI::Vulkan:
        str = STR_API_NAME_VULKAN;
        break;
    case rgProjectAPI::Unknown:
    default:
        // We shouldn't get here.
        ret = false;
        assert(ret);
        break;
    }
    return ret;
}

rgProjectAPI rgUtils::ProjectAPIToEnum(const std::string& str)
{
    rgProjectAPI projectAPI = rgProjectAPI::Unknown;

    if (str.compare(STR_API_NAME_OPENCL) == 0)
    {
        projectAPI = rgProjectAPI::OpenCL;
    }
    else if (str.compare(STR_API_NAME_VULKAN) == 0)
    {
        projectAPI = rgProjectAPI::Vulkan;
    }

    return projectAPI;
}

bool rgUtils::ProjectAPIToSourceFileExtension(rgProjectAPI api, std::string& extension)
{
    bool ret = true;
    switch (api)
    {
    case rgProjectAPI::OpenCL:
        extension = STR_SOURCE_FILE_EXTENSION_CL;
        break;
    case rgProjectAPI::Vulkan:
        extension = STR_SOURCE_FILE_EXTENSION_VULKAN_GLSL;
        break;
    case rgProjectAPI::Unknown:
    default:
        // We shouldn't get here.
        ret = false;
        assert(ret);
        break;
    }

    return ret;
}

bool rgUtils::IsProjectSourcePathsValid(std::shared_ptr<rgProject> pProject, int cloneIndex, QWidget* pParent)
{
    bool ret = false;

    rgConfigManager& configManager = rgConfigManager::Instance();

    // Extract the source file paths from the clone.
    std::vector<std::string> sourceFilepaths;
    configManager.GetProjectSourceFilePaths(pProject, cloneIndex, sourceFilepaths);

    // Create a list for files that the project wants to load, but couldn't.
    std::vector<std::string> missingFilesList;

    // Add all the project's source files into the rgBuildView.
    for (const std::string& filePath : sourceFilepaths)
    {
        // Check that the file exists before attempting to load it.
        bool isFileExists = rgUtils::IsFileExists(filePath);
        if (!isFileExists)
        {
            missingFilesList.push_back(filePath);
        }
    }

    // If the list of missing files is non-empty, ask the user where the files were moved to.
    if (!missingFilesList.empty())
    {
        // Present the user with a dialog that allows them to update the paths for any missing source files.
        std::map<std::string, std::string> updatedFilePaths;
        rgUtils::ShowBrowseMissingFilesDialog(pProject, missingFilesList, updatedFilePaths, pParent);
    }
    else
    {
        // The list of missing files is empty. All paths are valid.
        ret = true;
    }

    return ret;
}

bool rgUtils::IsSourceFileTypeValid(const std::string& str)
{
    bool ret = false;

    // Convert to QString for convenience (QString has "endsWith" functionality).
    QString qtString = str.c_str();

    // Check file headers.
    if (qtString.endsWith(STR_SOURCE_FILE_EXTENSION_CL) || qtString.endsWith(STR_PROJECT_FILE_EXTENSION))
    {
        ret = true;
    }

    return ret;
}

bool rgUtils::ShowBrowseMissingFilesDialog(std::shared_ptr<rgProject> pProject, const std::vector<std::string>& missingFilesList, std::map<std::string, std::string>& updatedFilePaths, QWidget* pParent)
{
    bool ret = false;

    assert(pProject != nullptr);
    if (pProject != nullptr)
    {
        // Ensure that the incoming list of filenames isn't empty.
        int numMissingFiles = static_cast<int>(missingFilesList.size());
        assert(numMissingFiles > 0);

        // Create a new dialog to let the user browse to the missing source files.
        rgBrowseMissingFileDialog* pMissingFileDialog = new rgBrowseMissingFileDialog(pProject->m_api, pParent);
        pMissingFileDialog->setModal(true);
        pMissingFileDialog->setWindowTitle(STR_BROWSE_MISSING_FILE_DIALOG_TITLE);

        // Add each missing file to the dialog.
        for (const std::string currentFile : missingFilesList)
        {
            pMissingFileDialog->AddFile(currentFile);
        }

        // Execute the dialog and get the result.
        rgBrowseMissingFileDialog::MissingFileDialogResult result;
        result = static_cast<rgBrowseMissingFileDialog::MissingFileDialogResult>(pMissingFileDialog->exec());

        switch (result)
        {
        case rgBrowseMissingFileDialog::OK:
        {
            // The user is finished browsing to the missing source files. Extract and return the map of updated file paths.
            auto updatedFilePathMap = pMissingFileDialog->GetUpdateFilePathsMap();
            updatedFilePaths = updatedFilePathMap;
            ret = true;
        }
        break;

        case rgBrowseMissingFileDialog::Cancel:
            // The user doesn't want to complete finding the missing source files. They intend to abandon opening the project file.
            ret = false;
            break;

        default:
            // Shouldn't get here.
            assert(false);
        }

        // Free the memory.
        delete pMissingFileDialog;
    }

    return ret;
}

void rgUtils::ShowErrorMessageBox(const char* pErrorMessage, QWidget* pWidget)
{
    if (pWidget == nullptr)
    {
        // The parent widget is null, so need to specify the Window icon
        // in addition to all the other settings to recreate the 'critical' look.
        QMessageBox messageBox;
        messageBox.setWindowIcon(QIcon(gs_ICON_RESOURCE_RGA_LOGO));
        messageBox.setFixedSize(500, 200);
        messageBox.setIcon(QMessageBox::Icon::Critical);
        messageBox.setText(pErrorMessage);
        messageBox.setWindowTitle("Error");
        messageBox.exec();
    }
    else
    {
        QMessageBox::critical(pWidget, "Error", pErrorMessage);
    }
}

bool rgUtils::ShowConfirmationMessageBox(const char* pDialogTitle, const char* pDialogText, QWidget* pParent)
{
    QMessageBox confirmationDialog(pParent);
    confirmationDialog.setWindowIcon(QIcon(gs_ICON_RESOURCE_RGA_LOGO));
    confirmationDialog.setWindowTitle(pDialogTitle);
    confirmationDialog.setText(pDialogText);
    confirmationDialog.setIcon(QMessageBox::Question);
    confirmationDialog.setModal(true);
    confirmationDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

    // Set button cursor to pointing hand cursor.
    QAbstractButton* pButton = confirmationDialog.button(QMessageBox::Button::Yes);
    assert(pButton != nullptr);
    if (pButton != nullptr)
    {
        pButton->setCursor(Qt::PointingHandCursor);
    }

    pButton = confirmationDialog.button(QMessageBox::Button::No);
    assert(pButton != nullptr);
    if (pButton != nullptr)
    {
        pButton->setCursor(Qt::PointingHandCursor);
    }

    // Return true if the user clicked yes, otherwise return false.
    return (confirmationDialog.exec() == QMessageBox::Yes);
}

bool rgUtils::OpenFileDialog(QWidget* pParent, rgProjectAPI api, std::string& selectedFilePath)
{
    bool ret = false;
    switch (api)
    {
    case rgProjectAPI::OpenCL:
    {
        QString filter = QString(STR_FILE_DIALOG_FILTER_OPENCL) + ";;" + STR_FILE_DIALOG_FILTER_ALL;
        ret = OpenFileDialogHelper(pParent, STR_FILE_DIALOG_CAPTION,
            rgConfigManager::Instance().GetLastSelectedFolder(), filter, selectedFilePath);
        break;
    }
    case rgProjectAPI::Vulkan:
    {
        QString filter = ConstructVulkanOpenFileFilter();
        ret = OpenFileDialogHelper(pParent, STR_FILE_DIALOG_CAPTION,
            rgConfigManager::Instance().GetLastSelectedFolder(), filter, selectedFilePath);
        break;
    }
    case rgProjectAPI::Unknown:
    default:
        // We shouldn't get here.
        assert(false);
        break;
    }

    // Update last selected directory when selected directory is valid.
    if (ret == true)
    {
        // Extract directory from full path.
        std::string fileDirectory;
        bool isOk = rgUtils::ExtractFileDirectory(selectedFilePath, fileDirectory);
        assert(isOk);

        if (isOk)
        {
            // Update last selected directory in global config.
            rgConfigManager::Instance().SetLastSelectedDirectory(fileDirectory);
        }
    }

    return ret;
}

bool rgUtils::OpenFileDialog(QWidget* pParent, std::string& selectedFilePath, const std::string& caption, const std::string& filter)
{
    bool ret = OpenFileDialogHelper(pParent, caption.c_str(),
        rgConfigManager::Instance().GetLastSelectedFolder(), filter.c_str(), selectedFilePath);

    return ret;
}

bool rgUtils::OpenFileDialogForMultipleFiles(QWidget* pParent, rgProjectAPI api, QStringList& selectedFilePaths)
{
    bool ret = false;
    switch (api)
    {
    case rgProjectAPI::OpenCL:
        ret = OpenMultipleFileDialogHelper(pParent, STR_FILE_DIALOG_CAPTION, STR_FILE_DIALOG_FILTER_OPENCL, selectedFilePaths);
        break;
    case rgProjectAPI::Unknown:
    default:
        // We shouldn't get here.
        assert(false);
        break;
    }

    // Update last selected directory when selected directory is valid.
    if (ret == true)
    {
        // Extract directory from full path.
        std::string fileDirectory;

        // Directory will be the same for all selected files so just use the first one.
        std::string firstSelectedFile = selectedFilePaths[0].toStdString();

        bool isOk = rgUtils::ExtractFileDirectory(firstSelectedFile, fileDirectory);
        assert(isOk);

        if (isOk)
        {
            // Update last selected directory in global config.
            rgConfigManager::Instance().SetLastSelectedDirectory(fileDirectory);
        }
    }

    return ret;
}

bool rgUtils::OpenFolderInFileBrowser(const std::string& folderPath)
{
    bool isBrowserOpened = false;

    bool isDirExists = rgUtils::IsDirExists(folderPath);
    assert(isDirExists);
    if (isDirExists)
    {
        // Opening a directory url launches the OS specific window manager.
        isBrowserOpened = QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath.c_str()));
        if (!isBrowserOpened)
        {
            // Tell the user that we've failed to open the file browser to the given folder.
            rgUtils::ShowErrorMessageBox(STR_ERR_FAILED_TO_OPEN_FILE_BROWSER);
        }
    }

    return isBrowserOpened;
}

bool rgUtils::OpenProjectDialog(QWidget* pParent, std::string& selectedFilePath)
{
    bool ret = false;

    // Get RGA's default projects folder.
    std::string initialFolder;
    rgConfigManager::GetDefaultProjectsFolder(initialFolder);

    // Open the file dialog.
    ret = OpenFileDialogHelper(pParent, STR_FILE_DIALOG_RGA_CAPTION, initialFolder, STR_FILE_DIALOG_RGA_FILTER, selectedFilePath);

    // Update last selected directory when selected directory is valid.
    if (ret == true)
    {
        // Extract directory from full path.
        std::string fileDirectory;
        bool isOk = rgUtils::ExtractFileDirectory(selectedFilePath, fileDirectory);
        assert(isOk);

        if (isOk)
        {
            // Update last selected directory in global config.
            rgConfigManager::Instance().SetLastSelectedDirectory(fileDirectory);
        }
    }

    return ret;
}

bool rgUtils::ExtractFileName(const std::string& fileFullPath, std::string& fileName, bool includeFileExtension/* = true*/)
{
    bool ret = false;
    fileName.clear();

    // Create a cross-platform file path object.
    gtString gtstrFullPath;
    gtstrFullPath << fileFullPath.c_str();
    osFilePath filePath;
    filePath.setFullPathFromString(gtstrFullPath);

    // Extract and convert the file name.
    gtString gtstrFileName;
    if (includeFileExtension)
    {
        filePath.getFileNameAndExtension(gtstrFileName);
    }
    else
    {
        filePath.getFileName(gtstrFileName);
    }
    fileName = gtstrFileName.asASCIICharArray();
    ret = !fileName.empty();

    return ret;
}

bool rgUtils::ExtractFileDirectory(const std::string& fileFullPath, std::string& pathToFileDirectory)
{
    bool ret = false;

    // Create a cross-platform file path object.
    gtString gtstrFullPath;
    gtstrFullPath << fileFullPath.c_str();
    osFilePath filePath;
    filePath.setFullPathFromString(gtstrFullPath);

    osDirectory fileDirectory;
    ret = filePath.getFileDirectory(fileDirectory);
    if (ret)
    {
        const osFilePath& asFilepath = fileDirectory.asFilePath();
        pathToFileDirectory = asFilepath.asString().asASCIICharArray();
    }

    return ret;
}

bool rgUtils::ExtractFileExtension(const std::string& filePathString, std::string& fileExtension)
{
    bool ret = false;
    fileExtension.clear();

    // Create a cross-platform file path object.
    gtString gtstrFullPath;
    gtstrFullPath << filePathString.c_str();
    osFilePath filePath;
    filePath.setFullPathFromString(gtstrFullPath);

    // Extract and convert the file name.
    gtString gtstrFileExtension;
    filePath.getFileExtension(gtstrFileExtension);

    fileExtension = gtstrFileExtension.asASCIICharArray();
    ret = !fileExtension.empty();

    return ret;
}

void rgUtils::GetDisplayText(const std::string& fileName, std::string& displayText, const int availableSpace, QWidget* pWidget, const int numBackChars)
{
    std::string extension;
    rgUtils::ExtractFileExtension(fileName, extension);

    // Always include the file extension (the +1 is to include the '.' from the file extension too).
    const int NUM_BACK_CHARS = numBackChars + static_cast<unsigned>(extension.length() + 1);
    const int NUM_FRONT_CHARS = gs_TEXT_TRUNCATE_LENGTH_FRONT;

    // Truncate filename within available space to get display text.
    displayText = rgUtils::TruncateString(fileName, NUM_FRONT_CHARS, NUM_BACK_CHARS, availableSpace, pWidget->font(), rgUtils::EXPAND_NONE);
}

bool rgUtils::ReadTextFile(const std::string& fileFullPath, QString& txt)
{
    bool ret = false;
    QFile f(fileFullPath.c_str());
    if (f.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream in(&f);

        // Support Unicode characters.
        in.setCodec("UTF-8");

        txt = in.readAll();
        ret = true;
        f.close();
    }
    return ret;
}

bool rgUtils::RenameFile(const std::string& oldFilepath, const std::string& newFilepath)
{
    bool wasRenamed = false;

    // Verify that the target file already exists.
    QFile f(oldFilepath.c_str());
    bool fileExists = f.exists();
    assert(fileExists);
    if (fileExists)
    {
        wasRenamed = f.rename(newFilepath.c_str());
        assert(wasRenamed);
    }

    return wasRenamed;
}

bool rgUtils::WriteTextFile(const std::string& targetFilePath, const std::string& txt)
{
    std::ofstream outStream(targetFilePath);
    outStream << txt;
    bool ret = !outStream.bad();
    outStream.close();
    if (!ret)
    {
        std::stringstream msg;
        msg << STR_ERR_CANNOT_WRITE_TO_FILE << targetFilePath << ".";
        rgUtils::ShowErrorMessageBox(msg.str().c_str());
    }
    return ret;
}

bool rgUtils::AppendFolderToPath(const std::string& basePath, const std::string& folderName, std::string& updatedPath)
{
    // Convert the base path to gtString.
    gtString gtStrBasePath;
    gtStrBasePath << basePath.c_str();

    // Convert the folder name to gtString.
    gtString gtStrSubDir;
    gtStrSubDir << folderName.c_str();

    // Append the path in a cross-platform manner.
    osDirectory dir;
    dir.setDirectoryFullPathFromString(gtStrBasePath);
    osFilePath baseFilePath(dir.asFilePath());
    baseFilePath.appendSubDirectory(gtStrSubDir);
    updatedPath = baseFilePath.asString().asASCIICharArray();

    return !updatedPath.empty();
}

bool rgUtils::AppendFileNameToPath(const std::string& basePath, const std::string& fileName, std::string& updatedPath)
{
    // Convert the base path to gtString.
    gtString gtStrBasePath;
    gtStrBasePath << basePath.c_str();

    // Convert the folder name to gtString.
    gtString gtStrFileName;
    gtStrFileName << fileName.c_str();

    // Append to the path in a cross-platform manner.
    osDirectory dir;
    dir.setDirectoryFullPathFromString(gtStrBasePath);
    osFilePath baseFilePath(dir.asFilePath());
    baseFilePath.setFileName(gtStrFileName);
    updatedPath = baseFilePath.asString().asASCIICharArray();

    return !updatedPath.empty();
}

bool rgUtils::AppendPathSeparator(const std::string& basePath, std::string& updatedPath)
{
    // Start with the base path.
    updatedPath.assign(basePath);

    // Append a separator to the end, and return.
    QString separatorString = QDir::separator();
    updatedPath.append(separatorString.toStdString());

    return !updatedPath.empty();
}

void rgUtils::StandardizePathSeparator(std::string& path)
{
    rgUtils::Replace(path, "\\\\", "/");
    rgUtils::Replace(path, "\\", "/");
}

bool rgUtils::IsFileExists(const std::string& fileFullPath)
{
    // Convert the full path to gtString.
    gtString gtStrFullPath;
    gtStrFullPath << fileFullPath.c_str();
    osFilePath filePath(gtStrFullPath);

    // Check if the file exists.
    bool ret = filePath.exists();
    return ret;
}

bool rgUtils::IsDirExists(const std::string& dirFullPath)
{
    // Convert the path to gtString.
    gtString gtStrFullPath;
    gtStrFullPath << dirFullPath.c_str();

    // Check if the directory exists.
    osDirectory dir;
    dir.setDirectoryFullPathFromString(gtStrFullPath);
    bool ret = dir.exists();
    return ret;
}

bool rgUtils::CreateFolder(const std::string& dirPath)
{
    bool ret = false;

    // Convert the path to gtString.
    gtString gtStrFullPath;
    gtStrFullPath << dirPath.c_str();

    // Create the directory if it doesn't already exist.
    osDirectory dir;
    dir.setDirectoryFullPathFromString(gtStrFullPath);
    if (!dir.exists())
    {
        ret = dir.create();
    }

    return ret;
}

bool rgUtils::IsValidFileName(const std::string& fileName)
{
    // Verify that no illegal characters are included in the file name.
    bool isValid =
        !fileName.empty() &&
        fileName.find('/') == std::string::npos &&
        fileName.find('\\') == std::string::npos &&
        fileName.find(':') == std::string::npos &&
        fileName.find('|') == std::string::npos &&
        fileName.find('<') == std::string::npos &&
        fileName.find('>') == std::string::npos &&
#ifdef _WIN32
        // Windows only.
        fileName.find('*') == std::string::npos &&
        fileName.find('\"') == std::string::npos;
#else
        // Linux only.
        fileName.find('&') == std::string::npos;
#endif
    return isValid;
}

bool rgUtils::IsValidProjectName(const std::string& fileName)
{
    const int MAX_PROJECT_NAME_LENGTH = 50;
    return IsValidFileName(fileName) &&
        !fileName.empty() &&
        fileName.size() <= MAX_PROJECT_NAME_LENGTH;
}

bool rgUtils::IsSpvBinFile(const std::string& filePath)
{
    bool isSpv  = false;

    if (rgUtils::IsFileExists(filePath))
    {
        QFile file(filePath.c_str());
        if (file.open(QFile::ReadOnly))
        {
            // Read the first 32-bit word of the file and check if it matches the SPIR-V binary magic number.
            uint32_t word;
            if (file.read(reinterpret_cast<char*>(&word), sizeof(word)))
            {
                isSpv = (word == SPV_BINARY_MAGIC_NUMBER);
            }
        }
    }

    return isSpv;
}

bool rgUtils::ConstructSpvDisasmFileName(const std::string& projFolder, const std::string& spvFileName, std::string& spvDisasmFileName)
{
    bool result = (!projFolder.empty() && !spvFileName.empty());
    if (result)
    {
        std::stringstream outFileName;
        std::string inFileName;
        rgUtils::ExtractFileName(spvFileName, inFileName);
        const std::string dirSep = QString(QDir::separator()).toStdString();

        outFileName << projFolder << dirSep << STR_PROJECT_SUBFOLDER_GENERATED << dirSep << inFileName << "." << STR_VK_FILE_EXT_SPIRV_TXT;
        spvDisasmFileName = outFileName.str();
    }

    return result;
}

std::pair<rgVulkanInputType, rgSrcLanguage> rgUtils::DetectInputFileType(const std::string& filePath)
{
    std::pair<rgVulkanInputType, rgSrcLanguage> ret = { rgVulkanInputType::Unknown, rgSrcLanguage::Unknown };

    if (rgUtils::IsSpvBinFile(filePath))
    {
        ret = { rgVulkanInputType::Spirv, rgSrcLanguage::SPIRV_Text };
    }
    else
    {
        QFileInfo fileInfo(filePath.c_str());
        const QString& ext = fileInfo.suffix().toLower();

        rgConfigManager& configManager = rgConfigManager::Instance();
        std::shared_ptr<rgGlobalSettings> pGlobalSettings = configManager.GetGlobalConfig();

        if (QString(QString(pGlobalSettings->m_inputFileExtSpvTxt.c_str())).split(STR_VK_FILE_EXT_DELIMITER).contains(ext))
        {
            ret = { rgVulkanInputType::SpirvTxt, rgSrcLanguage::SPIRV_Text };
        }
        else if (QString(pGlobalSettings->m_inputFileExtGlsl.c_str()).split(STR_VK_FILE_EXT_DELIMITER).contains(ext))
        {
            ret = { rgVulkanInputType::Glsl, rgSrcLanguage::GLSL };
        }
        else if (QString(pGlobalSettings->m_inputFileExtHlsl.c_str()).split(STR_VK_FILE_EXT_DELIMITER).contains(ext))
        {
            ret = { rgVulkanInputType::Hlsl, rgSrcLanguage::HLSL };
        }
    }

    return ret;
}

bool rgUtils::LoadAndApplyStyle(const std::vector<std::string>& stylesheetFileNames, QWidget* pWidget)
{
    bool ret = false;
    QString styleSheet = "";

    for (const auto& fileName : stylesheetFileNames)
    {
        std::string stylePath = STR_STYLESHEET_RESOURCE_PATH + fileName;

        // Open file.
        QFile stylesheetFile(stylePath.c_str());
        ret = stylesheetFile.open(QFile::ReadOnly);
        assert(ret);

        if (ret)
        {
            // Apply stylesheet.
            QString style(stylesheetFile.readAll());
            styleSheet += style;
        }
        else
        {
            break;
        }
    }
    pWidget->setStyleSheet(styleSheet);

    return ret;
}

bool rgUtils::LoadAndApplyStyle(const std::vector<std::string>& stylesheetFileNames, QApplication* pApplication)
{
    bool ret = false;
    QString styleSheet = "";

    for (const auto& fileName : stylesheetFileNames)
    {
        std::string stylePath = STR_STYLESHEET_RESOURCE_PATH + fileName;

        // Open file.
        QFile stylesheetFile(stylePath.c_str());
        ret = stylesheetFile.open(QFile::ReadOnly);
        assert(ret);

        if (ret)
        {
            // Apply stylesheet.
            QString style(stylesheetFile.readAll());
            styleSheet += style;
        }
        else
        {
            break;
        }
    }
    pApplication->style()->unpolish(pApplication);
    pApplication->setStyleSheet(styleSheet);

    return ret;
}

void rgUtils::SetToolAndStatusTip(const std::string& tip, QWidget* pWidget)
{
    if (pWidget != nullptr)
    {
        pWidget->setToolTip(tip.c_str());

        // Remove any formatting from the incoming string.
        const std::string plainText = GetPlainText(tip);
        pWidget->setStatusTip(plainText.c_str());
    }
}

void rgUtils::SetStatusTip(const std::string& tip, QWidget* pWidget)
{
    if (pWidget != nullptr)
    {
        // Remove any formatting from the incoming string.
        const std::string plainText = GetPlainText(tip);
        pWidget->setStatusTip(plainText.c_str());
    }
}

void rgUtils::CenterOnWidget(QWidget* pWidget, QWidget* pCenterOn)
{
    if (pWidget != nullptr && pCenterOn != nullptr)
    {
        // Get global coordinate of widget to be centered on.
        QPoint globalCoord = pCenterOn->mapToGlobal(QPoint(0, 0));

        // Calculate coordinates to center pWidget relative to pCenterOn.
        int xPos = globalCoord.x() + (pCenterOn->width() - pWidget->width()) / 2;
        int yPos = globalCoord.y() + (pCenterOn->height() - pWidget->height()) / 2;

        // Set coordinates.
        pWidget->move(xPos, yPos);
    }
}

void rgUtils::FocusOnFirstValidAncestor(QWidget* pWidget)
{
    if (pWidget != nullptr)
    {
        // Step through ancestors until one is found which accepts focus, or there is none found.
        QWidget* pAncestor = pWidget->parentWidget();
        while (pAncestor != nullptr && pAncestor->focusPolicy() == Qt::NoFocus)
        {
            pAncestor = pAncestor->parentWidget();
        }

        // Set focus on the ancestor if it exists.
        if (pAncestor != nullptr)
        {
            pAncestor->setFocus();
        }
    }
}

void rgUtils::StyleRepolish(QWidget* pWidget, bool repolishChildren)
{
    if (pWidget != nullptr)
    {
        bool isBlocked = false;

        // Check to see if recursive repolishing has been explicitly blocked by this widget.
        QVariant isBlockProperty = pWidget->property(gs_IS_REPOLISHING_BLOCKED);
        if (isBlockProperty.isValid())
        {
            // If the property is found on the widget, and set to true, recursive repolishing is cut short.
            isBlocked = isBlockProperty.toBool();
        }

        if (!isBlocked)
        {
            // Re-polish the widget if it has a style.
            if (pWidget->style() != nullptr)
            {
                pWidget->style()->unpolish(pWidget);
                pWidget->style()->polish(pWidget);
            }

            // Re-polish all the child widgets.
            if (repolishChildren)
            {
                for (QObject* pChildObject : pWidget->children())
                {
                    QWidget* pChildWidget = qobject_cast<QWidget*>(pChildObject);
                    StyleRepolish(pChildWidget, repolishChildren);
                }
            }
        }
    }
}

void rgUtils::SetBackgroundColor(QWidget* pWidget, const QColor& color)
{
    assert(pWidget != nullptr);
    if (pWidget != nullptr)
    {
        // Set the background color.
        QPalette palette = pWidget->palette();
        palette.setColor(QPalette::Background, color);
        pWidget->setAutoFillBackground(true);
        pWidget->setPalette(palette);
    }
}

std::string rgUtils::TruncateString(const std::string& text, unsigned int numFrontChars, unsigned int numBackChars, unsigned int availableWidth, const QFont& textFont, TruncateType truncateType)
{
    // Get font metrics for the given font.
    QFontMetrics fm(textFont);

    std::string truncatedString(text);
    std::string front;
    std::string back;

    // Start indices for the front and back segments of the truncated string.
    const size_t FRONT_START_INDEX = 0;
    const size_t BACK_START_INDEX = text.length() - numBackChars;

    // If the sum of the character minimums at the front and back of the string is
    // greater than the string length then no truncation is needed.
    if (numFrontChars + numBackChars < text.length())
    {
        switch (truncateType)
        {
            // Non-expanding truncation.
            case EXPAND_NONE:
            {
                front = text.substr(FRONT_START_INDEX, numFrontChars);
                back = text.substr(BACK_START_INDEX, numBackChars);

                // Combine strings into result string with delimeter.
                truncatedString = front + STR_TRUNCATED_STRING_DELIMETER + back;
            }
            break;

            // Expanding truncation.
            case EXPAND_FRONT:
            case EXPAND_BACK:
            {
                // Some of these variables/calculations are a little unnecessary, but they have
                // been written as such for convenience in understanding what the code is doing.
                const size_t FRONT_END_INDEX = numFrontChars;
                const size_t BACK_END_INDEX = text.length();
                const size_t FRONT_EXPAND_START_INDEX = FRONT_START_INDEX;
                const size_t BACK_EXPAND_START_INDEX = FRONT_END_INDEX;
                const size_t FRONT_EXPAND_LENGTH = BACK_START_INDEX - FRONT_START_INDEX;
                const size_t BACK_EXPAND_LENGTH = BACK_END_INDEX - FRONT_END_INDEX;

                // Determine front and back sub strings.
                if (truncateType == EXPAND_FRONT)
                {
                    front = text.substr(FRONT_EXPAND_START_INDEX, FRONT_EXPAND_LENGTH);
                    back = text.substr(BACK_START_INDEX, numBackChars);
                }
                else if (truncateType == EXPAND_BACK)
                {
                    front = text.substr(FRONT_START_INDEX, numFrontChars);
                    back = text.substr(BACK_EXPAND_START_INDEX, BACK_EXPAND_LENGTH);
                }

                // Combine strings into result string (check first without delimeter).
                truncatedString = front + back;

                // Check text width/length to see if truncation is needed.
                unsigned textWidth = fm.width(truncatedString.c_str());
                bool isWithinBounds = textWidth <= availableWidth;
                bool isAtMinLength = front.length() <= numFrontChars && back.length() <= numBackChars;

                const unsigned MAX_ATTEMPTS = 1024;
                unsigned attempts = 0;
                while (!isWithinBounds && !isAtMinLength && ++attempts < MAX_ATTEMPTS)
                {
                    // Reduce string (either front or back depending on truncate type) by one character.
                    if (truncateType == EXPAND_FRONT)
                    {
                        front.pop_back();
                    }
                    else if (truncateType == EXPAND_BACK)
                    {
                        back.erase(0, 1);
                    }

                    // Combine strings into result string.
                    truncatedString = front + STR_TRUNCATED_STRING_DELIMETER + back;

                    // Check text width/length to see if truncation is still needed.
                    textWidth = fm.width(truncatedString.c_str());
                    isWithinBounds = textWidth <= availableWidth;
                    isAtMinLength = front.length() <= numFrontChars && back.length() <= numBackChars;
                }

                // This should never happen, but is a safeguard against hanging.
                assert(attempts < MAX_ATTEMPTS);
            }
            break;

            // Invalid truncation type.
            default:
                assert(false);
        }
    }

    return truncatedString;
}

std::string rgUtils::GetPlainText(const std::string& text)
{
    const char* STR_BOLD_MARKUP_START = "<b>";
    const char* STR_BOLD_MARKUP_END   = "</b>";

    // Create a QString
    QString plainText(QString::fromStdString(text));

    // Remove "<b>" and "</b>".
    plainText.replace(STR_BOLD_MARKUP_START, "");
    plainText.replace(STR_BOLD_MARKUP_END, "");

    return plainText.toStdString();
}

bool rgUtils::IsInList(const std::string & list, const std::string & token, char delim)
{
    size_t start = 0, end = 0;
    bool stop = false, ret = false;
    while (!stop)
    {
        end = list.find(STR_VK_FILE_EXT_DELIMITER, start);
        ret = (token == list.substr(start, (end == std::string::npos ? std::string::npos : end - start)));
        stop = (ret || end == std::string::npos);
        start = end + 1;
    }

    return ret;
}

bool rgUtils::IsSpvasTextFile(const std::string& stageInputFile, std::string& stageAbbreviation)
{
    static const std::string DEFAULT_TEXT_FILE_EXTENSION = "txt";
    bool result = false;

    // Extract file extension.
    std::string fileExtension;
    rgUtils::ExtractFileExtension(stageInputFile, fileExtension);

    // Get global settings.
    rgConfigManager& configManager = rgConfigManager::Instance();
    std::shared_ptr<rgGlobalSettings> pGlobalSettings = configManager.GetGlobalConfig();

    // Extract allowed extensions.
    QStringList extentions = QString::fromStdString(pGlobalSettings->m_inputFileExtSpvTxt).split((s_OPTIONS_LIST_DELIMITER));

    // Check if the extension is txt and is in the list.
    if ((fileExtension.compare(DEFAULT_TEXT_FILE_EXTENSION) == 0) && (extentions.contains(QString::fromStdString(fileExtension))))
    {
        result = true;
    }

    return result;
}
