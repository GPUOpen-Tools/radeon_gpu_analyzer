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
#pragma warning(disable : 4309)
#endif
#include <external/amdt_os_wrappers/Include/osFilePath.h>
#include <external/amdt_os_wrappers/Include/osDirectory.h>
#include <external/amdt_base_tools/Include/gtString.h>
#ifdef _WIN32
#pragma warning(pop)
#endif

#include "QtCommon/CustomWidgets/ArrowIconWidget.h"
#include "QtCommon/CustomWidgets/ListWidget.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_browse_missing_file_dialog.h"
#include "radeon_gpu_analyzer_gui/qt/rg_hide_list_widget_event_filter.h"
#include "radeon_gpu_analyzer_gui/rg_config_manager.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_vulkan.h"

// Static constants

static const uint32_t SPV_BINARY_MAGIC_NUMBER = 0x07230203;

// *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - START ***

static bool OpenFileDialogHelper(QWidget*           parent,
                                 const QString&     caption,
                                 const std::string& initial_folder,
                                 const QString&     filter,
                                 std::string&       selected_file_path)
{
    bool ret = false;
    selected_file_path.clear();

    QString filename = QFileDialog::getOpenFileName(parent, caption, initial_folder.c_str(), filter);

    if (!filename.isNull())
    {
        selected_file_path = filename.toStdString();
        ret                = !selected_file_path.empty();
    }

    return ret;
}

static bool OpenMultipleFileDialogHelper(QWidget* parent, const QString& caption, const QString& filter, QStringList& selected_file_paths)
{
    bool ret = false;
    selected_file_paths.clear();

    QStringList filenames = QFileDialog::getOpenFileNames(parent, caption, RgConfigManager::Instance().GetLastSelectedFolder().c_str(), filter);

    if (!filenames.isEmpty())
    {
        selected_file_paths = filenames;
        ret                 = !selected_file_paths.empty();
    }

    return ret;
}

// Build the file filter for Open File dialog in Vulkan mode.
static QString ConstructVulkanOpenFileFilter()
{
    QString filter;
    auto    settings = RgConfigManager::Instance().GetGlobalConfig();
    assert(settings != nullptr);
    if (settings != nullptr)
    {
        // Convert the extensions stored in the Global Settings to the Qt file filter format.
        QStringList glsl_exts    = QString(settings->input_file_ext_glsl.c_str()).split(';');
        QStringList spv_txt_exts = QString(settings->input_file_ext_spv_txt.c_str()).split(';');
        QStringList spv_bin_exts = QString(settings->input_file_ext_spv_bin.c_str()).split(';');

        QString glsl_ext_list, spv_txt_ext_list, spv_bin_ext_list;

        for (QString& ext : glsl_exts)
        {
            glsl_ext_list += ("*." + ext + " ");
        }
        for (QString& ext : spv_txt_exts)
        {
            spv_txt_ext_list += ("*." + ext + " ");
        }
        for (QString& ext : spv_bin_exts)
        {
            spv_bin_ext_list += ("*." + ext + " ");
        }

        glsl_ext_list.chop(1);
        spv_txt_ext_list.chop(1);
        spv_bin_ext_list.chop(1);

        filter +=
            (QString(kStrFileDialogFilterGlslSpirv) + " (" + glsl_ext_list + " " + spv_txt_ext_list + " " + kStrFileDialogFilterSpirvBinaryExt + " " + ");;");
        filter += (QString(kStrFileDialogFilterSpirv) + ";;");
        filter += (QString(kStrFileDialogFilterGlsl) + " (" + glsl_ext_list + ");;");
        filter += (QString(kStrFileDialogFilterSpvTxt) + " (" + spv_txt_ext_list + ");;");
        filter += (QString(kStrFileDialogFilterSpirvBinaryExt) + " (" + spv_bin_ext_list + ");;");

        filter += kStrFileDialogFilterAll;
    }

    return filter;
}

// *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - END ***

void RgUtils::splitString(const std::string& str, char delim, std::vector<std::string>& dst)
{
    std::stringstream ss;
    ss.str(str);
    std::string substr;
    while (std::getline(ss, substr, delim))
    {
        dst.push_back(substr);
    }
}

std::string RgUtils::BuildSemicolonSeparatedBoolList(const std::vector<bool>& bool_list)
{
    std::stringstream stream;
    for (size_t i = 0; i < bool_list.size(); i++)
    {
        stream << bool_list[i] ? 1 : 0;

        // If this is not the last item, add the delimiter.
        if (i < (bool_list.size() - 1))
        {
            stream << RgConfigManager::kRgaListDelimiter;
        }
    }
    return stream.str();
}

std::string RgUtils::BuildSemicolonSeparatedStringList(const std::vector<std::string>& str_list)
{
    std::stringstream stream;
    for (size_t i = 0; i < str_list.size(); i++)
    {
        stream << str_list[i];

        // If this is not the last item, add the delimiter.
        if (i < (str_list.size() - 1))
        {
            stream << RgConfigManager::kRgaListDelimiter;
        }
    }
    return stream.str();
}

std::string RgUtils::BuildSemicolonSeparatedIntList(const std::vector<int>& int_list)
{
    std::stringstream stream;
    for (size_t i = 0; i < int_list.size(); i++)
    {
        stream << int_list[i];

        // If this is not the last item, add the delimiter.
        if (i < (int_list.size() - 1))
        {
            stream << RgConfigManager::kRgaListDelimiter;
        }
    }
    return stream.str();
}

bool RgUtils::IsContainsWhitespace(const std::string& text)
{
    bool is_contains_whitespace = false;

    // Step through each character in the string.
    for (size_t character_index = 0; character_index < text.length(); ++character_index)
    {
        // Check if the current character is whitespace.
        if (isspace(text[character_index]))
        {
            is_contains_whitespace = true;
            break;
        }
    }

    return is_contains_whitespace;
}

void RgUtils::LeftTrim(const std::string& text, std::string& trimmed_text)
{
    trimmed_text    = text;
    auto space_iter = std::find_if(trimmed_text.begin(), trimmed_text.end(), [](char ch) { return !std::isspace<char>(ch, std::locale::classic()); });
    trimmed_text.erase(trimmed_text.begin(), space_iter);
}

void RgUtils::RightTrim(const std::string& text, std::string& trimmed_text)
{
    trimmed_text    = text;
    auto space_iter = std::find_if(trimmed_text.rbegin(), trimmed_text.rend(), [](char ch) { return !std::isspace<char>(ch, std::locale::classic()); });
    trimmed_text.erase(space_iter.base(), trimmed_text.end());
}

void RgUtils::TrimLeadingAndTrailingWhitespace(const std::string& text, std::string& trimmed_text)
{
    // Trim the whitespace off the left and right sides of the incoming text.
    std::string left_trimmed;
    LeftTrim(text, left_trimmed);
    RightTrim(left_trimmed, trimmed_text);
}

void RgUtils::Replace(std::string& text, const std::string& target, const std::string& replacement)
{
    if (!target.empty())
    {
        // Search for instances of the given target text in the source string.
        size_t start_position = 0;
        while ((start_position = text.find(target, start_position)) != std::string::npos)
        {
            // Replace the target text in the string with the replacement text.
            text.replace(start_position, target.length(), replacement);

            // Update the start position for the next search.
            start_position += replacement.length();
        }
    }
}

std::string RgUtils::GenerateTemplateCode(RgProjectAPI api_name, const std::string& entry_point_prefix)
{
    std::stringstream str_builder;
    switch (api_name)
    {
    case RgProjectAPI::kOpenCL:
    {
        str_builder << kStrNewFileTemplateCodeOpenclA;
        if (!entry_point_prefix.empty())
        {
            str_builder << entry_point_prefix << "_";
        }
        str_builder << kStrNewFileTemplateCodeOpenclB;
        break;
    }
    case RgProjectAPI::kUnknown:
    default:
        // We shouldn't get here.
        assert(false);
        break;
    }
    return str_builder.str();
}

std::string RgUtils::GenerateDefaultProjectName()
{
    // Generate a timestamp to append to the base filename.
    QDateTime         right_now  = QDateTime::currentDateTime();
    QString           local_time = right_now.toString("yyMMdd-HHmmss");
    std::stringstream project_name(local_time.toStdString());
    return project_name.str();
}

const char* RgUtils::GenerateProjectName(RgProjectAPI api_name)
{
    static const char* kStrFileMenuProjectName = "Project";
    const char*        ret                     = nullptr;
    switch (api_name)
    {
    case RgProjectAPI::kOpenCL:
    case RgProjectAPI::kVulkan:
        ret = kStrFileMenuProjectName;
        break;
    default:
        // We shouldn't get here.
        assert(false);
        break;
    }
    return ret;
}

std::string RgUtils::GetProjectTitlePrefix(RgProjectAPI current_api)
{
    std::stringstream str;
    str << "<b>" << RgUtils::GenerateProjectName(current_api) << " name: </b>";
    return str.str();
}

bool RgUtils::GetStageShaderPath(const RgPipelineShaders& pipeline, RgPipelineStage stage, std::string& shader_path)
{
    bool res = false;

    size_t      stage_index           = static_cast<size_t>(stage);
    const auto& stage_input_file_path = pipeline.shader_stages[stage_index];
    if (!stage_input_file_path.empty())
    {
        // Extract the shader's file path.
        shader_path = stage_input_file_path;
        res         = true;
    }

    return res;
}

bool RgUtils::SetStageShaderPath(RgPipelineStage stage, const std::string& shader_path, RgPipelineShaders& pipeline)
{
    bool res = false;

    assert(!shader_path.empty());
    if (!shader_path.empty())
    {
        // Set the shader's file path within the pipeline.
        size_t stage_index                  = static_cast<size_t>(stage);
        pipeline.shader_stages[stage_index] = shader_path;
        res                                 = true;
    }

    return res;
}

bool RgUtils::GetComputeCapabilityToArchMapping(std::map<std::string, std::string>& device_name_mapping)
{
    device_name_mapping.clear();
    bool ret = false;

    // Get the current app mode.
    const std::string& current_mode = RgConfigManager::Instance().GetCurrentModeString();

    // Get the version info.
    std::shared_ptr<RgCliVersionInfo> version_info = RgConfigManager::Instance().GetVersionInfo();

    // Find the architecture node for our current mode.
    auto current_mode_architectures_iter = version_info->gpu_architectures.find(current_mode);
    bool is_mode_found                   = (current_mode_architectures_iter != version_info->gpu_architectures.end());
    assert(is_mode_found);
    if (is_mode_found)
    {
        // Step through each GPU hardware architecture.
        std::vector<RgGpuArchitecture> architectures = current_mode_architectures_iter->second;
        for (const RgGpuArchitecture& hardware_architecture : architectures)
        {
            const std::string& current_architecture = hardware_architecture.architecture_name;

            // Determine how many families are found within the architecture.
            std::vector<RgGpuFamily> gpu_families                 = hardware_architecture.gpu_families;
            int                      num_families_in_architecture = static_cast<int>(gpu_families.size());

            // Step through each family within the architecture.
            for (int family_index = 0; family_index < num_families_in_architecture; family_index++)
            {
                // Create a copy of the family info and sort by product name.
                RgGpuFamily current_family                      = gpu_families[family_index];
                device_name_mapping[current_family.family_name] = current_architecture;
            }
        }
    }

    ret = !device_name_mapping.empty();
    return ret;
}

bool RgUtils::GetGfxNotation(const std::string& code_name, std::string& gfx_notation)
{
    bool ret = false;

    // Mapping between codename and gfx compute capability notation.
    static const std::map<std::string, std::string> kGfxNotationMapping{
        std::make_pair<std::string, std::string>("Tahiti", "gfx600"),   std::make_pair<std::string, std::string>("Hainan", "gfx601"),
        std::make_pair<std::string, std::string>("Oland", "gfx601"),    std::make_pair<std::string, std::string>("Capeverde", "gfx601"),
        std::make_pair<std::string, std::string>("Pitcairn", "gfx601"), std::make_pair<std::string, std::string>("Kaveri", "gfx700"),
        std::make_pair<std::string, std::string>("Spectre", "gfx700"),  std::make_pair<std::string, std::string>("Spooky", "gfx700"),
        std::make_pair<std::string, std::string>("Hawaii", "gfx701"),   std::make_pair<std::string, std::string>("Kabini", "gfx703"),
        std::make_pair<std::string, std::string>("Kalindi", "gfx703"),  std::make_pair<std::string, std::string>("Godavari", "gfx703"),
        std::make_pair<std::string, std::string>("Mullins", "gfx703"),  std::make_pair<std::string, std::string>("Bonaire", "gfx704"),
        std::make_pair<std::string, std::string>("Carrizo", "gfx801"),  std::make_pair<std::string, std::string>("Bristol Ridge", "gfx801"),
        std::make_pair<std::string, std::string>("Iceland", "gfx802"),  std::make_pair<std::string, std::string>("Tonga", "gfx802"),
        std::make_pair<std::string, std::string>("Fiji", "gfx803"),     std::make_pair<std::string, std::string>("Ellesmere", "gfx803"),
        std::make_pair<std::string, std::string>("Baffin", "gfx803"),   std::make_pair<std::string, std::string>("Lexa", "gfx803"),
        std::make_pair<std::string, std::string>("Stoney", "gfx810"),
    };

    auto iter = kGfxNotationMapping.find(code_name);
    if (iter != kGfxNotationMapping.end())
    {
        gfx_notation = iter->second;
        ret          = true;
    }

    return ret;
}

std::string RgUtils::RemoveGfxNotation(const std::string& family_name)
{
    std::string fixed_group_name;
    size_t      bracket_pos = family_name.find("/");
    if (bracket_pos != std::string::npos)
    {
        fixed_group_name = family_name.substr(bracket_pos + 1);
    }
    else
    {
        fixed_group_name = family_name;
    }
    return fixed_group_name;
}

bool RgUtils::GetFirstValidOutputGpu(const RgBuildOutputsMap& build_outputs, std::string& first_valid_gpu, std::shared_ptr<RgCliBuildOutput>& output)
{
    bool ret = false;

    // Find the first target ASIC that appears to have valid compilation results.
    auto asic_outputs_iter = build_outputs.begin();
    for (; asic_outputs_iter != build_outputs.end(); ++asic_outputs_iter)
    {
        if (asic_outputs_iter->second != nullptr)
        {
            // The current ASIC appears to have valid output, so return the output's info.
            first_valid_gpu = asic_outputs_iter->first;
            output          = asic_outputs_iter->second;
            ret             = true;
            break;
        }
    }

    return ret;
}

void RgUtils::SetupComboList(QWidget* parent, ListWidget*& list_widget, ArrowIconWidget*& button, QObject*& event_filter, bool hide)
{
    assert(button != nullptr);
    if (button != nullptr)
    {
        // Create list widget for the combo box.
        list_widget = new ListWidget(parent, button, hide);

        assert(list_widget != nullptr);
        if (list_widget != nullptr)
        {
            list_widget->hide();

            // Also disable scrollbars on this list widget.
            list_widget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            list_widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

            // Install a custom event filter on this list widget
            // so it can be hidden when something else is clicked on.
            event_filter = new RgHideListWidgetEventFilter(list_widget, button);
            list_widget->installEventFilter(event_filter);
            qApp->installEventFilter(event_filter);

            // Start out the combo box with the first entry.
            list_widget->setCurrentRow(0);
        }
    }
}

std::string RgUtils::GenerateCloneName(int clone_index)
{
    // Generate a name based on the default clone name and the incoming index.
    std::stringstream clone_name;
    clone_name << kStrCloneFolderName;
    clone_name << clone_index;
    return clone_name.str();
}

std::string RgUtils::GetEntrypointsNameString(RgProjectAPI api)
{
    std::string result_string;

    switch (api)
    {
    case RgProjectAPI::kOpenCL:
    {
        result_string = kStrMenuBarFileItemEntrypointHeaderOpencl;
    }
    break;
    case RgProjectAPI::kUnknown:
    default:
    {
        // All other cases use API-agnostic label text.
        result_string = kStrMenuBarFileItemEntrypointHeaderDefault;
    }
    break;
    }

    return result_string;
}

bool RgUtils::ProjectAPIToString(RgProjectAPI api, std::string& str)
{
    bool ret = true;
    switch (api)
    {
    case RgProjectAPI::kOpenCL:
        str = kStrApiNameOpencl;
        break;
    case RgProjectAPI::kVulkan:
        str = kStrApiNameVulkan;
        break;
    case RgProjectAPI::kUnknown:
    default:
        // We shouldn't get here.
        ret = false;
        assert(ret);
        break;
    }
    return ret;
}

RgProjectAPI RgUtils::ProjectAPIToEnum(const std::string& str)
{
    RgProjectAPI project_api = RgProjectAPI::kUnknown;

    if (str.compare(kStrApiNameOpencl) == 0)
    {
        project_api = RgProjectAPI::kOpenCL;
    }
    else if (str.compare(kStrApiNameVulkan) == 0)
    {
        project_api = RgProjectAPI::kVulkan;
    }

    return project_api;
}

bool RgUtils::ProjectAPIToSourceFileExtension(RgProjectAPI api, std::string& extension)
{
    bool ret = true;
    switch (api)
    {
    case RgProjectAPI::kOpenCL:
        extension = kStrSourceFileExtensionCl;
        break;
    case RgProjectAPI::kVulkan:
        extension = kStrSourceFileExtensionVulkanGlsl;
        break;
    case RgProjectAPI::kUnknown:
    default:
        // We shouldn't get here.
        ret = false;
        assert(ret);
        break;
    }

    return ret;
}

bool RgUtils::IsProjectSourcePathsValid(std::shared_ptr<RgProject> project, int clone_index, QWidget* parent)
{
    bool ret = false;

    RgConfigManager& config_manager = RgConfigManager::Instance();

    // Extract the source file paths from the clone.
    std::vector<std::string> source_filepaths;
    config_manager.GetProjectSourceFilePaths(project, clone_index, source_filepaths);

    // Create a list for files that the project wants to load, but couldn't.
    std::vector<std::string> missing_files_list;

    // Add all the project's source files into the RgBuildView.
    for (const std::string& file_path : source_filepaths)
    {
        // Check that the file exists before attempting to load it.
        bool is_file_exists = RgUtils::IsFileExists(file_path);
        if (!is_file_exists)
        {
            missing_files_list.push_back(file_path);
        }
    }

    // If the list of missing files is non-empty, ask the user where the files were moved to.
    if (!missing_files_list.empty())
    {
        // Present the user with a dialog that allows them to update the paths for any missing source files.
        std::map<std::string, std::string> updated_file_paths;
        RgUtils::ShowBrowseMissingFilesDialog(project, missing_files_list, updated_file_paths, parent);
    }
    else
    {
        // The list of missing files is empty. All paths are valid.
        ret = true;
    }

    return ret;
}

bool RgUtils::IsSourceFileTypeValid(const std::string& str)
{
    bool ret = false;

    // Convert to QString for convenience (QString has "endsWith" functionality).
    QString qtString = str.c_str();

    // Check file headers.
    if (qtString.endsWith(kStrSourceFileExtensionCl) || qtString.endsWith(kStrProjectFileExtension))
    {
        ret = true;
    }

    return ret;
}

bool RgUtils::ShowBrowseMissingFilesDialog(std::shared_ptr<RgProject>          project,
                                           const std::vector<std::string>&     missing_files_list,
                                           std::map<std::string, std::string>& updated_file_paths,
                                           QWidget*                            parent)
{
    bool ret = false;

    assert(project != nullptr);
    if (project != nullptr)
    {
        // Ensure that the incoming list of filenames isn't empty.
        int num_missing_files = static_cast<int>(missing_files_list.size());
        assert(num_missing_files > 0);

        // Create a new dialog to let the user browse to the missing source files.
        RgBrowseMissingFileDialog* missing_file_dialog = new RgBrowseMissingFileDialog(project->api, parent);
        missing_file_dialog->setModal(true);
        missing_file_dialog->setWindowTitle(kStrBrowseMissingFileDialogTitle);

        // Add each missing file to the dialog.
        for (const std::string current_file : missing_files_list)
        {
            missing_file_dialog->AddFile(current_file);
        }

        // Execute the dialog and get the result.
        RgBrowseMissingFileDialog::MissingFileDialogResult result;
        result = static_cast<RgBrowseMissingFileDialog::MissingFileDialogResult>(missing_file_dialog->exec());

        switch (result)
        {
        case RgBrowseMissingFileDialog::kOK:
        {
            // The user is finished browsing to the missing source files. Extract and return the map of updated file paths.
            auto updated_file_path_map = missing_file_dialog->GetUpdateFilePathsMap();
            updated_file_paths         = updated_file_path_map;
            ret                        = true;
        }
        break;

        case RgBrowseMissingFileDialog::kCancel:
            // The user doesn't want to complete finding the missing source files. They intend to abandon opening the project file.
            ret = false;
            break;

        default:
            // Shouldn't get here.
            assert(false);
        }

        // Free the memory.
        delete missing_file_dialog;
    }

    return ret;
}

void RgUtils::ShowErrorMessageBox(const char* error_message, QWidget* widget)
{
    if (widget == nullptr)
    {
        // The parent widget is null, so need to specify the Window icon
        // in addition to all the other settings to recreate the 'critical' look.
        QMessageBox message_box;
        message_box.setWindowIcon(QIcon(kIconResourceRgaLogo));
        message_box.setFixedSize(500, 200);
        message_box.setIcon(QMessageBox::Icon::Critical);
        message_box.setText(error_message);
        message_box.setWindowTitle("Error");
        message_box.exec();
    }
    else
    {
        QMessageBox::critical(widget, "Error", error_message);
    }
}

bool RgUtils::ShowConfirmationMessageBox(const char* dialog_title, const char* dialog_text, QWidget* parent)
{
    QMessageBox confirmation_dialog(parent);
    confirmation_dialog.setWindowIcon(QIcon(kIconResourceRgaLogo));
    confirmation_dialog.setWindowTitle(dialog_title);
    confirmation_dialog.setText(dialog_text);
    confirmation_dialog.setIcon(QMessageBox::Question);
    confirmation_dialog.setModal(true);
    confirmation_dialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

    // Set button cursor to pointing hand cursor.
    QAbstractButton* button = confirmation_dialog.button(QMessageBox::Button::Yes);
    assert(button != nullptr);
    if (button != nullptr)
    {
        button->setCursor(Qt::PointingHandCursor);
    }

    button = confirmation_dialog.button(QMessageBox::Button::No);
    assert(button != nullptr);
    if (button != nullptr)
    {
        button->setCursor(Qt::PointingHandCursor);
    }

    // Return true if the user clicked yes, otherwise return false.
    return (confirmation_dialog.exec() == QMessageBox::Yes);
}

bool RgUtils::OpenFileDialog(QWidget* parent, RgProjectAPI api, std::string& selected_file_path)
{
    bool ret = false;
    switch (api)
    {
    case RgProjectAPI::kOpenCL:
    {
        QString filter = QString(kStrFileDialogFilterOpencl) + ";;" + kStrFileDialogFilterAll;
        ret            = OpenFileDialogHelper(parent, kStrFileDialogCaption, RgConfigManager::Instance().GetLastSelectedFolder(), filter, selected_file_path);
        break;
    }
    case RgProjectAPI::kVulkan:
    {
        QString filter = ConstructVulkanOpenFileFilter();
        ret            = OpenFileDialogHelper(parent, kStrFileDialogCaption, RgConfigManager::Instance().GetLastSelectedFolder(), filter, selected_file_path);
        break;
    }
    case RgProjectAPI::kUnknown:
    default:
        // We shouldn't get here.
        assert(false);
        break;
    }

    // Update last selected directory when selected directory is valid.
    if (ret == true)
    {
        // Extract directory from full path.
        std::string file_directory;
        bool        is_ok = RgUtils::ExtractFileDirectory(selected_file_path, file_directory);
        assert(is_ok);

        if (is_ok)
        {
            // Update last selected directory in global config.
            RgConfigManager::Instance().SetLastSelectedDirectory(file_directory);
        }
    }

    return ret;
}

bool RgUtils::OpenFileDialog(QWidget* parent, std::string& selected_file_path, const std::string& caption, const std::string& filter)
{
    bool ret = OpenFileDialogHelper(parent, caption.c_str(), RgConfigManager::Instance().GetLastSelectedFolder(), filter.c_str(), selected_file_path);

    return ret;
}

bool RgUtils::OpenFileDialogForMultipleFiles(QWidget* parent, RgProjectAPI api, QStringList& selected_file_paths)
{
    bool ret = false;
    switch (api)
    {
    case RgProjectAPI::kOpenCL:
        ret = OpenMultipleFileDialogHelper(parent, kStrFileDialogCaption, kStrFileDialogFilterOpencl, selected_file_paths);
        break;
    case RgProjectAPI::kUnknown:
    default:
        // We shouldn't get here.
        assert(false);
        break;
    }

    // Update last selected directory when selected directory is valid.
    if (ret == true)
    {
        // Extract directory from full path.
        std::string file_directory;

        // Directory will be the same for all selected files so just use the first one.
        std::string first_selected_file = selected_file_paths[0].toStdString();

        bool is_ok = RgUtils::ExtractFileDirectory(first_selected_file, file_directory);
        assert(is_ok);

        if (is_ok)
        {
            // Update last selected directory in global config.
            RgConfigManager::Instance().SetLastSelectedDirectory(file_directory);
        }
    }

    return ret;
}

bool RgUtils::OpenFolderInFileBrowser(const std::string& folder_path)
{
    bool is_browser_opened = false;

    bool is_dir_exists = RgUtils::IsDirExists(folder_path);
    assert(is_dir_exists);
    if (is_dir_exists)
    {
        // Opening a directory url launches the OS specific window manager.
        is_browser_opened = QDesktopServices::openUrl(QUrl::fromLocalFile(folder_path.c_str()));
        if (!is_browser_opened)
        {
            // Tell the user that we've failed to open the file browser to the given folder.
            RgUtils::ShowErrorMessageBox(kStrErrFailedToOpenFileBrowser);
        }
    }

    return is_browser_opened;
}

bool RgUtils::OpenProjectDialog(QWidget* parent, std::string& selected_file_path)
{
    bool ret = false;

    // Get RGA's default projects folder.
    std::string initial_folder, custom_folder;

    // Update the recent project list to reference the new path.
    RgConfigManager::GetDefaultProjectsFolder(initial_folder);

    // Check to see if the user wants to use a custom projects folder.
    RgConfigManager& config_manager = RgConfigManager::Instance();
    config_manager.GetCustomProjectsLocation(custom_folder);
    if (initial_folder.compare(custom_folder) != 0)
    {
        initial_folder = custom_folder;
    }

    // Open the file dialog.
    ret = OpenFileDialogHelper(parent, kStrFileDialogRgaCaption, initial_folder, kStrFileDialogRgaFilter, selected_file_path);

    // Update last selected directory when selected directory is valid.
    if (ret == true)
    {
        // Extract directory from full path.
        std::string file_directory;
        bool        is_ok = RgUtils::ExtractFileDirectory(selected_file_path, file_directory);
        assert(is_ok);

        if (is_ok)
        {
            // Update last selected directory in global config.
            RgConfigManager::Instance().SetLastSelectedDirectory(file_directory);
        }
    }

    return ret;
}

bool RgUtils::ExtractFileName(const std::string& file_full_path, std::string& filename, bool include_file_extension /* = true*/)
{
    bool ret = false;
    filename.clear();

    // Create a cross-platform file path object.
    gtString gtstr_full_path;
    gtstr_full_path << file_full_path.c_str();
    osFilePath file_path;
    file_path.setFullPathFromString(gtstr_full_path);

    // Extract and convert the file name.
    gtString gtstr_file_name;
    if (include_file_extension)
    {
        file_path.getFileNameAndExtension(gtstr_file_name);
    }
    else
    {
        file_path.getFileName(gtstr_file_name);
    }
    filename = gtstr_file_name.asASCIICharArray();
    ret      = !filename.empty();

    return ret;
}

bool RgUtils::ExtractFileDirectory(const std::string& file_full_path, std::string& path_to_file_directory)
{
    bool ret = false;

    // Create a cross-platform file path object.
    gtString gtstr_full_path;
    gtstr_full_path << file_full_path.c_str();
    osFilePath file_path;
    file_path.setFullPathFromString(gtstr_full_path);

    osDirectory file_directory;
    ret = file_path.getFileDirectory(file_directory);
    if (ret)
    {
        const osFilePath& as_file_path = file_directory.asFilePath();
        path_to_file_directory         = as_file_path.asString().asASCIICharArray();
    }

    return ret;
}

bool RgUtils::ExtractFileExtension(const std::string& file_path_string, std::string& file_extension)
{
    bool ret = false;
    file_extension.clear();

    // Create a cross-platform file path object.
    gtString gtstr_full_path;
    gtstr_full_path << file_path_string.c_str();
    osFilePath file_path;
    file_path.setFullPathFromString(gtstr_full_path);

    // Extract and convert the file name.
    gtString gtstr_file_extension;
    file_path.getFileExtension(gtstr_file_extension);

    file_extension = gtstr_file_extension.asASCIICharArray();
    ret            = !file_extension.empty();

    return ret;
}

void RgUtils::GetDisplayText(const std::string& filename, std::string& display_text, const int available_space, QWidget* widget, const int num_back_chars)
{
    std::string extension;
    RgUtils::ExtractFileExtension(filename, extension);

    // Always include the file extension (the +1 is to include the '.' from the file extension too).
    const int kNumBackChars  = num_back_chars + static_cast<unsigned>(extension.length() + 1);
    const int kNumFrontChars = kTextTruncateLengthFront;

    // Truncate filename within available space to get display text.
    display_text = RgUtils::TruncateString(filename, kNumFrontChars, kNumBackChars, available_space, widget->font(), RgUtils::kExpandNone);
}

bool RgUtils::ReadTextFile(const std::string& file_full_path, QString& txt)
{
    bool  ret = false;
    QFile f(file_full_path.c_str());
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

bool RgUtils::RenameFile(const std::string& old_file_path, const std::string& new_file_path)
{
    bool was_renamed = false;

    // Verify that the target file already exists.
    QFile f(old_file_path.c_str());
    bool  file_exists = f.exists();
    assert(file_exists);
    if (file_exists)
    {
        was_renamed = f.rename(new_file_path.c_str());
        assert(was_renamed);
    }

    return was_renamed;
}

bool RgUtils::WriteTextFile(const std::string& target_file_path, const std::string& txt)
{
    std::ofstream outStream(target_file_path);
    outStream << txt;
    bool ret = !outStream.bad();
    outStream.close();
    if (!ret)
    {
        std::stringstream msg;
        msg << kStrErrCannotWriteToFile << target_file_path << ".";
        RgUtils::ShowErrorMessageBox(msg.str().c_str());
    }
    return ret;
}

bool RgUtils::AppendFolderToPath(const std::string& base_path, const std::string& folder_name, std::string& updated_path)
{
    // Convert the base path to gtString.
    gtString gt_str_base_path;
    gt_str_base_path << base_path.c_str();

    // Convert the folder name to gtString.
    gtString gt_str_sub_dir;
    gt_str_sub_dir << folder_name.c_str();

    // Append the path in a cross-platform manner.
    osDirectory dir;
    dir.setDirectoryFullPathFromString(gt_str_base_path);
    osFilePath base_file_path(dir.asFilePath());
    base_file_path.appendSubDirectory(gt_str_sub_dir);
    updated_path = base_file_path.asString().asASCIICharArray();

    return !updated_path.empty();
}

bool RgUtils::AppendFileNameToPath(const std::string& base_path, const std::string& filename, std::string& updated_path)
{
    // Convert the base path to gtString.
    gtString gt_str_base_path;
    gt_str_base_path << base_path.c_str();

    // Convert the folder name to gtString.
    gtString gt_str_file_name;
    gt_str_file_name << filename.c_str();

    // Append to the path in a cross-platform manner.
    osDirectory dir;
    dir.setDirectoryFullPathFromString(gt_str_base_path);
    osFilePath base_file_path(dir.asFilePath());
    base_file_path.setFileName(gt_str_file_name);
    updated_path = base_file_path.asString().asASCIICharArray();

    return !updated_path.empty();
}

bool RgUtils::AppendPathSeparator(const std::string& base_path, std::string& updated_path)
{
    // Start with the base path.
    updated_path.assign(base_path);

    // Append a separator to the end, and return.
    QString separator_string = QDir::separator();
    updated_path.append(separator_string.toStdString());

    return !updated_path.empty();
}

void RgUtils::StandardizePathSeparator(std::string& path)
{
    RgUtils::Replace(path, "\\\\", "/");
    RgUtils::Replace(path, "\\", "/");
}

bool RgUtils::IsFileExists(const std::string& file_full_path)
{
    // Convert the full path to gtString.
    gtString gt_str_full_path;
    gt_str_full_path << file_full_path.c_str();
    osFilePath file_path(gt_str_full_path);

    // Check if the file exists.
    bool ret = file_path.exists();
    return ret;
}

bool RgUtils::IsDirExists(const std::string& dir_full_path)
{
    // Convert the path to gtString.
    gtString gt_str_full_path;
    gt_str_full_path << dir_full_path.c_str();

    // Check if the directory exists.
    osDirectory dir;
    dir.setDirectoryFullPathFromString(gt_str_full_path);
    bool ret = dir.exists();
    return ret;
}

bool RgUtils::CreateFolder(const std::string& dir_path)
{
    bool ret = false;

    // Convert the path to gtString.
    gtString gt_str_full_path;
    gt_str_full_path << dir_path.c_str();

    // Create the directory if it doesn't already exist.
    osDirectory dir;
    dir.setDirectoryFullPathFromString(gt_str_full_path);
    if (!dir.exists())
    {
        ret = dir.create();
    }

    return ret;
}

bool RgUtils::IsValidFileName(const std::string& filename)
{
    // Verify that no illegal characters are included in the file name.
    bool is_valid = !filename.empty() && filename.find('/') == std::string::npos && filename.find('\\') == std::string::npos &&
                    filename.find(':') == std::string::npos && filename.find('|') == std::string::npos && filename.find('<') == std::string::npos &&
                    filename.find('>') == std::string::npos && filename.find(' ') == std::string::npos &&
#ifdef _WIN32
                    // Windows only.
                    filename.find('*') == std::string::npos && filename.find('\"') == std::string::npos;
#else
                    // Linux only.
                    filename.find('&') == std::string::npos;
#endif
    return is_valid;
}

bool RgUtils::IsValidProjectName(const std::string& file_full_path, std::string& error_message)
{
	const int kMaxProjectNameLength = 50;
    QFileInfo file_info             = QFileInfo(QString::fromStdString(file_full_path));
    QString   file_name             = file_info.fileName();

    // Check for valid file name.
    bool is_valid = IsValidFileName(file_name.toStdString()) && !file_name.isEmpty() && file_name.size() <= kMaxProjectNameLength;

    if (!is_valid)
    {
        error_message = kStrErrIllegalProjectName;
    }
    else
    {
        // If the file name does not end with the project file extension, add it.
        // When creating a new project, no project file extension is present at this point,
        // but when opening an existing project, there is a project file extension present.
        // Adding the missing project file extension here makes the code below work for both
        // scenarios.
        if (!file_name.endsWith(kStrProjectFileExtension))
        {
            file_name = file_name + kStrProjectFileExtension;
        }

        // Check for two or more dots in the name.
        // If found, return a failure.
        const QRegularExpression reg("(\\.{2,})");
        QRegularExpressionMatch  match = reg.match(file_name);

        if (match.hasMatch())
        {
            is_valid      = false;
            error_message = kStrErrIllegalStringProjectName + std::string("\"") + match.captured(0).toStdString() + std::string("\":");
        }
    }

    return is_valid;
}

bool RgUtils::IsSpvBinFile(const std::string& file_path)
{
    bool is_spv = false;

    if (RgUtils::IsFileExists(file_path))
    {
        QFile file(file_path.c_str());
        if (file.open(QFile::ReadOnly))
        {
            // Read the first 32-bit word of the file and check if it matches the SPIR-V binary magic number.
            uint32_t word;
            if (file.read(reinterpret_cast<char*>(&word), sizeof(word)))
            {
                is_spv = (word == SPV_BINARY_MAGIC_NUMBER);
            }
        }
    }

    return is_spv;
}

bool RgUtils::ConstructSpvDisasmFileName(const std::string& proj_folder, const std::string& spv_file_name, std::string& spv_disasm_file_name)
{
    bool result = (!proj_folder.empty() && !spv_file_name.empty());
    if (result)
    {
        std::stringstream out_file_name;
        std::string       in_file_name;
        RgUtils::ExtractFileName(spv_file_name, in_file_name);
        const std::string dir_sep = QString(QDir::separator()).toStdString();

        out_file_name << proj_folder << dir_sep << kStrProjectSubfolderGenerated << dir_sep << in_file_name << "." << kStrVkFileExtSpirvTxt;
        spv_disasm_file_name = out_file_name.str();
    }

    return result;
}

std::pair<RgVulkanInputType, RgSrcLanguage> RgUtils::DetectInputFileType(const std::string& file_path)
{
    std::pair<RgVulkanInputType, RgSrcLanguage> ret = {RgVulkanInputType::kUnknown, RgSrcLanguage::Unknown};

    if (RgUtils::IsSpvBinFile(file_path))
    {
        ret = {RgVulkanInputType::kSpirv, RgSrcLanguage::kSPIRV_Text};
    }
    else
    {
        QFileInfo      file_info(file_path.c_str());
        const QString& ext = file_info.suffix().toLower();

        RgConfigManager&                  config_manager  = RgConfigManager::Instance();
        std::shared_ptr<RgGlobalSettings> global_settings = config_manager.GetGlobalConfig();

        if (QString(QString(global_settings->input_file_ext_spv_txt.c_str())).split(kStrVkFileExtDelimiter).contains(ext))
        {
            ret = {RgVulkanInputType::kSpirvTxt, RgSrcLanguage::kSPIRV_Text};
        }
        else if (QString(global_settings->input_file_ext_glsl.c_str()).split(kStrVkFileExtDelimiter).contains(ext))
        {
            ret = {RgVulkanInputType::kGlsl, RgSrcLanguage::kGLSL};
        }
        else if (QString(global_settings->input_file_ext_hlsl.c_str()).split(kStrVkFileExtDelimiter).contains(ext))
        {
            ret = {RgVulkanInputType::kHlsl, RgSrcLanguage::kHLSL};
        }
    }

    return ret;
}

bool RgUtils::LoadAndApplyStyle(const std::vector<std::string>& stylesheet_file_names, QWidget* widget)
{
    bool    ret        = false;
    QString styleSheet = "";

    for (const auto& filename : stylesheet_file_names)
    {
        std::string style_path = kStrStylesheetResourcePath + filename;

        // Open file.
        QFile stylesheetFile(style_path.c_str());
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
    widget->setStyleSheet(styleSheet);

    return ret;
}

bool RgUtils::LoadAndApplyStyle(const std::vector<std::string>& stylesheet_file_names, QApplication* application)
{
    bool    ret        = false;
    QString styleSheet = "";

    for (const auto& filename : stylesheet_file_names)
    {
        std::string style_path = kStrStylesheetResourcePath + filename;

        // Open file.
        QFile stylesheet_file(style_path.c_str());
        ret = stylesheet_file.open(QFile::ReadOnly);
        assert(ret);

        if (ret)
        {
            // Apply stylesheet.
            QString style(stylesheet_file.readAll());
            styleSheet += style;
        }
        else
        {
            break;
        }
    }
    application->style()->unpolish(application);
    application->setStyleSheet(styleSheet);

    return ret;
}

void RgUtils::SetToolAndStatusTip(const std::string& tip, QWidget* widget)
{
    if (widget != nullptr)
    {
        widget->setToolTip(tip.c_str());

        // Remove any formatting from the incoming string.
        const std::string plain_text = GetPlainText(tip);
        widget->setStatusTip(plain_text.c_str());
    }
}

void RgUtils::SetStatusTip(const std::string& tip, QWidget* widget)
{
    if (widget != nullptr)
    {
        // Remove any formatting from the incoming string.
        const std::string plain_text = GetPlainText(tip);
        widget->setStatusTip(plain_text.c_str());
    }
}

void RgUtils::CenterOnWidget(QWidget* widget, QWidget* center_on)
{
    if (widget != nullptr && center_on != nullptr)
    {
        // Get global coordinate of widget to be centered on.
        QPoint global_coord = center_on->mapToGlobal(QPoint(0, 0));

        // Calculate coordinates to center widget relative to pCenterOn.
        int xPos = global_coord.x() + (center_on->width() - widget->width()) / 2;
        int yPos = global_coord.y() + (center_on->height() - widget->height()) / 2;

        // Set coordinates.
        widget->move(xPos, yPos);
    }
}

void RgUtils::FocusOnFirstValidAncestor(QWidget* widget)
{
    if (widget != nullptr)
    {
        // Step through ancestors until one is found which accepts focus, or there is none found.
        QWidget* ancestor = widget->parentWidget();
        while (ancestor != nullptr && ancestor->focusPolicy() == Qt::NoFocus)
        {
            ancestor = ancestor->parentWidget();
        }

        // Set focus on the ancestor if it exists.
        if (ancestor != nullptr)
        {
            ancestor->setFocus();
        }
    }
}

void RgUtils::StyleRepolish(QWidget* widget, bool repolish_children)
{
    if (widget != nullptr)
    {
        bool is_blocked = false;

        // Check to see if recursive repolishing has been explicitly blocked by this widget.
        QVariant isBlockProperty = widget->property(kIsRepolishingBlocked);
        if (isBlockProperty.isValid())
        {
            // If the property is found on the widget, and set to true, recursive repolishing is cut short.
            is_blocked = isBlockProperty.toBool();
        }

        if (!is_blocked)
        {
            // Re-polish the widget if it has a style.
            if (widget->style() != nullptr)
            {
                widget->style()->unpolish(widget);
                widget->style()->polish(widget);
            }

            // Re-polish all the child widgets.
            if (repolish_children)
            {
                for (QObject* child_object : widget->children())
                {
                    QWidget* child_widget = qobject_cast<QWidget*>(child_object);
                    StyleRepolish(child_widget, repolish_children);
                }
            }
        }
    }
}

void RgUtils::SetBackgroundColor(QWidget* widget, const QColor& color)
{
    assert(widget != nullptr);
    if (widget != nullptr)
    {
        // Set the background color.
        QPalette palette = widget->palette();
        palette.setColor(QPalette::Background, color);
        widget->setAutoFillBackground(true);
        widget->setPalette(palette);
    }
}

std::string RgUtils::TruncateString(const std::string& text,
                                    unsigned int       num_front_chars,
                                    unsigned int       num_back_chars,
                                    unsigned int       available_width,
                                    const QFont&       text_font,
                                    TruncateType       truncate_type)
{
    // Get font metrics for the given font.
    QFontMetrics fm(text_font);

    std::string truncated_string(text);
    std::string front;
    std::string back;

    // Start indices for the front and back segments of the truncated string.
    const size_t kFrontStartIndex = 0;
    const size_t kBackStartIndex  = text.length() - num_back_chars;

    // If the sum of the character minimums at the front and back of the string is
    // greater than the string length then no truncation is needed.
    if (num_front_chars + num_back_chars < text.length())
    {
        switch (truncate_type)
        {
        // Non-expanding truncation.
        case kExpandNone:
        {
            front = text.substr(kFrontStartIndex, num_front_chars);
            back  = text.substr(kBackStartIndex, num_back_chars);

            // Combine strings into result string with delimeter.
            truncated_string = front + kStrTruncatedStringDelimeter + back;
        }
        break;

        // Expanding truncation.
        case kExpandFront:
        case kExpandBack:
        {
            // Some of these variables/calculations are a little unnecessary, but they have
            // been written as such for convenience in understanding what the code is doing.
            const size_t kFrontEndIndex         = num_front_chars;
            const size_t kBackEndIndex          = text.length();
            const size_t kFrontExpandStartIndex = kFrontStartIndex;
            const size_t kBackExpandStartIndex  = kFrontEndIndex;
            const size_t kFrontExpandLength     = kBackStartIndex - kFrontStartIndex;
            const size_t kBackExpandLength      = kBackEndIndex - kFrontEndIndex;

            // Determine front and back sub strings.
            if (truncate_type == kExpandFront)
            {
                front = text.substr(kFrontExpandStartIndex, kFrontExpandLength);
                back  = text.substr(kBackStartIndex, num_back_chars);
            }
            else if (truncate_type == kExpandBack)
            {
                front = text.substr(kFrontStartIndex, num_front_chars);
                back  = text.substr(kBackExpandStartIndex, kBackExpandLength);
            }

            // Combine strings into result string (check first without delimeter).
            truncated_string = front + back;

            // Check text width/length to see if truncation is needed.
            unsigned text_width       = fm.width(truncated_string.c_str());
            bool     is_within_bounds = text_width <= available_width;
            bool     is_at_min_length = front.length() <= num_front_chars && back.length() <= num_back_chars;

            const unsigned MAX_ATTEMPTS = 1024;
            unsigned       attempts     = 0;
            while (!is_within_bounds && !is_at_min_length && ++attempts < MAX_ATTEMPTS)
            {
                // Reduce string (either front or back depending on truncate type) by one character.
                if (truncate_type == kExpandFront)
                {
                    front.pop_back();
                }
                else if (truncate_type == kExpandBack)
                {
                    back.erase(0, 1);
                }

                // Combine strings into result string.
                truncated_string = front + kStrTruncatedStringDelimeter + back;

                // Check text width/length to see if truncation is still needed.
                text_width       = fm.width(truncated_string.c_str());
                is_within_bounds = text_width <= available_width;
                is_at_min_length = front.length() <= num_front_chars && back.length() <= num_back_chars;
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

    return truncated_string;
}

std::string RgUtils::GetPlainText(const std::string& text)
{
    const char* kStrBoldMarkupStart = "<b>";
    const char* kStrBoldMarkupEnd   = "</b>";

    // Create a QString
    QString plain_text(QString::fromStdString(text));

    // Remove "<b>" and "</b>".
    plain_text.replace(kStrBoldMarkupStart, "");
    plain_text.replace(kStrBoldMarkupEnd, "");

    return plain_text.toStdString();
}

bool RgUtils::IsInList(const std::string& list, const std::string& token, char delim)
{
    Q_UNUSED(delim);

    size_t start = 0, end = 0;
    bool   stop = false, ret = false;
    while (!stop)
    {
        end   = list.find(kStrVkFileExtDelimiter, start);
        ret   = (token == list.substr(start, (end == std::string::npos ? std::string::npos : end - start)));
        stop  = (ret || end == std::string::npos);
        start = end + 1;
    }

    return ret;
}

void RgUtils::FindSearchResultIndices(const QString& text, const QString& text_to_find, std::vector<size_t>& search_result_indices)
{
    // Make sure that neither text value is empty.
    if (!text.isEmpty() && !text_to_find.isEmpty())
    {
        // Step the cursor through the entire field of text to search.
        size_t cursor_index = text.indexOf(text_to_find, 0);

        // Step through the text to search until we hit the end.
        size_t search_string_length = text_to_find.size();
        while (cursor_index != std::string::npos)
        {
            // Found a result occurrence. Push it into the results list.
            search_result_indices.push_back(cursor_index);

            // Search for the next result location.
            cursor_index = text.indexOf(text_to_find, static_cast<int>(cursor_index + search_string_length));
        }
    }
}

bool RgUtils::IsSpvasTextFile(const std::string& stage_input_file, std::string& stage_abbreviation)
{
    Q_UNUSED(stage_abbreviation);

    static const std::string DEFAULT_TEXT_FILE_EXTENSION = "txt";
    bool                     result                      = false;

    // Extract file extension.
    std::string file_extension;
    RgUtils::ExtractFileExtension(stage_input_file, file_extension);

    // Get global settings.
    RgConfigManager&                  config_manager  = RgConfigManager::Instance();
    std::shared_ptr<RgGlobalSettings> global_settings = config_manager.GetGlobalConfig();

    // Extract allowed extensions.
    QStringList extentions = QString::fromStdString(global_settings->input_file_ext_spv_txt).split((kOptionsListDelimiter));

    // Check if the extension is txt and is in the list.
    if ((file_extension.compare(DEFAULT_TEXT_FILE_EXTENSION) == 0) && (extentions.contains(QString::fromStdString(file_extension))))
    {
        result = true;
    }

    return result;
}
