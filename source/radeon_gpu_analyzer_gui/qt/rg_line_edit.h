#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_LINE_EDIT_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_LINE_EDIT_H_

// Qt.
#include <QLineEdit>

// Infra.
#include "QtCommon/CustomWidgets/ArrowIconWidget.h"

class RgLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    RgLineEdit(QWidget* parent);
    virtual ~RgLineEdit() = default;

    // Set the highlight substring data.
    void SetHighlightSubStringData(QVector<StringHighlightData> string_highlight_data);

    // Indicate if the matching string should be highlighted.
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
    void LineEditFocusInEvent();

    // Signal the focus out event.
    void LineEditFocusOutEvent();

private:
    // The boolean to indicate if a string needs to be highlighted.
    bool highlight_sub_string_;

    // A vector to store string highlight values.
    QVector<StringHighlightData> string_highlight_data_ = {};
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_LINE_EDIT_H_
