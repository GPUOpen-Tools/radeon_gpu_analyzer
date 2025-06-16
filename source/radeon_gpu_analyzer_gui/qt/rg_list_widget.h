//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for RGA specific implementation of QLabel.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_LIST_WIDGET_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_LIST_WIDGET_H_

// Qt.
#include <QListWidget>

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_tree_widget.h"

class RgListWidget : public QListWidget
{
    Q_OBJECT

public:
    RgListWidget(QWidget* parent = nullptr);
    virtual ~RgListWidget() = default;

protected:
    // Re-implement the mouseMoveEvent method.
    virtual void mouseMoveEvent(QMouseEvent* event) override;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_LIST_WIDGET_H_
