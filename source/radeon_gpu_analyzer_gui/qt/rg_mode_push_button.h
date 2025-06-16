//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for RGA main window specific implementation of QPushButton for changing modes.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MODE_PUSH_BUTTON_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MODE_PUSH_BUTTON_H_

// Qt.
#include <QPushButton>

class Qwidget;

class RgModePushButton : public QPushButton
{
    Q_OBJECT

public:
    explicit RgModePushButton(QWidget* parent = nullptr);
    virtual ~RgModePushButton() {}
    void SetText(const std::string& string);
    void SetColor(const QColor& color);
    void SetHoverColor(const QColor& hover_color);
    void SetNonHoverColor(const QColor& non_hover_color);

protected:
    // Re-implement paintEvent method.
    virtual void paintEvent(QPaintEvent *event) override;

    // Re-implement key press event method.
    virtual void keyPressEvent(QKeyEvent* event) override;

    // Create vertices for the up arrow.
    void CreateVertices();

    // The number of vertices for the up arrow.
    static const int kNumberOfVertices = 3;

    // The vertices for the up arrow.
    QPointF vertices_[kNumberOfVertices];

    // The color of the arrow.
    QColor color_;

    // The font color for the button text.
    QColor font_color_;

    // The color to use when the user hovers over the button.
    QColor hover_color_;

    // The color to use when the user is not hovering over the button.
    QColor non_hover_color_;

    // The text to display on the button.
    std::string text_;

    // The width of the pen to use for the text.
    int pen_width_;

    // The size of the button.
    int size_;

    // The size of the text font.
    int font_size_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MODE_PUSH_BUTTON_H_
