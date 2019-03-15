// C++.
#include <cassert>

// Qt.
#include <QApplication>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateModel.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateTree.h>
#include <RadeonGPUAnalyzerGUI/Include/rgPipelineStateSearcher.h>

// The value used to represent the state where a search doesn't return any results.
static const int s_INVALID_SEARCH_LOCATION = -1;

rgPipelineStateSearcher::rgPipelineStateSearcher()
    : m_lastFoundPosition(s_INVALID_SEARCH_LOCATION)
{
}

uint32_t rgPipelineStateSearcher::GetSupportedOptions()
{
    return
        SupportedOptions::FindPrevious |
        SupportedOptions::FindNext |
        SupportedOptions::FilterTree |
        SupportedOptions::MatchCase;
}

bool rgPipelineStateSearcher::Find(const QString& searchString, SearchDirection direction)
{
    bool ret = SearchPsoTree(searchString, direction);

    // Select the search results in the view.
    SelectResults();

    return ret;
}

void rgPipelineStateSearcher::ResetSearch()
{
    // Reset the currently-selected result index.
    m_lastFoundPosition = s_INVALID_SEARCH_LOCATION;

    // Clear the existing search string and results.
    m_searchString.clear();
    m_searchResults.m_resultOccurrences.clear();
    m_searchResults.m_selectedIndex = s_INVALID_SEARCH_LOCATION;

    // Select the search results in the view.
    SelectResults();
}

void rgPipelineStateSearcher::SelectResults()
{
    // The PSO tree can potentially be resized or filtered as part of displaying search results.
    // Processing all pending events will allow the tree to re-layout, and update the vertical
    // scrollbar's minimum and maximum values. This is necessary to scroll to the proper result rows.
    qApp->processEvents();

    assert(m_pTargetView != nullptr);
    if (m_pTargetView != nullptr)
    {
        // If the last search is invalid, remove the highlighted search row from the view.
        if (m_lastFoundPosition == s_INVALID_SEARCH_LOCATION)
        {
            // Reset the highlighted search row since there are no results.
            m_pTargetView->SetHighlightedSearchResults(m_searchResults);

            if (!m_searchResults.m_resultOccurrences.empty())
            {
                m_lastFoundPosition = 0;
            }
        }

        // If the search found valid results, highlight the result.
        if (m_lastFoundPosition != s_INVALID_SEARCH_LOCATION)
        {
            // Update the selected result index.
            m_searchResults.m_selectedIndex = m_lastFoundPosition;

            // Verify that the search result index is valid.
            bool isValidIndex = m_lastFoundPosition >= 0 && m_lastFoundPosition < m_searchResults.m_resultOccurrences.size();
            assert(isValidIndex);
            if (isValidIndex)
            {
                // Highlight the row containing the search result, and scroll to it if necessary.
                m_pTargetView->SetHighlightedSearchResults(m_searchResults);

                // Scroll to the current search result.
                const OccurrenceLocation& currentOccurrence = m_searchResults.m_resultOccurrences[m_lastFoundPosition];
                assert(currentOccurrence.m_pResultRow != nullptr);
                if (currentOccurrence.m_pResultRow != nullptr)
                {
                    m_pTargetView->ScrollToRow(currentOccurrence.m_pResultRow);
                }
            }
        }
    }
}

void rgPipelineStateSearcher::SetSearchOptions(const SearchOptions& options)
{
    m_searchResults.m_searchOptions = options;
}

void rgPipelineStateSearcher::SetTargetModel(rgPipelineStateModel* pTargetModel)
{
    // Set the target model that will be searched.
    m_pTargetModel = pTargetModel;
}

void rgPipelineStateSearcher::SetTargetView(rgPipelineStateTree* pView)
{
    m_pTargetView = pView;
}

bool rgPipelineStateSearcher::SearchPsoTree(const QString& searchString, SearchDirection direction)
{
    bool hasSearchResults = false;

    // If the search text changed, re-search.
    if (m_searchString.compare(searchString) != 0)
    {
        // Reset the search before starting the next one.
        ResetSearch();

        // Only search if the target string is valid.
        if (!searchString.isEmpty())
        {
            // Update the search string.
            m_searchString = searchString;

            // Search the target pipeline state model.
            assert(m_pTargetModel != nullptr);
            if (m_pTargetModel != nullptr)
            {
                m_pTargetModel->Search(m_searchString, m_searchResults);
            }
        }
    }

    hasSearchResults = !m_searchResults.m_resultOccurrences.empty();
    if (hasSearchResults)
    {
        if (direction == ISearchable::SearchDirection::Previous)
        {
            // Step to the next result, looping back to the start if necessary.
            m_lastFoundPosition--;

            if (m_lastFoundPosition < 0)
            {
                m_lastFoundPosition = static_cast<int>(m_searchResults.m_resultOccurrences.size()) - 1;
            }
        }
        else if (direction == ISearchable::SearchDirection::Next)
        {
            // Step to the next result, looping back to the start if necessary.
            m_lastFoundPosition++;

            if (m_lastFoundPosition == m_searchResults.m_resultOccurrences.size())
            {
                m_lastFoundPosition = 0;
            }
        }
    }

    return hasSearchResults;
}