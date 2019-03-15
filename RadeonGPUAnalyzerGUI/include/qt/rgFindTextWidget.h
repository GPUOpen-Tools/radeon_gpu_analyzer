#pragma once

// Qt.
#include <QWidget>
#include <QAction>
#include <QTimer>

// Local.
#include "ui_rgFindTextWidget.h"

// Forward declarations.
class rgSourceCodeEditor;
namespace Ui
{
    class rgFindTextWidget;
}

// An interface that the rgFindTextWidget can use to search multiple sources.
class ISearchable
{
public:
    ISearchable() = default;
    virtual ~ISearchable() = default;

    // The direction to search in.
    enum class SearchDirection
    {
        Previous,
        Next
    };

    // A flag enumeration of supported search options.
    enum SupportedOptions : char
    {
        FindPrevious    = 1 << 0,
        FindNext        = 1 << 1,
        FilterTree      = 1 << 2,
        MatchCase       = 1 << 3,
        UseRegex        = 1 << 4,
    };

    // A structure of search option flags.
    struct SearchOptions
    {
        bool m_filterTree;
        bool m_matchCase;
        bool m_useRegex;
    };

    // Get the supported search option flags.
    virtual uint32_t GetSupportedOptions() = 0;

    // Find the given string in the search target. After finding the first result, the direction
    // is taken into account.
    virtual bool Find(const QString& searchString, SearchDirection direction) = 0;

    // Reset the current search results.
    virtual void ResetSearch() = 0;

    // Select the current results.
    virtual void SelectResults() = 0;

    // Set the search options.
    virtual void SetSearchOptions(const SearchOptions& options) = 0;
};

// A widget used to search for text in a searchable view.
class rgFindTextWidget : public QWidget
{
    Q_OBJECT

public:
    explicit rgFindTextWidget(QWidget* pParent = nullptr);
    virtual ~rgFindTextWidget() = default;

    // A custom keypress handler used to close the widget.
    virtual void keyPressEvent(QKeyEvent* pEvent) override;

    // Set the focus to the search textbox within the rgFindTextWidget.
    void SetFocused();

    // Set the search content for the find widget.
    void SetSearchContext(ISearchable* pSearchContext);

    // Set the text to search for.
    void SetSearchString(const std::string& searchString);

    // Update the visibility of the search options buttons, based on
    // the operations supported by the current searcher implementation.
    void ToggleOptionVisibility(uint32_t options);

signals:
    // A signal emitted when the widget should be hidden from view.
    void CloseWidgetSignal();

public slots:
    // Handler invoked when the user clicks the "Find previous" button.
    void HandleFindPreviousButtonClicked();

    // Handler invoked when the user clicks the "Find next" button.
    void HandleFindNextButtonClicked();

    // Handler invoked when the user clicks the Close button.
    void HandleCloseButtonClicked();

    // Handler invoked when the user pressed the "Enter" key with the search textbox selected.
    void HandleReturnPressedOnSearch();

    // Handler invoked when the user changes the check state of the "filter rows" option button.
    void HandleOptionButtonCheckChanged(bool checked);

    // Handler invoked whenever the search box text changes.
    void HandleSearchTextChanged(const QString& updatedText);

private:
    // Connect signals within the widget.
    void ConnectSignals();

    // Create the keyboard shortcut actions.
    void CreateActions();

    // Set the cursor to pointing hand cursor.
    void SetCursor();

    // Update the options in the search context.
    void UpdateSearchOptions();

    // An interface used to search a target widget.
    ISearchable* m_pSearchContext = nullptr;

    // The action used to find the previous search result.
    QAction* m_pFindPreviousAction = nullptr;

    // The action used to find the next search result.
    QAction* m_pFindNextAction = nullptr;

    // The generated find widget view object.
    Ui::rgFindTextWidget ui;

    // Timer to detect when the user stops typing.
    QTimer *m_pTypingTimer = new QTimer(this);
};