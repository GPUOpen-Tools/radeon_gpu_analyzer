#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_FIND_TEXT_WIDGET_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_FIND_TEXT_WIDGET_H_

// Qt.
#include <QWidget>
#include <QAction>
#include <QTimer>

// Local.
#include "ui_rg_find_text_widget.h"

// Forward declarations.
class RgSourceCodeEditor;
namespace Ui
{
    class RgFindTextWidget;
}

// An interface that the RgFindTextWidget can use to search multiple sources.
class ISearchable
{
public:
    ISearchable() = default;
    virtual ~ISearchable() = default;

    // The direction to search in.
    enum class SearchDirection
    {
        kPrevious,
        kNext
    };

    // A flag enumeration of supported search options.
    enum SupportedOptions : char
    {
        kFindPrevious    = 1 << 0,
        kFindNext        = 1 << 1,
        kFilterTree      = 1 << 2,
        kMatchCase       = 1 << 3,
        kUseRegex        = 1 << 4,
    };

    // A structure of search option flags.
    struct SearchOptions
    {
        bool filter_tree;
        bool match_case;
        bool use_regex;
    };

    // Get the supported search option flags.
    virtual uint32_t GetSupportedOptions() = 0;

    // Find the given string in the search target. After finding the first result, the direction
    // is taken into account.
    virtual bool Find(const QString& search_string, SearchDirection direction) = 0;

    // Reset the current search results.
    virtual void ResetSearch() = 0;

    // Select the current results.
    virtual void SelectResults() = 0;

    // Set the search options.
    virtual void SetSearchOptions(const SearchOptions& options) = 0;
};

// A widget used to search for text in a searchable view.
class RgFindTextWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RgFindTextWidget(QWidget* parent = nullptr);
    virtual ~RgFindTextWidget() = default;

    // A custom keypress handler used to close the widget.
    virtual void keyPressEvent(QKeyEvent* event) override;

    // Set the focus to the search textbox within the RgFindTextWidget.
    void SetFocused();

    // Set the search content for the find widget.
    void SetSearchContext(ISearchable* search_context);

    // Set the text to search for.
    void SetSearchString(const std::string& search_string);

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
    void HandleSearchTextChanged(const QString& updated_text);

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
    ISearchable* search_context_ = nullptr;

    // The action used to find the previous search result.
    QAction* find_previous_action_ = nullptr;

    // The action used to find the next search result.
    QAction* find_next_action_ = nullptr;

    // The generated find widget view object.
    Ui::RgFindTextWidget ui_;

    // Timer to detect when the user stops typing.
    QTimer* typing_timer_ = new QTimer(this);
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_FIND_TEXT_WIDGET_H_
