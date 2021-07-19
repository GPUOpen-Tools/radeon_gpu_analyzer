#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_CHECK_BOX_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_CHECK_BOX_H_

// Qt.
#include <QCheckBox>

class RgCheckBox : public QCheckBox
{
    Q_OBJECT

public:
    RgCheckBox(QWidget* parent);
    virtual ~RgCheckBox() = default;

protected:
    virtual void focusInEvent(QFocusEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* Event) override;

signals:
    void CheckBoxFocusInEvent();
    void CheckBoxFocusOutEvent();
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_CHECK_BOX_H_

