#pragma once

// Qt.
#include <QWidget>

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

    // Find the first, next, or final instance of the search string in the search space.
    virtual bool FindNext(const QString& searchString) = 0;

    // Reset the current search results.
    virtual void ResetSearch() = 0;

    // Select the current results.
    virtual void SelectResults() = 0;
};

// A widget used to search for text in a searchable view.
class rgFindTextWidget : public QWidget
{
    Q_OBJECT

public:
    explicit rgFindTextWidget(ISearchable* pSearchTarget, QWidget* pParent = nullptr);
    virtual ~rgFindTextWidget() = default;

    // Set the focus to the search textbox within the rgFindTextWidget.
    void SetFocused();

    // A custom keypress handler used to close the widget.
    virtual void keyPressEvent(QKeyEvent* pEvent) override;

signals:
    // A signal emitted when the widget should be hidden from view.
    void CloseWidgetSignal();

public slots:
    // Handler invoked when the user clicks the "Find next" button.
    void HandleFindNextButtonClicked();

    // Handler invoked when the user clicks the Close button.
    void HandleCloseButtonClicked();

    // Handler invoked when the user pressed the "Enter" key with the search textbox selected.
    void HandleReturnPressedOnSearch();

private:
    // Connect signals within the widget.
    void ConnectSignals();

    // Set the cursor to pointing hand cursor.
    void SetCursor();

    // An interface used to search a target widget.
    ISearchable* m_pSearchContext = nullptr;

    // The generated find widget view object.
    Ui::rgFindTextWidget ui;
};