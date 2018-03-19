#pragma once

// Qt.
#include <QWidget>

// Local.
#include "ui_rgAddCreateMenuItem.h"
#include <RadeonGPUAnalyzerGUI/include/qt/rgFileMenuItem.h>

// Forward declarations:
class QPushButton;

class rgAddCreateMenuItem :
    public rgFileMenuItem
{
    Q_OBJECT

public:
    explicit rgAddCreateMenuItem(rgFileMenu* pParent = nullptr);
    virtual ~rgAddCreateMenuItem() = default;

    // Getter for the Add button.
    QPushButton* GetAddButton() const;

    // Getter for the Create button.
    QPushButton* GetCreateButton() const;

private:
    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // The generated UI object for this view.
    Ui::rgAddCreateMenuItem ui;
};