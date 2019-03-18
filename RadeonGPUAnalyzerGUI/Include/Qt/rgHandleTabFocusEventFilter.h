#pragma once

// Qt.
#include <QObject>

class QEvent;

// This class is used as a memberless filter for qt events, and as such it
// only needs to exist once.
class rgHandleTabFocusEventFilter : public QObject
{
    Q_OBJECT

public:
    // Singleton get function.
    static rgHandleTabFocusEventFilter& Get();

signals:
    // A signal emitted when the user presses the tab key.
    void TabPressed();

    // A signal emitted when the user presses the shift+tab keys.
    void ShiftTabPressed();

private:
    rgHandleTabFocusEventFilter(QObject* pParent = nullptr) : QObject(pParent) {}
    ~rgHandleTabFocusEventFilter() = default;

protected:
    // Event filtering function.
    virtual bool eventFilter(QObject *pObject, QEvent *pEvent) override;
};