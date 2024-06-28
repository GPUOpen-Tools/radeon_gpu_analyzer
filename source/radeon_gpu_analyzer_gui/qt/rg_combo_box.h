#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_COMBO_BOX_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_COMBO_BOX_H_

// Infra.
#include "qt_common/custom_widgets/scaled_combo_box.h"

class RgComboBox : public QComboBox
{
    Q_OBJECT

public:
    RgComboBox(QWidget* parent);
    virtual ~RgComboBox() = default;

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;

signals:
    void ComboBoxFocusInEvent();
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_COMBO_BOX_H_

