#pragma once

// C++.
#include <memory>

// Qt.
#include <QtWidgets/QWidget>
#include <QMenu>

// Local.
#include "ui_rgStartTab.h"

// Forward declarations.
class QMenu;
class rgAppState;
class rgRecentProjectWidget;
enum class rgProjectAPI : char;

class rgStartTab : public QWidget
{
    Q_OBJECT

public:
    rgStartTab(QWidget* pParent);
    virtual ~rgStartTab() = default;

    // Initialize the start tab.
    void Initialize();

    // Populate the list of recent files in the welcome page.
    void PopulateRecentProjectsList();

    // Retrieve the widget displaying the list of recent programs.
    QWidget* GetRecentProgramsListWidget() const;

    // Get the first button used to create a new project.
    QPushButton* GetCreateNewFileButton();

signals:
    // Signal for the Open Program button click.
    void OpenProjectFileEvent();

    // Signal to load the project file at the given path.
    void OpenProjectFileAtPath(const std::string& projectFilePath);

    // Signal emitted when the Help->About item is clicked.
    void AboutEvent();

    // Signal emitted when the Help->Getting started guide item is clicked.
    void GettingStartedGuideEvent();

    // Signal emitted when the Help->Help manual item is clicked.
    void HelpManual();

protected slots:
    // Handler for when the user requests a context menu for recent files.
    void HandleContextMenuRequest(const QPoint& pos);

    // Handler for a click on a recent program item.
    void HandleRecentProjectClickedEvent(int buttonId);

protected:
    // Apply API-specific string constants to the view object's widgets.
    virtual void ApplyApiStringConstants() = 0;

    // Get a list of buttons to insert into the start page's "Start" section.
    virtual void GetStartButtons(std::vector<QPushButton*>& startButtons) = 0;

    // Re-implement eventFilter to handle up/down arrow keys.
    virtual bool eventFilter(QObject* pObject, QEvent* pEvent) override;

    // Ignore up/down key presses for top and bottom buttons.
    bool ProcessKeyPress(QKeyEvent* pKeyEvent, const QString& objectName);

    // The generated view object.
    Ui::rgStartTab ui;

    // Open recent item action.
    QAction* m_pOpenRecentAction = nullptr;

    // Open the project's folder action.
    QAction* m_pOpenContainingFolderAction = nullptr;

private:
    // Apply any string constants where needed.
    void ApplyStringConstants();

    // Connect the signals.
    void ConnectSignals();

    // Clear the list of recent program buttons.
    void ClearRecentProjectsList();

    // Create the context menu items.
    void CreateContextMenu();

    // Add the start buttons to the "Start" section of the start tab.
    void InitializeStartButtons();

    // Reorder the tab order to allow for the recent projects list additions/removals.
    void ReorderTabOrder();

    // Set the view's cursor for each relevant widget.
    void SetCursor();

    // Set the project API icon to recent projects push button.
    void SetProjectAPIIcon(rgProjectAPI api, rgRecentProjectWidget* pWidget);

    // The context menu for recent files.
    QMenu m_menu;

    // A button group used to handle the array of recent program buttons.
    QButtonGroup* m_pRecentProjectButtonGroup = nullptr;

    // The parent widget.
    QWidget* m_pParent = nullptr;
};