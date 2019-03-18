#pragma once

// Qt.
#include <QScrollArea>

class rgScrollArea : public QScrollArea
{
    Q_OBJECT

public:
    rgScrollArea(QWidget* pParent);
    virtual ~rgScrollArea() = default;

protected:
    // Re-implement the mousePressEvent.
    virtual void mousePressEvent(QMouseEvent* pEvent) override;

signals:
    void ScrollAreaClickedEvent();
};

