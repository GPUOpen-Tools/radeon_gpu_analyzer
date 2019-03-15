// C++.
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgCheckBox.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateEditorWidgetBool.h>

rgPipelineStateEditorWidgetBool::rgPipelineStateEditorWidgetBool(QWidget* pParent)
    : rgPipelineStateEditorWidget(pParent)
{
    ui.setupUi(this);

    // Set the bool checkbox as the focus proxy widget.
    setFocusProxy(ui.valueCheckbox);

    // Connect internal signals.
    ConnectSignals();
}

bool rgPipelineStateEditorWidgetBool::GetValue() const
{
    // Return the control's check state to determine the updated model value.
    return ui.valueCheckbox->isChecked();
}

void rgPipelineStateEditorWidgetBool::SetValue(bool value)
{
    // Set the control's check state based on the incoming model value.
    Qt::CheckState checkState = value ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
    ui.valueCheckbox->setCheckState(checkState);
}

void rgPipelineStateEditorWidgetBool::ConnectSignals()
{
    // Connect the checkbox check state changed handler.
    bool isConnected = connect(ui.valueCheckbox, &QCheckBox::stateChanged, this, &rgPipelineStateEditorWidgetBool::EditingFinished);
    assert(isConnected);

    // Connect the checkbox focus in handler.
    isConnected = connect(ui.valueCheckbox, &rgCheckBox::CheckBoxFocusInEvent, this, &rgPipelineStateEditorWidget::FocusInSignal);
    assert(isConnected);
}