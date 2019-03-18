// C++.
#include <cassert>

// Qt.
#include <QListWidget>
#include <QMouseEvent>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgListWidget.h>

rgListWidget::rgListWidget(QWidget* pParent) :
    QListWidget(pParent)
{
    setMouseTracking(true);
}

void rgListWidget::mouseMoveEvent(QMouseEvent* pEvent)
{
    setCursor(Qt::ArrowCursor);
    if (pEvent != nullptr)
    {
        const QPoint pos = pEvent->pos();

        const QModelIndex modelIndex = indexAt(pos);
        if (modelIndex.isValid())
        {
            const int hoveredRow = modelIndex.row();
            const int currentItemRow = currentRow();
            if (hoveredRow != currentItemRow)
            {
                setCursor(Qt::PointingHandCursor);

                // Pass the event onto the base class.
                QListWidget::mouseMoveEvent(pEvent);
            }
        }
    }
    else
    {
        // Pass the event onto the base class.
        QListWidget::mouseMoveEvent(pEvent);
    }
}