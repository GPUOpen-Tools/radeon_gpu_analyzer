//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for RgEditorElementArrayElementRemove class.
//=============================================================================

// C++.
#include <cassert>
#include <sstream>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_editor_element_array_element_add.h"
#include "radeon_gpu_analyzer_gui/qt/rg_editor_element_array_element_remove.h"
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_editor_widget_array_element_remove.h"
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_tree.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

// Delete confirmation dialog box message.
static const char* kConfirmationDialogMessageA = "The \"";
static const char* kConfirmationDialogMessageB = "\" item will be erased. Are you sure?";

// Dynamic tooltip strings (the tooltip for the trash can
// icon is being built dynamically based on the element's index).
static const char* kStrTrashIconRemove1 = "Remove ";
static const char* kStrTrashIconRemove2 = " item (index ";
static const char* kStrTrashIconRemove3 = ").";

RgEditorElementArrayElementRemove::RgEditorElementArrayElementRemove(QWidget* parent, const std::string& member_name)
    : RgEditorElement(parent, member_name, RgEditorDataType::kArrayElement)
{
    editor_widget_ = new RgPipelineStateEditorWidgetArrayElementRemove(this);
    assert(editor_widget_ != nullptr);
    if (editor_widget_ != nullptr)
    {
        ui_.editorLayout->insertWidget(0, editor_widget_);
    }

    // Connect internal signals to slots.
    ConnectSignals();
}

RgPipelineStateEditorWidget* RgEditorElementArrayElementRemove::GetEditorWidget()
{
    return editor_widget_;
}

void RgEditorElementArrayElementRemove::ConnectSignals()
{
    // Connect the delete button handler.
    bool is_connected = connect(editor_widget_, &RgPipelineStateEditorWidgetArrayElementRemove::DeleteButtonClicked,
        this, &RgEditorElementArrayElementRemove::HandleDeleteButtonClicked);
    assert(is_connected);

    // Connect the loss of focus handler.
    is_connected = connect(editor_widget_, &RgPipelineStateEditorWidgetArrayElementRemove::FocusOutSignal,
        this, &RgEditorElement::HandleEditorFocusOut);
    assert(is_connected);
}

void RgEditorElementArrayElementRemove::SetElementIndex(RgEditorElementArrayElementAdd* parent_array, int child_index)
{
    assert(parent_array != nullptr);
    if (parent_array != nullptr)
    {
        array_root_element_ = parent_array;
        child_index_ = child_index;

        // Update the tooltip.
        std::stringstream tooltip_txt;
        auto begin_pos = member_name_.find(" ");
        assert(begin_pos < member_name_.size()-2);
        if (begin_pos < member_name_.size() - 2)
        {
            // Build the tooltip string without the prefix.
            std::string member_name_no_prefix = member_name_.substr(begin_pos + 1);
            tooltip_txt << kStrTrashIconRemove1 << member_name_no_prefix << kStrTrashIconRemove2 << child_index_ << kStrTrashIconRemove3;
            editor_widget_->SetTrashCanIconTooltip(tooltip_txt.str().c_str());
        }
    }
}

void RgEditorElementArrayElementRemove::HandleDeleteButtonClicked()
{
    assert(array_root_element_ != nullptr);
    if (array_root_element_ != nullptr)
    {
        // Expand the row clicked on.
        ExpandTreeEntry();

        // Remove highlight from the previous row, and select the current row.
        RgPipelineStateTree* state_tree = GetParentStateTree();
        assert(state_tree != nullptr);
        if (state_tree != nullptr)
        {
            state_tree->SetCurrentSelection(nullptr);
        }

        // Show a confirmation dialog box.
        std::string message = kConfirmationDialogMessageA + member_name_ + kConfirmationDialogMessageB;
        bool result = RgUtils::ShowConfirmationMessageBox(kStrPipelineStateEditorDeleteElementConfirmationTitle, message.c_str(), this);

        if (result)
        {
            assert(state_tree != nullptr);
            if (state_tree != nullptr)
            {
                // Select the array root element before removing the child array element.
                state_tree->SetCurrentSelection(array_root_element_);
            }

            // Delete the given element from the array.
            array_root_element_->RemoveElement(child_index_);
        }
    }
}
