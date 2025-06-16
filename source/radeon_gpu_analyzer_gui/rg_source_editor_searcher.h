//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for searching a source editor using an RgFindTextWidget.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_SOURCE_EDITOR_SEARCHER_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_SOURCE_EDITOR_SEARCHER_H_

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_find_text_widget.h"

// Forward declarations.
class RgSourceCodeEditor;

// An object responsible for searching a source editor using an RgFindTextWidget.
class RgSourceEditorSearcher : public ISearchable
{
public:
    RgSourceEditorSearcher();
    virtual ~RgSourceEditorSearcher() = default;

    // Get the supported search option flags.
    virtual uint32_t GetSupportedOptions() override;

    // Find the given string in the search target view. After finding the first result, the
    // direction is taken into account.
    virtual bool Find(const QString& search_string, SearchDirection direction) override;

    // Reset the current search.
    virtual void ResetSearch() override;

    // Select the current search result in the source editor.
    virtual void SelectResults() override {};

    // Set the search options.
    virtual void SetSearchOptions(const SearchOptions& options) override;

    // Set the target source editor being searched.
    void SetTargetEditor(RgSourceCodeEditor* target_editor);

private:
    // Find all instances of the search string.
    bool FindResults(const QString& search_string);

    // Set cursor for the current editor without changing kernel/correlation context.
    void SetCodeEditorCursor(const QTextCursor& cursor);

    // A vector containing the character indices of the search results.
    std::vector<size_t> result_indices_;

    // The last search string.
    std::string last_search_string_;

    // The location in the document of the last search result.
    int last_found_position_;

    // Search option flags used to alter search criteria.
    ISearchable::SearchOptions search_options_ = {};

    // The source editor instance whose text is being searched.
    RgSourceCodeEditor* target_editor_ = nullptr;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_RG_SOURCE_EDITOR_SEARCHER_H_
