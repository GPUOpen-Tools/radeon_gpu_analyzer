#pragma once

// Local.
#include <ui_rgMenuBuildSettingsItem.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuItem.h>

// Forward declarations:
class QPushButton;

class rgMenuBuildSettingsItem :
    public rgMenuItem
{
    Q_OBJECT

public:
    explicit rgMenuBuildSettingsItem(rgMenu* pParent = nullptr);
    virtual ~rgMenuBuildSettingsItem() = default;

    // Get a pointer to the build settings button within the item.
    QPushButton* GetBuildSettingsButton() const;

    // Mark the item as saved or unsaved.
    void SetHasPendingChanges(bool hasPendingChanges);

    // Alter the visual style of the item if it is currently selected.
    void SetCurrent(bool isCurrent);

    // Get the value of "current" property.
    bool IsCurrent() const;

    // Simulate a click on the menu item.
    void ClickMenuItem() const;

signals:
    // Signal emitted when the build settings button is clicked.
    void BuildSettingsButtonClicked(bool checked);

private slots:
    // Handler for the "Build settings" push button.
    void HandleBuildSettingsButton(bool checked);

private:
    // Set the cursor to the specified type.
    void SetCursor(const QCursor& cursor);

    // Set the text for the build settings item.
    void SetItemText(const std::string& itemText);

    // Change appearance to "focus in".
    void GotFocus();

    // Change appearance to "focus out".
    void LostFocus();

    // Connect signals.
    void ConnectSignals();

    // The Build Settings file item interface.
    Ui::rgMenuBuildSettingsItem ui;

    // Flag to keep track of whether this item is currently selected.
    bool m_current = false;
};