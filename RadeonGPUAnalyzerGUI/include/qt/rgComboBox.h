#pragma once

// Qt.
#include <QComboBox>

class rgComboBox : public QComboBox
{
    Q_OBJECT

public:
    rgComboBox(QWidget* pParent);
    virtual ~rgComboBox() = default;

protected:
    virtual void mousePressEvent(QMouseEvent* pEvent) override;

signals:
    void ComboBoxFocusInEvent();
};

