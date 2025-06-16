//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for Pso editor widget array elemt remove.
//=============================================================================
// C++.
#include <cassert>
#include <sstream>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_editor_widget_array_element_remove.h"
#include "radeon_gpu_analyzer_gui/qt/rg_editor_element_array_element_add.h"

RgPipelineStateEditorWidgetArrayElementRemove::RgPipelineStateEditorWidgetArrayElementRemove(QWidget* parent)
    : RgPipelineStateEditorWidget(parent)
{
    ui_.setupUi(this);

    // Always show the delete element button.
    ui_.deleteElementButton->setVisible(true);

    // Update the cursor for the delete button.
    ui_.deleteElementButton->setCursor(Qt::PointingHandCursor);

    // Set the trashcan button as the focus proxy widget.
    setFocusProxy(ui_.deleteElementButton);

    // Connect the internal widget signals.
    ConnectSignals();
}

void RgPipelineStateEditorWidgetArrayElementRemove::SetTrashCanIconTooltip(const std::string& tooltip_str)
{
    ui_.deleteElementButton->setToolTip(tooltip_str.c_str());
}

void RgPipelineStateEditorWidgetArrayElementRemove::ConnectSignals()
{
    // Connect the add element button's signal to be forwarded to a local public signal.
    [[maybe_unused]] bool is_connected =
        connect(ui_.deleteElementButton, &QPushButton::clicked, this, &RgPipelineStateEditorWidgetArrayElementRemove::DeleteButtonClicked);
    assert(is_connected);
}
