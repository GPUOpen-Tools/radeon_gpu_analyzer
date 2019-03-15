#pragma once

// Qt.
#include <QPushButton>

class Qwidget;

class rgModePushButton : public QPushButton
{
    Q_OBJECT

public:
    explicit rgModePushButton(QWidget* pParent = nullptr);
    virtual ~rgModePushButton() {}
    void SetText(const std::string& string);
    void SetColor(const QColor& color);
    void SetHoverColor(const QColor& hoverColor);
    void SetNonHoverColor(const QColor& nonHoverColor);

protected:
    // Re-implement paintEvent method.
    virtual void paintEvent(QPaintEvent *pEvent) override;

    // Re-implement key press event method.
    virtual void keyPressEvent(QKeyEvent* pEvent) override;

    // Create vertices for the up arrow.
    void CreateVertices();

    // The number of vertices for the up arrow.
    static const int s_NUMBER_OF_VERTICES = 3;

    // The vertices for the up arrow.
    QPointF m_vertices[s_NUMBER_OF_VERTICES];

    // The color of the arrow.
    QColor m_color;

    // The font color for the button text.
    QColor m_fontColor;

    // The color to use when the user hovers over the button.
    QColor m_hoverColor;

    // The color to use when the user is not hovering over the button.
    QColor m_nonHoverColor;

    // The text to display on the button.
    std::string m_text;

    // The width of the pen to use for the text.
    int m_penWidth;

    // The size of the button.
    int m_size;

    // The size of the text font.
    int m_fontSize;
};