#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_PIPELINE_STATE_SEARCHER_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_PIPELINE_STATE_SEARCHER_H_

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_find_text_widget.h"

// Forward declarations.
class RgEditorElement;
class RgPipelineStateModel;
class RgPipelineStateTree;

// An object responsible for searching a source editor using an RgFindTextWidget.
class RgPipelineStateSearcher : public ISearchable
{
public:
    // A structure used to hold all search result occurrences.
    struct OccurrenceLocation
    {
        // The row where the search result was found.
        RgEditorElement* result_row;

        // The data index (the row's column) where the search result was found.
        int row_data_index;

        // The character index where the search result starts in the data string.
        int character_index;
    };

    // A structure used to track the state of the current search. Contains the original search
    // string, as well as all result occurrences, and the currently-selected result index.
    struct SearchResultData
    {
        // The original search string.
        std::string search_string;

        // The current search filtering options.
        SearchOptions search_options;

        // A vector of all search result locations.
        std::vector<OccurrenceLocation> result_occurrences;

        // The currently-selected search result index.
        int selected_index;
    };

public:
    RgPipelineStateSearcher();
    virtual ~RgPipelineStateSearcher() = default;

    // Get the supported search option flags.
    virtual uint32_t GetSupportedOptions() override;

    // Find the given string in the pipeline state tree.
    virtual bool Find(const QString& search_string, SearchDirection direction) override;

    // Reset the current search.
    virtual void ResetSearch() override;

    // Select the current search result in the source editor.
    virtual void SelectResults() override;

    // Set the search options.
    virtual void SetSearchOptions(const SearchOptions& options) override;

    // Set the target item model being searched.
    void SetTargetModel(RgPipelineStateModel* target_model);

    // Set the target view used to display search results.
    void SetTargetView(RgPipelineStateTree* view);

private:
    // Search the Pipeline State tree model.
    bool SearchPsoTree(const QString& search_string, SearchDirection direction);

    // The location in the document of the last search result.
    int last_found_position_;

    // The string to search for in the document.
    QString search_string_;

    // The flag used to determine whether or not matching case matters when searching.
    bool match_case_ = false;

    // The model instance being searched for the target text.
    RgPipelineStateModel* target_model_ = nullptr;

    // The TreeView instance used to display highlighted search results.
    RgPipelineStateTree* target_view_ = nullptr;

    // A list of search results.
    SearchResultData search_results_ = {};
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_PIPELINE_STATE_SEARCHER_H_
