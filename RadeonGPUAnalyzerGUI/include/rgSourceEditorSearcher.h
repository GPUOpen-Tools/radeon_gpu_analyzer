#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgFindTextWidget.h>

// Forward declarations.
class rgSourceCodeEditor;

// An object responsible for searching a source editor using an rgFindTextWidget.
class rgSourceEditorSearcher : public ISearchable
{
public:
    rgSourceEditorSearcher();
    virtual ~rgSourceEditorSearcher() = default;

    // Get the supported search option flags.
    virtual uint32_t GetSupportedOptions() override;

    // Find the given string in the search target view. After finding the first result, the
    // direction is taken into account.
    virtual bool Find(const QString& searchString, SearchDirection direction) override;

    // Reset the current search.
    virtual void ResetSearch() override;

    // Select the current search result in the source editor.
    virtual void SelectResults() override {};

    // Set the search options.
    virtual void SetSearchOptions(const SearchOptions& options) override;

    // Set the target source editor being searched.
    void SetTargetEditor(rgSourceCodeEditor* pTargetEditor);

private:
    // Find all instances of the search string.
    bool FindResults(const QString& searchString);

    // Set cursor for the current editor without changing kernel/correlation context.
    void SetCodeEditorCursor(const QTextCursor& cursor);

    // A vector containing the character indices of the search results.
    std::vector<size_t> m_resultIndices;

    // The last search string.
    std::string m_lastSearchString;

    // The location in the document of the last search result.
    int m_lastFoundPosition;

    // Search option flags used to alter search criteria.
    ISearchable::SearchOptions m_searchOptions = {};

    // The source editor instance whose text is being searched.
    rgSourceCodeEditor* m_pTargetEditor = nullptr;
};