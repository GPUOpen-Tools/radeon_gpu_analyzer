//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for RgHandleTabFocusEventFilter class.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_HANDLE_TAB_FOCUS_EVENT_FILTER_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_HANDLE_TAB_FOCUS_EVENT_FILTER_H_

// Qt.
#include <QObject>

class QEvent;

// This class is used as a memberless filter for qt events, and as such it
// only needs to exist once.
class RgHandleTabFocusEventFilter : public QObject
{
    Q_OBJECT

public:
    // Singleton get function.
    static RgHandleTabFocusEventFilter& Get();

signals:
    // A signal emitted when the user presses the tab key.
    void TabPressed();

    // A signal emitted when the user presses the shift+tab keys.
    void ShiftTabPressed();

private:
    RgHandleTabFocusEventFilter(QObject* parent = nullptr) : QObject(parent) {}
    ~RgHandleTabFocusEventFilter() = default;

protected:
    // Event filtering function.
    virtual bool eventFilter(QObject* object, QEvent* event) override;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_HANDLE_TAB_FOCUS_EVENT_FILTER_H_
