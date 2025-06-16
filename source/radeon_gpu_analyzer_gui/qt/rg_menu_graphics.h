//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for RGA Build view's File Menu for Graphics APIs.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_GRAPHICS_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_GRAPHICS_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_menu.h"

// Forward declarations.
class RgMenuPipelineStateItem;
class RgMenuFileItemGraphics;

// Indices for special case file items.
enum class GraphicsMenuFocusItems
{
    kPipelineStateButton,
    kBuildSettingsButton,
    kCount
};

// Indices for sub buttons.
// This is used in tabbing.
enum class GraphicsMenuTabFocusItems
{
    kNoButton,
    kRemoveButton,
    kAddExistingFileButton,
    kCount
};

class RgMenuGraphics : public RgMenu
{
    Q_OBJECT

public:
    RgMenuGraphics(QWidget* parent);
    virtual ~RgMenuGraphics() = default;

    // Reimplement mousePressEvent.
    virtual void mousePressEvent(QMouseEvent* event) override;

    // Initialize the view's default menu items.
    virtual void InitializeDefaultMenuItems(const std::shared_ptr<RgProjectClone> project_clone) override;

    // Select/focus on the item indicated by focus_index_.
    virtual void SelectFocusItem(FileMenuActionType action_type) override;

    // Select/focus on the item indicated by focus_index_ when tab pressed.
    virtual void SelectTabFocusItem(bool shift_tab_focus) override;

    // Getter for the pipeline state item.
    RgMenuPipelineStateItem* GetPipelineStateItem() const;

    // Get the stage item widget for the given stage.
    RgMenuFileItemGraphics* GetStageItem(RgPipelineStage stage) const;

    // Getter for the current stage.
    RgPipelineStage GetCurrentStage() const;

    // Get the current file menu item.
    RgMenuFileItemGraphics* GetCurrentFileMenuItem() const;

    // Set the build settings and pipeline state buttons to have no focus.
    virtual void SetButtonsNoFocus() override;

signals:
    // A signal emitted when the user clicks the "Add existing file" button within a shader stage item.
    void AddExistingFileButtonClicked(RgPipelineStage stage);

    // A signal emitted when the user clicks the "Create new file" button within a shader stage item.
    void CreateNewFileButtonClicked(RgPipelineStage stage);

    // A signal emitted when the user drags and drops a file.
    void DragAndDropExistingFile(RgPipelineStage stage, const std::string& filename);

    // A signal emitted when the user drags and drops a PSO file.
    void DragAndDropExistingPsoFile(const std::string& filename);

    // A signal to focus the next view.
    void FocusNextView();

    // A signal to focus the previous view.
    void FocusPrevView();

    // A signal emitted when the user clicks the "Remove file" button within a shader stage item.
    void RemoveFileButtonClicked(RgPipelineStage stage);

    // A signal emitted when the user clicks the "Restore original SRIR-V binary" button.
    void RestoreOriginalSpirvClicked(RgPipelineStage stage);

    // A signal to indicate if the pipeline state option should be disabled.
    void EnablePipelineMenuItem(bool is_enabled);

    // A signal to indicate if the build settings option should be disabled.
    void EnableBuildSettingsMenuItem(bool is_enabled);

public slots:
    // Handler for when a build has started.
    virtual void HandleBuildStarted() override;

    // Handler for when a build has ended.
    virtual void HandleBuildEnded() override;

    // Handler invoked when the user presses the Tab key to switch focus.
    virtual void HandleTabFocusPressed() override;

    // Handler invoked when the user presses the Shift+Tab keys to switch focus.
    virtual void HandleShiftTabFocusPressed() override;

    // Handler invoked when the user clicks on the build settings button.
    virtual void HandleBuildSettingsButtonClicked(bool /* checked */) override;

    // Handler invoked when the user clicks on the pipeline state button.
    virtual void HandlePipelineStateButtonClicked(bool /* checked */);

    // Handler invoked when file menu button focus needs to be removed.
    void HandleRemoveFileMenuButtonFocus();

    // Sets the current representing the given stage to current (on true)
    // or non-current (on false).
    void SetCurrent(RgPipelineStage stage, bool is_current);

private slots:
    // Handler for when a file is dragged and dropped.
    void HandleDragAndDropExistingFile();

    // Handler invoked when the user clicks "Add existing file" button.
    void HandleAddExistingFileButtonClicked(RgPipelineStage stage);

    // Handler to disable pipeline state menu item.
    void HandleEnablePipelineMenuItem(bool is_enabled);

    // Handler to disable build settings menu item.
    void HandleEnableBuildSettingsMenuItem(bool is_enabled);

protected:
    // Create a horizontal line. The line is created by creating a QFrame and setting its frame shape to a line.
    // Then the frame is added to a horizontal layout, which gets returned by the method.
    QHBoxLayout* CreateHorizontalLine() const;

    // Retrieve the number of extra buttons that get added to the menu after the file items.
    virtual int GetButtonCount() const override;

    // Reconstruct the menu item list in stage order.
    void RebuildMenuItemsList();

    // An array of shader stage items within the file menu. When editing a graphics pipeline, this
    // array is initialized with all 5 graphics pipeline stage items. When editing a compute
    // pipeline, the array will only contain a single compute stage item. Stage slots that do not
    // apply for the given pipeline will remain nullptr.
    std::array<RgMenuFileItemGraphics*, static_cast<size_t>(RgPipelineStage::kCount)> shader_stage_items_;

    // The pipeline state menu item.
    RgMenuPipelineStateItem* pipeline_state_item_ = nullptr;

    // Process sub button actions.
    virtual void ProcessSubButtonAction() = 0;

    // Remove file menu button focus.
    void RemoveFileMenuButtonFocus();

    // Remove focus from sub-buttons.
    void RemoveSubButtonFocus();

    // Hide the remove file push button from all file items.
    void HideRemoveFilePushButton();

    // Getter for the number of pipeline stages.
    size_t GetTotalPipelineStages() const;

    // Update the focus index.
    void UpdateFocusIndex(RgPipelineStage stage);

    // Update the focus index.
    void UpdateFocusIndex();

    // Keep track of which sub-button is currently highlighted within a file item.
    // This is needed to handle tabbing.
    GraphicsMenuTabFocusItems has_tab_focus_ = GraphicsMenuTabFocusItems::kNoButton;

    // Save the pipeline type (e.g. Graphics or Compute).
    RgPipelineType pipeline_type_;

    // The currently selected stage.
    RgPipelineStage current_stage_;

    // The total pipeline stages for the current pipeline.
    size_t total_pipeline_stages_;

private:
    // Connect signals for each pipeline stage item.
    void ConnectStageItemSignals();

    // Connect signals for build settings and Pipeline state buttons.
    void ConnectButtonSignals();

    // Clear style sheets for buttons.
    void ClearButtonStylesheets();

    // Get the number of pipeline stages to cycle through for this action type.
    size_t GetNumberPipelineStagesToCycleThrough(FileMenuActionType action_type) const;

    // Get the list of file menu items to process for this action type.
    std::vector<RgMenuFileItem*> GetFileMenuItemsToProcess(FileMenuActionType action_type) const;

    // Initialize the view's default shader stage menu items.
    void InitializeDefaultShaderStageItems(const std::shared_ptr<RgProjectClone> project);

    // Update the enabled state of all menu items. Can be used to disable usage during a build.
    void SetStageButtonsEnabled(bool is_enabled);

    // Set the cursor for stage items.
    void SetCursor(Qt::CursorShape shape);

    // Give focus to the next sub-button.
    void SelectFocusSubButton(RgMenuFileItemGraphics* item);

    // Increment the tab focus index value.
    void IncrementTabFocusIndex();

    // Decrement the tab focus index value.
    void DecrementTabFocusIndex();

    // Select the next file menu item.
    void SelectNextItem(FileMenuActionType action_type);

    // Select the previous file menu item.
    void SelectPreviousItem(FileMenuActionType action_type);

    // Deselect graphics menu file items.
    void DeselectFileItems();

    // Select the specified button.
    void SelectButton(size_t index);
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_GRAPHICS_H_
