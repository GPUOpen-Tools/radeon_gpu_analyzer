// C++.
#include <cassert>
#include <sstream>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_editor_element_enum.h"
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_editor_widget_enum.h"

RgEditorElementEnum::RgEditorElementEnum(QWidget* parent, const std::string& member_name, const RgEnumValuesVector& enumerators, uint32_t* value, bool is_bit_flags)
    : RgEditorElement(parent, member_name, RgEditorDataType::kEnum)
    , enumerators_(enumerators)
    , value_(value)
    , is_bit_flags_enum_(is_bit_flags)
    , parent_(parent)
{
    // Ensure that the value pointer is valid.
    assert(value_ != nullptr);

    editor_widget_ = new RgPipelineStateEditorWidgetEnum(is_bit_flags_enum_, parent_);
    editor_widget_->setObjectName(QString::fromStdString(member_name));
    assert(editor_widget_ != nullptr);
    if (editor_widget_ != nullptr)
    {
        // Initialize the editor widget with the list of enumerators and initial value.
        editor_widget_->SetEnumerators(enumerators);
        editor_widget_->SetValue(GetValue());

        // Insert the editor widget into the new row.
        ui_.editorLayout->insertWidget(0, editor_widget_);

        // Connect the editor's value change handler.
        bool is_connected = connect(editor_widget_, &RgPipelineStateEditorWidgetEnum::EditingFinished, this, &RgEditorElement::HandleValueChanged);
        assert(is_connected);

        // Connect the editor widget focus in signal.
        is_connected = connect(editor_widget_, &RgPipelineStateEditorWidgetEnum::FocusInSignal, this, &RgEditorElement::HandleEditorFocusIn);
        assert(is_connected);

        // Connect the editor widget focus out signal.
        is_connected = connect(editor_widget_, &RgPipelineStateEditorWidgetEnum::FocusOutSignal, this, &RgEditorElement::HandleEditorFocusOut);
        assert(is_connected);

        // Connect the list widget status signal.
        is_connected = connect(editor_widget_, &RgPipelineStateEditorWidgetEnum::EnumListWidgetStatusSignal, this, &RgEditorElementEnum::EnumListWidgetStatusSignal);
        assert(is_connected);

        // Connect the shortcut hot key signal.
        is_connected = connect(this, &RgEditorElementEnum::HotKeyPressedSignal, editor_widget_, &RgPipelineStateEditorWidgetEnum::HandleHotKeyPressedSignal);
        assert(is_connected);
    }
}

QVariant RgEditorElementEnum::Data(int column) const
{
    QVariant result = RgEditorElement::Data(column);

    if (column == static_cast<int>(RgRowData::kRowDataMemberValue))
    {
        // Get the current enumeration value.
        uint32_t current_value = GetValue();

        // Is this a normal enumeration or a set of bit flags?
        if (is_bit_flags_enum_)
        {
            std::string flag_bits;
            assert(editor_widget_ != nullptr);
            if (editor_widget_ != nullptr)
            {
                editor_widget_->GetFlagBitsString(flag_bits);
            }

            result = flag_bits.c_str();
        }
        else
        {
            // Search for the name/value pair of the current enumerator.
            RgEnumeratorSearcher searcher(current_value);
            auto enumeratorPairIter = std::find_if(enumerators_.begin(), enumerators_.end(), searcher);
            if (enumeratorPairIter != enumerators_.end())
            {
                result = enumeratorPairIter->name.c_str();
            }
        }
    }

    return result;
}

RgPipelineStateEditorWidget* RgEditorElementEnum::GetEditorWidget()
{
    return editor_widget_;
}

const RgEnumValuesVector& RgEditorElementEnum::GetEnumerators() const
{
    return enumerators_;
}

uint32_t RgEditorElementEnum::GetValue() const
{
    return *value_;
}

void RgEditorElementEnum::SetValue(uint32_t value)
{
    *value_ = value;

    if (value_changed_callback_ != nullptr)
    {
        value_changed_callback_();
    }
}

void RgEditorElementEnum::ValueChangedHandler()
{
    assert(editor_widget_ != nullptr);
    if (editor_widget_ != nullptr)
    {
        // Update the row's value using the editor widget's current value.
        uint32_t enum_value = editor_widget_->GetValue();
        SetValue(enum_value);
    }
}
