// C++.
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgFindTextWidget.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSourceCodeEditor.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

rgFindTextWidget::rgFindTextWidget(QWidget* pParent) :
    QWidget(pParent)
{
    // Initialize the interface object.
    ui.setupUi(this);

    // Set the tooltip and status tip for the Find next button.
    rgUtils::SetToolAndStatusTip(STR_EDIT_FIND_NEXT_TOOLTIP, ui.findNextPushButton);

    // Set the tooltip and status tip for the Find previous button.
    rgUtils::SetToolAndStatusTip(STR_EDIT_FIND_PREVIOUS_TOOLTIP, ui.findPreviousPushButton);

    // Set the tooltip for the Close button.
    rgUtils::SetToolAndStatusTip(STR_EDIT_FIND_CLOSE_TOOLTIP, ui.closePushButton);

    // Add a Search magnifying glass to the search textbox.
    ui.searchLineEdit->setClearButtonEnabled(true);
    ui.searchLineEdit->addAction(QIcon(gs_ICON_RESOURCE_FIND_MAGNIFYING_GLASS), QLineEdit::LeadingPosition);

    // Create the keyboard actions associated with the Find widget.
    CreateActions();

    // Connect all internal signals to handler slots.
    ConnectSignals();

    // Set the cursor to pointing hand cursor.
    SetCursor();

    // Set focus proxies for filter and match case buttons to be the line edit,
    // so each time one of these buttons are clicked, the focus will remain with
    // the line edit.
    ui.filterRowsPushButton->setFocusProxy(ui.searchLineEdit);
    ui.matchCasePushButton->setFocusProxy(ui.searchLineEdit);
}

void rgFindTextWidget::SetFocused()
{
    // Focus on the line edit and select all existing text to make new searches quicker.
    ui.searchLineEdit->setFocus();
    ui.searchLineEdit->selectAll();

    // Update the search context options every time the search widget gets focus.
    UpdateSearchOptions();
}

void rgFindTextWidget::SetSearchContext(ISearchable* pSearchContext)
{
    assert(pSearchContext != nullptr);
    if (pSearchContext != nullptr)
    {
        m_pSearchContext = pSearchContext;

        // Toggle the search option button visibility based on the searcher's supported options.
        uint32_t options = m_pSearchContext->GetSupportedOptions();
        ToggleOptionVisibility(options);
    }
}

void rgFindTextWidget::ToggleOptionVisibility(uint32_t options)
{
    ui.findPreviousPushButton->setVisible((options & ISearchable::SupportedOptions::FindPrevious) == ISearchable::SupportedOptions::FindPrevious);
    ui.findNextPushButton->setVisible((options & ISearchable::SupportedOptions::FindNext) == ISearchable::SupportedOptions::FindNext);
    ui.filterRowsPushButton->setVisible((options & ISearchable::SupportedOptions::FilterTree) == ISearchable::SupportedOptions::FilterTree);
    ui.matchCasePushButton->setVisible((options & ISearchable::SupportedOptions::MatchCase) == ISearchable::SupportedOptions::MatchCase);
}

void rgFindTextWidget::SetSearchString(const std::string& searchString)
{
    // Insert the text into the search line edit.
    ui.searchLineEdit->setText(searchString.c_str());
}

void rgFindTextWidget::keyPressEvent(QKeyEvent* pEvent)
{
    // If the user pressed the escape key, close the widget.
    if (pEvent->key() == Qt::Key_Escape)
    {
        // Reset the search when the user closes the Find widget.
        m_pSearchContext->ResetSearch();

        // Let the rgBuildView know that the Find widget has been closed.
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

    // Connect the find previous button to the associated action.
    assert(m_pFindPreviousAction != nullptr);
    if (m_pFindPreviousAction != nullptr)
    {
        // Connect the handler for the "Find previous" button.
        isConnected = connect(ui.findPreviousPushButton, &QPushButton::clicked, this, &rgFindTextWidget::HandleFindPreviousButtonClicked);
        assert(isConnected);

        // Add a hotkey action to the button.
        ui.findPreviousPushButton->addAction(m_pFindPreviousAction);
    }

    // Connect the find next button to the associated action.
    assert(m_pFindNextAction != nullptr);
    if (m_pFindNextAction != nullptr)
    {
        // Connect the handler for the "Find next" button.
        isConnected = connect(ui.findNextPushButton, &QPushButton::clicked, this, &rgFindTextWidget::HandleFindNextButtonClicked);
        assert(isConnected);

        // Add a hotkey action to the button.
        ui.findNextPushButton->addAction(m_pFindNextAction);
    }

    // Connect the handler invoked when the user presses the "Enter" key while the search textbox is selected.
    isConnected = connect(ui.searchLineEdit, &QLineEdit::returnPressed, this, &rgFindTextWidget::HandleReturnPressedOnSearch);
    assert(isConnected);

    // Connect the handler invoked when the user changes the
    // check state of any of the search option buttons.
    isConnected = connect(ui.filterRowsPushButton, &QPushButton::toggled, this, &rgFindTextWidget::HandleOptionButtonCheckChanged);
    assert(isConnected);
    isConnected = connect(ui.matchCasePushButton, &QPushButton::toggled, this, &rgFindTextWidget::HandleOptionButtonCheckChanged);
    assert(isConnected);

    isConnected = connect(ui.searchLineEdit, &QLineEdit::textChanged, this, &rgFindTextWidget::HandleSearchTextChanged);
    assert(isConnected);
}

void rgFindTextWidget::HandleCloseButtonClicked()
{
    // Reset the search results.
    m_pSearchContext->ResetSearch();

    emit CloseWidgetSignal();
}

void rgFindTextWidget::HandleFindPreviousButtonClicked()
{
    // Use the search context to look for the previous match.
    assert(m_pSearchContext != nullptr);
    if (m_pSearchContext != nullptr)
    {
        m_pSearchContext->Find(ui.searchLineEdit->text(), ISearchable::SearchDirection::Previous);
    }
}

void rgFindTextWidget::HandleFindNextButtonClicked()
{
    // Use the search context to look for the next match.
    assert(m_pSearchContext != nullptr);
    if (m_pSearchContext != nullptr)
    {
        m_pSearchContext->Find(ui.searchLineEdit->text(), ISearchable::SearchDirection::Next);
    }
}

void rgFindTextWidget::HandleReturnPressedOnSearch()
{
    // Return starts the next search.
    HandleFindNextButtonClicked();
}

void rgFindTextWidget::HandleOptionButtonCheckChanged(bool checked)
{
    // Update the search option flags in the search context.
    UpdateSearchOptions();
}

void rgFindTextWidget::HandleSearchTextChanged(const QString& updatedText)
{
    HandleReturnPressedOnSearch();
}

void rgFindTextWidget::CreateActions()
{
    // Create the "Find next" action.
    m_pFindNextAction = new QAction(tr(STR_EDIT_FIND_NEXT), this);
    assert(m_pFindNextAction != nullptr);
    if (m_pFindNextAction != nullptr)
    {
        // Configure the hotkey for the Find next action.
        m_pFindNextAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_FIND_NEXT));

        // Connect the handler for the "Find next" button.
        bool isConnected = connect(m_pFindNextAction, &QAction::triggered, this, &rgFindTextWidget::HandleFindNextButtonClicked);
        assert(isConnected);
    }

    // Create the "Find previous" action.
    m_pFindPreviousAction = new QAction(tr(STR_EDIT_FIND_PREVIOUS), this);
    assert(m_pFindPreviousAction != nullptr);
    if (m_pFindPreviousAction != nullptr)
    {
        // Configure the hotkey for the Find previous action.
        m_pFindPreviousAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_FIND_PREVIOUS));

        // Connect the handler for the "Find previous" button.
        bool isConnected = connect(m_pFindPreviousAction, &QAction::triggered, this, &rgFindTextWidget::HandleFindPreviousButtonClicked);
        assert(isConnected);
    }
}

void rgFindTextWidget::SetCursor()
{
    // Set the hand pointers for the buttons.
    ui.filterRowsPushButton->setCursor(Qt::PointingHandCursor);
    ui.matchCasePushButton->setCursor(Qt::PointingHandCursor);
    ui.findNextPushButton->setCursor(Qt::PointingHandCursor);
    ui.findPreviousPushButton->setCursor(Qt::PointingHandCursor);
    ui.closePushButton->setCursor(Qt::PointingHandCursor);
}

void rgFindTextWidget::UpdateSearchOptions()
{
    assert(m_pSearchContext != nullptr);
    if (m_pSearchContext != nullptr)
    {
        // Update the search options by sending the option button states to the search context.
        m_pSearchContext->SetSearchOptions({
            ui.filterRowsPushButton->isChecked(),
            ui.matchCasePushButton->isChecked()
        });

        // Trigger the search.
        m_pSearchContext->ResetSearch();
        m_pSearchContext->Find(ui.searchLineEdit->text(), ISearchable::SearchDirection::Next);
    }
}