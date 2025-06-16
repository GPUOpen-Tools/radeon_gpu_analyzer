//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for RgEditorElementNumeric class.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_EDITOR_ELEMENT_NUMERIC_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_EDITOR_ELEMENT_NUMERIC_H_

// C++
#include <cassert>

// Qt.
#include <QVariant>

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_editor_element.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_pipeline_state_editor_widget_numeric.h"

// A type mapping struct.
template<typename T>
struct RgEditorTypeMap {};

// Specializations of type mapping struct.
// This mapping allows a templated type to map to a given enum type declaration,
// which can then be provided to the base class.
template<> struct RgEditorTypeMap<int8_t>   { static const RgEditorDataType data_type = RgEditorDataType::kInt8;   };
template<> struct RgEditorTypeMap<int16_t>  { static const RgEditorDataType data_type = RgEditorDataType::kInt16;  };
template<> struct RgEditorTypeMap<int32_t>  { static const RgEditorDataType data_type = RgEditorDataType::kInt32;  };
template<> struct RgEditorTypeMap<uint8_t>  { static const RgEditorDataType data_type = RgEditorDataType::kUInt8;  };
template<> struct RgEditorTypeMap<uint16_t> { static const RgEditorDataType data_type = RgEditorDataType::kUInt16; };
template<> struct RgEditorTypeMap<uint32_t> { static const RgEditorDataType data_type = RgEditorDataType::kUInt32; };
template<> struct RgEditorTypeMap<float>    { static const RgEditorDataType data_type = RgEditorDataType::kFloat;  };
template<> struct RgEditorTypeMap<double>   { static const RgEditorDataType data_type = RgEditorDataType::kDouble; };

// A templated element that can be used to edit multiple types of numeric data.
template <class T>
class RgEditorElementNumeric : public RgEditorElement
{
public:
    RgEditorElementNumeric(QWidget* parent, const std::string& member_name, T* pDataPtr, std::function<void()> value_changed_callback = nullptr)
        : RgEditorElement(parent, member_name, RgEditorTypeMap<T>::data_type, value_changed_callback), value_(pDataPtr)
    {
        editor_widget_ = new RgPipelineStateEditorWidgetNumeric(this);
        assert(editor_widget_ != nullptr);
        if (editor_widget_ != nullptr)
        {
            // Set the type being edited by the widget.
            editor_widget_->SetType(RgEditorTypeMap<T>::data_type);
            editor_widget_->SetValue(GetValue());

            // Insert the editor widget into the row.
            ui_.editorLayout->insertWidget(0, editor_widget_);

            // Connect internal editor signals.
            bool is_connected = connect(editor_widget_, &RgPipelineStateEditorWidgetNumeric::EditingFinished, this, &RgEditorElement::HandleValueChanged);
            assert(is_connected);

            // Connect the editor widget focus in signal.
            is_connected = connect(editor_widget_, &RgPipelineStateEditorWidgetNumeric::FocusInSignal, this, &RgEditorElement::HandleEditorFocusIn);
            assert(is_connected);

            // Connect the editor widget focus out signal.
            is_connected = connect(editor_widget_, &RgPipelineStateEditorWidgetNumeric::FocusOutSignal, this, &RgEditorElement::HandleEditorFocusOut);
            assert(is_connected);
        }
    }
    virtual ~RgEditorElementNumeric() = default;

    // Retrieve the editor widget used by the row.
    virtual RgPipelineStateEditorWidget* GetEditorWidget() override
    {
        return editor_widget_;
    }

    // Get the data held within this item.
    virtual QVariant Data(int column) const override
    {
        if (column == static_cast<int>(RgRowData::kRowDataMemberValue))
        {
            return GetValue();
        }
        else
        {
            return RgEditorElement::Data(column);
        }
    }

    // Get the current value of the element.
    T GetValue() const
    {
        assert(value_ != nullptr);
        return *value_;
    }

    // Set the current value of the element.
    void SetValue(T value)
    {
        assert(value_ != nullptr);
        if (value_ != nullptr)
        {
            // Update the value bound to this element.
            *value_ = value;

            if (value_changed_callback_ != nullptr)
            {
                value_changed_callback_();
            }
        }
    }

protected:
    // Handle changes to the value.
    virtual void ValueChangedHandler()
    {
        assert(editor_widget_ != nullptr);
        if (editor_widget_ != nullptr)
        {
            QVariant editor_value = editor_widget_->GetValue();
            bool is_conversion_successful = false;

            switch (type_)
            {
                case RgEditorDataType::kInt8:
                case RgEditorDataType::kInt16:
                case RgEditorDataType::kInt32:
                    {
                        int value = editor_value.toInt(&is_conversion_successful);
                        if (is_conversion_successful)
                        {
                            SetValue(value);
                        }
                    }
                    break;
                case RgEditorDataType::kUInt8:
                case RgEditorDataType::kUInt16:
                case RgEditorDataType::kUInt32:
                    {
                        uint value = editor_value.toUInt(&is_conversion_successful);
                        if (is_conversion_successful)
                        {
                            SetValue(value);
                        }
                    }
                    break;
                case RgEditorDataType::kFloat:
                case RgEditorDataType::kDouble:
                    {
                        double value = editor_value.toFloat(&is_conversion_successful);
                        if (is_conversion_successful)
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
    RgPipelineStateEditorWidgetNumeric* editor_widget_ = nullptr;

    // The current value of the element.
    T* value_ = nullptr;
};

// A helper function used to create a numeric element without having to provide a template type argument.
template <typename J> static
RgEditorElement* MakeNumericElement(QWidget* parent, const std::string& member_name, J* data_ptr, std::function<void()> value_changed_callback = nullptr)
{
    return new RgEditorElementNumeric<J>(parent, member_name, data_ptr, value_changed_callback);
}
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_EDITOR_ELEMENT_NUMERIC_H_
