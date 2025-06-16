//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief implementation for Pso editor widget array element add.
//=============================================================================
// C++.
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_editor_element_array_element_add.h"
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_editor_widget_array_element_add.h"

RgPipelineStateEditorWidgetArrayElementAdd::RgPipelineStateEditorWidgetArrayElementAdd(RgEditorElementArrayElementAdd* parent)
    : RgPipelineStateEditorWidget(parent)
    , array_root_element_(parent)
{
    ui_.setupUi(this);

    // Update the cursor for the delete button.
    ui_.addElementButton->setCursor(Qt::PointingHandCursor);

    // Set the Plus button as the focus proxy.
    setFocusProxy(ui_.addElementButton);

    // Connect the internal widget signals.
    ConnectSignals();

    // If the array is resized, update the size label.
    array_root_element_->SetResizeCallback([=] { UpdateArraySizeLabel(); });
}

void RgPipelineStateEditorWidgetArrayElementAdd::HandleAddButtonClicked()
{
    assert(array_root_element_ != nullptr);
    if (array_root_element_ != nullptr)
    {
        // Add a new element to the array.
        array_root_element_->AddNewElement();

        // Update the array size label to match the new element count.
        UpdateArraySizeLabel();
    }
}

void RgPipelineStateEditorWidgetArrayElementAdd::ConnectSignals()
{
    // Connect the add element button's signal to be forwarded to a local public signal.
    [[maybe_unused]] bool is_connected =
        connect(ui_.addElementButton, &QPushButton::clicked, this, &RgPipelineStateEditorWidgetArrayElementAdd::HandleAddButtonClicked);
    assert(is_connected);
}

void RgPipelineStateEditorWidgetArrayElementAdd::UpdateArraySizeLabel()
{
    // Extract the array size element from the array root element.
    RgEditorElementNumeric<uint32_t>* size_element = array_root_element_->GetArraySizeElement();
    assert(size_element != nullptr);
    if (size_element != nullptr)
    {
        // Extract the array dimension from the size element, and set the count label.
        uint32_t current_size = size_element->GetValue();
        ui_.countLabel->setText(QString::number(current_size));

        // Hide/show the count label depending on the number of items.
        if (current_size > 0)
        {
            ui_.countLabel->show();
        }
        else
        {
            ui_.countLabel->hide();
        }
    }
}
