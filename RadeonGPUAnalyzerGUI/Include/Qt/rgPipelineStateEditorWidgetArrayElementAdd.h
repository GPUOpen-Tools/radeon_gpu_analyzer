#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateEditorWidget.h>
#include <ui_rgPipelineStateEditorWidgetArrayElementAdd.h>

// Forward declarations.
class QValidator;
namespace Ui {
class rgPipelineStateEditorWidgetArrayElementAdd;
}
class rgEditorElementArrayElementAdd;

class rgPipelineStateEditorWidgetArrayElementAdd : public rgPipelineStateEditorWidget
{
    Q_OBJECT

public:
    explicit rgPipelineStateEditorWidgetArrayElementAdd(rgEditorElementArrayElementAdd* pParent = nullptr);
    virtual ~rgPipelineStateEditorWidgetArrayElementAdd() = default;

private slots:
    // Handler invoked when the add button is clicked.
    void HandleAddButtonClicked();

private:
    // Connect the widget's internal signals.
    void ConnectSignals();

    // Update the array size label.
    void UpdateArraySizeLabel();

    // The array element being resized.
    rgEditorElementArrayElementAdd* m_pArrayRootElement = nullptr;

    // The generated UI object.
    Ui::rgPipelineStateEditorWidgetArrayElementAdd ui;
};