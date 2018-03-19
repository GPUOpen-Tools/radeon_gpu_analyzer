// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgSourceCodeEditor.h>
#include <RadeonGPUAnalyzerGUI/include/rgSourceEditorSearcher.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>

// The value used to represent the state where a search doesn't return any results.
static const int gs_INVALID_SEARCH_LOCATION = -1;

rgSourceEditorSearcher::rgSourceEditorSearcher()
    : m_lastFoundPosition(gs_INVALID_SEARCH_LOCATION)
{
}

bool rgSourceEditorSearcher::FindNext(const QString& searchString)
{
    bool ret = false;

    m_searchString = searchString;

    // Search the target source editor for the search text.
    if (!m_searchString.isEmpty())
    {
        for (size_t i = 0; !ret && i < 2; i++)
        {
            // Two passes: first one for standard search, the second for wrap-around.
            bool isWrapAround = (i == 1);

            QString stringToSearch = m_pTargetEditor->toPlainText();
            QString currentSearchString = m_searchString;

            if (!m_matchCase)
            {
                currentSearchString = m_searchString.toLower();
                stringToSearch = stringToSearch.toLower();
            }

            // If the current cursor position differs from the last found search position, reset the search.
            // If we are in wrap-around mode, start from the beginning.
            int searchStartPosition = isWrapAround ? 0 : m_pTargetEditor->textCursor().position();
            if (searchStartPosition != m_lastFoundPosition)
            {
                ResetSearch();
            }

            // If in the middle of a search, compute the start position for the next search.
            int offsetFromPreviousFind = 0;
            if (m_lastFoundPosition != gs_INVALID_SEARCH_LOCATION)
            {
                // Advance 1 single character to find the *next* instance of text- not the one that's currently focused.
                offsetFromPreviousFind = 1;
                searchStartPosition = m_lastFoundPosition + offsetFromPreviousFind;
            }

            // Start the search for the target string in the source editor at the search start position.
            int instancePosition = rgUtils::FindIndexOf(stringToSearch, currentSearchString, searchStartPosition);
            if (instancePosition != gs_INVALID_SEARCH_LOCATION)
            {
                // Determine the number of characters to advance the cursor by. It's an offset from the current position.
                int moveDelta = (instancePosition - searchStartPosition) + offsetFromPreviousFind;

                // Set the last find position.
                m_lastFoundPosition = instancePosition;

                // Move the source editor's cursor to the position of the string that was just found.
                QTextCursor currentCursor = m_pTargetEditor->textCursor();
                currentCursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, moveDelta);
                SetCodeEditorCursor(currentCursor);
                ret = true;
            }
            else
            {
                // If the search didn't turn up any results, invalidate the final search.
                ResetSearch();
            }
        }
    }

    if (ret)
    {
        // Select search results with the cursor.
        SelectResults();
    }

    return ret;
}

void rgSourceEditorSearcher::ResetSearch()
{
    m_lastFoundPosition = gs_INVALID_SEARCH_LOCATION;

    // Clear the current selection, since the search is over.
    QTextCursor clearedSelection = m_pTargetEditor->textCursor();
    clearedSelection.clearSelection();
    SetCodeEditorCursor(clearedSelection);
}

void rgSourceEditorSearcher::SelectResults()
{
    // Create a text cursor with the proper start/end range.
    QTextCursor searchMatchSelectionCursor = m_pTargetEditor->textCursor();
    searchMatchSelectionCursor.setPosition(m_lastFoundPosition);
    searchMatchSelectionCursor.setPosition(m_lastFoundPosition + m_searchString.size(), QTextCursor::KeepAnchor);

    // Set the cursor in the target source editor.
    SetCodeEditorCursor(searchMatchSelectionCursor);
}

void rgSourceEditorSearcher::SetTargetEditor(rgSourceCodeEditor* pTargetEditor)
{
    // Set the target editor that will be searched.
    m_pTargetEditor = pTargetEditor;
}

void rgSourceEditorSearcher::SetCodeEditorCursor(const QTextCursor& cursor)
{
    // Disable signals emitted by the Code Editor to prevent switching kernel/correlation context.
    // The signal block will be released right after getting out of scope of this function.
    QSignalBlocker  blocker(m_pTargetEditor);
    // Set cursor for the current Code Editor.
    m_pTargetEditor->setTextCursor(cursor);
}
