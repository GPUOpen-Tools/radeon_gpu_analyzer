//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for RgEditorElementArrayElementAdd class.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_EDITOR_ELEMENT_ARRAY_ELEMENT_ADD_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_EDITOR_ELEMENT_ARRAY_ELEMENT_ADD_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_editor_element.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_editor_element_numeric.h"

// Forward declarations.
class RgPipelineStateEditorWidgetArrayElementAdd;

class RgEditorElementArrayElementAdd : public RgEditorElement
{
    Q_OBJECT

public:
    RgEditorElementArrayElementAdd(QWidget* parent, const std::string& member_name, std::function<void(int)> element_removed_callback = nullptr);
    virtual ~RgEditorElementArrayElementAdd() = default;

    // Retrieve the editor widget used by the row.
    virtual RgPipelineStateEditorWidget* GetEditorWidget() override;

    // Resize the array by adding 1 new element to the end.
    void AddNewElement();

    // Remove the array element at the given index.
    void RemoveElement(int element_index);

    // A method used to invoke the resize callback.
    void InvokeElementResizedCallback();

    // Retrieve the element containing the array size.
    RgEditorElementNumeric<uint32_t>* GetArraySizeElement() const;

    // Set the array size element. This element is used to dictate the dimension of an array.
    void SetArraySizeElement(RgEditorElementNumeric<uint32_t>* array_size_element);

    // Set the maximum dimension that the array can be increased to.
    void SetMaximumArraySize(uint32_t size);

    // Set the resize callback, which is invoked when the array is resized.
    void SetResizeCallback(std::function<void()> resize_callback);

private:
    // The editor element containing the associated array's size.
    RgEditorElementNumeric<uint32_t>* array_size_element_ = nullptr;

    // The widget used to edit the array dimension.
    RgPipelineStateEditorWidgetArrayElementAdd* editor_widget_ = nullptr;

    // A callback invoked when the element value is changed.
    std::function<void(int)> element_removed_callback_ = nullptr;

    // A callback invoked when the array is finished being resized.
    std::function<void()> array_resized_callback_ = nullptr;

    // The maximum array size. When -1, the array dimension is not limited.
    int32_t maximum_size_ = -1;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_EDITOR_ELEMENT_ARRAY_ELEMENT_ADD_H_
