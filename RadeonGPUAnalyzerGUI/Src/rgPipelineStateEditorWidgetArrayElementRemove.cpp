// C++.
#include <cassert>
#include <sstream>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateEditorWidgetArrayElementRemove.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElementArrayElementAdd.h>

rgPipelineStateEditorWidgetArrayElementRemove::rgPipelineStateEditorWidgetArrayElementRemove(QWidget* pParent)
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

void rgPipelineStateEditorWidgetArrayElementRemove::SetTrashCanIconTooltip(const std::string& tooltipStr)
{
    ui.deleteElementButton->setToolTip(tooltipStr.c_str());
}

void rgPipelineStateEditorWidgetArrayElementRemove::ConnectSignals()
{
    // Connect the add element button's signal to be forwarded to a local public signal.
    bool isConnected = connect(ui.deleteElementButton, &QPushButton::clicked, this, &rgPipelineStateEditorWidgetArrayElementRemove::DeleteButtonClicked);
    assert(isConnected);
}