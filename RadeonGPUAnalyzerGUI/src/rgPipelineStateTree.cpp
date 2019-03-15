// C++.
#include <cassert>

// Qt.
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElementArray.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateEditorWidgetEnum.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateEditorWidgetArray.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateEditorWidgetArrayElement.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateTree.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateModel.h>

// A stylesheet applied to the entire pipeline state tree.
static const QString s_PSO_TREE_STYLESHEET = "background-color: rgb(255, 255, 255, 255);";

rgPipelineStateTree::rgPipelineStateTree(QWidget* pParent)
    : QScrollArea(pParent)
{
    ui.setupUi(this);

    // Apply the stylesheet to the custom tree widget.
    this->setStyleSheet(s_PSO_TREE_STYLESHEET);

    // Enable mouse hover events for each row.
    ui.scrollingPanel->setMouseTracking(true);
}

void rgPipelineStateTree::keyPressEvent(QKeyEvent* pEvent)
{
    bool isKeyPressHandled = false;

    // Get a pointer to the currently selected row.
    rgEditorElement* pSelectedRow = m_currentSelection.m_pSelectedRow;

    // Only handle key press events if there's a row already selected in the state tree.
    assert(pEvent != nullptr);
    if (pEvent != nullptr && pSelectedRow != nullptr)
    {
        if (pEvent->key() == Qt::Key_Left)
        {
            CollapseSelectedRow();

            isKeyPressHandled = true;
        }
        else if (pEvent->key() == Qt::Key_Right)
        {
            ExpandSelectedRow();

            isKeyPressHandled = true;
        }
        else if (pEvent->key() == Qt::Key_Up)
        {
            SelectPreviousRow();

            isKeyPressHandled = true;
        }
        else if (pEvent->key() == Qt::Key_Down)
        {
            SelectNextRow();

            isKeyPressHandled = true;
        }
    }

    if (!isKeyPressHandled)
    {
        // Invoke the base implementation.
        QScrollArea::keyPressEvent(pEvent);
    }
}

void rgPipelineStateTree::CollapseSelectedRow()
{
    rgEditorElement* pSelectedRow = m_currentSelection.m_pSelectedRow;
    assert(pSelectedRow != nullptr);
    if (pSelectedRow != nullptr)
    {
        int numChildren = pSelectedRow->ChildCount();

        // If the row is currently expanded, collapse it.
        if (numChildren > 0 && pSelectedRow->GetExpansionState() == rgRowExpansionState::Expanded)
        {
            pSelectedRow->SetExpansionState(rgRowExpansionState::Collapsed);
        }
        else
        {
            // If the row is currently collapsed, select the parent row.
            rgEditorElement* pParentElement = pSelectedRow->GetParentItem();
            if (pParentElement != nullptr)
            {
                // Select the parent row.
                SetCurrentSelection(pParentElement, m_currentSelection.m_focusedColumn);

                // The parent element got selected, but it may not be visible.
                // Scroll to the newly selected parent row if necessary.
                ScrollToRow(pParentElement);
            }
        }
    }
}

void rgPipelineStateTree::ExpandSelectedRow()
{
    rgEditorElement* pSelectedRow = m_currentSelection.m_pSelectedRow;
    assert(pSelectedRow != nullptr);
    if (pSelectedRow != nullptr)
    {
        pSelectedRow->SetExpansionState(rgRowExpansionState::Expanded);
    }
}

rgEditorElement* rgPipelineStateTree::GetPreviousRow(rgEditorElement* pFrom) const
{
    rgEditorElement* pPreviousRow = nullptr;

    assert(pFrom != nullptr);
    if (pFrom != nullptr && pFrom != m_pRootElement)
    {
        // Does the currently selected row have an index of 1 or more?
        int rowIndex = pFrom->GetRowIndex();
        if (rowIndex >= 1)
        {
            rgEditorElement* pParentRow = pFrom->GetParentItem();
            if (pParentRow != nullptr)
            {
                // Find the oldest visible sibling.
                for (int childIndex = rowIndex - 1; childIndex >= 0; childIndex--)
                {
                    // Get the sibling element directly above the selected row.
                    rgEditorElement* pPreviousSibling = pParentRow->GetChild(childIndex);
                    assert(pPreviousSibling != nullptr);
                    if (pPreviousSibling != nullptr && pPreviousSibling->isVisible())
                    {
                        // If the sibling is collapsed, stop searching and select the sibling.
                        int numChildren = pPreviousSibling->ChildCount();
                        if (numChildren == 0 || pPreviousSibling->GetExpansionState() == rgRowExpansionState::Collapsed)
                        {
                            pPreviousRow = pPreviousSibling;
                        }
                        else
                        {
                            // Find the last descendant child of the previous sibling.
                            pPreviousRow = pPreviousSibling->GetLastVisibleDescendant();
                        }
                        break;
                    }
                }

                // If none of the older siblings are visible, revert to
                // selecting the parent, which must be visible.
                if (pPreviousRow == nullptr)
                {
                    pPreviousRow = pParentRow;
                }
            }
        }
        else
        {
            // The row is the first child. Step up to the parent row.
            rgEditorElement* pParentElement = pFrom->GetParentItem();
            assert(pParentElement != nullptr);
            if (pParentElement != nullptr)
            {
                pPreviousRow = pParentElement;
            }
        }
    }

    return pPreviousRow;
}

rgEditorElement* rgPipelineStateTree::GetNextRow(rgEditorElement* pFrom) const
{
    rgEditorElement* pNextRow = nullptr;

    assert(pFrom != nullptr);
    if (pFrom != nullptr)
    {
        // Does the currently selected row have any children? Is the current row expanded?
        int numChildren = pFrom->ChildCount();
        rgRowExpansionState expandState = pFrom->GetExpansionState();
        if (numChildren > 0 && expandState == rgRowExpansionState::Expanded)
        {
            // Find the first visible child.
            for (int childIndex = 0; childIndex < numChildren; ++childIndex)
            {
                rgEditorElement* pChild = pFrom->GetChild(childIndex);
                assert(pChild != nullptr);
                if (pChild != nullptr && pChild->isVisible())
                {
                    // Select the first non-filtered child.
                    pNextRow = pChild;
                    break;
                }
            }
        }
        else
        {
            // Need to find the first parent ancestor with a single child.
            pNextRow = pFrom->FindNextAnscestor();
        }
    }

    return pNextRow;
}

void rgPipelineStateTree::SelectPreviousRow()
{
    if (m_currentSelection.m_pSelectedRow != nullptr)
    {
        // Get the row directly above the currently selected row.
        rgEditorElement* pPreviousRow = GetPreviousRow(m_currentSelection.m_pSelectedRow);

        // Only select and scroll to the new row if necessary.
        if (pPreviousRow != nullptr)
        {
            // Update the currently selected row.
            SetCurrentSelection(pPreviousRow);

            // Scroll to the new row if necessary.
            ScrollToRow(pPreviousRow);
        }
    }
}

void rgPipelineStateTree::SelectNextRow()
{
    if (m_currentSelection.m_pSelectedRow != nullptr)
    {
        // Get the next row directly underneath the currently selected row.
        rgEditorElement* pNextRow = GetNextRow(m_currentSelection.m_pSelectedRow);

        // Only select and scroll to the new row if necessary.
        if (pNextRow != nullptr)
        {
            // Update the currently selected row.
            SetCurrentSelection(pNextRow);

            // Scroll to the new row if necessary.
            ScrollToRow(pNextRow);
        }
    }
}

void rgPipelineStateTree::focusInEvent(QFocusEvent* pEvent)
{
    emit PipelineStateTreeFocusIn();

    QScrollArea::focusInEvent(pEvent);
}

void rgPipelineStateTree::focusOutEvent(QFocusEvent* pEvent)
{
    emit PipelineStateTreeFocusOut();

    QScrollArea::focusOutEvent(pEvent);
}

bool rgPipelineStateTree::focusNextPrevChild(bool next)
{
    bool res = false;

    rgEditorElement* pNextFocusRow = nullptr;

    // Only attempt to transfer focus if there's a selected row.
    if (m_currentSelection.m_pSelectedRow != nullptr)
    {
        if (next)
        {
            // If advancing to the next item, find the row directly below the selected row.
            pNextFocusRow = GetNextRow(m_currentSelection.m_pSelectedRow);
        }
        else
        {
            // If stepping backwards to the previous item, find the row above the selected row.
            pNextFocusRow = GetPreviousRow(m_currentSelection.m_pSelectedRow);
        }

        // Was the next row to focus on found?
        if (pNextFocusRow != nullptr)
        {
            // Update the currently selected row to the next focused row.
            SetCurrentSelection(pNextFocusRow);

            // Scroll to the new row if necessary.
            ScrollToRow(pNextFocusRow);

            // Focus on the editor widget within the row (if one exists).
            rgPipelineStateEditorWidget* pEditorWidget = pNextFocusRow->GetEditorWidget();
            if (pEditorWidget != nullptr)
            {
                // Successfully focused on the next focus target.
                pEditorWidget->setFocus();
                res = true;
            }
            else
            {
                // Focus on the row itself if there is no editor widget.
                pNextFocusRow->setFocus();
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

const rgPipelineStateTree::CurrentSelection& rgPipelineStateTree::GetCurrentSelection() const
{
    return m_currentSelection;
}

rgEditorElement* rgPipelineStateTree::GetRootItem() const
{
    return m_pRootElement;
}

bool rgPipelineStateTree::ComputeVerticalOffset(rgEditorElement* pRootElement, rgEditorElement* pTargetRow, int& offset)
{
    bool ret = false;

    assert(pRootElement != nullptr);
    assert(pTargetRow != nullptr);
    if (pRootElement != nullptr && pTargetRow != nullptr)
    {
        // We found the target row to count up to- we're done iterating.
        if (pRootElement == pTargetRow)
        {
            ret = true;
        }
        else
        {
            // Add the height of the current row to the vertical offset.
            offset += pRootElement->GetRowHeight();

            // Only take child rows into account if the parent row is expanded and not filtered out.
            if (pRootElement->GetExpansionState() == rgRowExpansionState::Expanded &&
                !pRootElement->m_isFilteredOut)
            {
                int numChildren = pRootElement->ChildCount();
                for (int childIndex = 0; childIndex < numChildren; ++childIndex)
                {
                    // Recursively add child height to the vertical offset.
                    rgEditorElement* pChild = pRootElement->GetChild(childIndex);
                    ret = ComputeVerticalOffset(pChild, pTargetRow, offset);
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

void rgPipelineStateTree::ScrollToRow(rgEditorElement* pRow)
{
    assert(pRow != nullptr);
    if (pRow != nullptr)
    {
        // Update the vertical scrollbar's position to bring the target widget into view.
        QScrollBar* pVerticalScrollbar = this->verticalScrollBar();
        assert(pVerticalScrollbar != nullptr);
        if (pVerticalScrollbar != nullptr)
        {
            // What's the current scroll position? What's the vertical extent of the rows that are
            // currently being displayed?
            int visibleTreeHeight = this->height();
            int currentSliderPosition = pVerticalScrollbar->sliderPosition();
            int currentVerticalExtentShown = currentSliderPosition + visibleTreeHeight;

            // Compute the vertical offset for the given row item.
            int verticalOffset = 0;
            ComputeVerticalOffset(m_pRootElement, pRow, verticalOffset);

            int elementHeight = pRow->GetRowHeight();

            // Scroll the tree only when the target element is not currently visible, and scrolling
            // is necessary to bring the target element into view.
            bool isScrollingUp = verticalOffset < currentSliderPosition;
            bool isScrollingDown = verticalOffset + elementHeight > currentVerticalExtentShown;
            if (isScrollingUp)
            {
                // When scrolling up, set the slider to the vertical offset of the target row.
                pVerticalScrollbar->setSliderPosition(verticalOffset);
            }
            else if (isScrollingDown)
            {
                int currentPosition = pVerticalScrollbar->sliderPosition();

                // Is there a big difference between the current scroll position and the new one?
                if (verticalOffset - currentPosition > visibleTreeHeight)
                {
                    // If the gap is large enough, snap the scrollbar to the row.
                    pVerticalScrollbar->setSliderPosition(verticalOffset);
                }
                else
                {
                    // When scrolling down, offset the slider position by the height of the row.
                    // This results in sliding the lower row into view at the bottom of the tree.
                    pVerticalScrollbar->setSliderPosition(currentPosition + elementHeight);
                }
            }
        }
    }
}

void rgPipelineStateTree::SetHighlightedSearchResults(const rgPipelineStateSearcher::SearchResultData& searchResults)
{
    // Save the search results.
    m_previousSearcherResults = m_searcherResults;

    // Update the search results structure.
    m_searcherResults = searchResults;

    // Update the highlight state of all rows in the tree.
    UpdateRowsWithSearchResults(m_pRootElement);
}

void rgPipelineStateTree::SetRootItem(rgEditorElement* pItem)
{
    assert(pItem != nullptr);
    if (pItem != nullptr)
    {
        // Erase knowledge of the current element, since it will be deleted shortly.
        SetCurrentSelection(nullptr);

        // Replace the current root element with the provided element.
        m_pRootElement = pItem;

        // Set the root element's parent state tree, so that all elements in the tree
        // are aware of the widget they are ultimately parented to in the hierarchy.
        m_pRootElement->SetParentStateTree(this);

        QVBoxLayout* pVerticalLayout = static_cast<QVBoxLayout*>(ui.scrollingPanel->layout());
        assert(pVerticalLayout != nullptr);
        if (pVerticalLayout != nullptr)
        {
            // When a root element has already been inserted, there are two children attached to
            // the scrolling panel. One is the root element of the tree to be removed, and the
            // other is a spacer used to push all elements up. Remove and destroy the original root
            // element before inserting the new root element above the spacer.
            if (ui.scrollingPanel->children().size() == 2)
            {
                QLayoutItem* pRootLayout = pVerticalLayout->takeAt(0);
                if (pRootLayout != nullptr)
                {
                    // Destroy the existing root element.
                    QWidget* pRootItem = pRootLayout->widget();
                    if (pRootItem != nullptr)
                    {
                        pVerticalLayout->removeWidget(pRootItem);
                        RG_SAFE_DELETE(pRootItem);
                    }
                }
            }

            // Add the given root tree item to the scrolling panel above the spacer.
            pVerticalLayout->insertWidget(0, m_pRootElement);

            // Initialize all rows in the tree.
            m_pRootElement->InitializeRows();
        }
    }
}

// A predicate used to find the given rgEditorElement within a list.
struct rgRowSearcher
{
    rgRowSearcher(rgEditorElement* m_pElement) : m_pElement(m_pElement) { }

    // Check if the occurrence matches the target row.
    bool operator()(const rgPipelineStateSearcher::OccurrenceLocation& occurrence) const
    {
        return occurrence.m_pResultRow == m_pElement;
    }

    // The element to search for.
    rgEditorElement* m_pElement = nullptr;
};

void rgPipelineStateTree::ClearPreviousSearchResults(rgEditorElement* pRootElement)
{
    auto firstOccurrence = m_previousSearcherResults.m_resultOccurrences.begin();
    auto lastOccurrence = m_previousSearcherResults.m_resultOccurrences.end();

    // Update the "is filtered" flag for each row. Don't filter anything if there aren't any search results.
    bool isFilteringEnabled = m_previousSearcherResults.m_searchOptions.m_filterTree &&
        !m_previousSearcherResults.m_resultOccurrences.empty();
    assert(pRootElement != nullptr);
    if (pRootElement != nullptr)
    {
        pRootElement->m_isFilteredOut = isFilteringEnabled;

        // Try to find the given element within the list of search results.
        rgRowSearcher searcher(pRootElement);
        auto occurrenceIter = std::find_if(firstOccurrence, lastOccurrence, searcher);

        // Iterate over m_searcherResults.m_resultOccurrences and highlight every
        // occurrence found on the current line.
        int count = 0;
        while (occurrenceIter != lastOccurrence)
        {
            if (occurrenceIter != lastOccurrence)
            {
                pRootElement->m_isFilteredOut = false;

                // Highlight the sub string in various columns.
                if ((*occurrenceIter).m_rowDataIndex == static_cast<int>(rgRowData::RowDataMemberName))
                {
                    // Column zero is occupied by a QLabel.
                    // Find and reset the substring in a QLabel.
                    pRootElement->ResetLabelSubString();
                }
                else if ((*occurrenceIter).m_rowDataIndex == static_cast<int>(rgRowData::RowDataMemberValue))
                {
                    // Check to see if this is a button or a numeric editor.
                    rgPipelineStateEditorWidgetEnum* pElement = qobject_cast<rgPipelineStateEditorWidgetEnum*>(pRootElement);
                    if (pElement != nullptr)
                    {
                        pRootElement->ResetButtonSubString();
                    }
                    else
                    {
                        pRootElement->ResetLineEditSubString();
                    }
                }
            }

            // Find the next occurrence.
            occurrenceIter = std::find_if(occurrenceIter + 1, lastOccurrence, searcher);
            count++;
        }
    }
}

void rgPipelineStateTree::UpdateRowsWithSearchResults(rgEditorElement* pRootElement)
{
    // First clear any previously highlighted search results.
    ClearPreviousSearchResults(pRootElement);

    auto firstOccurrence = m_searcherResults.m_resultOccurrences.begin();
    auto lastOccurrence = m_searcherResults.m_resultOccurrences.end();

    // Update the "is filtered" flag for each row. Don't filter anything if there aren't any search results.
    bool isFilteringEnabled = m_searcherResults.m_searchOptions.m_filterTree &&
        !m_searcherResults.m_resultOccurrences.empty();
    pRootElement->m_isFilteredOut = isFilteringEnabled;

    // Try to find the given element within the list of search results.
    rgRowSearcher searcher(pRootElement);
    auto occurrenceIter = std::find_if(firstOccurrence, lastOccurrence, searcher);

    // Clear the previous search string data.
    pRootElement->ClearSearchStringData();

    // Iterate over m_searcherResults.m_resultOccurrences and highlight every
    // occurrence found on the current line.
    int prevDataIndex = 0;
    if (occurrenceIter != lastOccurrence)
    {
        prevDataIndex = occurrenceIter->m_rowDataIndex;
    }

    while (occurrenceIter != lastOccurrence)
    {
        bool isCurrentMatch = false;
        if (occurrenceIter != lastOccurrence)
        {
            pRootElement->m_isFilteredOut = false;

            // Highlight the current match vs other matches.
            int occurrenceIndex = (occurrenceIter - firstOccurrence);
            if (occurrenceIndex == m_searcherResults.m_selectedIndex)
            {
                // The current result is being focused on.
                isCurrentMatch = true;
            }
            else
            {
                // This search result is only a single occurrence that needs to
                // be highlighted- it's not the current result.
                isCurrentMatch = false;
            }

            // Highlight the row with the current match.
            if (isCurrentMatch)
            {
                int columnIndex = m_searcherResults.m_resultOccurrences[occurrenceIndex].m_rowDataIndex;
                SetCurrentSelection(pRootElement, columnIndex, false);
            }

            // Highlight the sub string in various columns.
            if ((*occurrenceIter).m_rowDataIndex == static_cast<int>(rgRowData::RowDataMemberName))
            {
                // Column zero is occupied by a QLabel.
                // Find and highlight the substring in a QLabel.
                pRootElement->HighlightLabelSubString((*occurrenceIter).m_characterIndex, m_searcherResults.m_searchString, isCurrentMatch);
            }
            else if ((*occurrenceIter).m_rowDataIndex == static_cast<int>(rgRowData::RowDataMemberValue))
            {
                // Check to see if this is a button or a numeric editor.
                rgEditorElementEnum* pElement = qobject_cast<rgEditorElementEnum*>(pRootElement);
                if (pElement != nullptr)
                {
                    pRootElement->HighlightButtonSubString((*occurrenceIter).m_characterIndex, m_searcherResults.m_searchString, isCurrentMatch);
                }
                else
                {
                    pRootElement->HighlightLineEditSubString((*occurrenceIter).m_characterIndex, m_searcherResults.m_searchString, isCurrentMatch);
                }
            }
        }

        prevDataIndex = occurrenceIter->m_rowDataIndex;
        occurrenceIter = std::find_if(occurrenceIter + 1, lastOccurrence, searcher);

        // Clear the previous search string data, process the next match.
        if (occurrenceIter != lastOccurrence)
        {
            if (prevDataIndex != occurrenceIter->m_rowDataIndex)
            {
                pRootElement->ClearSearchStringDataVector();
            }
        }
    }

    // If filtering is enabled, determine if the row should be visible or hidden.
    if (isFilteringEnabled)
    {
        if (!pRootElement->m_isFilteredOut)
        {
            // If the current row is included in the filtered search results,
            // expand and make visible all parent ancestors.
            pRootElement->SetExpansionState(rgRowExpansionState::Expanded);
            SetAncestorsVisible(pRootElement);
        }
        else
        {
            // Hide the row if it's not relevant in the filtered search results.
            pRootElement->hide();
        }
    }
    else
    {
        // If filtering is not enabled, all rows should be visible.
        pRootElement->show();
        pRootElement->SetExpansionState(rgRowExpansionState::Expanded);
    }

    // Traverse to each child element.
    int numChildren = pRootElement->ChildCount();
    for (int childIndex = 0; childIndex < numChildren; ++childIndex)
    {
        // Update each child element.
        rgEditorElement* pChild = pRootElement->GetChild(childIndex);
        UpdateRowsWithSearchResults(pChild);
    }
}

void rgPipelineStateTree::SetAncestorsVisible(rgEditorElement* pElement)
{
    if (pElement != nullptr)
    {
        rgEditorElement* pParent = pElement->GetParentItem();
        if (pParent != nullptr)
        {
            // Make the parent visible and expanded.
            pParent->show();
            pParent->SetExpansionState(rgRowExpansionState::Expanded);

            // Update parent visibility recursively up to the root element.
            SetAncestorsVisible(pParent);
        }
    }
}

bool rgPipelineStateTree::eventFilter(QObject* pObject, QEvent* pEvent)
{
    Q_UNUSED(pObject);

    bool isFiltered = false;

    if (pEvent != nullptr)
    {
        if (pEvent->type() == QEvent::Type::Wheel)
        {
            // If any of the drop downs are open, ignore the wheel event.
            if (m_isListWidgetOpen)
            {
                isFiltered = true;
            }
        }
    }

    return isFiltered;
}

void rgPipelineStateTree::SetEnumListWidgetStatus(bool isOpen)
{
    m_isListWidgetOpen = isOpen;

    // Enable/disable the scrollbars.
    if (isOpen)
    {
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
    else
    {
        setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }
}

void rgPipelineStateTree::SetCurrentSelection(rgEditorElement* pElement, int column, bool setFocus)
{
    // If there's already a current element, deselect it.
    if (m_currentSelection.m_pSelectedRow != nullptr)
    {
        m_currentSelection.m_pSelectedRow->RemoveStyleFlag(rgStyleFlags::CurrentRow);
    }

    // If the new element is valid, select it.
    if (pElement != nullptr)
    {
        pElement->AddStyleFlag(rgStyleFlags::CurrentRow);
    }

    m_currentSelection.m_pSelectedRow = pElement;
    m_currentSelection.m_focusedColumn = column;

    if (setFocus && pElement != nullptr)
    {
        if (column != -1)
        {
            // Focus on the given column.
            pElement->SetFocusedColumn(static_cast<rgRowData>(column));
        }
        else
        {
            // Focus on the overall row to allow arrow key navigation.
            pElement->setFocus();
        }
    }
}