#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgFindTextWidget.h>

// Forward declarations.
class rgEditorElement;
class rgPipelineStateModel;
class rgPipelineStateTree;

// An object responsible for searching a source editor using an rgFindTextWidget.
class rgPipelineStateSearcher : public ISearchable
{
public:
    // A structure used to hold all search result occurrences.
    struct OccurrenceLocation
    {
        // The row where the search result was found.
        rgEditorElement* m_pResultRow;

        // The data index (the row's column) where the search result was found.
        int m_rowDataIndex;

        // The character index where the search result starts in the data string.
        int m_characterIndex;
    };

    // A structure used to track the state of the current search. Contains the original search
    // string, as well as all result occurrences, and the currently-selected result index.
    struct SearchResultData
    {
        // The original search string.
        std::string m_searchString;

        // The current search filtering options.
        SearchOptions m_searchOptions;

        // A vector of all search result locations.
        std::vector<OccurrenceLocation> m_resultOccurrences;

        // The currently-selected search result index.
        int m_selectedIndex;
    };

public:
    rgPipelineStateSearcher();
    virtual ~rgPipelineStateSearcher() = default;

    // Get the supported search option flags.
    virtual uint32_t GetSupportedOptions() override;

    // Find the given string in the pipeline state tree.
    virtual bool Find(const QString& searchString, SearchDirection direction) override;

    // Reset the current search.
    virtual void ResetSearch() override;

    // Select the current search result in the source editor.
    virtual void SelectResults() override;

    // Set the search options.
    virtual void SetSearchOptions(const SearchOptions& options) override;

    // Set the target item model being searched.
    void SetTargetModel(rgPipelineStateModel* pTargetModel);

    // Set the target view used to display search results.
    void SetTargetView(rgPipelineStateTree* pView);

private:
    // Search the Pipeline State tree model.
    bool SearchPsoTree(const QString& searchString, SearchDirection direction);

    // The location in the document of the last search result.
    int m_lastFoundPosition;

    // The string to search for in the document.
    QString m_searchString;

    // The flag used to determine whether or not matching case matters when searching.
    bool m_matchCase = false;

    // The model instance being searched for the target text.
    rgPipelineStateModel* m_pTargetModel = nullptr;

    // The TreeView instance used to display highlighted search results.
    rgPipelineStateTree* m_pTargetView = nullptr;

    // A list of search results.
    SearchResultData m_searchResults = {};
};