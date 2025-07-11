//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for the api mode tree widget.
//=============================================================================
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
    Q_UNUSED(event);

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

                const int column_count = item->columnCount();
                for (int column_number = 0; column_number < column_count; column_number++)
                {
                    item->setBackground(column_number, qApp->palette().color(QPalette::Highlight));
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
    Q_UNUSED(event);

    // Remove highlight from all of the items.
    // Set background color for all items to transparent.
    QTreeWidgetItemIterator it(this);
    while (*it)
    {
        (*it)->setBackground(kTreeWidgetIconColumnId, Qt::GlobalColor::transparent);
        (*it)->setBackground(kTreeWidgetApiColumnId, Qt::GlobalColor::transparent);
        ++it;
    }
}
