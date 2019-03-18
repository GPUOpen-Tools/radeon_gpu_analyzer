// C++.
#include <cassert>
#include <sstream>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElementEnum.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateEditorWidgetEnum.h>

rgEditorElementEnum::rgEditorElementEnum(QWidget* pParent, const std::string& memberName, const rgEnumValuesVector& enumerators, uint32_t* pValue, bool isBitFlags)
    : rgEditorElement(pParent, memberName, rgEditorDataType::Enum)
    , m_enumerators(enumerators)
    , m_pValue(pValue)
    , m_isBitFlagsEnum(isBitFlags)
    , m_pParent(pParent)
{
    // Ensure that the value pointer is valid.
    assert(m_pValue != nullptr);

    m_pEditorWidget = new rgPipelineStateEditorWidgetEnum(m_isBitFlagsEnum, m_pParent);
    m_pEditorWidget->setObjectName(QString::fromStdString(memberName));
    assert(m_pEditorWidget != nullptr);
    if (m_pEditorWidget != nullptr)
    {
        // Initialize the editor widget with the list of enumerators and initial value.
        m_pEditorWidget->SetEnumerators(enumerators);
        m_pEditorWidget->SetValue(GetValue());

        // Insert the editor widget into the new row.
        ui.editorLayout->insertWidget(0, m_pEditorWidget);

        // Connect the editor's value change handler.
        bool isConnected = connect(m_pEditorWidget, &rgPipelineStateEditorWidgetEnum::EditingFinished, this, &rgEditorElement::HandleValueChanged);
        assert(isConnected);

        // Connect the editor widget focus in signal.
        isConnected = connect(m_pEditorWidget, &rgPipelineStateEditorWidgetEnum::FocusInSignal, this, &rgEditorElement::HandleEditorFocusIn);
        assert(isConnected);

        // Connect the editor widget focus out signal.
        isConnected = connect(m_pEditorWidget, &rgPipelineStateEditorWidgetEnum::FocusOutSignal, this, &rgEditorElement::HandleEditorFocusOut);
        assert(isConnected);

        // Connect the list widget status signal.
        isConnected = connect(m_pEditorWidget, &rgPipelineStateEditorWidgetEnum::EnumListWidgetStatusSignal, this, &rgEditorElementEnum::EnumListWidgetStatusSignal);
        assert(isConnected);
    }
}

QVariant rgEditorElementEnum::Data(int column) const
{
    QVariant result = rgEditorElement::Data(column);

    if (column == static_cast<int>(rgRowData::RowDataMemberValue))
    {
        // Get the current enumeration value.
        uint32_t currentValue = GetValue();

        // Is this a normal enumeration or a set of bit flags?
        if (m_isBitFlagsEnum)
        {
            std::string flagBits;
            assert(m_pEditorWidget != nullptr);
            if (m_pEditorWidget != nullptr)
            {
                m_pEditorWidget->GetFlagBitsString(flagBits);
            }

            result = flagBits.c_str();
        }
        else
        {
            // Search for the name/value pair of the current enumerator.
            rgEnumeratorSearcher searcher(currentValue);
            auto enumeratorPairIter = std::find_if(m_enumerators.begin(), m_enumerators.end(), searcher);
            if (enumeratorPairIter != m_enumerators.end())
            {
                result = enumeratorPairIter->m_name.c_str();
            }
        }
    }

    return result;
}

rgPipelineStateEditorWidget* rgEditorElementEnum::GetEditorWidget()
{
    return m_pEditorWidget;
}

const rgEnumValuesVector& rgEditorElementEnum::GetEnumerators() const
{
    return m_enumerators;
}

uint32_t rgEditorElementEnum::GetValue() const
{
    return *m_pValue;
}

void rgEditorElementEnum::SetValue(uint32_t value)
{
    *m_pValue = value;

    if (m_valueChangedCallback != nullptr)
    {
        m_valueChangedCallback();
    }
}

void rgEditorElementEnum::ValueChangedHandler()
{
    assert(m_pEditorWidget != nullptr);
    if (m_pEditorWidget != nullptr)
    {
        // Update the row's value using the editor widget's current value.
        uint32_t enumValue = m_pEditorWidget->GetValue();
        SetValue(enumValue);
    }
}