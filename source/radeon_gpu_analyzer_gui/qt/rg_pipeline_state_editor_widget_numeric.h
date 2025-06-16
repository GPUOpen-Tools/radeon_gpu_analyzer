//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for Pso editor widget numeric.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_EDITOR_WIDGET_NUMERIC_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_EDITOR_WIDGET_NUMERIC_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_pipeline_state_editor_widget.h"
#include "ui_rg_pipeline_state_editor_widget_numeric.h"

// Forward declarations.
class QValidator;
namespace Ui {
class RgPipelineStateEditorWidgetNumeric;
}

class RgPipelineStateEditorWidgetNumeric : public RgPipelineStateEditorWidget
{
    Q_OBJECT

public:
    explicit RgPipelineStateEditorWidgetNumeric(QWidget* parent = nullptr);
    virtual ~RgPipelineStateEditorWidgetNumeric() = default;

    // Get the state of the editor checkbox.
    QVariant GetValue() const;

    // Set the type of numeric data being edited.
    void SetType(RgEditorDataType type);

    // Set the state of the editor checkbox.
    void SetValue(QVariant value);

    // Highlight the substring.
    void HighlightSubString(int start_location, const std::string& search_string);

    // Update the matched substrings.
    void UpdateStringMatchingLocation(int start_location, int length, const std::string& search_string);

protected:
    // The generated UI object.
    Ui::RgPipelineStateEditorWidgetNumeric ui_;

private:
    // Connect internal signals.
    void ConnectSignals();

    // A vector to store string highlight data.
    QVector<StringHighlightData> string_highlight_data_ = {};

    // Boolean to indicate substring highlight is requested.
    bool highlight_sub_string_;

    // The validator used to restrict editing to specific numeric types.
    QValidator* validator_ = nullptr;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_EDITOR_WIDGET_NUMERIC_H_
