// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QKeyEvent>
#include <QtWidgets/QDesktopWidget>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSettingsTab.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgGlobalSettingsView.h>
#include <RadeonGPUAnalyzerGUI/Include/rgConfigManager.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgFactory.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgAppState.h>

rgSettingsTab::rgSettingsTab(QWidget* pParent)
    : QWidget(pParent)
{
    // Setup the UI.
    ui.setupUi(this);
}

void rgSettingsTab::Initialize()
{
    // Create the global settings view and add it to the Settings Tab.
    rgConfigManager& configManager = rgConfigManager::Instance();
    rgProjectAPI currentApi = configManager.GetCurrentAPI();
    std::shared_ptr<rgGlobalSettings> pGlobalSettings = configManager.GetGlobalConfig();

    // Set various properties for the scroll area.
    QPalette palette;
    palette.setColor(QPalette::Background, Qt::GlobalColor::transparent);
    ui.scrollArea->setPalette(palette);
    ui.scrollArea->setFrameShape(QFrame::NoFrame);
    ui.scrollArea->setAlignment(Qt::AlignTop);
    ui.scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui.scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    ScalingManager::Get().RegisterObject(ui.settingsButtonsView);

    // Create the application settings view and add it to the settings tab.
    m_pGlobalSettingsView = new rgGlobalSettingsView(this, *pGlobalSettings);
    assert(m_pGlobalSettingsView != nullptr);
    if (m_pGlobalSettingsView != nullptr)
    {
        AddSettingsView(m_pGlobalSettingsView);
        ScalingManager::Get().RegisterObject(m_pGlobalSettingsView);
    }

    // Create the API-specific build settings view and add it to the Settings Tab.
    m_pBuildSettingsView = CreateApiBuildSettingsView();
    assert(m_pBuildSettingsView != nullptr);
    if (m_pBuildSettingsView != nullptr)
    {
        AddSettingsView(m_pBuildSettingsView);
        ScalingManager::Get().RegisterObject(m_pBuildSettingsView);
    }

    // Add vertical spacer to the scroll area contents to ensure settings are top aligned.
    ui.scrollAreaWidgetContents->layout()->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));

    // Connect signals within the start tab.
    ConnectSignals();

    // Apply style from stylesheet.
    std::vector<std::string> stylesheetFileNames;
    stylesheetFileNames.push_back(STR_MAIN_WINDOW_STYLESHEET_FILE);
    stylesheetFileNames.push_back(STR_APPLICATION_STYLESHEET_FILE);
    rgUtils::LoadAndApplyStyle(stylesheetFileNames, this);

    // Apply the stylesheets for the global settings.
    std::shared_ptr<rgFactory> pFactory = rgFactory::CreateFactory(GetApiType());
    assert(pFactory != nullptr);
    std::shared_ptr<rgAppState> pAppState = pFactory->CreateAppState();
    assert(pAppState != nullptr);
    if (pAppState != nullptr)
    {
        SetGlobalSettingsStylesheet(pAppState->GetGlobalSettingsViewStylesheet());
    }

    // Set the cursor type for specific widgets in the view.
    SetCursor();

    // Set the title for the API-specific build settings.
    UpdateBuildSettingsTitle(false);

    // Resize the list widget to fit its contents.
    const int extraWidthToAvoidScrollbar = 20 * ScalingManager::Get().GetScaleFactor();
    ui.settingsListWidget->setMinimumWidth(ui.settingsListWidget->sizeHintForColumn(0) + extraWidthToAvoidScrollbar);
    ScalingManager::Get().DisableScalingForObject(ui.settingsListWidget);

    // Set the settings list widget's current row to "Global".
    ui.settingsListWidget->setCurrentRow(static_cast<int>(SettingsListWidgetEntries::Global));

    // Set the focus to the settings list widget.
    ui.settingsListWidget->setFocus();

    // Install an event filter to prompt user to save settings when switching pages on the settings tab.
    ui.settingsListWidget->viewport()->installEventFilter(this);

    // Make sure all child objects (except those which were excluded) get registered for scaling.
    ScalingManager::Get().RegisterObject(this);
}

rgBuildSettingsView* rgSettingsTab::CreateApiBuildSettingsView()
{
    // Create an API-specific factory to create an API-specific rgBuildSettingsView.
    std::shared_ptr<rgFactory> pFactory = rgFactory::CreateFactory(GetApiType());
    assert(pFactory != nullptr);

    // Get the API-specific build settings from the rgConfigManager.
    std::shared_ptr<rgBuildSettings> pBuildSettings = rgConfigManager::Instance().GetUserGlobalBuildSettings(GetApiType());
    assert(pBuildSettings != nullptr);

    // If the factory and build settings are valid, then create an API-specific rgBuildSettingsView
    // with the API-specific settings.
    rgBuildSettingsView* pBuildSettingsView = nullptr;
    if (pFactory != nullptr && pBuildSettings != nullptr)
    {
        pBuildSettingsView = pFactory->CreateBuildSettingsView(parentWidget(), pBuildSettings, true);
    }

    return pBuildSettingsView;
}

void rgSettingsTab::AddSettingsView(rgBuildSettingsView* pSettingsView)
{
    assert(pSettingsView != nullptr);
    if (pSettingsView != nullptr)
    {
        // Add the view's title to the list widget.
        QString title(pSettingsView->GetTitleString().c_str());
        ui.settingsListWidget->addItem(title);
        ui.settingsListWidget->item(ui.settingsListWidget->count() - 1)->setToolTip(title);

        // Add the view to the scroll area.
        ui.scrollAreaWidgetContents->layout()->addWidget(pSettingsView);
    }
}

void rgSettingsTab::ConnectSignals()
{
    // Global settings view has pending changes.
    bool isConnected = connect(m_pGlobalSettingsView, &rgGlobalSettingsView::PendingChangesStateChanged, this, &rgSettingsTab::HandleGlobalPendingChangesStateChanged);
    assert(isConnected);

    // "Save" button.
    isConnected = connect(ui.settingsButtonsView, &rgSettingsButtonsView::SaveSettingsButtonClickedSignal, this, &rgSettingsTab::HandleSaveSettingsButtonClicked);
    assert(isConnected);

    // "Restore defaults" button.
    isConnected = connect(ui.settingsButtonsView, &rgSettingsButtonsView::RestoreDefaultSettingsButtonClickedSignal, this, &rgSettingsTab::HandleRestoreDefaultsSettingsClicked);
    assert(isConnected);

    // Connect the default API's settings view's handler for pending changes.
    isConnected = connect(static_cast<rgBuildSettingsView*>(m_pBuildSettingsView), &rgBuildSettingsView::PendingChangesStateChanged, this, &rgSettingsTab::HandleBuildSettingsPendingChangesStateChanged);
    assert(isConnected);

    // Connect the settings list widget to detect clicks.
    isConnected = connect(ui.settingsListWidget, &QListWidget::currentRowChanged, this, &rgSettingsTab::HandleSettingsListWidgetClick);
    assert(isConnected);
}

bool rgSettingsTab::eventFilter(QObject* pObject, QEvent* pEvent)
{
    assert(pObject != nullptr);
    assert(pEvent != nullptr);

    bool isFiltered = false;

    if (pEvent != nullptr)
    {
        const QEvent::Type eventType = pEvent->type();

        if (eventType == QEvent::MouseButtonPress)
        {
            const QMouseEvent* pMouseEvent = static_cast<QMouseEvent*>(pEvent);
            assert(pMouseEvent != nullptr);

            if (pMouseEvent != nullptr)
            {
                const QPoint localPoint = pMouseEvent->localPos().toPoint();
                QListWidgetItem* pItem = ui.settingsListWidget->itemAt(localPoint);
                QListWidgetItem* pCurrentItem = ui.settingsListWidget->currentItem();
                if (pItem != nullptr && pCurrentItem != nullptr)
                {
                    if (pItem != pCurrentItem)
                    {
                        // User clicked on the settings list widget.
                        if (PromptToSavePendingChanges() == false)
                        {
                            // User cancelled the dialog, so the event should be filtered out.
                            isFiltered = true;
                        }
                    }
                }
            }
        }
    }

    // Allow base class to filter the event if needed.
    if (!isFiltered)
    {
        isFiltered = QWidget::eventFilter(pObject, pEvent);
    }

    return isFiltered;
}

void rgSettingsTab::HandleSaveSettingsButtonClicked()
{
    // First verify all inputs before saving them by calling all their handlers.
    assert(m_pGlobalSettingsView != nullptr);
    if (m_pGlobalSettingsView != nullptr)
    {
        m_pGlobalSettingsView->HandleIncludeFilesViewerEditingFinished();
        m_pGlobalSettingsView->HandleLogFileEditingFinished();
    }

    // Disable the "Save" button.
    ui.settingsButtonsView->EnableSaveButton(false);

    assert(m_pGlobalSettingsView != nullptr);
    if (m_pGlobalSettingsView != nullptr && m_pGlobalSettingsView->GetHasPendingChanges())
    {
        // Save global application settings.
        m_pGlobalSettingsView->SaveSettings();
    }

    assert(m_pBuildSettingsView != nullptr);
    if (m_pBuildSettingsView != nullptr && m_pBuildSettingsView->GetHasPendingChanges())
    {
        // Save default build settings.
        m_pBuildSettingsView->SaveSettings();
    }
}

void rgSettingsTab::HandleRestoreDefaultsSettingsClicked()
{
    // Ask the user for confirmation.
    bool isConfirmation = rgUtils::ShowConfirmationMessageBox(STR_BUILD_SETTINGS_DEFAULT_SETTINGS_CONFIRMATION_TITLE, STR_BUILD_SETTINGS_DEFAULT_SETTINGS_CONFIRMATION, this);

    if (isConfirmation)
    {
        // Disable the "Save" button.
        ui.settingsButtonsView->EnableSaveButton(false);

        // Restore the default settings.
        if (m_pGlobalSettingsView != nullptr)
        {
            m_pGlobalSettingsView->RestoreDefaultSettings();
        }

        // Restore default settings for the API
        if (m_pBuildSettingsView != nullptr)
        {
            m_pBuildSettingsView->RestoreDefaultSettings();
        }
    }
}

void rgSettingsTab::UpdateBuildSettingsTitle(bool hasPendingChanges)
{
    QListWidgetItem* pItem = ui.settingsListWidget->item(static_cast<int>(SettingsListWidgetEntries::Api));
    assert(pItem != nullptr);
    if (pItem != nullptr)
    {
        std::string settingsTitle;

        assert(m_pBuildSettingsView != nullptr);
        if (m_pBuildSettingsView != nullptr)
        {
            settingsTitle = m_pBuildSettingsView->GetTitleString();
        }
        else
        {
            settingsTitle = std::string(STR_BUILD_SETTINGS_DEFAULT_TITLE).append(STR_BUILD_SETTINGS_DEFAULT_TITLE_B);
        }

        if (hasPendingChanges)
        {
            pItem->setText(QString(settingsTitle.c_str()) + STR_UNSAVED_FILE_SUFFIX);
        }
        else
        {
            pItem->setText(QString(settingsTitle.c_str()));
        }
    }
}

SettingsListWidgetEntries rgSettingsTab::GetSelectedSettingCategory() const
{
    // Cast the currently-selected row to a settings enum type.
    int currentRow = ui.settingsListWidget->currentRow();
    return static_cast<SettingsListWidgetEntries>(currentRow);
}

bool rgSettingsTab::PromptToSavePendingChanges()
{
    bool result = true;
    if (m_hasPendingChanges)
    {
        rgUnsavedItemsDialog::UnsavedFileDialogResult saveSettingsResult = SaveSettings();

        if (saveSettingsResult == rgUnsavedItemsDialog::UnsavedFileDialogResult::Yes)
        {
            HandleSaveSettingsButtonClicked();
        }
        else if (saveSettingsResult == rgUnsavedItemsDialog::UnsavedFileDialogResult::No)
        {
            RevertPendingChanges();
        }
        else
        {
            // User canceled the prompt.
            result = false;
        }
    }
    return result;
}

void rgSettingsTab::SavePendingChanges()
{
    HandleSaveSettingsButtonClicked();
}

rgUnsavedItemsDialog::UnsavedFileDialogResult rgSettingsTab::SaveSettings()
{
    rgUnsavedItemsDialog::UnsavedFileDialogResult result = rgUnsavedItemsDialog::No;

    if (m_hasBuildPendingChanges || m_hasApplicationPendingChanges)
    {
        // Create a modal unsaved file dialog.
        rgUnsavedItemsDialog* pUnsavedChangesDialog = new rgUnsavedItemsDialog(this);
        assert(pUnsavedChangesDialog != nullptr);
        if (pUnsavedChangesDialog != nullptr)
        {
            pUnsavedChangesDialog->setModal(true);
            pUnsavedChangesDialog->setWindowTitle(STR_UNSAVED_ITEMS_DIALOG_TITLE);

            // Add a message string to the dialog list.
            if (m_hasApplicationPendingChanges)
            {
                pUnsavedChangesDialog->AddFile(STR_SETTINGS_CONFIRMATION_APPLICATION_SETTINGS);
            }
            if (m_hasBuildPendingChanges)
            {
                pUnsavedChangesDialog->AddFile(m_pBuildSettingsView->GetTitleString().c_str());
            }

            // Register the dialog with the scaling manager.
            ScalingManager::Get().RegisterObject(pUnsavedChangesDialog);

            // Center the dialog on the view (registering with the scaling manager
            // shifts it out of the center so we need to manually center it).
            rgUtils::CenterOnWidget(pUnsavedChangesDialog, parentWidget());

            // Execute the dialog and get the result.
            result = static_cast<rgUnsavedItemsDialog::UnsavedFileDialogResult>(pUnsavedChangesDialog->exec());
        }
    }

    return result;
}

void rgSettingsTab::RevertPendingChanges()
{
    // Revert pending changes for the Global Application Settings.
    if (m_pGlobalSettingsView != nullptr)
    {
        m_pGlobalSettingsView->RevertPendingChanges();
    }

    // Revert pending changes for the API-specific Build Settings.
    if (m_pBuildSettingsView != nullptr)
    {
        m_pBuildSettingsView->RevertPendingChanges();
    }
}

void rgSettingsTab::SetGlobalSettingsStylesheet(const std::string& stylesheet)
{
    assert(m_pGlobalSettingsView != nullptr);
    if (m_pGlobalSettingsView != nullptr)
    {
        m_pGlobalSettingsView->setStyleSheet(stylesheet.c_str());
    }
}

void rgSettingsTab::HandleSettingsListWidgetClick(int index)
{
    assert(m_pBuildSettingsView != nullptr);
    assert(m_pGlobalSettingsView != nullptr);

    bool isSaveEnabled = false;

    switch (index)
    {
    case (static_cast<int>(SettingsListWidgetEntries::Global)):
    {
        if (m_pBuildSettingsView != nullptr)
        {
            m_pBuildSettingsView->hide();
        }

        if (m_pGlobalSettingsView != nullptr)
        {
            m_pGlobalSettingsView->show();

            isSaveEnabled = m_pGlobalSettingsView->GetHasPendingChanges();

            m_pGlobalSettingsView->SetInitialWidgetFocus();
        }
    }
    break;
    case (static_cast<int>(SettingsListWidgetEntries::Api)):
    {
        if (m_pGlobalSettingsView != nullptr)
        {
            m_pGlobalSettingsView->hide();
        }

        if (m_pBuildSettingsView != nullptr)
        {
            m_pBuildSettingsView->show();

            isSaveEnabled = m_pBuildSettingsView->GetHasPendingChanges();

            m_pBuildSettingsView->SetInitialWidgetFocus();
        }
    }
    break;
    default:
        // We shouldn't get here.
        assert(false);
        break;
    }

    // Enable or disable the save button.
    ui.settingsButtonsView->EnableSaveButton(isSaveEnabled);

    // Set the list widget cursor to arrow cursor.
    ui.settingsListWidget->setCursor(Qt::ArrowCursor);
}

void rgSettingsTab::HandleBuildSettingsPendingChangesStateChanged(bool hasPendingChanges)
{
    m_hasBuildPendingChanges = hasPendingChanges;

    // Enable the "Save" button located on settings buttons view.
    ui.settingsButtonsView->EnableSaveButton(hasPendingChanges);

    // Update the settings list widget "API" entry.
    UpdateBuildSettingsTitle(hasPendingChanges);

    NotifyOfPendingChanges();
}

void rgSettingsTab::HandleGlobalPendingChangesStateChanged(bool hasPendingChanges)
{
    // Update the "Save" button located on settings buttons view.
    ui.settingsButtonsView->EnableSaveButton(hasPendingChanges);

    // Update the settings list widget "Application" entry.
    QListWidgetItem* pItem = ui.settingsListWidget->item(static_cast<int>(SettingsListWidgetEntries::Global));
    assert(pItem);

    if (pItem != nullptr)
    {
        if (hasPendingChanges)
        {
            pItem->setText(QString(m_pGlobalSettingsView->GetTitleString().c_str()) + STR_UNSAVED_FILE_SUFFIX);
        }
        else
        {
            pItem->setText(QString(m_pGlobalSettingsView->GetTitleString().c_str()));
        }

        m_hasApplicationPendingChanges = hasPendingChanges;

        NotifyOfPendingChanges();
    }
}

void rgSettingsTab::NotifyOfPendingChanges()
{
    bool anyPendingChanges = m_hasApplicationPendingChanges || m_hasBuildPendingChanges;

    // Only emit the signal if the state of the pending changes is different
    // than it was before.
    if (m_hasPendingChanges != anyPendingChanges)
    {
        m_hasPendingChanges = anyPendingChanges;

        emit PendingChangesStateChanged(m_hasPendingChanges);
    }
}

void rgSettingsTab::SetCursor()
{
    // Set the cursor to pointing hand cursor.
    ui.settingsListWidget->setCursor(Qt::PointingHandCursor);
}
