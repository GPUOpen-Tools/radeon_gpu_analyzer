#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElement.h>

// Forward declarations.
class rgPipelineStateEditorWidgetBool;

class rgEditorElementBool : public rgEditorElement
{
    Q_OBJECT

public:
    rgEditorElementBool(QWidget* pParent, const std::string& memberName, uint32_t* pValue);
    virtual ~rgEditorElementBool() = default;

    // Get the data held within this item.
    virtual QVariant Data(int column) const;

    // Retrieve the editor widget used by the row.
    virtual rgPipelineStateEditorWidget* GetEditorWidget() override;

    // Get the current value of the element.
    bool GetValue() const;

    // Set the current value of the element.
    void SetValue(bool value);

protected:
    // A handler invoked when the user has changed the editable value.
    virtual void ValueChangedHandler() override;

private:
    // The current value of the element.
    uint32_t* m_pValue = nullptr;

    // The widget used to edit the boolean value.
    rgPipelineStateEditorWidgetBool* m_pEditorWidget = nullptr;
};