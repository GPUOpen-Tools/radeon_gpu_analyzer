#pragma once

// Qt.
#include <QLineEdit>

class rgLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    rgLineEdit(QWidget* pParent);
    virtual ~rgLineEdit() = default;

protected:
    virtual void focusInEvent(QFocusEvent* pEvent) override;
    virtual void focusOutEvent(QFocusEvent* pEvent) override;

signals:
    void LineEditFocusInEvent();
    void LineEditFocusOutEvent();
};

