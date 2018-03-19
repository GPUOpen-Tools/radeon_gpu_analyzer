// C++.
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgFindTextWidget.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgSourceCodeEditor.h>
#include <RadeonGPUAnalyzerGUI/include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>

rgFindTextWidget::rgFindTextWidget(ISearchable* pSearchContext, QWidget* pParent) :
    QWidget(pParent),
    m_pSearchContext(pSearchContext)
{
    // Initialize the interface object.
    ui.setupUi(this);

    // Add a Search magnifying glass to the search textbox.
    ui.searchLineEdit->setClearButtonEnabled(true);
    ui.searchLineEdit->addAction(QIcon(gs_ICON_RESOURCE_FIND_MAGNIFYING_GLASS), QLineEdit::LeadingPosition);

    // Connect all internal signals to handler slots.
    ConnectSignals();

    // Set the cursor to pointing hand cursor.
    SetCursor();
}

void rgFindTextWidget::SetFocused()
{
    ui.searchLineEdit->setFocus();
}

void rgFindTextWidget::keyPressEvent(QKeyEvent* pEvent)
{
    // If the user pressed the escape key, close the widget.
    if (pEvent->key() == Qt::Key_Escape)
    {
        emit CloseWidgetSignal();
    }

    // Invoke the baseclass QWidget implementation of the key handler.
    QWidget::keyPressEvent(pEvent);
}

void rgFindTextWidget::ConnectSignals()
{
    // Connect the handler for the close button.
    bool isConnected = connect(ui.closePushButton, &QPushButton::clicked, this, &rgFindTextWidget::HandleCloseButtonClicked);
    assert(isConnected);

    // Connect the handler for the "Find next" button.
    isConnected = connect(ui.findNextPushButton, &QPushButton::clicked, this, &rgFindTextWidget::HandleFindNextButtonClicked);
    assert(isConnected);

    // Connect the handler invoked when the user presses the "Enter" key while the search textbox is selected.
    isConnected = connect(ui.searchLineEdit, &QLineEdit::returnPressed, this, &rgFindTextWidget::HandleReturnPressedOnSearch);
    assert(isConnected);
}

void rgFindTextWidget::HandleCloseButtonClicked()
{
    emit CloseWidgetSignal();
}

void rgFindTextWidget::HandleFindNextButtonClicked()
{
    // Use the search context to look for the next match.
    assert(m_pSearchContext != nullptr);
    if (m_pSearchContext != nullptr)
    {
        m_pSearchContext->FindNext(ui.searchLineEdit->text());
    }
}

void rgFindTextWidget::HandleReturnPressedOnSearch()
{
    // Return starts the next search.
    HandleFindNextButtonClicked();
}

void rgFindTextWidget::SetCursor()
{
    ui.searchLineEdit->setCursor(Qt::PointingHandCursor);
    ui.closePushButton->setCursor(Qt::PointingHandCursor);
    ui.findNextPushButton->setCursor(Qt::PointingHandCursor);
}