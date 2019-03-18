// C++.
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSourceCodeEditor.h>
#include <RadeonGPUAnalyzerGUI/Include/rgSourceEditorSearcher.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

// The value used to represent the state where a search doesn't return any results.
static const int s_INVALID_SEARCH_LOCATION = -1;

rgSourceEditorSearcher::rgSourceEditorSearcher()
    : m_lastFoundPosition(s_INVALID_SEARCH_LOCATION)
{
}

uint32_t rgSourceEditorSearcher::GetSupportedOptions()
{
    return SupportedOptions::FindNext |
           SupportedOptions::FindPrevious |
           SupportedOptions::MatchCase;
}

bool rgSourceEditorSearcher::Find(const QString& searchString, SearchDirection direction)
{
    bool ret = false;

    if (searchString.isEmpty())
    {
        ResetSearch();
    }
    else
    {
        // The search string has changed since the last search. Search with the new string.
        if (searchString.compare(m_lastSearchString.c_str()) != 0)
        {
            // Reset the search results.
            ResetSearch();

            // Attempt to find new results.
            ret = FindResults(searchString);
        }
        else
        {
            ret = !m_resultIndices.empty();
        }
    }

    // Are there results to step through?
    if (ret && !m_resultIndices.empty())
    {
        if (direction == ISearchable::SearchDirection::Next)
        {
            // Step to the next result. Loop back around to the first result if necessary.
            m_lastFoundPosition++;
            if (m_lastFoundPosition >= m_resultIndices.size())
            {
                m_lastFoundPosition = 0;
            }
        }
        else if (direction == ISearchable::SearchDirection::Previous)
        {
            // Step to the previous result. Loop back around to the last result if necessary.
            m_lastFoundPosition--;
            if (m_lastFoundPosition < 0)
            {
                m_lastFoundPosition = static_cast<int>(m_resultIndices.size()) - 1;
            }
        }
        else
        {
            assert(false);
        }

        // Verify that the result index is valid.
        bool isValidResultIndex = m_lastFoundPosition >= 0 && m_lastFoundPosition < m_resultIndices.size();
        assert(isValidResultIndex);
        if (isValidResultIndex)
        {
            // Find the location of the current result.
            size_t occurrencePosition = m_resultIndices[m_lastFoundPosition];

            // Select the result in the editor.
            QTextCursor currentCursor = m_pTargetEditor->textCursor();
            currentCursor.setPosition(static_cast<int>(occurrencePosition));
            currentCursor.setPosition(static_cast<int>(occurrencePosition) + searchString.size(), QTextCursor::KeepAnchor);
            SetCodeEditorCursor(currentCursor);
        }
    }

    return ret;
}

void rgSourceEditorSearcher::ResetSearch()
{
    // Reset the last found position.
    m_lastFoundPosition = s_INVALID_SEARCH_LOCATION;

    // Reset the last search string.
    m_lastSearchString.clear();

    // Clear the result indices.
    m_resultIndices.clear();

    // Clear the current selection, since the search is over.
    QTextCursor clearedSelection = m_pTargetEditor->textCursor();
    clearedSelection.clearSelection();
    SetCodeEditorCursor(clearedSelection);
}

void rgSourceEditorSearcher::SetSearchOptions(const SearchOptions& options)
{
    m_searchOptions = options;
}

void rgSourceEditorSearcher::SetTargetEditor(rgSourceCodeEditor* pTargetEditor)
{
    // Set the target editor that will be searched.
    m_pTargetEditor = pTargetEditor;
}

void FindSearchResultIndices(const QString& text, const QString& textToFind, std::vector<size_t>& searchResultIndices)
{
    // Step the cursor through the entire field of text to search.
    size_t cursorIndex = text.indexOf(textToFind, 0);

    // Step through the text to search until we hit the end.
    size_t searchStringLength = textToFind.size();
    while (cursorIndex != std::string::npos)
    {
        // Found an result occurrence. Push it into the results list.
        searchResultIndices.push_back(cursorIndex);

        // Search for the next result location.
        cursorIndex = text.indexOf(textToFind, static_cast<int>(cursorIndex + searchStringLength));
    }
}

bool rgSourceEditorSearcher::FindResults(const QString& searchString)
{
    bool ret = false;

    assert(m_pTargetEditor != nullptr);
    if (m_pTargetEditor != nullptr)
    {
        // Search the target source editor for the search text.
        if (!searchString.isEmpty())
        {
            const QString& text = m_pTargetEditor->toPlainText();

            // Create a search predicate that search using the case sensitivity option.
            if (!m_searchOptions.m_matchCase)
            {
                // When case insensitive, convert the search string and document text to lowercase.
                QString lowercaseText = text;
                lowercaseText = lowercaseText.toLower();

                QString lowercaseTextToFind = searchString;
                lowercaseTextToFind = lowercaseTextToFind.toLower();

                // Perform the search on the converted text.
                FindSearchResultIndices(lowercaseText, lowercaseTextToFind, m_resultIndices);
            }
            else
            {
                // Search the text for the given search string.
                FindSearchResultIndices(text, searchString, m_resultIndices);
            }

            // If any results were found, store the search string.
            if (!m_resultIndices.empty())
            {
                m_lastSearchString = searchString.toStdString();
                ret = true;
            }
        }
    }

    return ret;
}

void rgSourceEditorSearcher::SetCodeEditorCursor(const QTextCursor& cursor)
{
    assert(m_pTargetEditor != nullptr);
    if (m_pTargetEditor != nullptr)
    {
        // Disable signals emitted by the Code Editor to prevent switching kernel/correlation context.
        // The signal block will be released right after getting out of scope of this function.
        QSignalBlocker  blocker(m_pTargetEditor);

        // Set cursor for the current Code Editor.
        m_pTargetEditor->setTextCursor(cursor);
    }
}
