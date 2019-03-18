#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElement.h>

// Forward declarations.
class rgEditorElementArray;
class rgPipelineStateEditorWidgetArrayElement;

class rgEditorElementArrayElement : public rgEditorElement
{
    Q_OBJECT

public:
    rgEditorElementArrayElement(QWidget* pParent, const std::string& memberName);
    virtual ~rgEditorElementArrayElement() = default;

    // Retrieve the editor widget used by the row.
    virtual rgPipelineStateEditorWidget* GetEditorWidget() override;

    // Update the array root element and child element index.
    void SetElementIndex(rgEditorElementArray* pParentArray, int childIndex);

private slots:
    // A handler invoked when the delete button is clicked.
    void HandleDeleteButtonClicked();

private:
    // Connect signals to slots.
    void ConnectSignals();

    // The index of the associated child element.
    int m_childIndex;

    // The array element being resized.
    rgEditorElementArray* m_pArrayRootElement = nullptr;

    // The widget used to remove a single array element.
    rgPipelineStateEditorWidgetArrayElement* m_pEditorWidget = nullptr;
};