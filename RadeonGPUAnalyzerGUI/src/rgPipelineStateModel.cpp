// C++.
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateModel.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElement.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>

rgPipelineStateModel::rgPipelineStateModel(QWidget* pParent)
    : QObject(pParent)
    , m_pParent(pParent)
{
}

rgEditorElement* rgPipelineStateModel::GetRootElement() const
{
    return m_pRootItem;
}

void rgPipelineStateModel::InitializeDefaultPipelineState(QWidget* pParent, rgPipelineType pipelineType)
{
    // Set the type of pipeline being edited with this model instance.
    m_pipelineType = pipelineType;

    // The CreateInfo root element in the tree.
    rgEditorElement* pPipelineCreateInfoRootItem = nullptr;

    switch (pipelineType)
    {
    case rgPipelineType::Graphics:
    {
        // Initialize the default graphics pipeline state configuration.
        InitializeDefaultGraphicsPipeline();

        // Initialize the model root element.
        pPipelineCreateInfoRootItem = InitializeGraphicsPipelineCreateInfo(pParent);
    }
    break;
    case rgPipelineType::Compute:
    {
        // Initialize the default compute pipeline state configuration.
        InitializeDefaultComputePipeline();

        // Initialize the model root element.
        pPipelineCreateInfoRootItem = InitializeComputePipelineCreateInfo(pParent);
    }
    break;
    default:
        // The pipeline type should not be anything other than Graphics or Compute.
        assert(false);
        break;
    }

    // Destroy the existing element tree from the root item.
    RG_SAFE_DELETE(m_pRootItem);

    // Assign the root item in the model.
    m_pRootItem = pPipelineCreateInfoRootItem;
}

void rgPipelineStateModel::Search(const QString& searchString, rgPipelineStateSearcher::SearchResultData& searchResults, rgEditorElement* pRootElement)
{
    // If no root element is provided, start at the model root element.
    if (pRootElement == nullptr)
    {
        assert(m_pRootItem != nullptr);
        if (m_pRootItem != nullptr)
        {
            QString searchStringCopy = searchString;

            // If we don't care about matching upper/lower case, convert everything to lower case.
            if (!searchResults.m_searchOptions.m_matchCase)
            {
                searchStringCopy = searchStringCopy.toLower();
            }

            searchResults.m_searchString = searchStringCopy.toStdString();

            // Start searching the tree at the root element.
            Search(searchStringCopy, searchResults, m_pRootItem);
        }
    }
    else
    {
        // Search all child elements.
        const int childCount = pRootElement->ChildCount();
        for (int childIndex = 0; childIndex < childCount; ++childIndex)
        {
            rgEditorElement* pChildElement = pRootElement->GetChild(childIndex);
            if (pChildElement != nullptr)
            {
                // Search each column in the child for the search string.
                int columnIndex = static_cast<int>(rgRowData::RowDataMemberName);
                const int lastColumnIndex = static_cast<int>(rgRowData::RowDataMemberValue);
                for (; columnIndex <= lastColumnIndex; ++columnIndex)
                {
                    QString fieldToSearch = pChildElement->Data(columnIndex).toString();

                    // If casing doesn't matter, compare everything in lower case.
                    if (!searchResults.m_searchOptions.m_matchCase)
                    {
                        fieldToSearch = fieldToSearch.toLower();
                    }

                    int startIndex = 0;
                    while ((startIndex = fieldToSearch.indexOf(searchString, startIndex)) != -1)
                    {
                        rgPipelineStateSearcher::OccurrenceLocation indices = { pChildElement, columnIndex, startIndex };
                        searchResults.m_resultOccurrences.push_back(indices);
                        startIndex += searchString.length();
                    }
                }

                // Recursively iterate to each child element to look for search results.
                if (pChildElement->ChildCount() > 0)
                {
                    // Search the child's children for the string.
                    Search(searchString, searchResults, pChildElement);
                }
            }
        }
    }
}

rgPipelineType rgPipelineStateModel::GetPipelineType() const
{
    return m_pipelineType;
}