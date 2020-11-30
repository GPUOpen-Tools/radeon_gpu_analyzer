#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateEditorWidget.h>
#include <ui_rgPipelineStateEditorWidgetArrayElementRemove.h>

// Forward declarations.
class QValidator;
namespace Ui {
class rgPipelineStateEditorWidgetArrayElementRemove;
}
class rgEditorElementArrayElementAdd;

class rgPipelineStateEditorWidgetArrayElementRemove : public rgPipelineStateEditorWidget
{
    Q_OBJECT

public:
    explicit rgPipelineStateEditorWidgetArrayElementRemove(QWidget* pParent = nullptr);
    virtual ~rgPipelineStateEditorWidgetArrayElementRemove() = default;

    // Set the tooltip for the trash can icon (being built dynamically based on the element's index).
    void SetTrashCanIconTooltip(const std::string& tooltipStr);

signals:
    // A signal emitted when the delete button is clicked.
    void DeleteButtonClicked();

private:
    // Connect the widget's internal signals.
    void ConnectSignals();

    // The array element being resized.
    rgEditorElementArrayElementAdd* m_pArrayRootElement = nullptr;

    // The generated UI object.
    Ui::rgPipelineStateEditorWidgetArrayElementRemove ui;
};