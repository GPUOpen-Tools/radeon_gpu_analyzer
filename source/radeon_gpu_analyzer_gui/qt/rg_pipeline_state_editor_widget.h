//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for Pso editor widget base element.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_EDITOR_WIDGET_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_EDITOR_WIDGET_H_

// Qt.
#include <QWidget>

 // Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_editor_element.h"

// The base class for all state item editor widgets.
class RgPipelineStateEditorWidget : public QWidget
{
    Q_OBJECT

public:
    RgPipelineStateEditorWidget(QWidget* parent = nullptr);
    virtual ~RgPipelineStateEditorWidget() = default;

    // An overridden keyPressEvent handler.
    virtual void keyPressEvent(QKeyEvent* event) override;

signals:
    // A signal emitted when the user has changed the editable value.
    void EditingFinished();

    // A signal emitted when the editor widget gets focus.
    void FocusInSignal();

    // A signal emitted when the editor widget loses focus.
    void FocusOutSignal();

protected:
    // The type of value being edited.
    RgEditorDataType type_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_EDITOR_WIDGET_H_
