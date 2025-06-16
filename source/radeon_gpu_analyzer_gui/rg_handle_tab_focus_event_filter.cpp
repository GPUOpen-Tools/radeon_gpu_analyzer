//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for RgHandleTabFocusEventFilter class.
//=============================================================================

// Qt.
#include <QEvent>
#include <QKeyEvent>
#include <QApplication>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_handle_tab_focus_event_filter.h"

RgHandleTabFocusEventFilter& RgHandleTabFocusEventFilter::Get()
{
    static RgHandleTabFocusEventFilter instance;
    return instance;
}

bool RgHandleTabFocusEventFilter::eventFilter(QObject *object, QEvent *event)
{
    bool ret = false;

    // Intercept tab key presses to handle focus change.
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
        Qt::KeyboardModifiers keyboard_modifiers = QApplication::keyboardModifiers();
        if ((keyboard_modifiers & Qt::ShiftModifier) && (key_event->key() == Qt::Key_Tab))
        {
            emit ShiftTabPressed();
            ret = true;
        }
        else if ((key_event->key() == Qt::Key_Tab))
        {
            emit TabPressed();
            ret = true;
        }
    }

    // Default event processing.
    if (!ret)
    {
        ret = QObject::eventFilter(object, event);
    }

    return ret;
}
