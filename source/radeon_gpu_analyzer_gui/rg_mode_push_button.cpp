//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for RGA main window specific implementation of QPushButton for changing modes.
//=============================================================================

// Qt.
#include <QPainter>
#include <QPainterPath>
#include <QPoint>
#include <QStyle>
#include <QWidget>
#include <QKeyEvent>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_mode_push_button.h"

static const int kTextOffsetX = 20;
static const int kTextOffsetY = 14;
static const int kCenterUpArrow = 10;
static const int kCenterDownArrow = 9;
static const int kButtonBaseSize = 18;
static const int kArrowSize = 12;
static const int kArrowXOffset = 13;
static const int kPenWidth = 3;
static const int kFontSize = 11;
static const int kGearIconOffsetY = 4;
static const int kGearIconOffsetX = 5;
static const char* kStrGearIconFile = ":/icons/gear_icon_white.svg";

RgModePushButton::RgModePushButton(QWidget* parent) :
    QPushButton(parent)
{
    // Set default values.
    size_ = kButtonBaseSize;
    color_ = Qt::GlobalColor::gray;
    font_color_ = Qt::GlobalColor::black;
    pen_width_ = kPenWidth;
    font_size_ = kFontSize;

    // Create the vertices.
    CreateVertices();
}

void RgModePushButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // Set up the painter.
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.save();

    // Create the gear icon.
    QIcon gearIcon(kStrGearIconFile);

    // Draw the gear icon.
    QRect rect = this->rect();
    rect.setY(kGearIconOffsetY);
    rect.setX(kGearIconOffsetX);
    style()->drawItemPixmap(&painter, rect, Qt::AlignLeft, gearIcon.pixmap(this->rect().height()  * 0.50));

    // If the widget is hovered over, or gets focus through tabbing, set the color to hover color.
    if (underMouse() || hasFocus())
    {
        color_ = hover_color_;
    }
    else
    {
        color_ = non_hover_color_;
    }
    QPalette pal = palette();
    pal.setColor(QPalette::Button, color_);
    setAutoFillBackground(true);
    setPalette(pal);

    // Draw the text.
    QFont font = this->font();
    font.setPixelSize(font_size_);
    painter.setFont(font);
    QPen pen;
    pen.setColor(font_color_);
    painter.setPen(pen);
    QString textQStr = QString::fromStdString(text_);
    painter.drawText(kTextOffsetX, kTextOffsetY, textQStr);

    // Calculate new points using the current scale factor.
    QPoint scaled_points[kNumberOfVertices];
    for (int i = 0; i < kNumberOfVertices; i++)
    {
        scaled_points[i].setX(vertices_[i].x());
        scaled_points[i].setY(vertices_[i].y());
    }

    // Calculate text width.
    QFontMetrics fm(font);
    int          end_of_text = fm.horizontalAdvance(textQStr) + kTextOffsetX;

    // Position the paint object.
    painter.translate(end_of_text, -kCenterUpArrow);

    // Rotate the paint object to generate an up arrow.
    painter.rotate(180);

    // Also translate the paint object towards the bottom of the rect.
    painter.translate(0, -3);
    painter.translate(-kArrowXOffset, (-size_ - kCenterDownArrow));

    // Create the upward arrow.
    QPolygon polygon;
    polygon << scaled_points[0]
            << scaled_points[1]
            << scaled_points[2]
            << scaled_points[0];

    // Draw filled arrow.
    QPainterPath path;
    path.addPolygon(polygon);
    painter.fillPath(path, QBrush(Qt::white));

    // Restore the painter object.
    painter.restore();

    // End the painter.
    painter.end();
}

void RgModePushButton::keyPressEvent(QKeyEvent* event)
{
    // Process the enter key.
    if ((event->key() == Qt::Key_Enter) || (event->key() == Qt::Key_Return))
    {
        emit clicked(false);
    }
    else
    {
        QPushButton::keyPressEvent(event);
    }
}

void RgModePushButton::CreateVertices()
{
    // Generate the vertices from the size input.
    // Vertex zero is at the top left.
    vertices_[0].setX(kArrowSize *.8);
    vertices_[0].setY(kArrowSize * .6);

    // Vertex 1 is halfway down the bottom side.
    vertices_[1].setX(kArrowSize / 2);
    vertices_[1].setY(kArrowSize * .9);

    // Vertex 2 is the top right.
    vertices_[2].setX(kArrowSize *.2);
    vertices_[2].setY(kArrowSize * .6);
}

void RgModePushButton::SetText(const std::string& text)
{
    text_ = text;

    update();
}

void RgModePushButton::SetColor(const QColor& color)
{
    font_color_ = color;

    update();
}

void RgModePushButton::SetHoverColor(const QColor& hover_color)
{
    hover_color_ = hover_color;
}

void RgModePushButton::SetNonHoverColor(const QColor& non_hover_color)
{
    non_hover_color_ = non_hover_color;
}
