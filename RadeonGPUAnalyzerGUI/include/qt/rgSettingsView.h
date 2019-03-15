#pragma once

// Qt.
#include <QWidget>

class rgSettingsView : public QWidget
{
    Q_OBJECT

public:
    rgSettingsView(QWidget* pParent);
    virtual ~rgSettingsView() = default;

    // Set the focus to target selection button.
    virtual void SetInitialWidgetFocus() = 0;
};
