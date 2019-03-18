#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateEditorWidget.h>
#include <ui_rgPipelineStateEditorWidgetArrayElement.h>

// Forward declarations.
class QValidator;
namespace Ui {
class rgPipelineStateEditorWidgetArrayElement;
}
class rgEditorElementArray;

class rgPipelineStateEditorWidgetArrayElement : public rgPipelineStateEditorWidget
{
    Q_OBJECT

public:
    explicit rgPipelineStateEditorWidgetArrayElement(QWidget* pParent = nullptr);
    virtual ~rgPipelineStateEditorWidgetArrayElement() = default;

    // Set the tooltip for the trash can icon (being built dynamically based on the element's index).
    void SetTrashCanIconTooltip(const std::string& tooltipStr);

signals:
    // A signal emitted when the delete button is clicked.
    void DeleteButtonClicked();

private:
    // Connect the widget's internal signals.
    void ConnectSignals();

    // The array element being resized.
    rgEditorElementArray* m_pArrayRootElement = nullptr;

    // The generated UI object.
    Ui::rgPipelineStateEditorWidgetArrayElement ui;
};