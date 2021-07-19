// C++.
#include <cassert>

// Qt.
#include <QKeyEvent>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_editor_widget.h"

RgPipelineStateEditorWidget::RgPipelineStateEditorWidget(QWidget* parent)
    : QWidget(parent)
    , type_(RgEditorDataType::kVoid)
{
}

void RgPipelineStateEditorWidget::keyPressEvent(QKeyEvent* event)
{
    assert(event != nullptr);
    if (event != nullptr && event->key() == Qt::Key_Escape)
    {
        // If the user pressed Escape while the widget is focused, finish editing and lose focus.
        emit FocusOutSignal();
    }

    // Invoke the base implementation.
    QWidget::keyPressEvent(event);
}
