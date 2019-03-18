#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenu.h>

// Forward declarations.
class rgMenuPipelineStateItem;
class rgMenuFileItemGraphics;

// Indices for special case file items.
enum class GraphicsMenuFocusItems
{
    PipelineStateButton,
    BuildSettingsButton,
    Count
};

// Indices for sub buttons.
// This is used in tabbing.
enum class GraphicsMenuTabFocusItems
{
    NoButton,
    RemoveButton,
    AddExistingFileButton,
    Count
};

class rgMenuGraphics : public rgMenu
{
    Q_OBJECT

public:
    rgMenuGraphics(QWidget* pParent);
    virtual ~rgMenuGraphics() = default;

    // Initialize the view's default menu items.
    virtual void InitializeDefaultMenuItems(const std::shared_ptr<rgProjectClone> pProjectClone) override;

    // Select/focus on the item indicated by m_focusIndex.
    virtual void SelectFocusItem(FileMenuActionType actionType) override;

    // Select/focus on the item indicated by m_focusIndex when tab pressed.
    virtual void SelectTabFocusItem(bool shiftTabFocus) override;

    // Getter for the pipeline state item.
    rgMenuPipelineStateItem* GetPipelineStateItem() const;

    // Get the stage item widget for the given stage.
    rgMenuFileItemGraphics* GetStageItem(rgPipelineStage stage) const;

    // Getter for the current stage.
    rgPipelineStage GetCurrentStage() const;

    // Get the current file menu item.
    rgMenuFileItemGraphics* GetCurrentFileMenuItem() const;

    // Set the build settings and pipeline state buttons to have no focus.
    virtual void SetButtonsNoFocus() override;

signals:
    // A signal emitted when the user clicks the "Add existing file" button within a shader stage item.
    void AddExistingFileButtonClicked(rgPipelineStage stage);

    // A signal emitted when the user clicks the "Create new file" button within a shader stage item.
    void CreateNewFileButtonClicked(rgPipelineStage stage);

    // A signal emitted when the user drags and drops a file.
    void DragAndDropExistingFile(rgPipelineStage stage, const std::string& fileName);

    // A signal emitted when the user drags and drops a PSO file.
    void DragAndDropExistingPsoFile(const std::string& fileName);

    // A signal to focus the next view.
    void FocusNextView();

    // A signal to focus the previous view.
    void FocusPrevView();

    // A signal emitted when the user clicks the "Remove file" button within a shader stage item.
    void RemoveFileButtonClicked(rgPipelineStage stage);

    // A signal emitted when the user clicks the "Restore original SRIR-V binary" button.
    void RestoreOriginalSpirvClicked(rgPipelineStage stage);

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
    void SetCurrent(rgPipelineStage stage, bool isCurrent);

private slots:
    // Handler for when a file is dragged and dropped.
    void HandleDragAndDropExistingFile();

    // Handler invoked when the user clicks "Add existing file" button.
    void HandleAddExistingFileButtonClicked(rgPipelineStage stage);

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
    std::array<rgMenuFileItemGraphics*, static_cast<size_t>(rgPipelineStage::Count)> m_shaderStageItems;

    // The pipeline state menu item.
    rgMenuPipelineStateItem* m_pPipelineStateItem = nullptr;

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
    void UpdateFocusIndex(rgPipelineStage stage);

    // Update the focus index.
    void UpdateFocusIndex();

    // Keep track of which sub-button is currently highlighted within a file item.
    // This is needed to handle tabbing.
    GraphicsMenuTabFocusItems m_hasTabFocus = GraphicsMenuTabFocusItems::NoButton;

    // Save the pipeline type (e.g. Graphics or Compute).
    rgPipelineType m_pipelineType;

    // The currently selected stage.
    rgPipelineStage m_currentStage;

    // The total pipeline stages for the current pipeline.
    size_t m_totalPipelineStages;

private:
    // Connect signals for each pipeline stage item.
    void ConnectStageItemSignals();

    // Connect signals for build settings and Pipeline state buttons.
    void ConnectButtonSignals();

    // Clear style sheets for buttons.
    void ClearButtonStylesheets();

    // Get the number of pipeline stages to cycle through for this action type.
    size_t GetNumberPipelineStagesToCycleThrough(FileMenuActionType actionType) const;

    // Get the list of file menu items to process for this action type.
    std::vector<rgMenuFileItem*> GetFileMenuItemsToProcess(FileMenuActionType actionType) const;

    // Initialize the view's default shader stage menu items.
    void InitializeDefaultShaderStageItems(const std::shared_ptr<rgProjectClone> pProject);

    // Update the enabled state of all menu items. Can be used to disable usage during a build.
    void SetStageButtonsEnabled(bool isEnabled);

    // Set the cursor for stage items.
    void SetCursor(Qt::CursorShape shape);

    // Give focus to the next sub-button.
    void SelectFocusSubButton(rgMenuFileItemGraphics* pItem);

    // Increment the tab focus index value.
    void IncrementTabFocusIndex();

    // Decrement the tab focus index value.
    void DecrementTabFocusIndex();

    // Select the next file menu item.
    void SelectNextItem(FileMenuActionType actionType);

    // Select the previous file menu item.
    void SelectPreviousItem(FileMenuActionType actionType);

    // Deselect graphics menu file items.
    void DeselectFileItems();

    // Select the specified button.
    void SelectButton(size_t index);
};