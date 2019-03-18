// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QKeyEvent>
#include <QButtonGroup>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgRecentProjectWidget.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgStartTab.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMainWindow.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>

rgStartTab::rgStartTab(QWidget* pParent)
    : QWidget(pParent),
    m_pParent(pParent)
{
    // Setup the UI.
    ui.setupUi(this);

    // Set the cursor for the context menu.
    m_menu.setCursor(Qt::PointingHandCursor);
}

void rgStartTab::Initialize()
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

void rgStartTab::ApplyStringConstants()
{
    // Apply string constants that apply to a specific API.
    ApplyApiStringConstants();

    // Set tooltips and status tips.
    rgUtils::SetToolAndStatusTip(STR_MENU_BAR_OPEN_PROJECT_TOOLTIP, ui.recentProjectsOtherPushButton);
    rgUtils::SetToolAndStatusTip(STR_MENU_BAR_HELP_ABOUT_TOOLTIP, ui.aboutRGAPushButton);
    rgUtils::SetToolAndStatusTip(STR_MENU_BAR_HELP_MANUAL_TOOLTIP, ui.helpManualPushButton);
    rgUtils::SetToolAndStatusTip(STR_MENU_BAR_HELP_GETTING_STARTED_GUIDE_TOOLTIP, ui.gettingStartedPushButton);
}

void rgStartTab::ConnectSignals()
{
    // Open recent project with the "Other..." button.
    bool isConnected = connect(this->ui.recentProjectsOtherPushButton, &QPushButton::pressed, this, &rgStartTab::OpenProjectFileEvent);
    assert(isConnected);

    // About RGA.
    isConnected = connect(this->ui.aboutRGAPushButton, &QPushButton::pressed, this, &rgStartTab::AboutEvent);
    assert(isConnected);

    // Getting started guide.
    isConnected = connect(this->ui.gettingStartedPushButton, &QPushButton::pressed, this, &rgStartTab::GettingStartedGuideEvent);
    assert(isConnected);

    // Help manual.
    isConnected = connect(this->ui.helpManualPushButton, &QPushButton::pressed, this, &rgStartTab::HelpManual);
    assert(isConnected);
}

void rgStartTab::ClearRecentProjectsList()
{
    // Only clear the list if it already exists.
    if (m_pRecentProjectButtonGroup != nullptr)
    {
        // Remove all Recent Project buttons added to the list.
        QLayout* pRecentProjectsList = ui.recentProgramsWrapper->layout();
        assert(pRecentProjectsList != nullptr);

        // Delete the custom recent projects widgets.
        QList<rgRecentProjectWidget*> widgetList = ui.recentProgramsWrapper->findChildren<rgRecentProjectWidget*>();
        for (rgRecentProjectWidget* pRecentProjectWidget : widgetList)
        {
            assert(pRecentProjectWidget != nullptr);
            if (pRecentProjectWidget != nullptr)
            {
                // Remove each button widget from the list of recent projects, and destroy it.
                pRecentProjectsList->removeWidget(pRecentProjectWidget);
                RG_SAFE_DELETE(pRecentProjectWidget);
            }
        }

        // Destroy the recent projects list button group.
        RG_SAFE_DELETE(m_pRecentProjectButtonGroup);

        // Reorder the tab order to allow for the recent projects list removals.
        ReorderTabOrder();
    }
}

void rgStartTab::CreateContextMenu()
{
    m_pOpenRecentAction = new QAction(STR_MAIN_WINDOW_LOAD_PROJECT, nullptr);
    m_menu.addAction(m_pOpenRecentAction);

    // Add a separator between the current menu items.
    m_menu.addSeparator();

    m_pOpenContainingFolderAction = new QAction(STR_FILE_CONTEXT_MENU_OPEN_CONTAINING_FOLDER, nullptr);
    m_menu.addAction(m_pOpenContainingFolderAction);
}

void rgStartTab::InitializeStartButtons()
{
    QVBoxLayout* pVerticalButtonsLayout = ui.startLinkLayout;
    assert(pVerticalButtonsLayout != nullptr);
    if (pVerticalButtonsLayout != nullptr)
    {
        // Retrieve the list of start buttons to add to the tab.
        std::vector<QPushButton*> startButtons;
        GetStartButtons(startButtons);

        // Add the Start buttons to the view.
        for (auto pStartButton : startButtons)
        {
            // Set the cursor for each of the start buttons being added to the start tab.
            pStartButton->setCursor(Qt::PointingHandCursor);

            // Add the widget to the start tab and the ScalingManager.
            pVerticalButtonsLayout->addWidget(pStartButton);
            ScalingManager::Get().RegisterObject(pStartButton);
        }
    }
}

void rgStartTab::ReorderTabOrder()
{
    std::vector<QPushButton*> startButtons;
    GetStartButtons(startButtons);

    QPushButton* pLastStartButton = nullptr;
    if (!startButtons.empty())
    {
        // Initialize to the first start button.
        pLastStartButton = startButtons[0];

        // Start at the second button if one exists.
        for (size_t startButtonIndex = 1; startButtonIndex < startButtons.size(); ++startButtonIndex)
        {
            setTabOrder(pLastStartButton, startButtons[startButtonIndex]);
            pLastStartButton = startButtons[startButtonIndex];
        }
    }

    // Get the recent projects buttons list.
    QList<QAbstractButton*> recentProjectsList;
    if (m_pRecentProjectButtonGroup != nullptr)
    {
        recentProjectsList = m_pRecentProjectButtonGroup->buttons();
    }

    // If the recent projects list is not empty, add tab order for these projects.
    if (!recentProjectsList.isEmpty())
    {
        setTabOrder(pLastStartButton, recentProjectsList.at(0));

        for (int i = 1; i < recentProjectsList.count() - 1; i++)
        {
            setTabOrder(recentProjectsList.at(i), recentProjectsList.at(i + 1));
        }
        setTabOrder(recentProjectsList.at(recentProjectsList.count() - 1), ui.recentProjectsOtherPushButton);
    }
    else
    {
        setTabOrder(pLastStartButton, ui.recentProjectsOtherPushButton);
    }

    setTabOrder(ui.recentProjectsOtherPushButton, ui.aboutRGAPushButton);
    setTabOrder(ui.aboutRGAPushButton, ui.helpManualPushButton);
    setTabOrder(ui.helpManualPushButton, ui.gettingStartedPushButton);
}

void rgStartTab::SetCursor()
{
    // Set the cursor to pointing hand cursor.
    ui.recentProjectsOtherPushButton->setCursor(Qt::PointingHandCursor);
    ui.aboutRGAPushButton->setCursor(Qt::PointingHandCursor);
    ui.gettingStartedPushButton->setCursor(Qt::PointingHandCursor);
    ui.helpManualPushButton->setCursor(Qt::PointingHandCursor);
}

void rgStartTab::SetProjectAPIIcon(rgProjectAPI api, rgRecentProjectWidget* pRecentProjectWidget)
{
    // Get and set the appropriate API icon.
    switch (api)
    {
    case rgProjectAPI::OpenCL:
    {
        pRecentProjectWidget->SetIcon(QIcon(":/icons/ApiLogos/openCLIconWide.png"));
        pRecentProjectWidget->SetIconProjectType(STR_API_NAME_OPENCL);
    }
    break;
    case rgProjectAPI::Vulkan:
    {
        pRecentProjectWidget->SetIcon(QIcon(":/icons/ApiLogos/vulkanIcon.png"));
        pRecentProjectWidget->SetIconProjectType(STR_API_NAME_VULKAN);
    }
    break;
    case rgProjectAPI::Unknown:
    default:
    {
        // Should not get here.
        assert(false);
    }
    break;
    }
}

void rgStartTab::PopulateRecentProjectsList()
{
    // Clear the existing list of recent project buttons added to the view.
    ClearRecentProjectsList();

    const std::vector<std::shared_ptr<rgRecentProject>>& recentProjects = rgConfigManager::Instance().GetRecentProjects();

    bool hasRecentProjects = recentProjects.size() > 0;

    // Change the visibility of the "No recent sessions" label depending on what's in the settings file.
    ui.noRecentSessionsDummyPushButton->setVisible(!hasRecentProjects);

    if (hasRecentProjects)
    {
        // Create a new button group to handle clicks on recent project buttons.
        m_pRecentProjectButtonGroup = new QButtonGroup(this);
        bool isConnected = connect(m_pRecentProjectButtonGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
            this, &rgStartTab::HandleRecentProjectClickedEvent);
        assert(isConnected);

        // If the button group handler is connected correctly, add a button for each recent project entry.
        if (isConnected)
        {
            QLayout* pRecentProjectsList = ui.recentProgramsWrapper->layout();
            assert(pRecentProjectsList != nullptr);

            int numRecentProjects = static_cast<int>(recentProjects.size());

            // The index of the button in the QButtonGroup. This is how the
            // button is being identified within the group when signals are being fired.
            int buttonIndex = numRecentProjects;
            for (std::vector<std::shared_ptr<rgRecentProject>>::const_reverse_iterator projectIter = recentProjects.rbegin();
                projectIter != recentProjects.rend(); ++projectIter)
            {
                if (*projectIter != nullptr)
                {
                    // Display the most recent projects at the end of the list.
                    const std::string& projectPath = (*projectIter)->projectPath;
                    rgProjectAPI projectApiType = (*projectIter)->apiType;

                    // Extract just the filename to display in the list of recent projects.
                    std::string projectName;
                    bool isOk = rgUtils::ExtractFileName(projectPath, projectName, true);
                    assert(isOk);

                    if (isOk)
                    {
                        // Build and set the status tip message.
                        std::stringstream statusTipMsg;
                        statusTipMsg << STR_MAIN_WINDOW_LOAD_PROJECT << " " << projectName.c_str();

                        // Add the layout to the list of recent project buttons.
                        rgRecentProjectWidget* pRecentProjectWidget = new rgRecentProjectWidget();
                        pRecentProjectsList->addWidget(pRecentProjectWidget);

                        // Set the text as the project name, and the tooltip as the full path.
                        pRecentProjectWidget->SetProjectName(QString::fromStdString(projectName));
                        pRecentProjectWidget->setToolTip(projectPath.c_str());
                        pRecentProjectWidget->setStatusTip(statusTipMsg.str().c_str());

                        // Set the appropriate project API icon.
                        SetProjectAPIIcon(projectApiType, pRecentProjectWidget);

                        // Set the context menu.
                        pRecentProjectWidget->setContextMenuPolicy(Qt::CustomContextMenu);

                        // Connect signal/slot for the context menu.
                        bool isConnected = connect(pRecentProjectWidget, &rgRecentProjectWidget::customContextMenuRequested, this, &rgStartTab::HandleContextMenuRequest);
                        assert(isConnected);

                        // Add the recent project button to the group.
                        m_pRecentProjectButtonGroup->addButton(pRecentProjectWidget->GetPushButton(), --buttonIndex);
                    }
                }
            }
        }
    }

    // Reorder the tab order to allow for the recent projects list additions.
    ReorderTabOrder();
}

QWidget* rgStartTab::GetRecentProgramsListWidget() const
{
    return ui.recentProgramsWrapper;
}

// Get the button used to create a new project.
QPushButton* rgStartTab::GetCreateNewFileButton()
{
    QPushButton* pButton = nullptr;

    std::vector<QPushButton*> startButtons;
    GetStartButtons(startButtons);

    if (!startButtons.empty())
    {
        // Return the first button in the list, which will create a new project.
        pButton = startButtons[0];
    }

    return pButton;
}

void rgStartTab::HandleContextMenuRequest(const QPoint& pos)
{
    // Set context menu mouse cursor
    m_menu.setCursor(Qt::PointingHandCursor);

    // The list of actions in the context menu.
    QList<QAction*> menuActions = m_menu.actions();

    // Extract the file name clicked on
    QObject* pSender = QObject::sender();
    rgRecentProjectWidget* pRecentProjectWidget = qobject_cast<rgRecentProjectWidget*>(pSender);
    assert(pRecentProjectWidget != nullptr);
    if (pRecentProjectWidget != nullptr)
    {
        QString fileName = pRecentProjectWidget->GetProjectName();

        foreach(auto pAction, menuActions)
        {
            if (pAction == m_pOpenRecentAction)
            {
                // Build the Load <file name> string.
                QString loadString;
                loadString.append(STR_MAIN_WINDOW_LOAD_PROJECT);
                loadString.append(" ");
                loadString.append(fileName);
                pAction->setText(loadString);
                break;
            }
        }

        QAction* pAction = m_menu.exec(QCursor::pos());

        // Process the context menu selection
        if (pAction != nullptr)
        {
            QString menuSelection = pAction->text();

            // Find out the index into the button group for this file
            QList<QAbstractButton *> buttonList = m_pRecentProjectButtonGroup->buttons();
            int recentFileIndex = 0;

            // Find the index for the button clicked on
            foreach(auto pButton, buttonList)
            {
                recentFileIndex++;
                if (fileName.compare(pButton->text()) == 0)
                {
                    break;
                }
            }

            // Determine the index of the recent item that was clicked.
            int selectedFileIndex = buttonList.count() - recentFileIndex;
            bool isIndexValid = (selectedFileIndex >= 0 && selectedFileIndex < buttonList.count());
            assert(isIndexValid);
            if (isIndexValid)
            {
                // Pull the recent project info out of the global settings structure.
                std::shared_ptr<rgGlobalSettings> pGlobalSettings = rgConfigManager::Instance().GetGlobalConfig();
                const std::string& projectPath = pGlobalSettings->m_recentProjects[selectedFileIndex]->projectPath;

                if (pAction == m_pOpenRecentAction)
                {
                    // Attempt to load the recent project.
                    assert(false);
                    emit OpenProjectFileAtPath(projectPath);
                }
                else if (pAction == m_pOpenContainingFolderAction)
                {
                    // Get the directory where the project's settings file lives.
                    std::string fileDirectory;
                    bool gotDirectory = rgUtils::ExtractFileDirectory(projectPath, fileDirectory);
                    assert(gotDirectory);

                    if (gotDirectory)
                    {
                        // Open a system file browser window pointing to the given directory.
                        rgUtils::OpenFolderInFileBrowser(fileDirectory);
                    }
                }
            }
        }
    }
}

void rgStartTab::HandleRecentProjectClickedEvent(int recentFileIndex)
{
    std::shared_ptr<rgGlobalSettings> pGlobalSettings = rgConfigManager::Instance().GetGlobalConfig();

    if (pGlobalSettings != nullptr)
    {
        bool isValidRange = (recentFileIndex >= 0 && recentFileIndex < pGlobalSettings->m_recentProjects.size());
        assert(isValidRange);

        // If the file index is valid, attempt to open the file.
        if (isValidRange)
        {
            assert(pGlobalSettings->m_recentProjects[recentFileIndex] != nullptr);
            if (pGlobalSettings->m_recentProjects[recentFileIndex] != nullptr)
            {
                // Pull the recent project info out of the global settings structure.
                std::string projectPath = pGlobalSettings->m_recentProjects[recentFileIndex]->projectPath;

                // Attempt to load the project.
                emit OpenProjectFileAtPath(projectPath);
            }
        }
    }
}

bool rgStartTab::eventFilter(QObject* pObject, QEvent* pEvent)
{
    assert(pObject != nullptr);
    assert(pEvent != nullptr);

    bool filtered = false;

    if (pEvent != nullptr && pEvent->type() == QEvent::KeyPress)
    {
        QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(pEvent);
        assert(pKeyEvent != nullptr);

        // Find out if we need to process this key press.
        QString objectName;
        if (pObject != nullptr)
        {
            objectName = pObject->objectName();
        }
        bool processKeyPress = ProcessKeyPress(pKeyEvent, objectName);

        if (processKeyPress)
        {
            if (pObject == ui.recentProgramsWrapper)
            {
                if (pKeyEvent != nullptr)
                {
                    switch (pKeyEvent->key())
                    {
                    case Qt::Key_Up:
                        {
                            // Give focus to the last item under the "Start" section.
                            std::vector<QPushButton*> startButtons;
                            GetStartButtons(startButtons);
                            if (!startButtons.empty())
                            {
                                QPushButton* pLastStartButton = *(startButtons.rbegin());
                                pLastStartButton->setFocus();
                            }
                            filtered = true;
                        }
                        break;
                    case Qt::Key_Down:
                        // Give focus to the next widget.
                        ui.recentProjectsOtherPushButton->setFocus();
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
        filtered = QWidget::eventFilter(pObject, pEvent);
    }

    return filtered;
}

bool rgStartTab::ProcessKeyPress(QKeyEvent* pKeyEvent, const QString& objectName)
{
    assert(pKeyEvent != nullptr);

    static const char* STR_NEW_FILE_PUSH_BUTTON = "newFilePushButton";
    static const char* STR_GETTING_STARTED_PUSH_BUTTON = "gettingStartedPushButton";

    bool result = true;

    if (pKeyEvent != nullptr)
    {
        if (objectName.compare(STR_NEW_FILE_PUSH_BUTTON) == 0 && pKeyEvent->key() == Qt::Key_Up)
        {
            result = false;
        }
        else if (objectName.compare(STR_GETTING_STARTED_PUSH_BUTTON) == 0 && pKeyEvent->key() == Qt::Key_Down)
        {
            result = false;
        }
    }

    return result;
}