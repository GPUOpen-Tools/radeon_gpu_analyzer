// C++.
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_check_box.h"
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_editor_widget_bool.h"

RgPipelineStateEditorWidgetBool::RgPipelineStateEditorWidgetBool(QWidget* parent)
    : RgPipelineStateEditorWidget(parent)
{
    ui_.setupUi(this);

    // Set the bool checkbox as the focus proxy widget.
    setFocusProxy(ui_.valueCheckbox);

    // Connect internal signals.
    ConnectSignals();
}

bool RgPipelineStateEditorWidgetBool::GetValue() const
{
    // Return the control's check state to determine the updated model value.
    return ui_.valueCheckbox->isChecked();
}

void RgPipelineStateEditorWidgetBool::SetValue(bool value)
{
    // Set the control's check state based on the incoming model value.
    Qt::CheckState check_state = value ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
    ui_.valueCheckbox->setCheckState(check_state);
}

void RgPipelineStateEditorWidgetBool::ConnectSignals()
{
    // Connect the checkbox check state changed handler.
    bool is_connected = connect(ui_.valueCheckbox, &QCheckBox::stateChanged, this, &RgPipelineStateEditorWidgetBool::EditingFinished);
    assert(is_connected);

    // Connect the checkbox focus in handler.
    is_connected = connect(ui_.valueCheckbox, &RgCheckBox::CheckBoxFocusInEvent, this, &RgPipelineStateEditorWidget::FocusInSignal);
    assert(is_connected);
}
