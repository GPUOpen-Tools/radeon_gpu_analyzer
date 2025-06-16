//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for RgEditorElementBool class.
//=============================================================================

// C++.
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_editor_element_bool.h"
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_editor_widget_bool.h"

RgEditorElementBool::RgEditorElementBool(QWidget* parent, const std::string& member_name, uint32_t* value)
    : RgEditorElement(parent, member_name, RgEditorDataType::kBool)
    , value_(value)
{
    editor_widget_ = new RgPipelineStateEditorWidgetBool(this);
    assert(editor_widget_ != nullptr);
    if (editor_widget_ != nullptr)
    {
        // Set the value for the widget.
        editor_widget_->SetValue(GetValue());

        // Insert the editor widget into the row.
        ui_.editorLayout->insertWidget(0, editor_widget_);

        // Connect internal editor signals.
        bool is_connected = connect(editor_widget_, &RgPipelineStateEditorWidgetBool::EditingFinished, this, &RgEditorElement::HandleValueChanged);
        assert(is_connected);

        // Connect the editor widget focus in signal.
        is_connected = connect(editor_widget_, &RgPipelineStateEditorWidgetBool::FocusInSignal, this, &RgEditorElement::HandleEditorFocusIn);
        assert(is_connected);

        // Connect the editor widget focus out signal.
        is_connected = connect(editor_widget_, &RgPipelineStateEditorWidgetBool::FocusOutSignal, this, &RgEditorElement::HandleEditorFocusOut);
        assert(is_connected);
    }
}

QVariant RgEditorElementBool::Data(int column) const
{
    if (column == static_cast<int>(RgRowData::kRowDataMemberValue))
    {
        return GetValue();
    }
    else
    {
        return RgEditorElement::Data(column);
    }
}

RgPipelineStateEditorWidget* RgEditorElementBool::GetEditorWidget()
{
    return editor_widget_;
}

bool RgEditorElementBool::GetValue() const
{
    assert(value_ != nullptr);
    return *value_ > 0 ? true : false;
}

void RgEditorElementBool::SetValue(bool value)
{
    assert(value_ != nullptr);
    if (value_ != nullptr)
    {
        *value_ = value;
    }
}

void RgEditorElementBool::ValueChangedHandler()
{
    assert(editor_widget_ != nullptr);
    if (editor_widget_ != nullptr)
    {
        // Update the row's check state based on the editor widget's check state.
        bool is_checked = editor_widget_->GetValue();
        SetValue(is_checked);
    }
}
