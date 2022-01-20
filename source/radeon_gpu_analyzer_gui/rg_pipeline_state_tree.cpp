// C++.
#include <cassert>

// Qt.
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_editor_widget_enum.h"
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_editor_widget_array_element_remove.h"
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_tree.h"
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_model.h"

// A stylesheet applied to the entire pipeline state tree.
static const QString kStrPsoTreeStylesheet = "background-color: rgba(255, 255, 255, 255);";

RgPipelineStateTree::RgPipelineStateTree(QWidget* parent)
    : QScrollArea(parent)
{
    ui_.setupUi(this);

    // Apply the stylesheet to the custom tree widget.
    this->setStyleSheet(kStrPsoTreeStylesheet);

    // Enable mouse hover events for each row.
    ui_.scrollingPanel->setMouseTracking(true);
}

void RgPipelineStateTree::keyPressEvent(QKeyEvent* event)
{
    bool is_key_press_handled = false;

    // Get a pointer to the currently selected row.
    RgEditorElement* selected_row = current_selection_.selected_row;

    // Only handle key press events if there's a row already selected in the state tree.
    assert(event != nullptr);
    if (event != nullptr && selected_row != nullptr)
    {
        if (event->key() == Qt::Key_Left)
        {
            CollapseSelectedRow();

            is_key_press_handled = true;
        }
        else if (event->key() == Qt::Key_Right)
        {
            ExpandSelectedRow();

            is_key_press_handled = true;
        }
        else if (event->key() == Qt::Key_Up)
        {
            SelectPreviousRow();

            is_key_press_handled = true;
        }
        else if (event->key() == Qt::Key_Down)
        {
            SelectNextRow();

            is_key_press_handled = true;
        }
    }

    if (!is_key_press_handled)
    {
        // Invoke the base implementation.
        QScrollArea::keyPressEvent(event);
    }
}

void RgPipelineStateTree::CollapseSelectedRow()
{
    RgEditorElement* selected_row = current_selection_.selected_row;
    assert(selected_row != nullptr);
    if (selected_row != nullptr)
    {
        int num_children = selected_row->ChildCount();

        // If the row is currently expanded, collapse it.
        if (num_children > 0 && selected_row->GetExpansionState() == RgRowExpansionState::kExpanded)
        {
            selected_row->SetExpansionState(RgRowExpansionState::kCollapsed);
        }
        else
        {
            // If the row is currently collapsed, select the parent row.
            RgEditorElement* parent_element = selected_row->GetParentItem();
            if (parent_element != nullptr)
            {
                // Select the parent row.
                SetCurrentSelection(parent_element, current_selection_.focused_column);

                // The parent element got selected, but it may not be visible.
                // Scroll to the newly selected parent row if necessary.
                ScrollToRow(parent_element);
            }
        }
    }
}

void RgPipelineStateTree::ExpandSelectedRow()
{
    RgEditorElement* selected_row = current_selection_.selected_row;
    assert(selected_row != nullptr);
    if (selected_row != nullptr)
    {
        selected_row->SetExpansionState(RgRowExpansionState::kExpanded);
    }
}

RgEditorElement* RgPipelineStateTree::GetPreviousRow(RgEditorElement* from) const
{
    RgEditorElement* previous_row = nullptr;

    assert(from != nullptr);
    if (from != nullptr && from != root_element_)
    {
        // Does the currently selected row have an index of 1 or more?
        int row_index = from->GetRowIndex();
        if (row_index >= 1)
        {
            RgEditorElement* parent_row = from->GetParentItem();
            if (parent_row != nullptr)
            {
                // Find the oldest visible sibling.
                for (int child_index = row_index - 1; child_index >= 0; child_index--)
                {
                    // Get the sibling element directly above the selected row.
                    RgEditorElement* previous_sibling = parent_row->GetChild(child_index);
                    assert(previous_sibling != nullptr);
                    if (previous_sibling != nullptr && previous_sibling->isVisible())
                    {
                        // If the sibling is collapsed, stop searching and select the sibling.
                        int num_children = previous_sibling->ChildCount();
                        if (num_children == 0 || previous_sibling->GetExpansionState() == RgRowExpansionState::kCollapsed)
                        {
                            previous_row = previous_sibling;
                        }
                        else
                        {
                            // Find the last descendant child of the previous sibling.
                            previous_row = previous_sibling->GetLastVisibleDescendant();
                        }
                        break;
                    }
                }

                // If none of the older siblings are visible, revert to
                // selecting the parent, which must be visible.
                if (previous_row == nullptr)
                {
                    previous_row = parent_row;
                }
            }
        }
        else
        {
            // The row is the first child. Step up to the parent row.
            RgEditorElement* parent_element = from->GetParentItem();
            assert(parent_element != nullptr);
            if (parent_element != nullptr)
            {
                previous_row = parent_element;
            }
        }
    }

    return previous_row;
}

RgEditorElement* RgPipelineStateTree::GetNextRow(RgEditorElement* from) const
{
    RgEditorElement* next_row = nullptr;

    assert(from != nullptr);
    if (from != nullptr)
    {
        // Does the currently selected row have any children? Is the current row expanded?
        int num_children = from->ChildCount();
        RgRowExpansionState expand_state = from->GetExpansionState();
        if (num_children > 0 && expand_state == RgRowExpansionState::kExpanded)
        {
            // Find the first visible child.
            for (int child_index = 0; child_index < num_children; ++child_index)
            {
                RgEditorElement* child = from->GetChild(child_index);
                assert(child != nullptr);
                if (child != nullptr && child->isVisible())
                {
                    // Select the first non-filtered child.
                    next_row = child;
                    break;
                }
            }
        }
        else
        {
            // Need to find the first parent ancestor with a single child.
            next_row = from->FindNextAnscestor();
        }
    }

    return next_row;
}

void RgPipelineStateTree::SelectPreviousRow()
{
    if (current_selection_.selected_row != nullptr)
    {
        // Get the row directly above the currently selected row.
        RgEditorElement* previous_row = GetPreviousRow(current_selection_.selected_row);

        // Only select and scroll to the new row if necessary.
        if (previous_row != nullptr)
        {
            // Update the currently selected row.
            SetCurrentSelection(previous_row);

            // Scroll to the new row if necessary.
            ScrollToRow(previous_row);
        }
    }
}

void RgPipelineStateTree::SelectNextRow()
{
    if (current_selection_.selected_row != nullptr)
    {
        // Get the next row directly underneath the currently selected row.
        RgEditorElement* next_row = GetNextRow(current_selection_.selected_row);

        // Only select and scroll to the new row if necessary.
        if (next_row != nullptr)
        {
            // Update the currently selected row.
            SetCurrentSelection(next_row);

            // Scroll to the new row if necessary.
            ScrollToRow(next_row);
        }
    }
}

void RgPipelineStateTree::focusInEvent(QFocusEvent* event)
{
    emit PipelineStateTreeFocusIn();

    QScrollArea::focusInEvent(event);
}

void RgPipelineStateTree::focusOutEvent(QFocusEvent* event)
{
    emit PipelineStateTreeFocusOut();

    QScrollArea::focusOutEvent(event);
}

bool RgPipelineStateTree::focusNextPrevChild(bool next)
{
    bool res = false;

    RgEditorElement* next_focus_row = nullptr;

    // Only attempt to transfer focus if there's a selected row.
    if (current_selection_.selected_row != nullptr)
    {
        if (next)
        {
            // If advancing to the next item, find the row directly below the selected row.
            next_focus_row = GetNextRow(current_selection_.selected_row);
        }
        else
        {
            // If stepping backwards to the previous item, find the row above the selected row.
            next_focus_row = GetPreviousRow(current_selection_.selected_row);
        }

        // Was the next row to focus on found?
        if (next_focus_row != nullptr)
        {
            // Update the currently selected row to the next focused row.
            SetCurrentSelection(next_focus_row);

            // Scroll to the new row if necessary.
            ScrollToRow(next_focus_row);

            // Focus on the editor widget within the row (if one exists).
            RgPipelineStateEditorWidget* editor_widget = next_focus_row->GetEditorWidget();
            if (editor_widget != nullptr)
            {
                // Successfully focused on the next focus target.
                editor_widget->setFocus();
                res = true;
            }
            else
            {
                // Focus on the row itself if there is no editor widget.
                next_focus_row->setFocus();
            }
        }
    }
    else
    {
        // Invoke default handling to find the next focused widget.
        res = QScrollArea::focusNextPrevChild(next);
    }

    return res;
}

const RgPipelineStateTree::CurrentSelection& RgPipelineStateTree::GetCurrentSelection() const
{
    return current_selection_;
}

RgEditorElement* RgPipelineStateTree::GetRootItem() const
{
    return root_element_;
}

bool RgPipelineStateTree::ComputeVerticalOffset(RgEditorElement* root_element, RgEditorElement* target_row, int& offset)
{
    bool ret = false;

    assert(root_element != nullptr);
    assert(target_row != nullptr);
    if (root_element != nullptr && target_row != nullptr)
    {
        // We found the target row to count up to- we're done iterating.
        if (root_element == target_row)
        {
            ret = true;
        }
        else
        {
            // Add the height of the current row to the vertical offset.
            offset += root_element->GetRowHeight();

            // Only take child rows into account if the parent row is expanded and not filtered out.
            if (root_element->GetExpansionState() == RgRowExpansionState::kExpanded &&
                !root_element->is_filtered_out_)
            {
                int num_children = root_element->ChildCount();
                for (int child_index = 0; child_index < num_children; ++child_index)
                {
                    // Recursively add child height to the vertical offset.
                    RgEditorElement* child = root_element->GetChild(child_index);
                    ret = ComputeVerticalOffset(child, target_row, offset);
                    if (ret)
                    {
                        break;
                    }
                }
            }
        }
    }

    return ret;
}

void RgPipelineStateTree::ScrollToRow(RgEditorElement* row)
{
    assert(row != nullptr);
    if (row != nullptr)
    {
        // Update the vertical scrollbar's position to bring the target widget into view.
        QScrollBar* vertical_scrollbar = this->verticalScrollBar();
        assert(vertical_scrollbar != nullptr);
        if (vertical_scrollbar != nullptr)
        {
            // What's the current scroll position? What's the vertical extent of the rows that are
            // currently being displayed?
            int visible_tree_height = this->height();
            int current_slider_position = vertical_scrollbar->sliderPosition();
            int current_vertical_extent_shown = current_slider_position + visible_tree_height;

            // Compute the vertical offset for the given row item.
            int vertical_offset = 0;
            ComputeVerticalOffset(root_element_, row, vertical_offset);

            int element_height = row->GetRowHeight();

            // Scroll the tree only when the target element is not currently visible, and scrolling
            // is necessary to bring the target element into view.
            bool is_scrolling_up = vertical_offset < current_slider_position;
            bool is_scrolling_down = vertical_offset + element_height > current_vertical_extent_shown;
            if (is_scrolling_up)
            {
                // When scrolling up, set the slider to the vertical offset of the target row.
                vertical_scrollbar->setSliderPosition(vertical_offset);
            }
            else if (is_scrolling_down)
            {
                int current_position = vertical_scrollbar->sliderPosition();

                // Is there a big difference between the current scroll position and the new one?
                if (vertical_offset - current_position > visible_tree_height)
                {
                    // If the gap is large enough, snap the scrollbar to the row.
                    vertical_scrollbar->setSliderPosition(vertical_offset);
                }
                else
                {
                    // When scrolling down, offset the slider position by the height of the row.
                    // This results in sliding the lower row into view at the bottom of the tree.
                    vertical_scrollbar->setSliderPosition(current_position + element_height);
                }
            }
        }
    }
}

void RgPipelineStateTree::SetHighlightedSearchResults(const RgPipelineStateSearcher::SearchResultData& search_results)
{
    // Save the search results.
    previous_searcher_results_ = searcher_results_;

    // Update the search results structure.
    searcher_results_ = search_results;

    // Update the highlight state of all rows in the tree.
    UpdateRowsWithSearchResults(root_element_);
}

void RgPipelineStateTree::SetRootItem(RgEditorElement* item)
{
    assert(item != nullptr);
    if (item != nullptr)
    {
        // Erase knowledge of the current element, since it will be deleted shortly.
        SetCurrentSelection(nullptr);

        // Replace the current root element with the provided element.
        root_element_ = item;

        // Set the root element's parent state tree, so that all elements in the tree
        // are aware of the widget they are ultimately parented to in the hierarchy.
        root_element_->SetParentStateTree(this);

        QVBoxLayout* vertical_layout = static_cast<QVBoxLayout*>(ui_.scrollingPanel->layout());
        assert(vertical_layout != nullptr);
        if (vertical_layout != nullptr)
        {
            // When a root element has already been inserted, there are two children attached to
            // the scrolling panel. One is the root element of the tree to be removed, and the
            // other is a spacer used to push all elements up. Remove and destroy the original root
            // element before inserting the new root element above the spacer.
            if (ui_.scrollingPanel->children().size() == 2)
            {
                QLayoutItem* root_layout = vertical_layout->takeAt(0);
                if (root_layout != nullptr)
                {
                    // Destroy the existing root element.
                    QWidget* root_item = root_layout->widget();
                    if (root_item != nullptr)
                    {
                        vertical_layout->removeWidget(root_item);
                        RG_SAFE_DELETE(root_item);
                    }
                }
            }

            // Add the given root tree item to the scrolling panel above the spacer.
            vertical_layout->insertWidget(0, root_element_);

            // Initialize all rows in the tree.
            root_element_->InitializeRows();
        }
    }
}

// A predicate used to find the given rgEditorElement within a list.
struct RgRowSearcher
{
    RgRowSearcher(RgEditorElement* element) : element(element) { }

    // Check if the occurrence matches the target row.
    bool operator()(const RgPipelineStateSearcher::OccurrenceLocation& occurrence) const
    {
        return occurrence.result_row == element;
    }

    // The element to search for.
    RgEditorElement* element = nullptr;
};

void RgPipelineStateTree::ClearPreviousSearchResults(RgEditorElement* root_element)
{
    auto first_occurrence = previous_searcher_results_.result_occurrences.begin();
    auto last_occurrence = previous_searcher_results_.result_occurrences.end();

    // Update the "is filtered" flag for each row. Don't filter anything if there aren't any search results.
    bool is_filtering_enabled = previous_searcher_results_.search_options.filter_tree &&
        !previous_searcher_results_.result_occurrences.empty();
    assert(root_element != nullptr);
    if (root_element != nullptr)
    {
        root_element->is_filtered_out_ = is_filtering_enabled;

        // Try to find the given element within the list of search results.
        RgRowSearcher searcher(root_element);
        auto occurrenceIter = std::find_if(first_occurrence, last_occurrence, searcher);

        // Iterate over m_searcherResults.m_resultOccurrences and highlight every
        // occurrence found on the current line.
        int count = 0;
        while (occurrenceIter != last_occurrence)
        {
            if (occurrenceIter != last_occurrence)
            {
                root_element->is_filtered_out_ = false;

                // Highlight the sub string in various columns.
                if ((*occurrenceIter).row_data_index == static_cast<int>(RgRowData::kRowDataMemberName))
                {
                    // Column zero is occupied by a QLabel.
                    // Find and reset the substring in a QLabel.
                    root_element->ResetLabelSubString();
                }
                else if ((*occurrenceIter).row_data_index == static_cast<int>(RgRowData::kRowDataMemberValue))
                {
                    // Check to see if this is a button or a numeric editor.
                    RgPipelineStateEditorWidgetEnum* element = qobject_cast<RgPipelineStateEditorWidgetEnum*>(root_element);
                    if (element != nullptr)
                    {
                        root_element->ResetButtonSubString();
                    }
                    else
                    {
                        root_element->ResetLineEditSubString();
                    }
                }
            }

            // Find the next occurrence.
            occurrenceIter = std::find_if(occurrenceIter + 1, last_occurrence, searcher);
            count++;
        }
    }
}

void RgPipelineStateTree::UpdateRowsWithSearchResults(RgEditorElement* root_element)
{
    // First clear any previously highlighted search results.
    ClearPreviousSearchResults(root_element);

    auto first_occurrence = searcher_results_.result_occurrences.begin();
    auto last_occurrence = searcher_results_.result_occurrences.end();

    // Update the "is filtered" flag for each row. Don't filter anything if there aren't any search results.
    bool is_filtering_enabled = searcher_results_.search_options.filter_tree &&
        !searcher_results_.result_occurrences.empty();
    root_element->is_filtered_out_ = is_filtering_enabled;

    // Try to find the given element within the list of search results.
    RgRowSearcher searcher(root_element);
    auto occurrence_iter = std::find_if(first_occurrence, last_occurrence, searcher);

    // Clear the previous search string data.
    root_element->ClearSearchStringData();

    // Iterate over m_searcherResults.m_resultOccurrences and highlight every
    // occurrence found on the current line.
    int prev_data_index = 0;
    if (occurrence_iter != last_occurrence)
    {
        prev_data_index = occurrence_iter->row_data_index;
    }

    while (occurrence_iter != last_occurrence)
    {
        bool is_current_match = false;
        if (occurrence_iter != last_occurrence)
        {
            root_element->is_filtered_out_ = false;

            // Highlight the current match vs other matches.
            int occurrence_index = (occurrence_iter - first_occurrence);
            if (occurrence_index == searcher_results_.selected_index)
            {
                // The current result is being focused on.
                is_current_match = true;
            }
            else
            {
                // This search result is only a single occurrence that needs to
                // be highlighted- it's not the current result.
                is_current_match = false;
            }

            // Highlight the row with the current match.
            if (is_current_match)
            {
                int column_index = searcher_results_.result_occurrences[occurrence_index].row_data_index;
                SetCurrentSelection(root_element, column_index, false);
            }

            // Highlight the sub string in various columns.
            if ((*occurrence_iter).row_data_index == static_cast<int>(RgRowData::kRowDataMemberName))
            {
                // Column zero is occupied by a QLabel.
                // Find and highlight the substring in a QLabel.
                root_element->HighlightLabelSubString((*occurrence_iter).character_index, searcher_results_.search_string, is_current_match);
            }
            else if ((*occurrence_iter).row_data_index == static_cast<int>(RgRowData::kRowDataMemberValue))
            {
                // Check to see if this is a button or a numeric editor.
                RgEditorElementEnum* element = qobject_cast<RgEditorElementEnum*>(root_element);
                if (element != nullptr)
                {
                    root_element->HighlightButtonSubString((*occurrence_iter).character_index, searcher_results_.search_string, is_current_match);
                }
                else
                {
                    root_element->HighlightLineEditSubString((*occurrence_iter).character_index, searcher_results_.search_string, is_current_match);
                }
            }
        }

        prev_data_index = occurrence_iter->row_data_index;
        occurrence_iter = std::find_if(occurrence_iter + 1, last_occurrence, searcher);

        // Clear the previous search string data, process the next match.
        if (occurrence_iter != last_occurrence)
        {
            if (prev_data_index != occurrence_iter->row_data_index)
            {
                root_element->ClearSearchStringDataVector();
            }
        }
    }

    // If filtering is enabled, determine if the row should be visible or hidden.
    if (is_filtering_enabled)
    {
        if (!root_element->is_filtered_out_)
        {
            // If the current row is included in the filtered search results,
            // expand and make visible all parent ancestors.
            root_element->SetExpansionState(RgRowExpansionState::kExpanded);
            SetAncestorsVisible(root_element);
        }
        else
        {
            // Hide the row if it's not relevant in the filtered search results.
            root_element->hide();
        }
    }
    else
    {
        // If filtering is not enabled, all rows should be visible.
        root_element->show();
        root_element->SetExpansionState(RgRowExpansionState::kExpanded);
    }

    // Traverse to each child element.
    int num_children = root_element->ChildCount();
    for (int child_index = 0; child_index < num_children; ++child_index)
    {
        // Update each child element.
        RgEditorElement* child = root_element->GetChild(child_index);
        UpdateRowsWithSearchResults(child);
    }
}

void RgPipelineStateTree::SetAncestorsVisible(RgEditorElement* element)
{
    if (element != nullptr)
    {
        RgEditorElement* parent = element->GetParentItem();
        if (parent != nullptr)
        {
            // Make the parent visible and expanded.
            parent->show();
            parent->SetExpansionState(RgRowExpansionState::kExpanded);

            // Update parent visibility recursively up to the root element.
            SetAncestorsVisible(parent);
        }
    }
}

bool RgPipelineStateTree::eventFilter(QObject* object, QEvent* event)
{
    Q_UNUSED(object);

    bool is_filtered = false;

    if (event != nullptr)
    {
        if (event->type() == QEvent::Type::Wheel)
        {
            // If any of the drop downs are open, ignore the wheel event.
            if (is_list_widget_open_)
            {
                is_filtered = true;
            }
        }
    }

    return is_filtered;
}

void RgPipelineStateTree::SetEnumListWidgetStatus(bool is_open)
{
    is_list_widget_open_ = is_open;

    // Enable/disable the scrollbars.
    if (is_open)
    {
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
    else
    {
        setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }
}

void RgPipelineStateTree::SetCurrentSelection(RgEditorElement* element, int column, bool set_focus)
{
    // If there's already a current element, deselect it.
    if (current_selection_.selected_row != nullptr)
    {
        current_selection_.selected_row->RemoveStyleFlag(RgStyleFlags::CurrentRow);
    }

    // If the new element is valid, select it.
    if (element != nullptr)
    {
        element->AddStyleFlag(RgStyleFlags::CurrentRow);
    }

    current_selection_.selected_row = element;
    current_selection_.focused_column = column;

    if (set_focus && element != nullptr)
    {
        if (column != -1)
        {
            // Focus on the given column.
            element->SetFocusedColumn(static_cast<RgRowData>(column));
        }
        else
        {
            // Focus on the overall row to allow arrow key navigation.
            element->setFocus();
        }
    }
}
