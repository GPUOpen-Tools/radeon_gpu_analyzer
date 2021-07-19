#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_EDITOR_WIDGET_BOOL_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_EDITOR_WIDGET_BOOL_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_pipeline_state_editor_widget.h"
#include "ui_rg_pipeline_state_editor_widget_bool.h"

namespace Ui {
class RgPipelineStateEditorWidgetBool;
}

class RgPipelineStateEditorWidgetBool : public RgPipelineStateEditorWidget
{
    Q_OBJECT

public:
    explicit RgPipelineStateEditorWidgetBool(QWidget* parent = nullptr);
    virtual ~RgPipelineStateEditorWidgetBool() = default;

    // Get the state of the editor checkbox.
    bool GetValue() const;

    // Set the state of the editor checkbox.
    void SetValue(bool value);

private:
    // Connect internal signals.
    void ConnectSignals();

    // The generated UI object.
    Ui::RgPipelineStateEditorWidgetBool ui_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_EDITOR_WIDGET_BOOL_H_
