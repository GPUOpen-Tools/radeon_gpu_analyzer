#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BROWSE_BUTTON_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BROWSE_BUTTON_H_

// Qt.
#include <QPushButton>

class RgBrowseButton : public QPushButton
{
    Q_OBJECT

public:
    RgBrowseButton(QWidget* parent);
    virtual ~RgBrowseButton() = default;

protected:
    // Re-implement focusInEvent.
    virtual void focusInEvent(QFocusEvent* event) override;

    // Re-implement focusOutEvent.
    virtual void focusOutEvent(QFocusEvent* event) override;

signals:
    // Signal the focus in event.
    void BrowseButtonFocusInEvent();

    // Signal the focus out event.
    void BrowseButtonFocusOutEvent();
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BROWSE_BUTTON_H_

