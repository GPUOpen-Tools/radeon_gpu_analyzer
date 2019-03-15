// Qt.
#include <QEvent>
#include <QKeyEvent>
#include <QApplication>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgHandleTabFocusEventFilter.h>

rgHandleTabFocusEventFilter& rgHandleTabFocusEventFilter::Get()
{
    static rgHandleTabFocusEventFilter instance;
    return instance;
}

bool rgHandleTabFocusEventFilter::eventFilter(QObject *pObject, QEvent *pEvent)
{
    bool ret = false;

    // Intercept tab key presses to handle focus change.
    if (pEvent->type() == QEvent::KeyPress)
    {
        QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(pEvent);
        Qt::KeyboardModifiers keyboardModifiers = QApplication::keyboardModifiers();
        if ((keyboardModifiers & Qt::ShiftModifier) && (pKeyEvent->key() == Qt::Key_Tab))
        {
            emit ShiftTabPressed();
            ret = true;
        }
        else if ((pKeyEvent->key() == Qt::Key_Tab))
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