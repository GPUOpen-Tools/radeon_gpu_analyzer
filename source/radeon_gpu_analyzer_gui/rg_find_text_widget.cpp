// C++.
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_find_text_widget.h"
#include "radeon_gpu_analyzer_gui/qt/rg_source_code_editor.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

RgFindTextWidget::RgFindTextWidget(QWidget* parent) :
    QWidget(parent)
{
    // Initialize the interface object.
    ui_.setupUi(this);

    // Set the tooltip and status tip for the Find next button.
    RgUtils::SetToolAndStatusTip(kStrEditFindNextTooltip, ui_.findNextPushButton);

    // Set the tooltip and status tip for the Find previous button.
    RgUtils::SetToolAndStatusTip(kStrEditFindPreviousTooltip, ui_.findPreviousPushButton);

    // Set the tooltip for the Close button.
    RgUtils::SetToolAndStatusTip(kStrEditFindCloseTooltip, ui_.closePushButton);

    // Add a Search magnifying glass to the search textbox.
    ui_.searchLineEdit->setClearButtonEnabled(true);
    ui_.searchLineEdit->addAction(QIcon(kIconResourceFindMagnifyingGlass), QLineEdit::LeadingPosition);

    // Create the keyboard actions associated with the Find widget.
    CreateActions();

    // Connect all internal signals to handler slots.
    ConnectSignals();

    // Set the cursor to pointing hand cursor.
    SetCursor();

    // Set focus proxies for filter and match case buttons to be the line edit,
    // so each time one of these buttons are clicked, the focus will remain with
    // the line edit.
    ui_.filterRowsPushButton->setFocusProxy(ui_.searchLineEdit);
    ui_.matchCasePushButton->setFocusProxy(ui_.searchLineEdit);
}

void RgFindTextWidget::SetFocused()
{
    // Focus on the line edit and select all existing text to make new searches quicker.
    ui_.searchLineEdit->setFocus();
    ui_.searchLineEdit->selectAll();

    // Update the search context options every time the search widget gets focus.
    UpdateSearchOptions();
}

void RgFindTextWidget::SetSearchContext(ISearchable* search_context)
{
    assert(search_context != nullptr);
    if (search_context != nullptr)
    {
        search_context_ = search_context;

        // Toggle the search option button visibility based on the searcher's supported options.
        uint32_t options = search_context_->GetSupportedOptions();
        ToggleOptionVisibility(options);
    }
}

void RgFindTextWidget::ToggleOptionVisibility(uint32_t options)
{
    ui_.findPreviousPushButton->setVisible((options & ISearchable::SupportedOptions::kFindPrevious) == ISearchable::SupportedOptions::kFindPrevious);
    ui_.findNextPushButton->setVisible((options & ISearchable::SupportedOptions::kFindNext) == ISearchable::SupportedOptions::kFindNext);
    ui_.filterRowsPushButton->setVisible((options & ISearchable::SupportedOptions::kFilterTree) == ISearchable::SupportedOptions::kFilterTree);
    ui_.matchCasePushButton->setVisible((options & ISearchable::SupportedOptions::kMatchCase) == ISearchable::SupportedOptions::kMatchCase);
}

void RgFindTextWidget::SetSearchString(const std::string& search_string)
{
    // Insert the text into the search line edit.
    ui_.searchLineEdit->setText(search_string.c_str());
}

void RgFindTextWidget::keyPressEvent(QKeyEvent* event)
{
    // If the user pressed the escape key, close the widget.
    if (event->key() == Qt::Key_Escape)
    {
        // Reset the search when the user closes the Find widget.
        search_context_->ResetSearch();

        // Let the RgBuildView know that the Find widget has been closed.
        emit CloseWidgetSignal();
    }

    // Invoke the baseclass QWidget implementation of the key handler.
    QWidget::keyPressEvent(event);
}

void RgFindTextWidget::ConnectSignals()
{
    // Connect the handler for the close button.
    bool is_connected = connect(ui_.closePushButton, &QPushButton::clicked, this, &RgFindTextWidget::HandleCloseButtonClicked);
    assert(is_connected);

    // Connect the find previous button to the associated action.
    assert(find_previous_action_ != nullptr);
    if (find_previous_action_ != nullptr)
    {
        // Connect the handler for the "Find previous" button.
        is_connected = connect(ui_.findPreviousPushButton, &QPushButton::clicked, this, &RgFindTextWidget::HandleFindPreviousButtonClicked);
        assert(is_connected);

        // Add a hotkey action to the button.
        ui_.findPreviousPushButton->addAction(find_previous_action_);
    }

    // Connect the find next button to the associated action.
    assert(find_next_action_ != nullptr);
    if (find_next_action_ != nullptr)
    {
        // Connect the handler for the "Find next" button.
        is_connected = connect(ui_.findNextPushButton, &QPushButton::clicked, this, &RgFindTextWidget::HandleFindNextButtonClicked);
        assert(is_connected);

        // Add a hotkey action to the button.
        ui_.findNextPushButton->addAction(find_next_action_);
    }

    // Connect the handler invoked when the user presses the "Enter" key while the search textbox is selected.
    is_connected = connect(ui_.searchLineEdit, &QLineEdit::returnPressed, this, &RgFindTextWidget::HandleReturnPressedOnSearch);
    assert(is_connected);

    // Connect the handler invoked when the user changes the
    // check state of any of the search option buttons.
    is_connected = connect(ui_.filterRowsPushButton, &QPushButton::toggled, this, &RgFindTextWidget::HandleOptionButtonCheckChanged);
    assert(is_connected);
    is_connected = connect(ui_.matchCasePushButton, &QPushButton::toggled, this, &RgFindTextWidget::HandleOptionButtonCheckChanged);
    assert(is_connected);

    is_connected = connect(ui_.searchLineEdit, &QLineEdit::textChanged, this, &RgFindTextWidget::HandleSearchTextChanged);
    assert(is_connected);
}

void RgFindTextWidget::HandleCloseButtonClicked()
{
    // Reset the search results.
    search_context_->ResetSearch();

    emit CloseWidgetSignal();
}

void RgFindTextWidget::HandleFindPreviousButtonClicked()
{
    // Use the search context to look for the previous match.
    assert(search_context_ != nullptr);
    if (search_context_ != nullptr)
    {
        search_context_->Find(ui_.searchLineEdit->text(), ISearchable::SearchDirection::kPrevious);
    }
}

void RgFindTextWidget::HandleFindNextButtonClicked()
{
    // Use the search context to look for the next match.
    assert(search_context_ != nullptr);
    if (search_context_ != nullptr)
    {
        search_context_->Find(ui_.searchLineEdit->text(), ISearchable::SearchDirection::kNext);
    }
}

void RgFindTextWidget::HandleReturnPressedOnSearch()
{
    // Return starts the next search.
    HandleFindNextButtonClicked();
}

void RgFindTextWidget::HandleOptionButtonCheckChanged(bool checked)
{
    Q_UNUSED(checked);

    // Update the search option flags in the search context.
    UpdateSearchOptions();
}

void RgFindTextWidget::HandleSearchTextChanged(const QString& updated_text)
{
    Q_UNUSED(updated_text);

    HandleReturnPressedOnSearch();
}

void RgFindTextWidget::CreateActions()
{
    // Create the "Find next" action.
    find_next_action_ = new QAction(tr(kStrEditFindNext), this);
    assert(find_next_action_ != nullptr);
    if (find_next_action_ != nullptr)
    {
        // Configure the hotkey for the Find next action.
        find_next_action_->setShortcut(QKeySequence(kActionHotkeyFindNext));

        // Connect the handler for the "Find next" button.
        [[maybe_unused]] bool is_connected = connect(find_next_action_, &QAction::triggered, this, &RgFindTextWidget::HandleFindNextButtonClicked);
        assert(is_connected);
    }

    // Create the "Find previous" action.
    find_previous_action_ = new QAction(tr(kStrEditFindPrevious), this);
    assert(find_previous_action_ != nullptr);
    if (find_previous_action_ != nullptr)
    {
        // Configure the hotkey for the Find previous action.
        find_previous_action_->setShortcut(QKeySequence(kActionHotkeyFindPrevious));

        // Connect the handler for the "Find previous" button.
        [[maybe_unused]] bool is_connected = connect(find_previous_action_, &QAction::triggered, this, &RgFindTextWidget::HandleFindPreviousButtonClicked);
        assert(is_connected);
    }
}

void RgFindTextWidget::SetCursor()
{
    // Set the hand pointers for the buttons.
    ui_.filterRowsPushButton->setCursor(Qt::PointingHandCursor);
    ui_.matchCasePushButton->setCursor(Qt::PointingHandCursor);
    ui_.findNextPushButton->setCursor(Qt::PointingHandCursor);
    ui_.findPreviousPushButton->setCursor(Qt::PointingHandCursor);
    ui_.closePushButton->setCursor(Qt::PointingHandCursor);
}

void RgFindTextWidget::UpdateSearchOptions()
{
    assert(search_context_ != nullptr);
    if (search_context_ != nullptr)
    {
        // Update the search options by sending the option button states to the search context.
        search_context_->SetSearchOptions({
            ui_.filterRowsPushButton->isChecked(),
            ui_.matchCasePushButton->isChecked()
        });

        // Trigger the search.
        search_context_->ResetSearch();
        search_context_->Find(ui_.searchLineEdit->text(), ISearchable::SearchDirection::kNext);
    }
}
