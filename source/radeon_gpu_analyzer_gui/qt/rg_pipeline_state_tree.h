//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for Pso Tree scroll area.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_TREE_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_TREE_H_

// Qt.
#include <QScrollArea>

// Local.
#include "source/radeon_gpu_analyzer_gui/rg_pipeline_state_searcher.h"
#include "ui_rg_pipeline_state_tree.h"

// Forward declarations.
class RgEditorElement;

// A custom TreeView.
class RgPipelineStateTree : public QScrollArea
{
    Q_OBJECT

public:
    // A struct containing the currently selected row and column.
    // If row_ is nullptr, there is no current selection.
    struct CurrentSelection
    {
        // The currently selected row.
        RgEditorElement* selected_row = nullptr;

        // The currently selected column.
        int focused_column;
    };

public:
    explicit RgPipelineStateTree(QWidget* parent = nullptr);
    virtual ~RgPipelineStateTree() = default;

    // Re-implement eventFilter.
    bool eventFilter(QObject* object, QEvent* event) override;

    // Re-implement the key press event.
    virtual void keyPressEvent(QKeyEvent* event) override;

    // Re-implement focus in event.
    virtual void focusInEvent(QFocusEvent* event) override;

    // Re-implement focus out event.
    virtual void focusOutEvent(QFocusEvent* event) override;

    // Re-implement handling of stepping forward and back through focusable widgets.
    virtual bool focusNextPrevChild(bool next) override;

    // Retrieve the currently-selected row and column info.
    const CurrentSelection& GetCurrentSelection() const;

    // Get the root item in the tree.
    RgEditorElement* GetRootItem() const;

    // Scroll to the given row within the PSO tree.
    void ScrollToRow(RgEditorElement* row);

    // Set the highlighted row index used to show search results.
    void SetHighlightedSearchResults(const RgPipelineStateSearcher::SearchResultData& search_results);

    // Set the root item displayed in the tree.
    void SetRootItem(RgEditorElement* item);

    // Set the current element to the given row.
    void SetCurrentSelection(RgEditorElement* element, int selected_Column = -1, bool set_focus = true);

    // Set the list widget open/close status.
    void SetEnumListWidgetStatus(bool is_open);

signals:
    // A signal to indicate pipeline state tree focus in.
    void PipelineStateTreeFocusIn();

    // A signal to indicate pipeline state tree focus out.
    void PipelineStateTreeFocusOut();

protected:
    // The root element in the tree.
    RgEditorElement* root_element_ = nullptr;

private:
    // Compute the vertical offset to the given element, starting at a root element.
    bool ComputeVerticalOffset(RgEditorElement* root_element, RgEditorElement* target_row, int& offset);

    // Collapse the currently selected row.
    void CollapseSelectedRow();

    // Expand the currently selected row.
    void ExpandSelectedRow();

    // Get the row directly above the given row.
    RgEditorElement* GetPreviousRow(RgEditorElement* from) const;

    // Get the row directly below the given row.
    RgEditorElement* GetNextRow(RgEditorElement* from) const;

    // Select the row directly above the currently selected row.
    void SelectPreviousRow();

    // Select the row directly below the currently selected row.
    void SelectNextRow();

    // Reset all previously highlighted search results.
    void ClearPreviousSearchResults(RgEditorElement* root_element);

    // Update the visibility and expanded state of all parent ancestors of the given item.
    void SetAncestorsVisible(RgEditorElement* element);

    // Update all rows in the state tree recursively, taking into account the current search results.
    void UpdateRowsWithSearchResults(RgEditorElement* root_element);

    // The previously highlighted search results.
    RgPipelineStateSearcher::SearchResultData previous_searcher_results_;

    // The current search results being highlighted.
    RgPipelineStateSearcher::SearchResultData searcher_results_;

    // A structure containing the current selection info.
    CurrentSelection current_selection_ = {};

    // Keep track of whether a list widget is open so scrolling can be enabled/disabled.
    bool is_list_widget_open_ = false;

    // The generated view object.
    Ui::RgPipelineStateTree ui_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_TREE_H_
