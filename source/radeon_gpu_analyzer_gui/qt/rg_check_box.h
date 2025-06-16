//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for RGA specific implementation of QCheckBox.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_CHECK_BOX_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_CHECK_BOX_H_

// Infra.
#include "qt_common/custom_widgets/scaled_check_box.h"

class RgCheckBox : public ScaledCheckBox
{
    Q_OBJECT

public:
    RgCheckBox(QWidget* parent);
    virtual ~RgCheckBox() = default;

protected:
    virtual void focusInEvent(QFocusEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;

signals:
    void CheckBoxFocusInEvent();
    void CheckBoxFocusOutEvent();
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_CHECK_BOX_H_

