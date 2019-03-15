#pragma once

// Qt.
#include <QLabel>

// Infra.
#include <QtCommon/CustomWidgets/ArrowIconWidget.h>

class rgLabel : public QLabel
{
    Q_OBJECT

public:
    rgLabel(QWidget* pParent);
    virtual ~rgLabel() = default;

    // Set the highlight string data.
    void SetHighlightSubStringData(QVector<StringHighlightData> stringHighlightData);

    // Set the boolean to highlight the substring.
    void SetHighlightSubString(bool value);

protected:
    // Reimplement focusInEvent.
    virtual void focusInEvent(QFocusEvent* pEvent) override;

    // Reimplement focusOutEvent.
    virtual void focusOutEvent(QFocusEvent* pEvent) override;

    // Reimplement paintEvent.
    virtual void paintEvent(QPaintEvent* pEvent) override;

signals:
    // Signal the focus in event.
    void LabelFocusInEventSignal();

    // Signal the focus out event.
    void LabelFocusOutEventSignal();

private:
    // The boolean to indicate if a string needs to be highlighted.
    bool m_highlightSubString;

    // A vector to store string highlight values.
    QVector<StringHighlightData> m_stringHighlightData = {};
};
