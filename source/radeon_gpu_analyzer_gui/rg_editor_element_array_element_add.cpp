// C++.
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_editor_element_array_element_add.h"
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_editor_widget_array_element_add.h"

RgEditorElementArrayElementAdd::RgEditorElementArrayElementAdd(QWidget* parent, const std::string& member_name, std::function<void(int)> element_removed_callback)
    : RgEditorElement(parent, member_name, RgEditorDataType::kArray)
    , element_removed_callback_(element_removed_callback)
{
    editor_widget_ = new RgPipelineStateEditorWidgetArrayElementAdd(this);
    assert(editor_widget_ != nullptr);
    if (editor_widget_ != nullptr)
    {
        ui_.editorLayout->insertWidget(0, editor_widget_);

        // Connect the loss of focus handler.
        bool is_connected = connect(editor_widget_, &RgPipelineStateEditorWidget::FocusOutSignal,
            this, &RgEditorElement::HandleEditorFocusOut);
        assert(is_connected);
    }
}

RgPipelineStateEditorWidget* RgEditorElementArrayElementAdd::GetEditorWidget()
{
    return editor_widget_;
}

void RgEditorElementArrayElementAdd::AddNewElement()
{
    assert(array_size_element_ != nullptr);
    if (array_size_element_ != nullptr)
    {
        // Is there a maximum size specified for this array?
        bool maximum_size_reached = false;
        uint32_t current_array_size = array_size_element_->GetValue();
        if (maximum_size_ > 0)
        {
            maximum_size_reached = current_array_size == static_cast<uint32_t>(maximum_size_);
        }

        // Increase the array dimension if possible.
        if (!maximum_size_reached)
        {
            array_size_element_->SetValue(current_array_size + 1);
        }
    }
}

void RgEditorElementArrayElementAdd::RemoveElement(int element_index)
{
    // Remove the given element, and shift all remaining elements down in the array.
    assert(element_removed_callback_ != nullptr);
    if (element_removed_callback_ != nullptr)
    {
        // Invoke the callback used to handle elements being removed from the array.
        element_removed_callback_(element_index);
    }

    // Update the array size element.
    int current_array_size = static_cast<int>(array_size_element_->GetValue());
    array_size_element_->SetValue(current_array_size - 1);

    // Invoke the resized callback.
    InvokeElementResizedCallback();
}

void RgEditorElementArrayElementAdd::InvokeElementResizedCallback()
{
    // Invoke the array resized callback if available.
    if (array_resized_callback_ != nullptr)
    {
        array_resized_callback_();
    }

    // If the array was resized, and no longer has any child elements, update the indentation.
    if (child_items_.empty())
    {
        UpdateIndentation();
    }
}

RgEditorElementNumeric<uint32_t>* RgEditorElementArrayElementAdd::GetArraySizeElement() const
{
    return array_size_element_;
}

void RgEditorElementArrayElementAdd::SetArraySizeElement(RgEditorElementNumeric<uint32_t>* array_size_element)
{
    assert(array_size_element != nullptr);
    if (array_size_element != nullptr)
    {
        array_size_element_ = array_size_element;
    }
}

void RgEditorElementArrayElementAdd::SetMaximumArraySize(uint32_t maximum_size)
{
    maximum_size_ = maximum_size;
}

void RgEditorElementArrayElementAdd::SetResizeCallback(std::function<void()> resize_callback)
{
    assert(resize_callback != nullptr);
    if (resize_callback != nullptr)
    {
        array_resized_callback_ = resize_callback;
    }
}
