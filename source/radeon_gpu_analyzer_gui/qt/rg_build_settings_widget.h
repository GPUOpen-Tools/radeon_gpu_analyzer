//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for Build settings widget.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_SETTINGS_WIDGET_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_SETTINGS_WIDGET_H_

// Qt.
#include <QFrame>

class RgBuildSettingsWidget : public QFrame
{
    Q_OBJECT

public:
    RgBuildSettingsWidget(QWidget* parent);
    virtual ~RgBuildSettingsWidget() = default;

protected:
    virtual void focusInEvent(QFocusEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;

signals:
    void FrameFocusInEventSignal();
    void FrameFocusOutEventSignal();
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_SETTINGS_WIDGET_H_

