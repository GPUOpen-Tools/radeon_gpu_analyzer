// C++.
#include <cassert>

// Qt.
#include <QTreeWidget>
#include <QMouseEvent>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgTreeWidget.h>

static const int s_TREE_WIDGET_ICON_COLUMN_ID = 0;
static const int s_TREE_WIDGET_API_COLUMN_ID = 1;

rgTreeWidget::rgTreeWidget(QWidget* pParent) :
    QTreeWidget(pParent)
{
    setMouseTracking(true);
}

void rgTreeWidget::focusOutEvent(QFocusEvent *event)
{
    hide();
}

void rgTreeWidget::mouseMoveEvent(QMouseEvent* pEvent)
{
    setCursor(Qt::ArrowCursor);
    if (pEvent != nullptr)
    {
        const QPoint pos = pEvent->pos();

        QTreeWidgetItem* pItem = itemAt(pos);
        if (pItem != nullptr)
        {
            const int row = indexOfTopLevelItem(pItem);
            if (row != 0)
            {
                setCursor(Qt::PointingHandCursor);

                QColor lightBlue(229, 243, 255);
                const int columnCount = pItem->columnCount();
                for (int columnNumber = 0; columnNumber < columnCount; columnNumber++)
                {
                    pItem->setBackgroundColor(columnNumber, lightBlue);
                }

                // Pass the event onto the base class.
                QTreeWidget::mouseMoveEvent(pEvent);
            }
        }
    }
    else
    {
        // Pass the event onto the base class.
        QTreeWidget::mouseMoveEvent(pEvent);
    }
}

void rgTreeWidget::leaveEvent(QEvent* pEvent)
{
    // Remove highlight from all of the items.
    // Set background color for all items to transparent.
    QTreeWidgetItemIterator it(this);
    while (*it)
    {
        (*it)->setBackgroundColor(s_TREE_WIDGET_ICON_COLUMN_ID, Qt::GlobalColor::transparent);
        (*it)->setBackgroundColor(s_TREE_WIDGET_API_COLUMN_ID, Qt::GlobalColor::transparent);
        ++it;
    }
}