// C++.
#include <cassert>
#include <sstream>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElementArrayElementAdd.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElementArrayElementRemove.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateEditorWidgetArrayElementRemove.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateTree.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

// Delete confirmation dialog box message.
static const char* s_CONFIRMATION_DIALOG_MESSAGE_A = "The \"";
static const char* s_CONFIRMATION_DIALOG_MESSAGE_B = "\" item will be erased. Are you sure?";

// Dynamic tooltip strings (the tooltip for the trash can
// icon is being built dynamically based on the element's index).
static const char* STR_TRASH_ICON_REMOVE_1 = "Remove ";
static const char* STR_TRASH_ICON_REMOVE_2 = " item (index ";
static const char* STR_TRASH_ICON_REMOVE_3 = ").";

rgEditorElementArrayElementRemove::rgEditorElementArrayElementRemove(QWidget* pParent, const std::string& memberName)
    : rgEditorElement(pParent, memberName, rgEditorDataType::ArrayElement)
{
    m_pEditorWidget = new rgPipelineStateEditorWidgetArrayElementRemove(this);
    assert(m_pEditorWidget != nullptr);
    if (m_pEditorWidget != nullptr)
    {
        ui.editorLayout->insertWidget(0, m_pEditorWidget);
    }

    // Connect internal signals to slots.
    ConnectSignals();
}

rgPipelineStateEditorWidget* rgEditorElementArrayElementRemove::GetEditorWidget()
{
    return m_pEditorWidget;
}

void rgEditorElementArrayElementRemove::ConnectSignals()
{
    // Connect the delete button handler.
    bool isConnected = connect(m_pEditorWidget, &rgPipelineStateEditorWidgetArrayElementRemove::DeleteButtonClicked,
        this, &rgEditorElementArrayElementRemove::HandleDeleteButtonClicked);
    assert(isConnected);

    // Connect the loss of focus handler.
    isConnected = connect(m_pEditorWidget, &rgPipelineStateEditorWidgetArrayElementRemove::FocusOutSignal,
        this, &rgEditorElement::HandleEditorFocusOut);
    assert(isConnected);
}

void rgEditorElementArrayElementRemove::SetElementIndex(rgEditorElementArrayElementAdd* pParentArray, int childIndex)
{
    assert(pParentArray != nullptr);
    if (pParentArray != nullptr)
    {
        m_pArrayRootElement = pParentArray;
        m_childIndex = childIndex;

        // Update the tooltip.
        std::stringstream tooltipTxt;
        auto beginPos = m_memberName.find(" ");
        assert(beginPos < m_memberName.size()-2);
        if (beginPos < m_memberName.size() - 2)
        {
            // Build the tooltip string without the prefix.
            std::string memebrNameNoPrefix = m_memberName.substr(beginPos + 1);
            tooltipTxt << STR_TRASH_ICON_REMOVE_1 << memebrNameNoPrefix << STR_TRASH_ICON_REMOVE_2 << m_childIndex << STR_TRASH_ICON_REMOVE_3;
            m_pEditorWidget->SetTrashCanIconTooltip(tooltipTxt.str().c_str());
        }
    }
}

void rgEditorElementArrayElementRemove::HandleDeleteButtonClicked()
{
    assert(m_pArrayRootElement != nullptr);
    if (m_pArrayRootElement != nullptr)
    {
        // Expand the row clicked on.
        ExpandTreeEntry();

        // Remove highlight from the previous row, and select the current row.
        rgPipelineStateTree* pStateTree = GetParentStateTree();
        assert(pStateTree != nullptr);
        if (pStateTree != nullptr)
        {
            pStateTree->SetCurrentSelection(nullptr);
        }

        // Show a confirmation dialog box.
        std::string message = s_CONFIRMATION_DIALOG_MESSAGE_A + m_memberName + s_CONFIRMATION_DIALOG_MESSAGE_B;
        bool result = rgUtils::ShowConfirmationMessageBox(STR_PIPELINE_STATE_EDITOR_DELETE_ELEMENT_CONFIRMATION_TITLE, message.c_str(), this);

        if (result)
        {
            assert(pStateTree != nullptr);
            if (pStateTree != nullptr)
            {
                // Select the array root element before removing the child array element.
                pStateTree->SetCurrentSelection(m_pArrayRootElement);
            }

            // Delete the given element from the array.
            m_pArrayRootElement->RemoveElement(m_childIndex);
        }
    }
}