// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QKeyEvent>
#include <QtWidgets/QDesktopWidget>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_settings_tab.h"
#include "radeon_gpu_analyzer_gui/qt/rg_global_settings_view.h"
#include "radeon_gpu_analyzer_gui/rg_config_manager.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_factory.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/qt/rg_app_state.h"

RgSettingsTab::RgSettingsTab(QWidget* parent)
    : QWidget(parent)
{
    // Setup the UI.
    ui_.setupUi(this);
}

void RgSettingsTab::Initialize()
{
    // Create the global settings view and add it to the Settings Tab.
    RgConfigManager& config_manager = RgConfigManager::Instance();
    std::shared_ptr<RgGlobalSettings> global_settings = config_manager.GetGlobalConfig();

    // Set various properties for the scroll area.
    QPalette palette;
    palette.setColor(QPalette::Background, Qt::GlobalColor::transparent);
    ui_.scrollArea->setPalette(palette);
    ui_.scrollArea->setFrameShape(QFrame::NoFrame);
    ui_.scrollArea->setAlignment(Qt::AlignTop);
    ui_.scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui_.scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    ScalingManager::Get().RegisterObject(ui_.settingsButtonsView);

    // Create the application settings view and add it to the settings tab.
    global_settings_view_ = new RgGlobalSettingsView(this, *global_settings);
    assert(global_settings_view_ != nullptr);
    if (global_settings_view_ != nullptr)
    {
        AddSettingsView(global_settings_view_);
        ScalingManager::Get().RegisterObject(global_settings_view_);
    }

    // Create the API-specific build settings view and add it to the Settings Tab.
    build_settings_view_ = CreateApiBuildSettingsView();
    assert(build_settings_view_ != nullptr);
    if (build_settings_view_ != nullptr)
    {
        AddSettingsView(build_settings_view_);
        ScalingManager::Get().RegisterObject(build_settings_view_);
    }

    // Add vertical spacer to the scroll area contents to ensure settings are top aligned.
    ui_.scrollAreaWidgetContents->layout()->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));

    // Connect signals within the start tab.
    ConnectSignals();

    // Apply style from stylesheet.
    std::vector<std::string> stylesheet_file_names;
    stylesheet_file_names.push_back(kStrMainWindowStylesheetFile);
    stylesheet_file_names.push_back(kStrApplicationStylesheetFile);
    RgUtils::LoadAndApplyStyle(stylesheet_file_names, this);

    // Apply the stylesheets for the global settings.
    std::shared_ptr<RgFactory> factory = RgFactory::CreateFactory(GetApiType());
    assert(factory != nullptr);
    std::shared_ptr<RgAppState> app_state = factory->CreateAppState();
    assert(app_state != nullptr);
    if (app_state != nullptr)
    {
        SetGlobalSettingsStylesheet(app_state->GetGlobalSettingsViewStylesheet());
        SetBuildSettingsStylesheet(app_state->GetBuildSettingsViewStylesheet());
    }

    // Set the cursor type for specific widgets in the view.
    SetCursor();

    // Set the title for the API-specific build settings.
    UpdateBuildSettingsTitle(false);

    // Resize the list widget to fit its contents.
    const int extra_width_to_avoid_scrollbar = 20 * ScalingManager::Get().GetScaleFactor();
    ui_.settingsListWidget->setMinimumWidth(ui_.settingsListWidget->sizeHintForColumn(0) + extra_width_to_avoid_scrollbar);
    ScalingManager::Get().DisableScalingForObject(ui_.settingsListWidget);

    // Set the settings list widget's current row to "Global".
    ui_.settingsListWidget->setCurrentRow(static_cast<int>(SettingsListWidgetEntries::kGlobal));

    // Set the focus to the settings list widget.
    ui_.settingsListWidget->setFocus();

    // Install an event filter to prompt user to save settings when switching pages on the settings tab.
    ui_.settingsListWidget->viewport()->installEventFilter(this);

    // Make sure all child objects (except those which were excluded) get registered for scaling.
    ScalingManager::Get().RegisterObject(this);
}

RgBuildSettingsView* RgSettingsTab::CreateApiBuildSettingsView()
{
    // Create an API-specific factory to create an API-specific RgBuildSettingsView.
    std::shared_ptr<RgFactory> factory = RgFactory::CreateFactory(GetApiType());
    assert(factory != nullptr);

    // Get the API-specific build settings from the RgConfigManager.
    std::shared_ptr<RgBuildSettings> build_settings = RgConfigManager::Instance().GetUserGlobalBuildSettings(GetApiType());
    assert(build_settings != nullptr);

    // If the factory and build settings are valid, then create an API-specific RgBuildSettingsView
    // with the API-specific settings.
    RgBuildSettingsView* build_settings_view = nullptr;
    if (factory != nullptr && build_settings != nullptr)
    {
        build_settings_view = factory->CreateBuildSettingsView(parentWidget(), build_settings, true);
    }

    return build_settings_view;
}

void RgSettingsTab::AddSettingsView(RgBuildSettingsView* settings_view)
{
    assert(settings_view != nullptr);
    if (settings_view != nullptr)
    {
        // Add the view's title to the list widget.
        QString title(settings_view->GetTitleString().c_str());
        ui_.settingsListWidget->addItem(title);
        ui_.settingsListWidget->item(ui_.settingsListWidget->count() - 1)->setToolTip(title);

        // Add the view to the scroll area.
        ui_.scrollAreaWidgetContents->layout()->addWidget(settings_view);
    }
}

void RgSettingsTab::ConnectSignals()
{
    // Global settings view has pending changes.
    bool is_connected = connect(global_settings_view_, &RgGlobalSettingsView::PendingChangesStateChanged, this, &RgSettingsTab::HandleGlobalPendingChangesStateChanged);
    assert(is_connected);

    // Global settings view has empty input file names.
    is_connected = connect(global_settings_view_, &RgGlobalSettingsView::InputFileNameBlankSignal, this, &RgSettingsTab::HandleInputFileNameBlank);
    assert(is_connected);

    // "Save" button.
    is_connected = connect(ui_.settingsButtonsView, &RgSettingsButtonsView::SaveSettingsButtonClickedSignal, this, &RgSettingsTab::HandleSaveSettingsButtonClicked);
    assert(is_connected);

    // "Restore default settings" button.
    is_connected = connect(ui_.settingsButtonsView, &RgSettingsButtonsView::RestoreDefaultSettingsButtonClickedSignal, this, &RgSettingsTab::HandleRestoreDefaultsSettingsClicked);
    assert(is_connected);

    // Connect the default API's settings view's handler for pending changes.
    is_connected = connect(static_cast<RgBuildSettingsView*>(build_settings_view_), &RgBuildSettingsView::PendingChangesStateChanged, this, &RgSettingsTab::HandleBuildSettingsPendingChangesStateChanged);
    assert(is_connected);

    // Connect the settings list widget to detect clicks.
    is_connected = connect(ui_.settingsListWidget, &QListWidget::currentRowChanged, this, &RgSettingsTab::HandleSettingsListWidgetClick);
    assert(is_connected);

    // Connect the settings command line update.
    is_connected = connect(this, &RgSettingsTab::UpdateCommandLineTextSignal, build_settings_view_, &RgBuildSettingsView::UpdateCommandLineText);
    assert(is_connected);
}

void RgSettingsTab::HandleInputFileNameBlank(bool is_blank)
{
    // Disable clicking on the list widget.
    if (is_blank)
    {
        ui_.settingsListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    }
    else
    {
        ui_.settingsListWidget->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    }
}

bool RgSettingsTab::eventFilter(QObject* object, QEvent* event)
{
    assert(object != nullptr);
    assert(event != nullptr);

    bool is_filtered = false;

    if (event != nullptr)
    {
        const QEvent::Type eventType = event->type();

        if (eventType == QEvent::MouseButtonPress)
        {
            const QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
            assert(mouse_event != nullptr);

            if (mouse_event != nullptr)
            {
                const QPoint local_point = mouse_event->localPos().toPoint();
                QListWidgetItem* item = ui_.settingsListWidget->itemAt(local_point);
                QListWidgetItem* current_item = ui_.settingsListWidget->currentItem();
                if (item != nullptr && current_item != nullptr)
                {
                    if (item != current_item)
                    {
                        if (global_settings_view_->IsInputFileBlank() == false && PromptToSavePendingChanges() == false)
                        {
                            // User canceled the dialog, so the event should be filtered out.
                            is_filtered = true;
                        }
                    }
                }
            }
        }
    }

    // Allow base class to filter the event if needed.
    if (!is_filtered)
    {
        is_filtered = QWidget::eventFilter(object, event);
    }

    return is_filtered;
}

void RgSettingsTab::SelectNextListWidgetItem(const int key_pressed)
{
    const int current_row = ui_.settingsListWidget->currentRow();
    if (key_pressed == Qt::Key_Up)
    {
        // Process the key up event only if the existing selection is not the top one.
        if (current_row == static_cast<int>(SettingsListWidgetEntries::kApi))
        {
            const int next_row = static_cast<int>(SettingsListWidgetEntries::kGlobal);
            ui_.settingsListWidget->setCurrentRow(next_row);
        }
    }
    else if (key_pressed == Qt::Key_Down)
    {
        // Process the key down event only if the existing selection is not the bottom one.
        if (current_row == static_cast<int>(SettingsListWidgetEntries::kGlobal))
        {
            const int nextRow = static_cast<int>(SettingsListWidgetEntries::kApi);
            ui_.settingsListWidget->setCurrentRow(nextRow);
        }
    }
}

void RgSettingsTab::SaveSettings()
{
    // First verify all inputs before saving them by calling all their handlers.
    assert(global_settings_view_ != nullptr);
    if (global_settings_view_ != nullptr)
    {
        global_settings_view_->HandleIncludeFilesViewerEditingFinished();
        global_settings_view_->HandleLogFileEditingFinished();
    }

    // Disable the "Save" button.
    ui_.settingsButtonsView->EnableSaveButton(false);

    assert(global_settings_view_ != nullptr);
    if (global_settings_view_ != nullptr && global_settings_view_->GetHasPendingChanges())
    {
        // Save global application settings.
        global_settings_view_->SaveSettings();
    }

    assert(build_settings_view_ != nullptr);
    if (build_settings_view_ != nullptr && build_settings_view_->GetHasPendingChanges())
    {
        // Save default build settings.
        build_settings_view_->SaveSettings();
    }
}

void RgSettingsTab::HandleSaveSettingsButtonClicked()
{
    PromptToSavePendingChanges();
}

void RgSettingsTab::HandleRestoreDefaultsSettingsClicked()
{
    // Ask the user for confirmation.
    bool is_confirmation = RgUtils::ShowConfirmationMessageBox(kStrBuildSettingsDefaultSettingsConfirmationTitle, kStrBuildSettingsDefaultSettingsConfirmation, this);

    if (is_confirmation)
    {
        // Disable the "Save" button.
        ui_.settingsButtonsView->EnableSaveButton(false);

        // Only restore the settings for the current view.
        const SettingsListWidgetEntries current_row = static_cast<SettingsListWidgetEntries>(ui_.settingsListWidget->currentRow());
        switch (current_row)
        {
        case (SettingsListWidgetEntries::kGlobal):
        {
            // Restore default values for global settings.
            if (global_settings_view_ != nullptr)
            {
                global_settings_view_->RestoreDefaultSettings();
            }
        }
        break;
        case (SettingsListWidgetEntries::kApi):
        {
            // Restore default values for the default API settings.
            if (build_settings_view_ != nullptr)
            {
                build_settings_view_->RestoreDefaultSettings();

                // Also update the settings command line.
                emit UpdateCommandLineTextSignal();
            }
        }
        break;
        default:
            // We shouldn't get here.
            assert(false);
            break;
        }
    }
}

void RgSettingsTab::UpdateBuildSettingsTitle(bool has_pending_changes)
{
    QListWidgetItem* item = ui_.settingsListWidget->item(static_cast<int>(SettingsListWidgetEntries::kApi));
    assert(item != nullptr);
    if (item != nullptr)
    {
        std::string settings_title;

        assert(build_settings_view_ != nullptr);
        if (build_settings_view_ != nullptr)
        {
            settings_title = build_settings_view_->GetTitleString();
        }
        else
        {
            settings_title = std::string(kStrBuildSettingsDefaultTitle).append(kStrBuildSettingsDefaultTitle);
        }

        if (has_pending_changes)
        {
            item->setText(QString(settings_title.c_str()) + kStrUnsavedFileSuffix);
        }
        else
        {
            item->setText(QString(settings_title.c_str()));
        }
    }
}

SettingsListWidgetEntries RgSettingsTab::GetSelectedSettingCategory() const
{
    // Cast the currently-selected row to a settings enum type.
    int current_row = ui_.settingsListWidget->currentRow();
    return static_cast<SettingsListWidgetEntries>(current_row);
}

bool RgSettingsTab::PromptToSavePendingChanges()
{
    bool result = true;
    if (has_pending_changes_)
    {
        RgUnsavedItemsDialog::UnsavedFileDialogResult save_settings_result = ShowSaveSettingsConfirmationDialog();

        if (save_settings_result == RgUnsavedItemsDialog::UnsavedFileDialogResult::kYes)
        {
            SaveSettings();
        }
        else if (save_settings_result == RgUnsavedItemsDialog::UnsavedFileDialogResult::kNo)
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

void RgSettingsTab::SavePendingChanges()
{
    PromptToSavePendingChanges();
}

RgUnsavedItemsDialog::UnsavedFileDialogResult RgSettingsTab::ShowSaveSettingsConfirmationDialog()
{
    RgUnsavedItemsDialog::UnsavedFileDialogResult result = RgUnsavedItemsDialog::kNo;

    if (has_build_pending_changes_ || has_application_pending_changes_)
    {
        // Create a modal unsaved file dialog.
        RgUnsavedItemsDialog* unsaved_changes_dialog = new RgUnsavedItemsDialog(this);
        assert(unsaved_changes_dialog != nullptr);
        if (unsaved_changes_dialog != nullptr)
        {
            unsaved_changes_dialog->setModal(true);
            unsaved_changes_dialog->setWindowTitle(kStrUnsavedItemsDialogTitle);

            // Add a message string to the dialog list.
            if (has_application_pending_changes_)
            {
                unsaved_changes_dialog->AddFile(kStrSettingsConfirmationApplicationSettings);
            }
            if (has_build_pending_changes_)
            {
                unsaved_changes_dialog->AddFile(build_settings_view_->GetTitleString().c_str());
            }

            // Register the dialog with the scaling manager.
            ScalingManager::Get().RegisterObject(unsaved_changes_dialog);

            // Center the dialog on the view (registering with the scaling manager
            // shifts it out of the center so we need to manually center it).
            RgUtils::CenterOnWidget(unsaved_changes_dialog, parentWidget());

            // Execute the dialog and get the result.
            result = static_cast<RgUnsavedItemsDialog::UnsavedFileDialogResult>(unsaved_changes_dialog->exec());

            // Set the focus to settings tab so keyboard shortcuts work.
            setFocus();
        }
    }

    return result;
}

void RgSettingsTab::RevertPendingChanges()
{
    // Revert pending changes for the Global Application Settings.
    if (global_settings_view_ != nullptr)
    {
        global_settings_view_->RevertPendingChanges();
    }

    // Revert pending changes for the API-specific Build Settings.
    if (build_settings_view_ != nullptr)
    {
        build_settings_view_->RevertPendingChanges();
    }
}

void RgSettingsTab::SetGlobalSettingsStylesheet(const std::string& stylesheet)
{
    assert(global_settings_view_ != nullptr);
    if (global_settings_view_ != nullptr)
    {
        global_settings_view_->setStyleSheet(stylesheet.c_str());
    }
}

void RgSettingsTab::SetBuildSettingsStylesheet(const std::string& stylesheet)
{
    assert(build_settings_view_ != nullptr);
    if (build_settings_view_ != nullptr)
    {
        build_settings_view_->setStyleSheet(stylesheet.c_str());
    }
}

void RgSettingsTab::HandleSettingsListWidgetClick(int index)
{
    assert(build_settings_view_ != nullptr);
    assert(global_settings_view_ != nullptr);

    bool is_saved_enabled = false;

    if (global_settings_view_->IsInputFileBlank() == false)
    {
        switch (index)
        {
        case (static_cast<int>(SettingsListWidgetEntries::kGlobal)):
        {
            if (build_settings_view_ != nullptr)
            {
                build_settings_view_->hide();
            }

            if (global_settings_view_ != nullptr)
            {
                global_settings_view_->show();

                is_saved_enabled = global_settings_view_->GetHasPendingChanges();

                global_settings_view_->SetInitialWidgetFocus();
            }
        }
        break;
        case (static_cast<int>(SettingsListWidgetEntries::kApi)):
        {
            if (global_settings_view_ != nullptr)
            {
                global_settings_view_->hide();
            }

            if (build_settings_view_ != nullptr)
            {
                build_settings_view_->show();

                is_saved_enabled = build_settings_view_->GetHasPendingChanges();

                build_settings_view_->SetInitialWidgetFocus();
            }
        }
        break;
        default:
            // We shouldn't get here.
            assert(false);
            break;
        }

        // Enable or disable the save button.
        ui_.settingsButtonsView->EnableSaveButton(is_saved_enabled);

        // Set the list widget cursor to arrow cursor.
        ui_.settingsListWidget->setCursor(Qt::ArrowCursor);

        // Set the focus to settings buttons view.
        ui_.settingsButtonsView->setFocus();
    }
    else
    {
        if (global_settings_view_ != nullptr)
        {
            // Reset the current item for the list widget to the previous one.
            switch (index)
            {
            case (static_cast<int>(SettingsListWidgetEntries::kGlobal)):
            {
                QSignalBlocker signal_blocker(ui_.settingsListWidget);
                ui_.settingsListWidget->selectionModel()->setCurrentIndex(ui_.settingsListWidget->model()->index(static_cast<int>(SettingsListWidgetEntries::kApi), 0), QItemSelectionModel::SelectionFlag::Select);
            }
            break;
            case (static_cast<int>(SettingsListWidgetEntries::kApi)):
            {
                QSignalBlocker signal_blocker(ui_.settingsListWidget);
                ui_.settingsListWidget->selectionModel()->setCurrentIndex(ui_.settingsListWidget->model()->index(static_cast<int>(SettingsListWidgetEntries::kGlobal), 0), QItemSelectionModel::SelectionFlag::Select);
            }
            break;
            default:
                // We shouldn't get here.
                assert(false);
                break;
            }

            // Display the empty input file name message box.
            global_settings_view_->ProcessInputFileBlank();
        }
    }
}

void RgSettingsTab::HandleBuildSettingsPendingChangesStateChanged(bool has_pending_changes)
{
    has_build_pending_changes_ = has_pending_changes;

    // Enable the "Save" button located on settings buttons view.
    ui_.settingsButtonsView->EnableSaveButton(has_pending_changes);

    // Update the settings list widget "API" entry.
    UpdateBuildSettingsTitle(has_pending_changes);

    NotifyOfPendingChanges();
}

void RgSettingsTab::HandleGlobalPendingChangesStateChanged(bool has_pending_changes)
{
    // Update the "Save" button located on settings buttons view.
    ui_.settingsButtonsView->EnableSaveButton(has_pending_changes);

    // Update the settings list widget "Application" entry.
    QListWidgetItem* item = ui_.settingsListWidget->item(static_cast<int>(SettingsListWidgetEntries::kGlobal));
    assert(item);

    if (item != nullptr)
    {
        if (has_pending_changes)
        {
            item->setText(QString(global_settings_view_->GetTitleString().c_str()) + kStrUnsavedFileSuffix);
        }
        else
        {
            item->setText(QString(global_settings_view_->GetTitleString().c_str()));
        }

        has_application_pending_changes_ = has_pending_changes;

        NotifyOfPendingChanges();
    }
}

void RgSettingsTab::NotifyOfPendingChanges()
{
    bool any_pending_changes = has_application_pending_changes_ || has_build_pending_changes_;

    // Only emit the signal if the state of the pending changes is different
    // than it was before.
    if (has_pending_changes_ != any_pending_changes)
    {
        has_pending_changes_ = any_pending_changes;

        emit PendingChangesStateChanged(has_pending_changes_);
    }
}

void RgSettingsTab::SetCursor()
{
    // Set the cursor to pointing hand cursor.
    ui_.settingsListWidget->setCursor(Qt::PointingHandCursor);
}

RgListWidget* RgSettingsTab::GetSettingsListWidget()
{
    return ui_.settingsListWidget;
}

RgGlobalSettingsView* RgSettingsTab::GetGlobalSettingsView()
{
    return global_settings_view_;
}

