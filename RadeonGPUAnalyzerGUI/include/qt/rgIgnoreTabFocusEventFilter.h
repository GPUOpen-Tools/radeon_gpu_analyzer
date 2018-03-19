#pragma once

// Qt.
#include <QObject>

class QEvent;

// This class is used as a memberless filter for qt events, and as such it
// only needs to exist once.
class rgIgnoreTabFocusEventFilter : public QObject
{
    Q_OBJECT

public:
    // Singleton get function.
    static rgIgnoreTabFocusEventFilter& Get();

signals:
    // A signal emitted when the user presses the tab key.
    void TabPressed();

private:
    rgIgnoreTabFocusEventFilter(QObject* pParent = nullptr) : QObject(pParent) {}
    ~rgIgnoreTabFocusEventFilter() = default;

protected:
    // Event filtering function.
    virtual bool eventFilter(QObject *pObject, QEvent *pEvent);
};