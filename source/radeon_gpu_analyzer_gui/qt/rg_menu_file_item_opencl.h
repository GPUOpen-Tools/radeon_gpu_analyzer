#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_FILE_ITEM_OPENCL_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_FILE_ITEM_OPENCL_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_menu_file_item.h"
#include "source/radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "ui_rg_menu_file_item_opencl.h"

// Forward declarations.
class QMenu;
class QStandardItemModel;
class RgEntrypointItemStyleDelegate;
struct RgEntryOutput;

/// @brief Extend user roles to store extra info for copying kernel names
/// in the copy from context menu.
enum RgMenuUserRoles
{
    kCopyNameRole = Qt::UserRole + 1,
    kUserRolesCount
};

// A model responsible for managing a file item's entry point list.
// Must be declared here to allow a Qt interface to be generated for it.
class RgMenuItemEntryListModel : public QObject
{
    Q_OBJECT

public:
    RgMenuItemEntryListModel(QWidget* parent = nullptr);
    virtual ~RgMenuItemEntryListModel() = default;

    // Get the item model for the entry point list.
    QStandardItemModel* GetEntryItemModel() const;

    // Add a new entry point to the file's entry point list.
    void AddEntry(const RgEntryOutput& entrypoint);

    // Clear all entries from the list model.
    void ClearEntries();

    // Set the width of the entry point widget.
    void SetEntryPointWidgetWidth(int width);

    // Set the entry point label widget.
    void SetEntryPointTreeWidget(RgMenuEntryPointTree* tree);

    // Get the entry point names.
    void GetEntryPointNames(std::vector<std::string>& entrypoint_names);

    // Get the entry point name at a given index.
    std::string GetEntryPointName(const int index) const;

    // Get the entry point name from the display name.
    std::string GetEntryPointName(const std::string& display_entrypoint_name) const;

    // Gets the extremely long name of selected entry point name, if it exists.
    bool GetSelectedEntrypointExtremelyLongName(const std::string& entrypoint_name, std::string& extremely_long_name) const;

signals:
    // A signal emitted when the user changes the selected entry point in the entry point list.
    void SelectedEntrypointChanged(const std::string& input_file_path, int selected_index);

private:
    // A model responsible for holding the entry point item list.
    QStandardItemModel* entry_point_item_model_ = nullptr;

    // The width of the entry point widget.
    int entry_point_widget_width_ = 0;

    // The entry point tree.
    RgMenuEntryPointTree* entry_point_tree_ = nullptr;

    // Keep track of entry point name.
    std::vector<std::string> entry_point_names_;

    //Keep track of the display name.
    std::vector<std::string> display_names_;

    //Keep track of the extremely long kernel names.
    std::unordered_map<std::string, std::string> extremely_long_kernel_names_map;
};

// An object used to represent a single shader file within the file menu.
class RgMenuFileItemOpencl : public RgMenuFileItem
{
    Q_OBJECT

public:
    explicit RgMenuFileItemOpencl(const std::string& file_full_path, RgMenu* parent = nullptr);
    virtual ~RgMenuFileItemOpencl() = default;

    // Handler for mouse hover enter events.
    virtual void enterEvent(QEnterEvent* event) override;

    // Handler for mouse hover leave events.
    virtual void leaveEvent(QEvent* event) override;

    // Handler for a double-click on the item.
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

    // Handler invoked when the user clicks this menu item.
    virtual void mousePressEvent(QMouseEvent* event) override;

    // Handler invoked when this item is resized.
    virtual void resizeEvent(QResizeEvent* event) override;

    // Handler invoked when this item is shown.
    virtual void showEvent(QShowEvent* event) override;

    // Handler invoked when the user hits a key.
    virtual void keyPressEvent(QKeyEvent* event) override;

    // Remove all entrypoints from the item's entry point list.
    void ClearEntrypointsList();

    // Retrieve the list of entry point names for this input file.
    void GetEntrypointNames(std::vector<std::string>& entrypoint_names) const;

    // Get the name of the currently selected entrypoint, if there is one.
    // Returns "false" if no entry is currently selected or the selected one is not valid any more.
    bool GetSelectedEntrypointName(std::string& entrypoint_name) const;

    // Gets the extremely long name of selected entry point name, if it exists.
    bool GetSelectedEntrypointExtremelyLongName(const std::string& entrypoint_name, std::string& extremely_long_name) const;

    // Alter the visual style of the item if it is hovered or not.
    virtual void SetHovered(bool is_hovered) override;

    // Alter the visual style of the item if it is currently selected.
    virtual void SetCurrent(bool is_current) override;

    // Set the visibility of the entrypoints list for the file.
    void ShowEntrypointsList(bool show_list);

    // Switch to the given entry point in the item's dropdown.
    void SwitchToEntrypointByName(const std::string& entrypoint_name);

    // Update the file item with the latest build outputs.
    void UpdateBuildOutputs(const std::vector<RgEntryOutput>& entry_outputs);

signals:
    // Signal emitted when the item's close button is clicked.
    void MenuItemCloseButtonClicked(const std::string& full_path);

    // Signal emitted when the user changes the selected entrypoint.
    void SelectedEntrypointChanged(const std::string& file_path, const std::string& selected_entrypoint_name);

private slots:
    // Handler invoked when the user would like to remove a file.
    void HandleRemoveItemRequest();

    // Handler invoked when the user clicks an item in the entry point list.
    void HandleEntrypointClicked();

    // Handler invoked when the user enters the entry point list tree view
    void HandleTableItemEntered(const QModelIndex& model_index);

public slots:
    // Handler invoked when the project build is successful.
    void HandleProjectBuildSuccess();

protected:
    // Initialize signals.
    void ConnectSignals();

    // Create the file's entry point list.
    void InitializeEntrypointsList();

    // Update the item's file label text.
    virtual void UpdateFilenameLabelText() override;

    // Get the rename item line edit widget.
    virtual QLineEdit* GetRenameLineEdit() override;

    // Get the item text label widget.
    virtual QLabel* GetItemLabel() override;

    // Update the file path for this item.
    void UpdateFilepath(const std::string& new_file_path);

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // A model responsible for the data within a file menu item's "entrypoint list" view.
    RgMenuItemEntryListModel* entry_list_model_ = nullptr;

    // The style delegate used to paint the entry point list correctly.
    RgEntrypointItemStyleDelegate* entry_point_style_delegate_ = nullptr;

    // The name of last selected entry point name.
    std::string last_selected_entry__name_;

    // The generated view object.
    Ui::RgMenuFileItemOpenCL ui_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_FILE_ITEM_OPENCL_H_
