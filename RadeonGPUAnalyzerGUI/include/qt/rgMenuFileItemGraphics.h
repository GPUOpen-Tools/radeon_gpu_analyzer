#pragma once

// Local.
#include <ui_rgMenuFileItemGraphics.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuFileItem.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuGraphics.h>

// Forward declarations:
class QPushButton;

class rgMenuFileItemGraphics : public rgMenuFileItem
{
    Q_OBJECT

public:
    explicit rgMenuFileItemGraphics(rgMenu* pParent, rgPipelineStage stage);
    virtual ~rgMenuFileItemGraphics() = default;

    // An enumeration used to determine which state the stage item's buttons are in.
    enum class StageButtonsMode
    {
        // When in AddCreate mode, the stage does not yet have a source file assigned.
        AddCreate,

        // When in remove mode, the stage has a source file assigned, and the remove button will remove it.
        Remove
    };

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

    // Handler invoked when the user drags a file over.
    virtual void dragEnterEvent(QDragEnterEvent* pEvent) override;

    // Handler invoked when the user drops a dragged file.
    virtual void dropEvent(QDropEvent *event) override;

    // Handler invoked when the drag event leaves this widget.
    virtual void dragLeaveEvent(QDragLeaveEvent* pEvent) override;

    // Handler invoked when the user is moving the mouse while dragging.
    virtual void dragMoveEvent(QDragMoveEvent *event) override;

    // Alter the visual style of the item if it is hovered or not.
    virtual void SetHovered(bool isHovered) override;

    // Alter the visual style of the item if it is currently selected.
    virtual void SetCurrent(bool isCurrent) override;

    // Alter the visual style of the item if the stage is occupied by a file.
    void SetStageIsOccupied(bool isOccupied);

    // Set the cursor to the specified type.
    void SetCursor(const QCursor& cursor);

    // Set the shader file for the stage.
    void SetShaderFile(const std::string& filename, rgVulkanInputType fileType);

    // Set the enabledness of the Add/Create/Close buttons for the stage item.
    void SetButtonsEnabled(bool isEnabled);

    // Set the state of the buttons displayed in the stage item.
    void SetButtonsMode(StageButtonsMode buttonMode);

    // Highlight the indicated button.
    void SetButtonHighlighted(const GraphicsMenuTabFocusItems button);

    // Remove the highlight from all sub-buttons.
    void RemoveSubButtonFocus();

    // Hide the remove file push button.
    void HideRemoveFilePushButton();

    // Process the click on the remove button.
    void ProcessRemoveButtonClick();

    // Process the click on the add existing file button.
    void ProcessAddExistingFileButtonClick();

    // Return the stage value.
    rgPipelineStage GetStage() const;

    // Get the type of the file associated with this item.
    rgVulkanInputType GetFileType() const;

    // Add "Restore original SPIR-V binary" item to the Context Menu.
    void AddContextMenuActionRestoreSpv();

    // Remove "Restore original SPIR-V binary" item from the Context Menu.
    void RemoveContextMenuActionRestoreSpv();

signals:
    // A signal emitted when the "Add existing file" button is clicked.
    void AddExistingFileButtonClicked(rgPipelineStage stage);

    // A signal emitted when the "Create source file" button is clicked.
    void CreateSourceFileButtonClicked(rgPipelineStage stage);

    // A signal emitted when the user drags and drops a file.
    void DragAndDropExistingFile(rgPipelineStage stage, const std::string& fileName);

    // A signal emitted when the "Remove source file" button is clicked.
    void RemoveSourceFileButtonClicked(rgPipelineStage stage);

    // A signal emitted when the "Restore original SPIR-V binary" is clicked.
    void RestoreOriginalSpvButtonClicked(rgPipelineStage stage);

private slots:
    // Handler invoked when the "Add existing file" button is clicked.
    void HandleAddExistingFileButtonClicked();

    // Handler invoked when the "Create source file" button is clicked.
    void HandleCreateSourceFileButtonClicked();

    // Handler invoked when the "Remove source file" button is clicked.
    void HandleRemoveSourceFileButtonClicked();

    // Handler invoked when the user clicks the "Restore original SPIR-V binary" context menu item.
    void HandleRestoreOriginalSpvClicked();

protected:
    // Initialize the graphics-specific part of context menu.
    void InitializeContextMenuGraphics();

    // Connect signals.
    void ConnectSignals();

    // Set the text for the item based on the stage.
    void SetStringConstants();

    // Update the item's file label text.
    virtual void UpdateFilenameLabelText() override;

    // Get the rename item line edit widget.
    virtual QLineEdit* GetRenameLineEdit() override;

    // Get the item text label widget.
    virtual QLabel* GetItemLabel() override;

    // The Build Settings file item interface.
    Ui::rgMenuFileItemGraphics ui;

    // The pipeline stage that this item represents.
    rgPipelineStage m_stage;

    // The type of the stage shader file.
    rgVulkanInputType m_fileType = rgVulkanInputType::Unknown;

    // Graphics-specific actions for context menu.
    struct
    {
        QAction* pSeparator  = nullptr;
        QAction* pRestoreSpv = nullptr;
    }
    m_contextMenuActionsGraphics;
};