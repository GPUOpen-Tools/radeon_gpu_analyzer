#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SCROLL_AREA_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SCROLL_AREA_H_

// Qt.
#include <QScrollArea>

class RgScrollArea : public QScrollArea
{
    Q_OBJECT

public:
    RgScrollArea(QWidget* parent);
    virtual ~RgScrollArea() = default;

protected:
    // Re-implement the mousePressEvent.
    virtual void mousePressEvent(QMouseEvent* event) override;

signals:
    void ScrollAreaClickedEvent();
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SCROLL_AREA_H_

