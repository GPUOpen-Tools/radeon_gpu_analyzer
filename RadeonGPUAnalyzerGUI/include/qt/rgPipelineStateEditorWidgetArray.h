#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateEditorWidget.h>
#include <ui_rgPipelineStateEditorWidgetArray.h>

// Forward declarations.
class QValidator;
namespace Ui {
class rgPipelineStateEditorWidgetArray;
}
class rgEditorElementArray;

class rgPipelineStateEditorWidgetArray : public rgPipelineStateEditorWidget
{
    Q_OBJECT

public:
    explicit rgPipelineStateEditorWidgetArray(rgEditorElementArray* pParent = nullptr);
    virtual ~rgPipelineStateEditorWidgetArray() = default;

private slots:
    // Handler invoked when the add button is clicked.
    void HandleAddButtonClicked();

private:
    // Connect the widget's internal signals.
    void ConnectSignals();

    // Update the array size label.
    void UpdateArraySizeLabel();

    // The array element being resized.
    rgEditorElementArray* m_pArrayRootElement = nullptr;

    // The generated UI object.
    Ui::rgPipelineStateEditorWidgetArray ui;
};