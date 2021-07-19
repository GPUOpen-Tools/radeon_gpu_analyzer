// C++.
#include <cassert>

// Qt.
#include <QTreeWidget>
#include <QMouseEvent>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_tree_widget.h"

static const int kTreeWidgetIconColumnId = 0;
static const int kTreeWidgetApiColumnId = 1;

RgTreeWidget::RgTreeWidget(QWidget* parent) :
    QTreeWidget(parent)
{
    setMouseTracking(true);
}

void RgTreeWidget::focusOutEvent(QFocusEvent *event)
{
    hide();
}

void RgTreeWidget::mouseMoveEvent(QMouseEvent* event)
{
    setCursor(Qt::ArrowCursor);
    if (event != nullptr)
    {
        const QPoint pos = event->pos();

        QTreeWidgetItem* item = itemAt(pos);
        if (item != nullptr)
        {
            const int row = indexOfTopLevelItem(item);
            if (row != 0)
            {
                setCursor(Qt::PointingHandCursor);

                QColor light_blue(229, 243, 255);
                const int column_count = item->columnCount();
                for (int column_number = 0; column_number < column_count; column_number++)
                {
                    item->setBackgroundColor(column_number, light_blue);
                }

                // Pass the event onto the base class.
                QTreeWidget::mouseMoveEvent(event);
            }
        }
    }
    else
    {
        // Pass the event onto the base class.
        QTreeWidget::mouseMoveEvent(event);
    }
}

void RgTreeWidget::leaveEvent(QEvent* event)
{
    // Remove highlight from all of the items.
    // Set background color for all items to transparent.
    QTreeWidgetItemIterator it(this);
    while (*it)
    {
        (*it)->setBackgroundColor(kTreeWidgetIconColumnId, Qt::GlobalColor::transparent);
        (*it)->setBackgroundColor(kTreeWidgetApiColumnId, Qt::GlobalColor::transparent);
        ++it;
    }
}
