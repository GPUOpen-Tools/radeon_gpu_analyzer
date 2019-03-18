#pragma once

// Qt.
#include <QWidget>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSettingsView.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>
#include <ui_rgPipelineStateView.h>

// Forward declarations.
class rgEditorElement;
class rgEditorElementArray;
class rgPipelineStateModel;
class rgPipelineStateSearcher;

namespace Ui {
class rgPipelineStateView;
}

// The view object used to edit pipeline state configuration.
class rgPipelineStateView : public rgSettingsView
{
    Q_OBJECT

public:
    explicit rgPipelineStateView(QWidget* pParent = nullptr);
    virtual ~rgPipelineStateView() = default;

    // An overridden resize handler responsible for recomputing editor geometry.
    virtual void resizeEvent(QResizeEvent* pEvent) override;

    // An overridden "widget was hidden" handler used to emit a signal indicating a visibility change.
    virtual void hideEvent(QHideEvent* pEvent) override;

    // Handler invoked when the user drags a file over.
    virtual void dragEnterEvent(QDragEnterEvent* pEvent) override;

    // Handler invoked when the user drops a dragged file.
    virtual void dropEvent(QDropEvent *pEvent) override;

    // Overridden focus in event.
    virtual void focusInEvent(QFocusEvent* pEvent);

    // Overridden focus out event.
    virtual void focusOutEvent(QFocusEvent* pEvent);

    // Set the focus to target selection button.
    virtual void SetInitialWidgetFocus() override;

    // Get the pipeline state searcher, which is used by the Find Widget to search the PSO tree.
    rgPipelineStateSearcher* GetSearcher() const;

    // Get the currently-selected text in the state editor tree.
    bool GetSelectedText(std::string& selectedTextString) const;

    // Initialize the API-specific model holding the PSO tree structure.
    void InitializeModel(rgPipelineStateModel* pPipelineStateModel);

    // Insert the find widget into the grid.
    void InsertFindWidget(QWidget* pWidget);

    // Reset the search in find text widget.
    void ResetSearch();

    // Scale the settings tree using the scaling manager.
    void ScaleSettingsTree();

    // Set enum list widget status.
    void SetEnumListWidgetStatus(bool isOpen);

signals:
    // A signal emitted when a PSO editor file is dragged and dropped.
    void DragAndDropExistingFile(const std::string& filePath);

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
    rgPipelineStateSearcher* m_pPipelineStateSearcher = nullptr;

    // A pointer to the tree view being searched. This is used to highlight result rows.
    rgPipelineStateTree* m_pTreeView = nullptr;

    // The generated view object.
    Ui::rgPipelineStateView ui;

private slots:
    // Handler invoked when a node in the tree should be expanded.
    void HandleNodeExpanded(rgEditorElementArray* pArrayRoot);

private:
    // Connect interface signals to handlers.
    void ConnectSignals();

    // The pipeline type.
    rgPipelineType m_pipelineType;

};