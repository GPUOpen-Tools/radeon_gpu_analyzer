//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for base class implementation of the start tab.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_START_TAB_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_START_TAB_H_

// C++.
#include <memory>

// Qt.
#include <QtWidgets/QWidget>
#include <QMenu>

// Local.
#include "ui_rg_start_tab.h"

// Forward declarations.
class QMenu;
class RgAppState;
class RgRecentProjectWidget;
enum class RgProjectAPI : char;

class RgStartTab : public QWidget
{
    Q_OBJECT

public:
    RgStartTab(QWidget* parent);
    virtual ~RgStartTab() = default;

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
    void OpenProjectFileAtPath(const std::string& project_file_path);

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
    void HandleRecentProjectClickedEvent(QAbstractButton* recent_file_button);

    // Handle when the color theme is changed. Sets the link button stylesheets and icons for the recent files list.
    void OnColorThemeChanged();

protected:
    // Apply API-specific string constants to the view object's widgets.
    virtual void ApplyApiStringConstants() = 0;

    // Get a list of buttons to insert into the start page's "Start" section.
    virtual void GetStartButtons(std::vector<QPushButton*>& start_buttons) = 0;

    // Re-implement eventFilter to handle up/down arrow keys.
    virtual bool eventFilter(QObject* object, QEvent* event) override;

    // Ignore up/down key presses for top and bottom buttons.
    bool ProcessKeyPress(QKeyEvent* key_event, const QString& object_name);

    // The generated view object.
    Ui::RgStartTab ui_;

    // Open recent item action.
    QAction* open_recent_action_ = nullptr;

    // Open the project's folder action.
    QAction* open_containing_folder_action_ = nullptr;

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

    // Set the sytlesheet for the link buttons;
    void SetLinkButtonStylesheet();

    // Set the project API icon to recent projects push button.
    void SetProjectAPIIcon(RgProjectAPI api, RgRecentProjectWidget* widget);

    // The context menu for recent files.
    QMenu menu_;

    // A button group used to handle the array of recent program buttons.
    QButtonGroup* recent_project_button_group_ = nullptr;

    // The parent widget.
    QWidget* parent_ = nullptr;
};
#endif  // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_START_TAB_H_
