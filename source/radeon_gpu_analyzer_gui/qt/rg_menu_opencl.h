#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_OPENCL_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_OPENCL_H_

// C++.
#include <string>
#include <list>
#include <map>
#include <memory>

// Qt.
#include <QString>

// Local.
#include "source/radeon_gpu_analyzer_gui/rg_data_types.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_menu.h"

// Forward declarations.
class QWidget;
class QVBoxLayout;
class RgMenuItem;
class RgMenuTitlebar;
class RgMenuBuildSettingsItem;
class RgMenuFileItemOpencl;
class RgAddCreateMenuItem;
class RgBuildView;
struct RgCliBuildOutput;

class RgMenuOpencl : public RgMenu
{
    Q_OBJECT

public:
    RgMenuOpencl(QWidget* parent);
    virtual ~RgMenuOpencl() = default;

    // Initialize the view's default menu items.
    virtual void InitializeDefaultMenuItems(const std::shared_ptr<RgProjectClone> project_clone) override;

    // Connect the signals for the view's default menu items.
    virtual void ConnectDefaultItemSignals() override;

    // Connect signals for the menu file item.
    virtual void ConnectMenuFileItemSignals(RgMenuFileItem* menu_item) override;

    // Add an item to the menu.
    bool AddItem(const std::string& full_path, bool is_new_file_item);

    // Clear the file menu's build outputs for each source file in the project.
    void ClearBuildOutputs();

    // Gets whether or not showing entry point lists within file items is enabled.
    bool GetIsShowEntrypointListEnabled() const;

    // Offset the currently selected row in the entry point list by advancing or regressing the current index by the given offset.
    // This function will return true only when (current index + offset) is a valid index in the entry point list.
    bool OffsetCurrentFileEntrypoint(int offset);

    // Toggle the visibility of the entry point list for file items.
    void SetIsShowEntrypointListEnabled(bool is_enabled);

    // Update the file menu with the latest build output.
    void UpdateBuildOutput(const RgBuildOutputsMap& build_outputs);

    // Set the build settings button to have no focus.
    virtual void SetButtonsNoFocus() override;

    // Handler invoked when the user drags a file over.
    virtual void dragEnterEvent(QDragEnterEvent* event) override;

    // Handler invoked when the user drops a dragged file.
    virtual void dropEvent(QDropEvent* event) override;

    // Handler invoked when the user is moving the mouse while dragging.
    virtual void dragMoveEvent(QDragMoveEvent* event) override;

signals:
    // A signal emitted when the user drags and drops a file.
    void DragAndDropExistingFile(const std::string& filename);

    // Signal emitted when the user changes the selected entry point index for a given file.
    void SelectedEntrypointChanged(const std::string& input_file_path, const std::string& selected_entrypoint_name);

    // Signal emitted when the project was built successfully.
    void ProjectBuildSuccess();

public slots:
    // Handler for when a build has started.
    virtual void HandleBuildStarted() override;

    // Handler for when a build has ended.
    virtual void HandleBuildEnded() override;

    // Select/focus on the item indicated by focus_index_.
    virtual void SelectFocusItem(FileMenuActionType action_type) override;

    // Select/focus on the item indicated by focus_index_ when tab pressed.
    virtual void SelectTabFocusItem(bool shift_tab_focucs) override;

    // Handler invoked when the user presses the activate key (Enter).
    // This is used to handle pressing add/create buttons when in focus.
    virtual void HandleActivateItemAction() override;

    // Handler invoked when the user clicks on the build settings button.
    virtual void HandleBuildSettingsButtonClicked(bool checked) override;

    // Handler invoked when the selected entry point has been changed.
    void HandleSelectedEntrypointChanged(const std::string& target_gpu, const std::string& input_file_path, const std::string& selected_entrypoint_name);

    // Handler invoked when the user presses the Tab key to switch focus.
    virtual void HandleTabFocusPressed() override;

    // Handler invoked when the user presses the Shift+Tab keys to switch focus.
    virtual void HandleShiftTabFocusPressed() override;

    // Handler invoked when the user adds a new file so the buttons get reset.
    void HandleSourceFileAdded();

    // Handler invoked when the user changes the currently selected file item.
    virtual void HandleSelectedFileChanged(RgMenuFileItem* selected) override;

protected slots:
    // Handler invoked when the user triggers the next menu item action.
    virtual void HandleNextItemAction() override;

    // Handler invoked when the user triggers the previous menu item action.
    virtual void HandlePreviousItemAction();

protected:
    // Retrieve the number of extra buttons that get added to the menu after the file items.
    virtual int GetButtonCount() const override;

private:
    // The default menu item.
    RgAddCreateMenuItem* add_create_menu_item_ = nullptr;

    // A flag used to determine if the kernel list in file items are able to expand.
    bool is_show_entrypoint_list_enabled_ = true;

    // Connect signals for build settings and Pipeline state buttons.
    void ConnectButtonSignals();
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_OPENCL_H
