#pragma once

// Qt.
#include <QScrollArea>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgPipelineStateSearcher.h>
#include <ui_rgPipelineStateTree.h>

// Forward declarations.
class rgEditorElement;

// A custom TreeView.
class rgPipelineStateTree : public QScrollArea
{
    Q_OBJECT

public:
    // A struct containing the currently selected row and column.
    // If m_pRow is nullptr, there is no current selection.
    struct CurrentSelection
    {
        // The currently selected row.
        rgEditorElement* m_pSelectedRow;

        // The currently selected column.
        int m_focusedColumn;
    };

public:
    explicit rgPipelineStateTree(QWidget* pParent = nullptr);
    virtual ~rgPipelineStateTree() = default;

    // Re-implement eventFilter.
    bool eventFilter(QObject* pObject, QEvent* pEvent) override;

    // Re-implement the key press event.
    virtual void keyPressEvent(QKeyEvent* pEvent) override;

    // Re-implement focus in event.
    virtual void focusInEvent(QFocusEvent* pEvent) override;

    // Re-implement focus out event.
    virtual void focusOutEvent(QFocusEvent* pEvent) override;

    // Re-implement handling of stepping forward and back through focusable widgets.
    virtual bool focusNextPrevChild(bool next) override;

    // Retrieve the currently-selected row and column info.
    const CurrentSelection& GetCurrentSelection() const;

    // Get the root item in the tree.
    rgEditorElement* GetRootItem() const;

    // Scroll to the given row within the PSO tree.
    void ScrollToRow(rgEditorElement* pRow);

    // Set the highlighted row index used to show search results.
    void SetHighlightedSearchResults(const rgPipelineStateSearcher::SearchResultData& searchResults);

    // Set the root item displayed in the tree.
    void SetRootItem(rgEditorElement* pItem);

    // Set the current element to the given row.
    void SetCurrentSelection(rgEditorElement* pElement, int selectedColumn = -1, bool setFocus = true);

    // Set the list widget open/close status.
    void SetEnumListWidgetStatus(bool isOpen);

signals:
    // A signal to indicate pipeline state tree focus in.
    void PipelineStateTreeFocusIn();

    // A signal to indicate pipeline state tree focus out.
    void PipelineStateTreeFocusOut();

private:
    // Compute the vertical offset to the given element, starting at a root element.
    bool ComputeVerticalOffset(rgEditorElement* pRootElement, rgEditorElement* pTargetRow, int& offset);

    // Collapse the currently selected row.
    void CollapseSelectedRow();

    // Expand the currently selected row.
    void ExpandSelectedRow();

    // Get the row directly above the given row.
    rgEditorElement* GetPreviousRow(rgEditorElement* pFrom) const;

    // Get the row directly below the given row.
    rgEditorElement* GetNextRow(rgEditorElement* pFrom) const;

    // Select the row directly above the currently selected row.
    void SelectPreviousRow();

    // Select the row directly below the currently selected row.
    void SelectNextRow();

    // Reset all previously highlighted search results.
    void ClearPreviousSearchResults(rgEditorElement* pRootElement);

    // Update the visibility and expanded state of all parent ancestors of the given item.
    void SetAncestorsVisible(rgEditorElement* pElement);

    // Update all rows in the state tree recursively, taking into account the current search results.
    void UpdateRowsWithSearchResults(rgEditorElement* pRootElement);

    // The previously highlighted search results.
    rgPipelineStateSearcher::SearchResultData m_previousSearcherResults;

    // The current search results being highlighted.
    rgPipelineStateSearcher::SearchResultData m_searcherResults;

    // The root element in the tree.
    rgEditorElement* m_pRootElement = nullptr;

    // A structure containing the current selection info.
    CurrentSelection m_currentSelection = {};

    // Keep track of whether a list widget is open so scrolling can be enabled/disabled.
    bool m_isListWidgetOpen = false;

    // The generated view object.
    Ui::rgPipelineStateTree ui;
};