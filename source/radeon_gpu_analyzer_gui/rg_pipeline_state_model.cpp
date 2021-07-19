// C++.
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_model.h"
#include "radeon_gpu_analyzer_gui/qt/rg_editor_element.h"
#include "radeon_gpu_analyzer_gui/rg_data_types.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"

RgPipelineStateModel::RgPipelineStateModel(QWidget* parent)
    : QObject(parent)
    , parent_(parent)
{
}

RgEditorElement* RgPipelineStateModel::GetRootElement() const
{
    return root_item_;
}

void RgPipelineStateModel::InitializeDefaultPipelineState(QWidget* parent, RgPipelineType pipeline_type)
{
    // Set the type of pipeline being edited with this model instance.
    pipeline_type_ = pipeline_type;

    // The CreateInfo root element in the tree.
    RgEditorElement* pipeline_create_info_root_item = nullptr;

    switch (pipeline_type)
    {
    case RgPipelineType::kGraphics:
    {
        // Initialize the default graphics pipeline state configuration.
        InitializeDefaultGraphicsPipeline();

        // Initialize the model root element.
        pipeline_create_info_root_item = InitializeGraphicsPipelineCreateInfo(parent);
    }
    break;
    case RgPipelineType::kCompute:
    {
        // Initialize the default compute pipeline state configuration.
        InitializeDefaultComputePipeline();

        // Initialize the model root element.
        pipeline_create_info_root_item = InitializeComputePipelineCreateInfo(parent);
    }
    break;
    default:
        // The pipeline type should not be anything other than Graphics or Compute.
        assert(false);
        break;
    }

    // Destroy the existing element tree from the root item.
    RG_SAFE_DELETE(root_item_);

    // Assign the root item in the model.
    root_item_ = pipeline_create_info_root_item;
}

void RgPipelineStateModel::Search(const QString& search_string, RgPipelineStateSearcher::SearchResultData& search_results, RgEditorElement* root_element)
{
    // If no root element is provided, start at the model root element.
    if (root_element == nullptr)
    {
        assert(root_item_ != nullptr);
        if (root_item_ != nullptr)
        {
            QString search_string_copy = search_string;

            // If we don't care about matching upper/lower case, convert everything to lower case.
            if (!search_results.search_options.match_case)
            {
                search_string_copy = search_string_copy.toLower();
            }

            search_results.search_string = search_string_copy.toStdString();

            // Start searching the tree at the root element.
            Search(search_string_copy, search_results, root_item_);
        }
    }
    else
    {
        // Search all child elements.
        const int child_count = root_element->ChildCount();
        for (int child_index = 0; child_index < child_count; ++child_index)
        {
            RgEditorElement* child_element = root_element->GetChild(child_index);
            if (child_element != nullptr)
            {
                // Search each column in the child for the search string.
                int column_index = static_cast<int>(RgRowData::kRowDataMemberName);
                const int last_column_index = static_cast<int>(RgRowData::kRowDataMemberValue);
                for (; column_index <= last_column_index; ++column_index)
                {
                    QString field_to_search = child_element->Data(column_index).toString();

                    // If casing doesn't matter, compare everything in lower case.
                    if (!search_results.search_options.match_case)
                    {
                        field_to_search = field_to_search.toLower();
                    }

                    int start_index = 0;
                    while ((start_index = field_to_search.indexOf(search_string, start_index)) != -1)
                    {
                        RgPipelineStateSearcher::OccurrenceLocation indices = { child_element, column_index, start_index };
                        search_results.result_occurrences.push_back(indices);
                        start_index += search_string.length();
                    }
                }

                // Recursively iterate to each child element to look for search results.
                if (child_element->ChildCount() > 0)
                {
                    // Search the child's children for the string.
                    Search(search_string, search_results, child_element);
                }
            }
        }
    }
}

RgPipelineType RgPipelineStateModel::GetPipelineType() const
{
    return pipeline_type_;
}
