#pragma once

// Qt.
#include <QWidget>

 // Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElement.h>

// The base class for all state item editor widgets.
class rgPipelineStateEditorWidget : public QWidget
{
    Q_OBJECT

public:
    rgPipelineStateEditorWidget(QWidget* pParent = nullptr);
    virtual ~rgPipelineStateEditorWidget() = default;

    // An overridden keyPressEvent handler.
    virtual void keyPressEvent(QKeyEvent* pEvent) override;

signals:
    // A signal emitted when the user has changed the editable value.
    void EditingFinished();

    // A signal emitted when the editor widget gets focus.
    void FocusInSignal();

    // A signal emitted when the editor widget loses focus.
    void FocusOutSignal();

protected:
    // The type of value being edited.
    rgEditorDataType m_type;
};