#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_PIPELINE_STATE_ITEM_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_PIPELINE_STATE_ITEM_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/rg_data_types.h"
#include "ui_rg_menu_pipeline_state_item.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_menu_item.h"

// Forward declarations:
class QPushButton;

class RgMenuPipelineStateItem
    : public RgMenuItem
{
    Q_OBJECT

public:
    explicit RgMenuPipelineStateItem(RgPipelineType pipeline_type, RgMenu* parent = nullptr);
    virtual ~RgMenuPipelineStateItem() = default;

    // Handler invoked when the user drags a file over.
    virtual void dragEnterEvent(QDragEnterEvent* event) override;

    // Handler invoked when the user drops a dragged file.
    virtual void dropEvent(QDropEvent *event) override;

    // Handler invoked when the user leaves the button.
    virtual void dragLeaveEvent(QDragLeaveEvent* event);

    // Get a pointer to the pipeline state button within the item.
    QPushButton* GetPipelineStateButton() const;

    // Set the cursor to the specified type.
    void SetCursor(const QCursor& cursor);

    // Alter the visual style of the item if it is currently selected.
    void SetCurrent(bool is_current);

    // Get the current state.
    bool IsCurrent() const;

    // Simulate a click on the menu item.
    void ClickMenuItem() const;

signals:
    // A signal emitted when a PSO editor file is dragged and dropped.
    void DragAndDropExistingFile(const std::string& file_path);

    // Signal emitted when the pipeline state button is clicked.
    void PipelineStateButtonClicked(RgMenuPipelineStateItem* item);

    // A signal to indicate if the pipeline state option should be enabled.
    void EnablePipelineMenuItem(bool is_enabled);

    // A signal to indicate if the build settings option should be enabled.
    void EnableBuildSettingsMenuItem(bool is_enabled);

private slots:
    // Handler invoked when the Pipeline State button is clicked.
    void HandlePipelineStateButtonClicked(bool checked);

private:
    // Connect signals.
    void ConnectSignals();

    // Set the text for the build settings item.
    void SetItemText(const std::string& item_text);

    // Change appearance to "focus in".
    void GotFocus();

    // Change appearance to "focus out".
    void LostFocus();

    // The Build Settings file item interface.
    Ui::rgMenuPipelineStateItem ui_;

    // Flag to keep track of whether this item is currently selected.
    bool current_ = false;

    // The pipeline type.
    RgPipelineType pipeline_type_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_PIPELINE_STATE_ITEM_H_
