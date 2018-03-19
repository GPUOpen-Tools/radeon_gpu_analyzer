#pragma once

// Qt.
#include <QFrame>

// Local.
#include "ui_rgSourceEditorTitlebar.h"

// A title bar inserted above the source editor that displays the correlation status.
class rgSourceEditorTitlebar : public QFrame
{
    Q_OBJECT

public:
    rgSourceEditorTitlebar(QWidget* pParent = nullptr);
    virtual ~rgSourceEditorTitlebar() = default;

    // Update the title bar view based on whether or not a file's correlation state is enabled.
    void SetIsCorrelationEnabled(bool isEnabled);

    // Change the visibility of the title bar contents.
    void SetTitlebarContentsVisibility(bool isVisible);

private:
    // Set the mouse cursor to pointing hand cursor.
    void SetCursor();

    // The generated view object for the title bar.
    Ui::rgSourceEditorTitlebar ui;
};