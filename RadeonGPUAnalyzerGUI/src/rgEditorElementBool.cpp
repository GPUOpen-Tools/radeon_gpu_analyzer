// C++.
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElementBool.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateEditorWidgetBool.h>

rgEditorElementBool::rgEditorElementBool(QWidget* pParent, const std::string& memberName, uint32_t* pValue)
    : rgEditorElement(pParent, memberName, rgEditorDataType::Bool)
    , m_pValue(pValue)
{
    m_pEditorWidget = new rgPipelineStateEditorWidgetBool(this);
    assert(m_pEditorWidget != nullptr);
    if (m_pEditorWidget != nullptr)
    {
        // Insert the editor widget into the row.
        ui.editorLayout->insertWidget(0, m_pEditorWidget);

        // Connect internal editor signals.
        bool isConnected = connect(m_pEditorWidget, &rgPipelineStateEditorWidgetBool::EditingFinished, this, &rgEditorElement::HandleValueChanged);
        assert(isConnected);

        // Connect the editor widget focus in signal.
        isConnected = connect(m_pEditorWidget, &rgPipelineStateEditorWidgetBool::FocusInSignal, this, &rgEditorElement::HandleEditorFocusIn);
        assert(isConnected);

        // Connect the editor widget focus out signal.
        isConnected = connect(m_pEditorWidget, &rgPipelineStateEditorWidgetBool::FocusOutSignal, this, &rgEditorElement::HandleEditorFocusOut);
        assert(isConnected);
    }
}

QVariant rgEditorElementBool::Data(int column) const
{
    if (column == static_cast<int>(rgRowData::RowDataMemberValue))
    {
        return GetValue();
    }
    else
    {
        return rgEditorElement::Data(column);
    }
}

rgPipelineStateEditorWidget* rgEditorElementBool::GetEditorWidget()
{
    return m_pEditorWidget;
}

bool rgEditorElementBool::GetValue() const
{
    assert(m_pValue != nullptr);
    return *m_pValue > 0 ? true : false;
}

void rgEditorElementBool::SetValue(bool value)
{
    assert(m_pValue != nullptr);
    if (m_pValue != nullptr)
    {
        *m_pValue = value;
    }
}

void rgEditorElementBool::ValueChangedHandler()
{
    assert(m_pEditorWidget != nullptr);
    if (m_pEditorWidget != nullptr)
    {
        // Update the row's check state based on the editor widget's check state.
        bool isChecked = m_pEditorWidget->GetValue();
        SetValue(isChecked);
    }
}