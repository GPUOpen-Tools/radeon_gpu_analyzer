#pragma once

// Qt.
#include <QCheckBox>

class rgCheckBox : public QCheckBox
{
    Q_OBJECT

public:
    rgCheckBox(QWidget* pParent);
    virtual ~rgCheckBox() = default;

protected:
    virtual void focusInEvent(QFocusEvent* pEvent) override;
    virtual void focusOutEvent(QFocusEvent* pEvent) override;
    virtual void mouseMoveEvent(QMouseEvent* pEvent) override;

signals:
    void CheckBoxFocusInEvent();
    void CheckBoxFocusOutEvent();
};

