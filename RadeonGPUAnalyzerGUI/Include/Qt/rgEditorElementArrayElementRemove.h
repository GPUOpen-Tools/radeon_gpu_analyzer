#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElement.h>

// Forward declarations.
class rgEditorElementArrayElementAdd;
class rgPipelineStateEditorWidgetArrayElementRemove;

class rgEditorElementArrayElementRemove : public rgEditorElement
{
    Q_OBJECT

public:
    rgEditorElementArrayElementRemove(QWidget* pParent, const std::string& memberName);
    virtual ~rgEditorElementArrayElementRemove() = default;

    // Retrieve the editor widget used by the row.
    virtual rgPipelineStateEditorWidget* GetEditorWidget() override;

    // Update the array root element and child element index.
    void SetElementIndex(rgEditorElementArrayElementAdd* pParentArray, int childIndex);

private slots:
    // A handler invoked when the delete button is clicked.
    void HandleDeleteButtonClicked();

private:
    // Connect signals to slots.
    void ConnectSignals();

    // The index of the associated child element.
    int m_childIndex;

    // The array element being resized.
    rgEditorElementArrayElementAdd* m_pArrayRootElement = nullptr;

    // The widget used to remove a single array element.
    rgPipelineStateEditorWidgetArrayElementRemove* m_pEditorWidget = nullptr;
};