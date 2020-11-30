// C++.
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElementArrayElementAdd.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateEditorWidgetArrayElementAdd.h>

rgPipelineStateEditorWidgetArrayElementAdd::rgPipelineStateEditorWidgetArrayElementAdd(rgEditorElementArrayElementAdd* pParent)
    : rgPipelineStateEditorWidget(pParent)
    , m_pArrayRootElement(pParent)
{
    ui.setupUi(this);

    // Update the cursor for the delete button.
    ui.addElementButton->setCursor(Qt::PointingHandCursor);

    // Set the Plus button as the focus proxy.
    setFocusProxy(ui.addElementButton);

    // Connect the internal widget signals.
    ConnectSignals();

    // If the array is resized, update the size label.
    m_pArrayRootElement->SetResizeCallback([=] { UpdateArraySizeLabel(); });
}

void rgPipelineStateEditorWidgetArrayElementAdd::HandleAddButtonClicked()
{
    assert(m_pArrayRootElement != nullptr);
    if (m_pArrayRootElement != nullptr)
    {
        // Add a new element to the array.
        m_pArrayRootElement->AddNewElement();

        // Update the array size label to match the new element count.
        UpdateArraySizeLabel();
    }
}

void rgPipelineStateEditorWidgetArrayElementAdd::ConnectSignals()
{
    // Connect the add element button's signal to be forwarded to a local public signal.
    bool isConnected = connect(ui.addElementButton, &QPushButton::clicked, this, &rgPipelineStateEditorWidgetArrayElementAdd::HandleAddButtonClicked);
    assert(isConnected);
}

void rgPipelineStateEditorWidgetArrayElementAdd::UpdateArraySizeLabel()
{
    // Extract the array size element from the array root element.
    rgEditorElementNumeric<uint32_t>* pSizeElement = m_pArrayRootElement->GetArraySizeElement();
    assert(pSizeElement != nullptr);
    if (pSizeElement != nullptr)
    {
        // Extract the array dimension from the size element, and set the count label.
        uint32_t currentSize = pSizeElement->GetValue();
        ui.countLabel->setText(QString::number(currentSize));

        // Hide/show the count label depending on the number of items.
        if (currentSize > 0)
        {
            ui.countLabel->show();
        }
        else
        {
            ui.countLabel->hide();
        }
    }
}