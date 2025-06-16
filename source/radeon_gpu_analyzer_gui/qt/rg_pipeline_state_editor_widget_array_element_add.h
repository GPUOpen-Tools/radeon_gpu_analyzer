//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for Pso editor widget array element add.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_EDITOR_WIDGET_ARRAY_ELEMENT_ADD_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_EDITOR_WIDGET_ARRAY_ELEMENT_ADD_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_pipeline_state_editor_widget.h"
#include "ui_rg_pipeline_state_editor_widget_array_element_add.h"

// Forward declarations.
class QValidator;
namespace Ui {
class RgPipelineStateEditorWidgetArrayElementAdd;
}
class RgEditorElementArrayElementAdd;

class RgPipelineStateEditorWidgetArrayElementAdd : public RgPipelineStateEditorWidget
{
    Q_OBJECT

public:
    explicit RgPipelineStateEditorWidgetArrayElementAdd(RgEditorElementArrayElementAdd* parent = nullptr);
    virtual ~RgPipelineStateEditorWidgetArrayElementAdd() = default;

private slots:
    // Handler invoked when the add button is clicked.
    void HandleAddButtonClicked();

private:
    // Connect the widget's internal signals.
    void ConnectSignals();

    // Update the array size label.
    void UpdateArraySizeLabel();

    // The array element being resized.
    RgEditorElementArrayElementAdd* array_root_element_ = nullptr;

    // The generated UI object.
    Ui::RgPipelineStateEditorWidgetArrayElementAdd ui_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_EDITOR_WIDGET_ARRAY_ELEMENT_ADD_H_
