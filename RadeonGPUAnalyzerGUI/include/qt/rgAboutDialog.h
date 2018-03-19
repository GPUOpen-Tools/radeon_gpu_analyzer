#pragma once

// Qt.
#include <QDialog>

// Local.
#include "ui_rgAboutDialog.h"

class rgAboutDialog :
    public QDialog
{
    Q_OBJECT

public:
    rgAboutDialog(QWidget* pParent);
    virtual ~rgAboutDialog() = default;

private:
    // The generated interface view object.
    Ui::rgAboutDialog ui;
};

