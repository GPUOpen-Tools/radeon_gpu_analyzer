// Qt.
#include <QEvent>
#include <QKeyEvent>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgIgnoreTabFocusEventFilter.h>

rgIgnoreTabFocusEventFilter& rgIgnoreTabFocusEventFilter::Get()
{
    static rgIgnoreTabFocusEventFilter instance;
    return instance;
}

bool rgIgnoreTabFocusEventFilter::eventFilter(QObject *pObject, QEvent *pEvent)
{
    bool ret = false;

    // Intercept tab key presses to ignore focus change.
    if (pEvent->type() == QEvent::KeyPress)
    {
        QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(pEvent);
        if (pKeyEvent->key() == Qt::Key_Tab)
        {
            emit TabPressed();
            ret = true;
        }
    }

    // Default event processing.
    if (!ret)
    {
        ret = QObject::eventFilter(pObject, pEvent);
    }

    return ret;
}