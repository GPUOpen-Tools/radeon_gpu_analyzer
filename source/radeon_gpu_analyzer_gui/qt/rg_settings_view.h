//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for the settings view.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SETTINGS_VIEW_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SETTINGS_VIEW_H_

// Qt.
#include <QWidget>

class RgSettingsView : public QWidget
{
    Q_OBJECT

public:
    RgSettingsView(QWidget* parent);
    virtual ~RgSettingsView() = default;

    // Set the focus to target selection button.
    virtual void SetInitialWidgetFocus() = 0;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SETTINGS_VIEW_H_
