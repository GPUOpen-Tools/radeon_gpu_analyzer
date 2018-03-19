#pragma once

// Local.
#include <ui_rgFileMenuBuildSettingsItem.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgFileMenuItem.h>

// Forward declarations:
class QPushButton;

class rgFileMenuBuildSettingsItem :
    public rgFileMenuItem
{
    Q_OBJECT

public:
    explicit rgFileMenuBuildSettingsItem(rgFileMenu* pParent = nullptr);
    virtual ~rgFileMenuBuildSettingsItem() = default;

    // Get a pointer to the build settings button within the item.
    QPushButton* GetBuildSettingsButton() const;

    // Mark the item as saved or unsaved.
    void SetHasPendingChanges(bool hasPendingChanges);

private slots:
    // Handler for the "Build settings" push button.
    void HandleBuildSettingsButton(bool checked);

private:
    // Set the text for the build settings item.
    void SetItemText(const std::string& itemText);

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // Connect signals.
    void ConnectSignals();

    // The Build Settings file item interface.
    Ui::rgFileMenuBuildSettingsItem ui;
};