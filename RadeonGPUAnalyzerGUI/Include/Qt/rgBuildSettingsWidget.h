#pragma once

// Qt.
#include <QFrame>

class rgBuildSettingsWidget : public QFrame
{
    Q_OBJECT

public:
    rgBuildSettingsWidget(QWidget* pParent);
    virtual ~rgBuildSettingsWidget() = default;

protected:
    virtual void focusInEvent(QFocusEvent* pEvent) override;
    virtual void focusOutEvent(QFocusEvent* pEvent) override;

signals:
    void FrameFocusInEventSignal();
    void FrameFocusOutEventSignal();
};

