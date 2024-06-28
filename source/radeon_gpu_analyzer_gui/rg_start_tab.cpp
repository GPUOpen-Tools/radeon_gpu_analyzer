// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QKeyEvent>
#include <QButtonGroup>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_recent_project_widget.h"
#include "radeon_gpu_analyzer_gui/qt/rg_start_tab.h"
#include "radeon_gpu_analyzer_gui/qt/rg_main_window.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_binary.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"

RgStartTab::RgStartTab(QWidget* parent)
    : QWidget(parent),
    parent_(parent)
{
    // Setup the UI.
    ui_.setupUi(this);

    // Set the cursor for the context menu.
    menu_.setCursor(Qt::PointingHandCursor);
}

void RgStartTab::Initialize()
{
    // Connect signals within the start tab.
    ConnectSignals();

    // Create the context menu for recent files
    CreateContextMenu();

    // Apply string constants to the view's widgets.
    ApplyStringConstants();

    // Initialize the start buttons.
    InitializeStartButtons();

    // Populate the list of recent projects.
    PopulateRecentProjectsList();

    // Set the cursor type for specific widgets in the view.
    SetCursor();

    // Install an event filter to handle up/down arrow keys on the recent files list widget.
    qApp->installEventFilter(this);
}

void RgStartTab::ApplyStringConstants()
{
    // Apply string constants that apply to a specific API.
    ApplyApiStringConstants();

    // Set tooltips and status tips.
    RgUtils::SetToolAndStatusTip(kStrMenuBarOpenProjectTooltip, ui_.recentProjectsOtherPushButton);
    RgUtils::SetToolAndStatusTip(kStrMenuBarHelpAboutTooltip, ui_.aboutRGAPushButton);
    RgUtils::SetToolAndStatusTip(kStrMenuBarHelpManualTooltip, ui_.helpManualPushButton);
    RgUtils::SetToolAndStatusTip(kStrMenuBarHelpGettingStartedGuideTooltip, ui_.gettingStartedPushButton);
}

void RgStartTab::ConnectSignals()
{
    // Open recent project with the "Other..." button.
    bool is_connected = connect(this->ui_.recentProjectsOtherPushButton, &QPushButton::pressed, this, &RgStartTab::OpenProjectFileEvent);
    assert(is_connected);

    // About RGA.
    is_connected = connect(this->ui_.aboutRGAPushButton, &QPushButton::pressed, this, &RgStartTab::AboutEvent);
    assert(is_connected);

    // Getting started guide.
    is_connected = connect(this->ui_.gettingStartedPushButton, &QPushButton::pressed, this, &RgStartTab::GettingStartedGuideEvent);
    assert(is_connected);

    // Help manual.
    is_connected = connect(this->ui_.helpManualPushButton, &QPushButton::pressed, this, &RgStartTab::HelpManual);
    assert(is_connected);
}

void RgStartTab::ClearRecentProjectsList()
{
    // Only clear the list if it already exists.
    if (recent_project_button_group_ != nullptr)
    {
        // Remove all Recent Project buttons added to the list.
        QLayout* recent_projects_list = ui_.recentProgramsWrapper->layout();
        assert(recent_projects_list != nullptr);

        // Delete the custom recent projects widgets.
        QList<RgRecentProjectWidget*> widget_list = ui_.recentProgramsWrapper->findChildren<RgRecentProjectWidget*>();
        for (RgRecentProjectWidget* recent_project_widget : widget_list)
        {
            assert(recent_project_widget != nullptr);
            if (recent_project_widget != nullptr)
            {
                // Remove each button widget from the list of recent projects, and destroy it.
                recent_projects_list->removeWidget(recent_project_widget);
                RG_SAFE_DELETE(recent_project_widget);
            }
        }

        // Destroy the recent projects list button group.
        RG_SAFE_DELETE(recent_project_button_group_);

        // Reorder the tab order to allow for the recent projects list removals.
        ReorderTabOrder();
    }
}

void RgStartTab::CreateContextMenu()
{
    open_recent_action_ = new QAction(kStrMainWindowLoadProject, nullptr);
    menu_.addAction(open_recent_action_);

    // Add a separator between the current menu items.
    menu_.addSeparator();

    open_containing_folder_action_ = new QAction(kStrFileContextMenuOpenContainingFolder, nullptr);
    menu_.addAction(open_containing_folder_action_);
}

void RgStartTab::InitializeStartButtons()
{
    QVBoxLayout* vertical_buttons_layout = ui_.startLinkLayout;
    assert(vertical_buttons_layout != nullptr);
    if (vertical_buttons_layout != nullptr)
    {
        // Retrieve the list of start buttons to add to the tab.
        std::vector<QPushButton*> start_buttons;
        GetStartButtons(start_buttons);

        // Add the Start buttons to the view.
        for (auto start_button : start_buttons)
        {
            // Set the cursor for each of the start buttons being added to the start tab.
            start_button->setCursor(Qt::PointingHandCursor);

            // Add the widget to the start tab.
            vertical_buttons_layout->addWidget(start_button);
        }
    }
}

void RgStartTab::ReorderTabOrder()
{
    std::vector<QPushButton*> start_buttons;
    GetStartButtons(start_buttons);

    QPushButton* last_start_button = nullptr;
    if (!start_buttons.empty())
    {
        // Initialize to the first start button.
        last_start_button = start_buttons[0];

        // Start at the second button if one exists.
        for (size_t start_button_index = 1; start_button_index < start_buttons.size(); ++start_button_index)
        {
            setTabOrder(last_start_button, start_buttons[start_button_index]);
            last_start_button = start_buttons[start_button_index];
        }
    }

    // Get the recent projects buttons list.
    QList<QAbstractButton*> recent_projects_list;
    if (recent_project_button_group_ != nullptr)
    {
        recent_projects_list = recent_project_button_group_->buttons();
    }

    // If the recent projects list is not empty, add tab order for these projects.
    if (!recent_projects_list.isEmpty())
    {
        setTabOrder(last_start_button, recent_projects_list.at(0));

        for (int i = 1; i < recent_projects_list.count() - 1; i++)
        {
            setTabOrder(recent_projects_list.at(i), recent_projects_list.at(i + 1));
        }
        setTabOrder(recent_projects_list.at(recent_projects_list.count() - 1), ui_.recentProjectsOtherPushButton);
    }
    else
    {
        setTabOrder(last_start_button, ui_.recentProjectsOtherPushButton);
    }

    setTabOrder(ui_.recentProjectsOtherPushButton, ui_.aboutRGAPushButton);
    setTabOrder(ui_.aboutRGAPushButton, ui_.helpManualPushButton);
    setTabOrder(ui_.helpManualPushButton, ui_.gettingStartedPushButton);
}

void RgStartTab::SetCursor()
{
    // Set the cursor to pointing hand cursor.
    ui_.recentProjectsOtherPushButton->setCursor(Qt::PointingHandCursor);
    ui_.aboutRGAPushButton->setCursor(Qt::PointingHandCursor);
    ui_.gettingStartedPushButton->setCursor(Qt::PointingHandCursor);
    ui_.helpManualPushButton->setCursor(Qt::PointingHandCursor);
}

void RgStartTab::SetProjectAPIIcon(RgProjectAPI api, RgRecentProjectWidget* recent_project_widget)
{
    // Get and set the appropriate API icon.
    switch (api)
    {
    case RgProjectAPI::kOpenCL:
    {
        recent_project_widget->SetIcon(QIcon(":/icons/api_logos/opencl_icon_wide.png"));
        recent_project_widget->SetIconProjectType(kStrApiNameOpencl);
    }
    break;
    case RgProjectAPI::kVulkan:
    {
        recent_project_widget->SetIcon(QIcon(":/icons/api_logos/vulkan_icon.png"));
        recent_project_widget->SetIconProjectType(kStrApiNameVulkan);
    }
    break;
    case RgProjectAPI::kBinary:
    {
        recent_project_widget->SetIcon(QIcon(":/icons/api_logos/binary_icon_wide.png"));
        recent_project_widget->SetIconProjectType(kStrApiAbbreviationBinary);
    }
    break;
    case RgProjectAPI::kUnknown:
    default:
    {
        // Should not get here.
        assert(false);
    }
    break;
    }
}

void RgStartTab::PopulateRecentProjectsList()
{
    // Clear the existing list of recent project buttons added to the view.
    ClearRecentProjectsList();

    const std::vector<std::shared_ptr<RgRecentProject>>& recent_projects = RgConfigManager::Instance().GetRecentProjects();

    bool has_recent_projects = recent_projects.size() > 0;

    // Change the visibility of the "No recent sessions" label depending on what's in the settings file.
    ui_.noRecentSessionsDummyPushButton->setVisible(!has_recent_projects);

    if (has_recent_projects)
    {
        // Create a new button group to handle clicks on recent project buttons.
        recent_project_button_group_ = new QButtonGroup(this);
        bool is_connected            = connect(recent_project_button_group_, &QButtonGroup::buttonClicked, this, &RgStartTab::HandleRecentProjectClickedEvent);
        assert(is_connected);

        // If the button group handler is connected correctly, add a button for each recent project entry.
        if (is_connected)
        {
            QLayout* recent_projects_list = ui_.recentProgramsWrapper->layout();
            assert(recent_projects_list != nullptr);

            int num_recent_projects = static_cast<int>(recent_projects.size());

            // The index of the button in the QButtonGroup. This is how the
            // button is being identified within the group when signals are being fired.
            int button_index = num_recent_projects;
            for (std::vector<std::shared_ptr<RgRecentProject>>::const_reverse_iterator project_iter = recent_projects.rbegin();
                project_iter != recent_projects.rend(); ++project_iter)
            {
                if (*project_iter != nullptr)
                {
                    // Display the most recent projects at the end of the list.
                    const std::string& project_path = (*project_iter)->project_path;
                    RgProjectAPI project_api_type = (*project_iter)->api_type;

                    // Extract just the filename to display in the list of recent projects.
                    std::string project_name;
                    bool is_ok = RgUtils::ExtractFileName(project_path, project_name, true);
                    assert(is_ok);

                    if (is_ok)
                    {
                        // Build and set the status tip message.
                        std::stringstream status_tip_msg;
                        status_tip_msg << kStrMainWindowLoadProject << " " << project_name.c_str();

                        // Add the layout to the list of recent project buttons.
                        RgRecentProjectWidget* recent_project_widget = new RgRecentProjectWidget();
                        recent_projects_list->addWidget(recent_project_widget);

                        // Set the text as the project name, and the tooltip as the full path.
                        recent_project_widget->SetProjectName(QString::fromStdString(project_name));
                        recent_project_widget->setToolTip(project_path.c_str());
                        recent_project_widget->setStatusTip(status_tip_msg.str().c_str());

                        // Set the appropriate project API icon.
                        SetProjectAPIIcon(project_api_type, recent_project_widget);

                        // Set the context menu.
                        recent_project_widget->setContextMenuPolicy(Qt::CustomContextMenu);

                        // Connect signal/slot for the context menu.
                        is_connected = connect(recent_project_widget, &RgRecentProjectWidget::customContextMenuRequested, this, &RgStartTab::HandleContextMenuRequest);
                        assert(is_connected);

                        // Add the recent project button to the group.
                        recent_project_button_group_->addButton(recent_project_widget->GetPushButton(), --button_index);
                    }
                }
            }
        }
    }

    // Reorder the tab order to allow for the recent projects list additions.
    ReorderTabOrder();
}

QWidget* RgStartTab::GetRecentProgramsListWidget() const
{
    return ui_.recentProgramsWrapper;
}

// Get the button used to create a new project.
QPushButton* RgStartTab::GetCreateNewFileButton()
{
    QPushButton* button = nullptr;

    std::vector<QPushButton*> start_button;
    GetStartButtons(start_button);

    if (!start_button.empty())
    {
        // Return the first button in the list, which will create a new project.
        button = start_button[0];
    }

    return button;
}

void RgStartTab::HandleContextMenuRequest(const QPoint& pos)
{
    Q_UNUSED(pos);

    // Set context menu mouse cursor
    menu_.setCursor(Qt::PointingHandCursor);

    // The list of actions in the context menu.
    QList<QAction*> menu_actions = menu_.actions();

    // Extract the file name clicked on
    QObject* sender = QObject::sender();
    RgRecentProjectWidget* recent_project_widget = qobject_cast<RgRecentProjectWidget*>(sender);
    assert(recent_project_widget != nullptr);
    if (recent_project_widget != nullptr)
    {
        QString filename = recent_project_widget->GetProjectName();

        foreach(auto action, menu_actions)
        {
            if (action == open_recent_action_)
            {
                // Build the Load <file name> string.
                QString load_string;
                load_string.append(kStrMainWindowLoadProject);
                load_string.append(" ");
                load_string.append(filename);
                action->setText(load_string);
                break;
            }
        }

        QAction* action = menu_.exec(QCursor::pos());

        // Process the context menu selection
        if (action != nullptr)
        {
            QString menu_selection = action->text();

            // Find out the index into the button group for this file
            QList<QAbstractButton *> button_list = recent_project_button_group_->buttons();
            int recent_file_index = 0;

            // Find the index for the button clicked on
            foreach(auto button, button_list)
            {
                recent_file_index++;
                if (filename.compare(button->text()) == 0)
                {
                    break;
                }
            }

            // Determine the index of the recent item that was clicked.
            int selected_file_index = button_list.count() - recent_file_index;
            bool is_index_valid = (selected_file_index >= 0 && selected_file_index < button_list.count());
            assert(is_index_valid);
            if (is_index_valid)
            {
                // Pull the recent project info out of the global settings structure.
                std::shared_ptr<RgGlobalSettings> global_settings = RgConfigManager::Instance().GetGlobalConfig();
                const std::string& project_path = global_settings->recent_projects[selected_file_index]->project_path;

                if (action == open_recent_action_)
                {
                    // Attempt to load the recent project.
                    assert(false);
                    emit OpenProjectFileAtPath(project_path);
                }
                else if (action == open_containing_folder_action_)
                {
                    // Get the directory where the project's settings file lives.
                    std::string file_directory;
                    bool got_directory = RgUtils::ExtractFileDirectory(project_path, file_directory);
                    assert(got_directory);

                    if (got_directory)
                    {
                        // Open a system file browser window pointing to the given directory.
                        RgUtils::OpenFolderInFileBrowser(file_directory);
                    }
                }
            }
        }
    }
}

void RgStartTab::HandleRecentProjectClickedEvent(QAbstractButton* recent_file_button)
{
    std::shared_ptr<RgGlobalSettings> global_settings = RgConfigManager::Instance().GetGlobalConfig();

    if (global_settings != nullptr && recent_project_button_group_ != nullptr && recent_file_button != nullptr)
    {
        int  recent_file_index = recent_project_button_group_->id(recent_file_button);
        bool is_valid_range = (recent_file_index >= 0 && recent_file_index < global_settings->recent_projects.size());
        assert(is_valid_range);

        // If the file index is valid, attempt to open the file.
        if (is_valid_range)
        {
            assert(global_settings->recent_projects[recent_file_index] != nullptr);
            if (global_settings->recent_projects[recent_file_index] != nullptr)
            {
                // Pull the recent project info out of the global settings structure.
                std::string project_path = global_settings->recent_projects[recent_file_index]->project_path;

                // Attempt to load the project.
                emit OpenProjectFileAtPath(project_path);
            }
        }
    }
}

bool RgStartTab::eventFilter(QObject* object, QEvent* event)
{
    assert(object != nullptr);
    assert(event != nullptr);

    bool filtered = false;

    if (event != nullptr && event->type() == QEvent::KeyPress)
    {
        QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
        assert(key_event != nullptr);

        // Find out if we need to process this key press.
        QString object_name;
        if (object != nullptr)
        {
            object_name = object->objectName();
        }
        bool proecss_key_press = ProcessKeyPress(key_event, object_name);

        if (proecss_key_press)
        {
            if (object == ui_.recentProgramsWrapper)
            {
                if (key_event != nullptr)
                {
                    switch (key_event->key())
                    {
                    case Qt::Key_Up:
                        {
                            // Give focus to the last item under the "Start" section.
                            std::vector<QPushButton*> start_buttons;
                            GetStartButtons(start_buttons);
                            if (!start_buttons.empty())
                            {
                                QPushButton* last_start_button = *(start_buttons.rbegin());
                                last_start_button->setFocus();
                            }
                            filtered = true;
                        }
                        break;
                    case Qt::Key_Down:
                        // Give focus to the next widget.
                        ui_.recentProjectsOtherPushButton->setFocus();
                        filtered = true;

                        break;
                    }
                }
            }
        }
        else
        {
            // Ignore up/down key presses for top and bottom buttons.
            filtered = true;
        }
    }

    // Allow base class to filter the event if needed.
    if (!filtered)
    {
        filtered = QWidget::eventFilter(object, event);
    }

    return filtered;
}

bool RgStartTab::ProcessKeyPress(QKeyEvent* key_event, const QString& object_name)
{
    assert(key_event != nullptr);

    static const char* kStrNewFilePushButton = "newFilePushButton";
    static const char* kStrGettingStartedPushButton = "gettingStartedPushButton";

    bool result = true;

    if (key_event != nullptr)
    {
        if (object_name.compare(kStrNewFilePushButton) == 0 && key_event->key() == Qt::Key_Up)
        {
            result = false;
        }
        else if (object_name.compare(kStrGettingStartedPushButton) == 0 && key_event->key() == Qt::Key_Down)
        {
            result = false;
        }
    }

    return result;
}
