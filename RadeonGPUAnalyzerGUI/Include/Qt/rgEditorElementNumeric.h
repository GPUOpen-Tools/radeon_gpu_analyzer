#pragma once

// C++
#include <cassert>

// Qt.
#include <QVariant>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElement.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateEditorWidgetNumeric.h>

// A type mapping struct.
template<typename T>
struct rgEditorTypeMap {};

// Specializations of type mapping struct.
// This mapping allows a templated type to map to a given enum type declaration,
// which can then be provided to the base class.
template<> struct rgEditorTypeMap<int8_t>   { static const rgEditorDataType dataType = rgEditorDataType::Int8;   };
template<> struct rgEditorTypeMap<int16_t>  { static const rgEditorDataType dataType = rgEditorDataType::Int16;  };
template<> struct rgEditorTypeMap<int32_t>  { static const rgEditorDataType dataType = rgEditorDataType::Int32;  };
template<> struct rgEditorTypeMap<uint8_t>  { static const rgEditorDataType dataType = rgEditorDataType::UInt8;  };
template<> struct rgEditorTypeMap<uint16_t> { static const rgEditorDataType dataType = rgEditorDataType::UInt16; };
template<> struct rgEditorTypeMap<uint32_t> { static const rgEditorDataType dataType = rgEditorDataType::UInt32; };
template<> struct rgEditorTypeMap<float>    { static const rgEditorDataType dataType = rgEditorDataType::Float;  };
template<> struct rgEditorTypeMap<double>   { static const rgEditorDataType dataType = rgEditorDataType::Double; };

// A templated element that can be used to edit multiple types of numeric data.
template <class T>
class rgEditorElementNumeric : public rgEditorElement
{
public:
    rgEditorElementNumeric(QWidget* pParent, const std::string& memberName, T* pDataPtr, std::function<void()> valueChangedCallback = nullptr)
        : rgEditorElement(pParent, memberName, rgEditorTypeMap<T>::dataType, valueChangedCallback), m_pValue(pDataPtr)
    {
        m_pEditorWidget = new rgPipelineStateEditorWidgetNumeric(this);
        assert(m_pEditorWidget != nullptr);
        if (m_pEditorWidget != nullptr)
        {
            // Set the type being edited by the widget.
            m_pEditorWidget->SetType(rgEditorTypeMap<T>::dataType);
            m_pEditorWidget->SetValue(GetValue());

            // Insert the editor widget into the row.
            ui.editorLayout->insertWidget(0, m_pEditorWidget);

            // Connect internal editor signals.
            bool isConnected = connect(m_pEditorWidget, &rgPipelineStateEditorWidgetNumeric::EditingFinished, this, &rgEditorElement::HandleValueChanged);
            assert(isConnected);

            // Connect the editor widget focus in signal.
            isConnected = connect(m_pEditorWidget, &rgPipelineStateEditorWidgetNumeric::FocusInSignal, this, &rgEditorElement::HandleEditorFocusIn);
            assert(isConnected);

            // Connect the editor widget focus out signal.
            isConnected = connect(m_pEditorWidget, &rgPipelineStateEditorWidgetNumeric::FocusOutSignal, this, &rgEditorElement::HandleEditorFocusOut);
            assert(isConnected);
        }
    }
    virtual ~rgEditorElementNumeric() = default;

    // Retrieve the editor widget used by the row.
    virtual rgPipelineStateEditorWidget* GetEditorWidget() override
    {
        return m_pEditorWidget;
    }

    // Get the data held within this item.
    virtual QVariant Data(int column) const override
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

    // Get the current value of the element.
    T GetValue() const
    {
        assert(m_pValue != nullptr);
        return *m_pValue;
    }

    // Set the current value of the element.
    void SetValue(T value)
    {
        assert(m_pValue != nullptr);
        if (m_pValue != nullptr)
        {
            // Update the value bound to this element.
            *m_pValue = value;

            if (m_valueChangedCallback != nullptr)
            {
                m_valueChangedCallback();
            }
        }
    }

protected:
    // Handle changes to the value.
    virtual void ValueChangedHandler()
    {
        assert(m_pEditorWidget != nullptr);
        if (m_pEditorWidget != nullptr)
        {
            QVariant editorValue = m_pEditorWidget->GetValue();
            bool conversionSuccessful = false;

            switch (m_type)
            {
                case rgEditorDataType::Int8:
                case rgEditorDataType::Int16:
                case rgEditorDataType::Int32:
                    {
                        int value = editorValue.toInt(&conversionSuccessful);
                        if (conversionSuccessful)
                        {
                            SetValue(value);
                        }
                    }
                    break;
                case rgEditorDataType::UInt8:
                case rgEditorDataType::UInt16:
                case rgEditorDataType::UInt32:
                    {
                        uint value = editorValue.toUInt(&conversionSuccessful);
                        if (conversionSuccessful)
                        {
                            SetValue(value);
                        }
                    }
                    break;
                case rgEditorDataType::Float:
                case rgEditorDataType::Double:
                    {
                        double value = editorValue.toFloat(&conversionSuccessful);
                        if (conversionSuccessful)
                        {
                            SetValue(value);
                        }
                    }
                    break;
                default:
                    // If we got here, the data type is not editable.
                    assert(false);
            }
        }
    }

private:
    // The widget used to edit the numeric value.
    rgPipelineStateEditorWidgetNumeric* m_pEditorWidget = nullptr;

    // The current value of the element.
    T* m_pValue = nullptr;
};

// A helper function used to create a numeric element without having to provide a template type argument.
template <typename J> static
rgEditorElement* MakeNumericElement(QWidget* pParent, const std::string& memberName, J* pDataPtr, std::function<void()> valueChangedCallback = nullptr)
{
    return new rgEditorElementNumeric<J>(pParent, memberName, pDataPtr, valueChangedCallback);
}