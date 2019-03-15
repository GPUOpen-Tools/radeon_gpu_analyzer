#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateEditorWidget.h>
#include <ui_rgPipelineStateEditorWidgetNumeric.h>

// Forward declarations.
class QValidator;
namespace Ui {
class rgPipelineStateEditorWidgetNumeric;
}

class rgPipelineStateEditorWidgetNumeric : public rgPipelineStateEditorWidget
{
    Q_OBJECT

public:
    explicit rgPipelineStateEditorWidgetNumeric(QWidget* pParent = nullptr);
    virtual ~rgPipelineStateEditorWidgetNumeric() = default;

    // Get the state of the editor checkbox.
    QVariant GetValue() const;

    // Set the type of numeric data being edited.
    void SetType(rgEditorDataType type);

    // Set the state of the editor checkbox.
    void SetValue(QVariant value);

    // Highlight the substring.
    void HighlightSubString(int startLocation, const std::string& searchString);

    // Update the matched substrings.
    void UpdateStringMatchingLocation(int startLocation, int length, const std::string& searchString);

private:
    // Connect internal signals.
    void ConnectSignals();

    // A vector to store string highlight data.
    QVector<StringHighlightData> m_stringHighlightData = {};

    // Boolean to indicate substring highlight is requested.
    bool m_highlightSubString;

    // The validator used to restrict editing to specific numeric types.
    QValidator* m_pValidator = nullptr;

    // The generated UI object.
    Ui::rgPipelineStateEditorWidgetNumeric ui;
};