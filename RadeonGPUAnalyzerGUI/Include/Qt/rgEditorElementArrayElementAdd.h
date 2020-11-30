#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElement.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElementNumeric.h>

// Forward declarations.
class rgPipelineStateEditorWidgetArrayElementAdd;

class rgEditorElementArrayElementAdd : public rgEditorElement
{
    Q_OBJECT

public:
    rgEditorElementArrayElementAdd(QWidget* pParent, const std::string& memberName, std::function<void(int)> elementRemovedCallback = nullptr);
    virtual ~rgEditorElementArrayElementAdd() = default;

    // Retrieve the editor widget used by the row.
    virtual rgPipelineStateEditorWidget* GetEditorWidget() override;

    // Resize the array by adding 1 new element to the end.
    void AddNewElement();

    // Remove the array element at the given index.
    void RemoveElement(int elementIndex);

    // A method used to invoke the resize callback.
    void InvokeElementResizedCallback();

    // Retrieve the element containing the array size.
    rgEditorElementNumeric<uint32_t>* GetArraySizeElement() const;

    // Set the array size element. This element is used to dictate the dimension of an array.
    void SetArraySizeElement(rgEditorElementNumeric<uint32_t>* pArraySizeElement);

    // Set the maximum dimension that the array can be increased to.
    void SetMaximumArraySize(uint32_t size);

    // Set the resize callback, which is invoked when the array is resized.
    void SetResizeCallback(std::function<void()> resizeCallback);

private:
    // The editor element containing the associated array's size.
    rgEditorElementNumeric<uint32_t>* m_pArraySizeElement = nullptr;

    // The widget used to edit the array dimension.
    rgPipelineStateEditorWidgetArrayElementAdd* m_pEditorWidget = nullptr;

    // A callback invoked when the element value is changed.
    std::function<void(int)> m_elementRemovedCallback = nullptr;

    // A callback invoked when the array is finished being resized.
    std::function<void()> m_arrayResizedCallback = nullptr;

    // The maximum array size. When -1, the array dimension is not limited.
    int32_t m_maximumSize = -1;
};