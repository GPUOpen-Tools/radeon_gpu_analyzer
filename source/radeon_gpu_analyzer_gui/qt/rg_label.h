#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_LABEL_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_LABEL_H_

// Qt.
#include <QLabel>

// Infra.
#include "QtCommon/CustomWidgets/ArrowIconWidget.h"

class RgLabel : public QLabel
{
    Q_OBJECT

public:
    RgLabel(QWidget* parent);
    virtual ~RgLabel() = default;

    // Set the highlight string data.
    void SetHighlightSubStringData(QVector<StringHighlightData> string_highlight_data);

    // Set the boolean to highlight the substring.
    void SetHighlightSubString(bool value);

protected:
    // Reimplement focusInEvent.
    virtual void focusInEvent(QFocusEvent* event) override;

    // Reimplement focusOutEvent.
    virtual void focusOutEvent(QFocusEvent* event) override;

    // Reimplement paintEvent.
    virtual void paintEvent(QPaintEvent* event) override;

signals:
    // Signal the focus in event.
    void LabelFocusInEventSignal();

    // Signal the focus out event.
    void LabelFocusOutEventSignal();

private:
    // The boolean to indicate if a string needs to be highlighted.
    bool should_highlight_sub_string_;

    // A vector to store string highlight values.
    QVector<StringHighlightData> string_highlight_data_ = {};
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_LABEL_H_
