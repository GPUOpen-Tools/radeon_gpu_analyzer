// C++.
#include <cassert>

// Qt.
#include <QListWidget>
#include <QMouseEvent>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_list_widget.h"

RgListWidget::RgListWidget(QWidget* parent) :
    QListWidget(parent)
{
    setMouseTracking(true);
}

void RgListWidget::mouseMoveEvent(QMouseEvent* event)
{
    setCursor(Qt::ArrowCursor);
    if (event != nullptr)
    {
        const QPoint pos = event->pos();

        const QModelIndex model_index = indexAt(pos);
        if (model_index.isValid())
        {
            const int hovered_row = model_index.row();
            const int current_item_row = currentRow();
            if (hovered_row != current_item_row)
            {
                setCursor(Qt::PointingHandCursor);

                // Pass the event onto the base class.
                QListWidget::mouseMoveEvent(event);
            }
        }
    }
    else
    {
        // Pass the event onto the base class.
        QListWidget::mouseMoveEvent(event);
    }
}
