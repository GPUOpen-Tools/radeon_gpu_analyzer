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
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <QtCommon/CustomWidgets/ArrowIconWidget.h>
#include <QtCommon/CustomWidgets/ListWidget.h>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgBrowseMissingFileDialog.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgHideListWidgetEventFilter.h>
#include <RadeonGPUAnalyzerGUI/include/rgConfigManager.h>
#include <RadeonGPUAnalyzerGUI/include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>

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

    QStringList filenames = QFileDialog::getOpenFileNames(pParent, caption, rgConfigManager::GetLastSelectedFolder().c_str(), filter);

    if (!filenames.isEmpty())
    {
        selectedFilePaths = filenames;
        ret = !selectedFilePaths.empty();
    }

    return ret;
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

unsigned int rgUtils::FindIndexOf(const QString& textToFind, const QString& text, int startPosition)
{
    int findIndex = -1;

    int position = textToFind.indexOf(text, startPosition);
    if (position != startPosition && position != -1)
    {
        findIndex = position;
    }

    return findIndex;
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
    case OpenCL:
    {
        strBuilder << STR_NEW_FILE_TEMPLATE_CODE_OPENCL_A;
        if (!entryPointPrefix.empty())
        {
            strBuilder << entryPointPrefix << "_";
        }
        strBuilder << STR_NEW_FILE_TEMPLATE_CODE_OPENCL_B;
        break;
    }
    case Unknown:
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
    QString localTime = rightNow.toString("yyyyMMdd-HHmmss");
    std::stringstream projectName(localTime.toStdString());
    return projectName.str();
}

const char* rgUtils::GenerateProjectName(rgProjectAPI apiName)
{
    static const char* STR_FILE_MENU_PROJECT_NAME = "Project";
    const char* pRet = nullptr;
    switch (apiName)
    {
    case OpenCL:
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
    case OpenCL:
        {
            resultString = STR_MENU_BAR_FILE_ITEM_ENTRYPOINT_HEADER_OPENCL;
        }
        break;
    case Unknown:
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
    case OpenCL:
        str = STR_API_NAME_OPENCL;
        break;
    case Unknown:
    default:
        // We shouldn't get here.
        ret = false;
        assert(ret);
        break;
    }
    return ret;
}

bool rgUtils::ProjectAPIToSourceFileExtension(rgProjectAPI api, std::string& extension)
{
    bool ret = true;
    switch (api)
    {
    case OpenCL:
        extension = STR_CL_SOURCE_FILE_EXTENSION;
        break;

    case Unknown:
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
    if (qtString.endsWith(STR_CL_SOURCE_FILE_EXTENSION) || qtString.endsWith(STR_PROJECT_FILE_EXTENSION))
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
    }

    return ret;
}

void rgUtils::ShowErrorMessageBox(const char* pErrorMessage, QWidget* pWidget)
{
    QMessageBox messageBox;
    messageBox.critical(pWidget, "Error", pErrorMessage);
    messageBox.setFixedSize(500, 200);
}

bool rgUtils::ShowConfirmationMessageBox(const char* pDialogTitle, const char* pDialogText, QWidget* pParent)
{
    QMessageBox confirmationDialog(pParent);
    confirmationDialog.setWindowTitle(pDialogTitle);
    confirmationDialog.setText(pDialogText);
    confirmationDialog.setIcon(QMessageBox::Question);
    confirmationDialog.setModal(true);
    confirmationDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

    // Return true if the user clicked yes, otherwise return false.
    return (confirmationDialog.exec() == QMessageBox::Yes);
}

bool rgUtils::OpenFileDialog(QWidget* pParent, rgProjectAPI api, std::string& selectedFilePath)
{
    bool ret = false;
    switch (api)
    {
    case OpenCL:
        ret = OpenFileDialogHelper(pParent, STR_FILE_DIALOG_CL_CAPTION,
            rgConfigManager::GetLastSelectedFolder(), STR_FILE_DIALOG_CL_FILTER, selectedFilePath);
        break;
    case Unknown:
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
            rgConfigManager::SetLastSelectedDirectory(fileDirectory);
        }
    }

    return ret;
}

bool rgUtils::OpenFileDialogForMultipleFiles(QWidget* pParent, rgProjectAPI api, QStringList& selectedFilePaths)
{
    bool ret = false;
    switch (api)
    {
    case OpenCL:
        ret = OpenMultipleFileDialogHelper(pParent, STR_FILE_DIALOG_CL_CAPTION, STR_FILE_DIALOG_CL_FILTER, selectedFilePaths);
        break;
    case Unknown:
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
            rgConfigManager::SetLastSelectedDirectory(fileDirectory);
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
            rgConfigManager::SetLastSelectedDirectory(fileDirectory);
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

bool rgUtils::LoadAndApplyStyle(const std::string& fileName, QWidget* pWidget)
{
    bool ret = false;

    std::string stylePath = STR_STYLESHEET_RESOURCE_PATH + fileName;

    // Open file.
    QFile stylesheetFile(stylePath.c_str());
    ret = stylesheetFile.open(QFile::ReadOnly);
    assert(ret);

    if (ret)
    {
        // Apply stylesheet.
        QString style(stylesheetFile.readAll());
        pWidget->setStyleSheet(style);
    }

    return ret;
}

bool rgUtils::LoadAndApplyStyle(const std::string& fileName, QApplication* pApplication)
{
    bool ret = false;

    std::string stylePath = STR_STYLESHEET_RESOURCE_PATH + fileName;

    // Open file.
    QFile stylesheetFile(stylePath.c_str());
    ret = stylesheetFile.open(QFile::ReadOnly);
    assert(ret);

    if (ret)
    {
        // Apply stylesheet.
        QString style(stylesheetFile.readAll());
        pApplication->setStyleSheet(style);
    }

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


