// C++.
#include <cassert>

// Qt.
#include <QApplication>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_model.h"
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_tree.h"
#include "radeon_gpu_analyzer_gui/rg_pipeline_state_searcher.h"

// The value used to represent the state where a search doesn't return any results.
static const int kInvalidSearchLocation = -1;

RgPipelineStateSearcher::RgPipelineStateSearcher()
    : last_found_position_(kInvalidSearchLocation)
{
}

uint32_t RgPipelineStateSearcher::GetSupportedOptions()
{
    return
        SupportedOptions::kFindPrevious |
        SupportedOptions::kFindNext |
        SupportedOptions::kFilterTree |
        SupportedOptions::kMatchCase;
}

bool RgPipelineStateSearcher::Find(const QString& search_string, SearchDirection direction)
{
    bool ret = SearchPsoTree(search_string, direction);

    // Select the search results in the view.
    SelectResults();

    return ret;
}

void RgPipelineStateSearcher::ResetSearch()
{
    // Reset the currently-selected result index.
    last_found_position_ = kInvalidSearchLocation;

    // Clear the existing search string and results.
    search_string_.clear();
    search_results_.result_occurrences.clear();
    search_results_.selected_index = kInvalidSearchLocation;

    // Select the search results in the view.
    SelectResults();
}

void RgPipelineStateSearcher::SelectResults()
{
    // The PSO tree can potentially be resized or filtered as part of displaying search results.
    // Processing all pending events will allow the tree to re-layout, and update the vertical
    // scrollbar's minimum and maximum values. This is necessary to scroll to the proper result rows.
    qApp->processEvents();

    assert(target_view_ != nullptr);
    if (target_view_ != nullptr)
    {
        // If the last search is invalid, remove the highlighted search row from the view.
        if (last_found_position_ == kInvalidSearchLocation)
        {
            // Reset the highlighted search row since there are no results.
            target_view_->SetHighlightedSearchResults(search_results_);

            if (!search_results_.result_occurrences.empty())
            {
                last_found_position_ = 0;
            }
        }

        // If the search found valid results, highlight the result.
        if (last_found_position_ != kInvalidSearchLocation)
        {
            // Update the selected result index.
            search_results_.selected_index = last_found_position_;

            // Verify that the search result index is valid.
            bool is_valid_index = last_found_position_ >= 0 && last_found_position_ < search_results_.result_occurrences.size();
            assert(is_valid_index);
            if (is_valid_index)
            {
                // Highlight the row containing the search result, and scroll to it if necessary.
                target_view_->SetHighlightedSearchResults(search_results_);

                // Scroll to the current search result.
                const OccurrenceLocation& current_occurrence = search_results_.result_occurrences[last_found_position_];
                assert(current_occurrence.result_row != nullptr);
                if (current_occurrence.result_row != nullptr)
                {
                    target_view_->ScrollToRow(current_occurrence.result_row);
                }
            }
        }
    }
}

void RgPipelineStateSearcher::SetSearchOptions(const SearchOptions& options)
{
    search_results_.search_options = options;
}

void RgPipelineStateSearcher::SetTargetModel(RgPipelineStateModel* target_model)
{
    // Set the target model that will be searched.
    target_model_ = target_model;
}

void RgPipelineStateSearcher::SetTargetView(RgPipelineStateTree* view)
{
    target_view_ = view;
}

bool RgPipelineStateSearcher::SearchPsoTree(const QString& search_string, SearchDirection direction)
{
    bool has_search_results = false;

    // If the search text changed, re-search.
    if (search_string_.compare(search_string) != 0)
    {
        // Reset the search before starting the next one.
        ResetSearch();

        // Only search if the target string is valid.
        if (!search_string.isEmpty())
        {
            // Update the search string.
            search_string_ = search_string;

            // Search the target pipeline state model.
            assert(target_model_ != nullptr);
            if (target_model_ != nullptr)
            {
                target_model_->Search(search_string_, search_results_);
            }
        }
    }

    has_search_results = !search_results_.result_occurrences.empty();
    if (has_search_results)
    {
        if (direction == ISearchable::SearchDirection::kPrevious)
        {
            // Step to the next result, looping back to the start if necessary.
            last_found_position_--;

            if (last_found_position_ < 0)
            {
                last_found_position_ = static_cast<int>(search_results_.result_occurrences.size()) - 1;
            }
        }
        else if (direction == ISearchable::SearchDirection::kNext)
        {
            // Step to the next result, looping back to the start if necessary.
            last_found_position_++;

            if (last_found_position_ == search_results_.result_occurrences.size())
            {
                last_found_position_ = 0;
            }
        }
    }

    return has_search_results;
}
