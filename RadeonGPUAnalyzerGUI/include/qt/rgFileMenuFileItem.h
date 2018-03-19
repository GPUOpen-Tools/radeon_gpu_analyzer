#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgFileMenuItem.h>
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>
#include <ui_rgFileMenuFileItem.h>

// Forward declarations.
class QMenu;
class QStandardItemModel;
class rgEntrypointItemStyleDelegate;
struct rgEntryOutput;

// A model responsible for managing a file item's entrypoint list.
// Must be declared here to allow a Qt interface to be generated for it.
class rgFileMenuItemEntryListModel : public QWidget
{
    Q_OBJECT

public:
    rgFileMenuItemEntryListModel(QWidget* pParent = nullptr);
    virtual ~rgFileMenuItemEntryListModel() = default;

    // Get the item model for the entrypoint list.
    QStandardItemModel* GetEntryItemModel() const;

    // Add a new entrypoint to the file's entrypoint list.
    void AddEntry(const std::string& entrypointName);

    // Clear all entries from the list model.
    void ClearEntries();

signals:
    // A signal emitted when the user changes the selected entrypoint in the entrypoint list.
    void SelectedEntrypointChanged(const std::string& inputFilePath, int selectedIndex);

private:
    // A model responsible for holding the entrypoint item list.
    QStandardItemModel* m_pEntrypointItemModel = nullptr;
};

// An object used to represent a single shader file within the file menu.
class rgFileMenuFileItem : public rgFileMenuItem
{
    Q_OBJECT

public:
    explicit rgFileMenuFileItem(const std::string& fileFullPath, rgFileMenu* pParent = nullptr);
    virtual ~rgFileMenuFileItem() = default;

    // Handler for mouse hover enter events.
    virtual void enterEvent(QEvent* pEvent) override;

    // Handler for mouse hover leave events.
    virtual void leaveEvent(QEvent* pEvent) override;

    // Handler for a double-click on the item.
    virtual void mouseDoubleClickEvent(QMouseEvent* pEvent) override;

    // Handler invoked when the user clicks this menu item.
    virtual void mousePressEvent(QMouseEvent* pEvent) override;

    // Handler invoked when this item is resized.
    virtual void resizeEvent(QResizeEvent* pEvent) override;

    // Handler invoked when this item is shown.
    virtual void showEvent(QShowEvent* pEvent) override;

    // Handler invoked when the user hits a key.
    virtual void keyPressEvent(QKeyEvent* pEvent) override;

    // Remove all entrypoints from the item's entrypoint list.
    void ClearEntrypointsList();

    // Retrieve the list of entrypoint names for this input file.
    void GetEntrypointNames(std::vector<std::string>& entrypointNames);

    // Get the name of the currently selected entrypoint, if there is one.
    // Returns "false" if no entry is currently selected or the selected one is not valid any more.
    bool GetSelectedEntrypointName(std::string& entrypointName) const;

    // Get the filename associated with this menu item.
    const std::string& GetFilename() const;

    // Alter the visual style of the item if it is selected/non-selected.
    void SetIsSelected(bool isSelected);

    // Mark the item as saved or unsaved (denoted by "*" after the filename).
    void SetIsSaved(bool isSaved);

    // Toggle the visibility of the renaming controls.
    void ShowRenameControls(bool isRenaming);

    // Show the context menu in a default position.
    void OpenContextMenu();

    // Set the visibility of the entrypoints list for the file.
    void ShowEntrypointsList(bool showList);

    // Switch to the given entrypoint in the item's dropdown.
    void SwitchToEntrypointByName(const std::string& entrypointName);

    // Update the file item with the latest build outputs.
    void UpdateBuildOutputs(const std::vector<rgEntryOutput>& entryOutputs);

signals:
    // Signal emitted when the file has been renamed.
    void FileRenamed(const std::string& oldFilepath, const std::string& newFilepath);

    // Signal emitted when the menu item is clicked.
    void MenuItemSelected(rgFileMenuFileItem* pSelectedItem);

    // Signal emitted when the item's close button is clicked.
    void MenuItemCloseButtonClicked(const std::string fullPath);

    // Signal emitted when the user changes the selected entrypoint.
    void SelectedEntrypointChanged(const std::string& filePath, const std::string& selectedEntrypointName);

private slots:
    // Handler invoked when a rename operation is finished.
    void HandleRenameFinished();

    // Handler invoked when the focused widget changes.
    void HandleFocusChanged(QWidget* pOld, QWidget* pNow);

    // Handler invoked when the user clicks the item's close button.
    void HandleCloseButtonClicked();

    // Handler invoked when the user clicks the item's "Open in file browser" context menu item.
    void HandleOpenInFileBrowserClicked();

    // Handler invoked when the user clicks the item's "Rename" context menu item.
    void HandleRenameClicked();

    // Handler invoked when the user wants to open the context menu for the item.
    void HandleOpenContextMenu(const QPoint& localClickPosition);

    // Handler invoked when the user clicks an item in the entrypoint list.
    void HandleEntrypointClicked();

    // Handler invoked when the user enters the entry point list tree view
    void HandleTableItemEntered(const QModelIndex& modelIndex);

private:
    // Initialize signals.
    void ConnectSignals();

    // Create the file's entrypoint list.
    void InitializeEntrypointsList();

    // Initialize the context menu and all options within it.
    void InitializeContextMenu();

    // Refresh the label text, truncating as needed to fit size and suffix requirements.
    void RefreshLabelText();

    // Renames the file, but it will remain in the same location.
    bool RenameFile();

    // Update the file path for this item.
    void UpdateFilepath(const std::string& newFilepath);

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // A model responsible for the data within a file menu item's "entrypoint list" view.
    rgFileMenuItemEntryListModel* m_pEntryListModel = nullptr;

    // The context menu to display when the user right-clicks on a file item.
    QMenu* m_pContextMenu = nullptr;

    // The context menu item used to open the location of the program in the file browser.
    QAction* m_pOpenContainingFolderAction = nullptr;

    // The context menu item used to rename the selected file.
    QAction* m_pRenameFileAction = nullptr;

    // The context menu item used to remove the selected file.
    QAction* m_pRemoveFileAction = nullptr;

    // The style delegate used to paint the entrypoint list correctly.
    rgEntrypointItemStyleDelegate* m_pEntrypointStyleDelegate = nullptr;

    // The full path to the file.
    std::string m_fullFilepath;

    // The filename of the file.
    std::string m_filename;

    // The boolean to keep track of escape key being pressed.
    bool m_escapePressed = false;

    // The name of last selected entrypoint name.
    std::string m_lastSelectedEntryName;

    // Whether or not the file has been saved.
    bool m_isSaved;

    Ui::rgFileMenuFileItem ui;
};