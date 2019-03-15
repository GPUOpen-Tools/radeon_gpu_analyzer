// C++.
#include <cassert>
#include <sstream>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateEditorWidgetArrayElement.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElementArray.h>

rgPipelineStateEditorWidgetArrayElement::rgPipelineStateEditorWidgetArrayElement(QWidget* pParent)
    : rgPipelineStateEditorWidget(pParent)
{
    ui.setupUi(this);

    // Always show the delete element button.
    ui.deleteElementButton->setVisible(true);

    // Update the cursor for the delete button.
    ui.deleteElementButton->setCursor(Qt::PointingHandCursor);

    // Set the trashcan button as the focus proxy widget.
    setFocusProxy(ui.deleteElementButton);

    // Connect the internal widget signals.
    ConnectSignals();
}

void rgPipelineStateEditorWidgetArrayElement::SetTrashCanIconTooltip(const std::string& tooltipStr)
{
    ui.deleteElementButton->setToolTip(tooltipStr.c_str());
}

void rgPipelineStateEditorWidgetArrayElement::ConnectSignals()
{
    // Connect the add element button's signal to be forwarded to a local public signal.
    bool isConnected = connect(ui.deleteElementButton, &QPushButton::clicked, this, &rgPipelineStateEditorWidgetArrayElement::DeleteButtonClicked);
    assert(isConnected);
}