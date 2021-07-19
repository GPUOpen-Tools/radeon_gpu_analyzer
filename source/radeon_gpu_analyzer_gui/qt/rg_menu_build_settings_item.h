#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_BUILD_SETTINGS_ITEM_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_BUILD_SETTINGS_ITEM_H_

// Local.
#include "ui_rg_menu_build_settings_item.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_menu_item.h"

// Forward declarations:
class QPushButton;

class RgMenuBuildSettingsItem :
    public RgMenuItem
{
    Q_OBJECT

public:
    explicit RgMenuBuildSettingsItem(RgMenu* parent = nullptr);
    virtual ~RgMenuBuildSettingsItem() = default;

    // Get a pointer to the build settings button within the item.
    QPushButton* GetBuildSettingsButton() const;

    // Mark the item as saved or unsaved.
    void SetHasPendingChanges(bool has_pending_changes);

    // Alter the visual style of the item if it is currently selected.
    void SetCurrent(bool is_current);

    // Get the value of "current" property.
    bool IsCurrent() const;

    // Simulate a click on the menu item.
    void ClickMenuItem() const;

signals:
    // Signal emitted when the build settings button is clicked.
    void BuildSettingsButtonClicked(bool checked);

    // A signal to indicate if the pipeline state option should be enabled.
    void EnablePipelineMenuItem(bool is_enabled);

    // A signal to indicate if the build settings option should be enabled.
    void EnableBuildSettingsMenuItem(bool is_enabled);

private slots:
    // Handler for the "Build settings" push button.
    void HandleBuildSettingsButtonClicked(bool checked);

private:
    // Set the cursor to the specified type.
    void SetCursor(const QCursor& cursor);

    // Set the text for the build settings item.
    void SetItemText(const std::string& item_text);

    // Change appearance to "focus in".
    void GotFocus();

    // Change appearance to "focus out".
    void LostFocus();

    // Connect signals.
    void ConnectSignals();

    // The Build Settings file item interface.
    Ui::RgMenuBuildSettingsItem ui_;

    // Flag to keep track of whether this item is currently selected.
    bool current_ = false;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_BUILD_SETTINGS_ITEM_H_
