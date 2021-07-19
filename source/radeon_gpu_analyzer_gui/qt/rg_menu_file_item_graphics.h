#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_FILE_ITEM_GRAPHICS_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_FILE_ITEM_GRAPHICS_H_

// Local.
#include "ui_rg_menu_file_item_graphics.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_menu_file_item.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_menu_graphics.h"

// Forward declarations:
class QPushButton;

class RgMenuFileItemGraphics : public RgMenuFileItem
{
    Q_OBJECT

public:
    explicit RgMenuFileItemGraphics(RgMenu* parent, RgPipelineStage stage);
    virtual ~RgMenuFileItemGraphics() = default;

    // An enumeration used to determine which state the stage item's buttons are in.
    enum class StageButtonsMode
    {
        // When in AddCreate mode, the stage does not yet have a source file assigned.
        kAddCreate,

        // When in remove mode, the stage has a source file assigned, and the remove button will remove it.
        kRemove
    };

    // Handler for mouse hover enter events.
    virtual void enterEvent(QEvent* event) override;

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

    // Handler invoked when the user drags a file over.
    virtual void dragEnterEvent(QDragEnterEvent* event) override;

    // Handler invoked when the user drops a dragged file.
    virtual void dropEvent(QDropEvent *event) override;

    // Handler invoked when the drag event leaves this widget.
    virtual void dragLeaveEvent(QDragLeaveEvent* event) override;

    // Handler invoked when the user is moving the mouse while dragging.
    virtual void dragMoveEvent(QDragMoveEvent *event) override;

    // Alter the visual style of the item if it is hovered or not.
    virtual void SetHovered(bool is_hovered) override;

    // Alter the visual style of the item if it is currently selected.
    virtual void SetCurrent(bool is_current) override;

    // Alter the visual style of the item if the stage is occupied by a file.
    void SetStageIsOccupied(bool is_occupied);

    // Set the cursor to the specified type.
    void SetCursor(const QCursor& cursor);

    // Set the shader file for the stage.
    void SetShaderFile(const std::string& filename, RgVulkanInputType file_type);

    // Set the enabledness of the Add/Create/Close buttons for the stage item.
    void SetButtonsEnabled(bool is_enabled);

    // Set the state of the buttons displayed in the stage item.
    void SetButtonsMode(StageButtonsMode button_mode);

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
    RgPipelineStage GetStage() const;

    // Get the type of the file associated with this item.
    RgVulkanInputType GetFileType() const;

    // Add "Restore original SPIR-V binary" item to the Context Menu.
    void AddContextMenuActionRestoreSpv();

    // Remove "Restore original SPIR-V binary" item from the Context Menu.
    void RemoveContextMenuActionRestoreSpv();

signals:
    // A signal emitted when the "Add existing file" button is clicked.
    void AddExistingFileButtonClicked(RgPipelineStage stage);

    // A signal emitted when the "Create source file" button is clicked.
    void CreateSourceFileButtonClicked(RgPipelineStage stage);

    // A signal emitted when the user drags and drops a file.
    void DragAndDropExistingFile(RgPipelineStage stage, const std::string& filename);

    // A signal emitted when the "Remove source file" button is clicked.
    void RemoveSourceFileButtonClicked(RgPipelineStage stage);

    // A signal emitted when the "Restore original SPIR-V binary" is clicked.
    void RestoreOriginalSpvButtonClicked(RgPipelineStage stage);

    // A signal to indicate if the pipeline state option should be disabled.
    void EnablePipelineMenuItem(bool is_enabled);

    // A signal to indicate if the build settings option should be disabled.
    void EnableBuildSettingsMenuItem(bool is_enabled);

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
    Ui::RgMenuFileItemGraphics ui_;

    // The pipeline stage that this item represents.
    RgPipelineStage stage_;

    // The type of the stage shader file.
    RgVulkanInputType file_type_ = RgVulkanInputType::kUnknown;

    // Graphics-specific actions for context menu.
    struct
    {
        QAction* separator  = nullptr;
        QAction* restore_spv = nullptr;
    }
    context_menu_actions_graphics_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_FILE_ITEM_GRAPHICS_H_
