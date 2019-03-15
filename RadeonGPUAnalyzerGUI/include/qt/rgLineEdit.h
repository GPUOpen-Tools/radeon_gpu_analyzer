#pragma once

// Qt.
#include <QLineEdit>

// Infra.
#include <QtCommon/CustomWidgets/ArrowIconWidget.h>

class rgLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    rgLineEdit(QWidget* pParent);
    virtual ~rgLineEdit() = default;

    // Set the highlight substring data.
    void SetHighlightSubStringData(QVector<StringHighlightData> stringHighlightData);

    // Indicate if the matching string should be highlighted.
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
    void LineEditFocusInEvent();

    // Signal the focus out event.
    void LineEditFocusOutEvent();

private:
    // The boolean to indicate if a string needs to be highlighted.
    bool m_highlightSubString;

    // A vector to store string highlight values.
    QVector<StringHighlightData> m_stringHighlightData = {};
};

