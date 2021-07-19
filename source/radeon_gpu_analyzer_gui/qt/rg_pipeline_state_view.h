#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_VIEW_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_VIEW_H_

// Qt.
#include <QWidget>

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_settings_view.h"
#include "source/radeon_gpu_analyzer_gui/rg_data_types.h"
#include "ui_rg_pipeline_state_view.h"

// Forward declarations.
class RgEditorElement;
class RgEditorElementArrayElementAdd;
class RgPipelineStateModel;
class RgPipelineStateSearcher;

namespace Ui {
class RgPipelineStateView;
}

// The view object used to edit pipeline state configuration.
class RgPipelineStateView : public RgSettingsView
{
    Q_OBJECT

public:
    explicit RgPipelineStateView(QWidget* parent = nullptr);
    virtual ~RgPipelineStateView() = default;

    // An overridden resize handler responsible for recomputing editor geometry.
    virtual void resizeEvent(QResizeEvent* event) override;

    // An overridden "widget was hidden" handler used to emit a signal indicating a visibility change.
    virtual void hideEvent(QHideEvent* event) override;

    // Handler invoked when the user drags a file over.
    virtual void dragEnterEvent(QDragEnterEvent* event) override;

    // Handler invoked when the user drops a dragged file.
    virtual void dropEvent(QDropEvent *event) override;

    // Overridden focus in event.
    virtual void focusInEvent(QFocusEvent* event);

    // Overridden focus out event.
    virtual void focusOutEvent(QFocusEvent* event);

    // Set the focus to target selection button.
    virtual void SetInitialWidgetFocus() override;

    // Get the pipeline state searcher, which is used by the Find Widget to search the PSO tree.
    RgPipelineStateSearcher* GetSearcher() const;

    // Get the currently-selected text in the state editor tree.
    bool GetSelectedText(std::string& selected_text_string) const;

    // Initialize the API-specific model holding the PSO tree structure.
    void InitializeModel(RgPipelineStateModel* pipeline_state_model);

    // Insert the find widget into the grid.
    void InsertFindWidget(QWidget* widget);

    // Reset the search in find text widget.
    void ResetSearch();

    // Scale the settings tree using the scaling manager.
    void ScaleSettingsTree();

    // Set enum list widget status.
    void SetEnumListWidgetStatus(bool is_open);

signals:
    // A signal emitted when a PSO editor file is dragged and dropped.
    void DragAndDropExistingFile(const std::string& file_path);

    // A signal emitted when the source editor is hidden.
    void EditorHidden();

    // A signal emitted when the source editor is resized.
    void EditorResized();

    // A signal emitted when the save button is clicked.
    void SaveButtonClicked();

    // A signal emitted when the load button is clicked.
    void LoadButtonClicked();

    // A signal to indicate pipeline state tree focus in.
    void PipelineStateTreeFocusIn();

    // A signal to indicate pipeline state tree focus out.
    void PipelineStateTreeFocusOut();

public slots:
    // Handler invoked when a new PSO file has been loaded.
    void HandlePsoFileLoaded();

protected:
    // The searcher used to search the pipeline state model tree.
    RgPipelineStateSearcher* pipeline_state_searcher_ = nullptr;

    // A pointer to the tree view being searched. This is used to highlight result rows.
    RgPipelineStateTree* tree_view_ = nullptr;

    // The generated view object.
    Ui::RgPipelineStateView ui_;

private slots:
    // Handler invoked when a node in the tree should be expanded.
    void HandleNodeExpanded(RgEditorElementArrayElementAdd* array_root);

private:
    // Connect interface signals to handlers.
    void ConnectSignals();

    // The pipeline type.
    RgPipelineType pipeline_type_;

};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_VIEW_H_
