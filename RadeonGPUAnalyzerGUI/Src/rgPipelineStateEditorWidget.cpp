// C++.
#include <cassert>

// Qt.
#include <QKeyEvent>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateEditorWidget.h>

rgPipelineStateEditorWidget::rgPipelineStateEditorWidget(QWidget* pParent)
    : QWidget(pParent)
    , m_type(rgEditorDataType::Void)
{
}

void rgPipelineStateEditorWidget::keyPressEvent(QKeyEvent* pEvent)
{
    assert(pEvent != nullptr);
    if (pEvent != nullptr && pEvent->key() == Qt::Key_Escape)
    {
        // If the user pressed Escape while the widget is focused, finish editing and lose focus.
        emit FocusOutSignal();
    }

    // Invoke the base implementation.
    QWidget::keyPressEvent(pEvent);
}