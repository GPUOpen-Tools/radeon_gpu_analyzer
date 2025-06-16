//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for Pso editor widget array elemt remove.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_EDITOR_WIDGET_ARRAY_ELEMENT_REMOVE_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_EDITOR_WIDGET_ARRAY_ELEMENT_REMOVE_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_pipeline_state_editor_widget.h"
#include "ui_rg_pipeline_state_editor_widget_array_element_remove.h"

// Forward declarations.
class QValidator;
namespace Ui {
class RgPipelineStateEditorWidgetArrayElementRemove;
}
class RgEditorElementArrayElementAdd;

class RgPipelineStateEditorWidgetArrayElementRemove : public RgPipelineStateEditorWidget
{
    Q_OBJECT

public:
    explicit RgPipelineStateEditorWidgetArrayElementRemove(QWidget* parent = nullptr);
    virtual ~RgPipelineStateEditorWidgetArrayElementRemove() = default;

    // Set the tooltip for the trash can icon (being built dynamically based on the element's index).
    void SetTrashCanIconTooltip(const std::string& tooltip_str);

signals:
    // A signal emitted when the delete button is clicked.
    void DeleteButtonClicked();

private:
    // Connect the widget's internal signals.
    void ConnectSignals();

    // The array element being resized.
    RgEditorElementArrayElementAdd* array_root_element_ = nullptr;

    // The generated UI object.
    Ui::RgPipelineStateEditorWidgetArrayElementRemove ui_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_EDITOR_WIDGET_ARRAY_ELEMENT_REMOVE_H_
