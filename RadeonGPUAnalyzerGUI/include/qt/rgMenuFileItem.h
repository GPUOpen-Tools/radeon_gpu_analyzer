#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuItem.h>

// Forward declarations.
class QAction;
class QLabel;
class QLineEdit;
class QMenu;

class rgMenuFileItem : public rgMenuItem
{
    Q_OBJECT

public:
    rgMenuFileItem(const std::string& fullFilePath, rgMenu* pParent = nullptr);
    virtual ~rgMenuFileItem() = default;

    // Alter the visual style of the item if it is hovered or not.
    virtual void SetHovered(bool isHovered) = 0;

    // Alter the visual style of the item if it is currently selected.
    virtual void SetCurrent(bool isCurrent) = 0;

    // Connect signals for this file item.
    void ConnectSignals();

    // Get the filename associated with this menu item.
    const std::string& GetFilename() const;

    // Show the context menu in a default position.
    void OpenContextMenu();

    // Toggle the visibility of the renaming controls.
    void ShowRenameControls(bool isRenaming);

    // Mark the item as saved or unsaved (denoted by "*" after the filename).
    void SetIsSaved(bool isSaved);

signals:
    // Signal emitted when the file has been renamed.
    void FileRenamed(const std::string& oldFilepath, const std::string& newFilepath);

    // Signal emitted when the menu item is clicked.
    void MenuItemSelected(rgMenuFileItem* pSelectedItem);

public slots:
    // Handler invoked when a rename operation is finished.
    void HandleEnterPressed();

    // Handler invoked when the focused widget changes.
    void HandleFocusChanged(QWidget* pOld, QWidget* pNow);

private slots:
    // Handler invoked when the user clicks the item's "Open in file browser" context menu item.
    void HandleOpenInFileBrowserClicked();

    // Handler invoked when the user clicks the item's "Rename" context menu item.
    void HandleRenameClicked();

    // Handler invoked when the user wants to open the context menu for the item.
    void HandleOpenContextMenu(const QPoint& localClickPosition);

protected:
    // Update the item's file label text.
    virtual void UpdateFilenameLabelText() = 0;

    // Get the rename item line edit widget.
    virtual QLineEdit* GetRenameLineEdit() = 0;

    // Get the item text label widget.
    virtual QLabel* GetItemLabel() = 0;

    // Initialize the context menu and all options within it.
    void InitializeContextMenu();

    // Renames the file, but it will remain in the same location.
    bool RenameFile();

    // Update the file path for this item.
    void UpdateFilepath(const std::string& newFilepath);

    // The full path to the file.
    std::string m_fullFilepath;

    // The filename of the file.
    std::string m_filename;

    // The context menu to display when the user right-clicks on a file item.
    QMenu* m_pContextMenu = nullptr;

    // Actions for context menu.
    struct
    {
        QAction* pOpenContainingFolder = nullptr;
        QAction* pRenameFile = nullptr;
        QAction* pRemoveFile = nullptr;
    }
    m_contextMenuActions;

    // Whether or not the file has been saved.
    bool m_isSaved;

    // The flag to keep track of escape key being pressed.
    bool m_isEscapePressed = false;
};