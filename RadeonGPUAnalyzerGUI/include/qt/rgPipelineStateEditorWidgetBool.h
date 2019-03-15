#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateEditorWidget.h>
#include <ui_rgPipelineStateEditorWidgetBool.h>

namespace Ui {
class rgPipelineStateEditorWidgetBool;
}

class rgPipelineStateEditorWidgetBool : public rgPipelineStateEditorWidget
{
    Q_OBJECT

public:
    explicit rgPipelineStateEditorWidgetBool(QWidget* pParent = nullptr);
    virtual ~rgPipelineStateEditorWidgetBool() = default;

    // Get the state of the editor checkbox.
    bool GetValue() const;

    // Set the state of the editor checkbox.
    void SetValue(bool value);

private:
    // Connect internal signals.
    void ConnectSignals();

    // The generated UI object.
    Ui::rgPipelineStateEditorWidgetBool ui;
};