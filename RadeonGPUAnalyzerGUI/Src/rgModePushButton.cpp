// Qt.
#include <QPainter>
#include <QPainterPath>
#include <QPoint>
#include <QStyle>
#include <QWidget>
#include <QKeyEvent>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgModePushButton.h>

static const int s_TEXT_OFFSET_X = 20;
static const int s_TEXT_OFFSET_Y = 14;
static const int s_CENTER_UP_ARROW = 10;
static const int s_CENTER_DOWN_ARROW = 9;
static const int s_BUTTON_BASE_SIZE = 18;
static const int s_ARROW_SIZE = 12;
static const int s_ARROW_X_OFFSET = 13;
static const int s_PEN_WIDTH = 3;
static const int s_FONT_SIZE = 11;
static const int s_GEAR_ICON_OFFSET_Y = 4;
static const int s_GEAR_ICON_OFFSET_X = 5;
static const char* s_GEAR_ICON_FILE = ":/icons/gearIconWhite.svg";

rgModePushButton::rgModePushButton(QWidget* pParent) :
    QPushButton(pParent)
{
    // Set default values.
    m_size = s_BUTTON_BASE_SIZE;
    m_color = Qt::GlobalColor::gray;
    m_fontColor = Qt::GlobalColor::black;
    m_penWidth = s_PEN_WIDTH;
    m_fontSize = s_FONT_SIZE;

    // Create the vertices.
    CreateVertices();
}

void rgModePushButton::paintEvent(QPaintEvent *pEvent)
{
    Q_UNUSED(pEvent);

    // Set up the painter.
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.save();

    // Create the gear icon.
    QIcon gearIcon(s_GEAR_ICON_FILE);

    // Draw the gear icon.
    QRect rect = this->rect();
    rect.setY(s_GEAR_ICON_OFFSET_Y);
    rect.setX(s_GEAR_ICON_OFFSET_X);
    style()->drawItemPixmap(&painter, rect, Qt::AlignLeft, gearIcon.pixmap(this->rect().height()  * 0.50));

    // If the widget is hovered over, or gets focus through tabbing, set the color to hover color.
    if (underMouse() || hasFocus())
    {
        m_color = m_hoverColor;
    }
    else
    {
        m_color = m_nonHoverColor;
    }
    QPalette pal = palette();
    pal.setColor(QPalette::Button, m_color);
    setAutoFillBackground(true);
    setPalette(pal);

    // Get the scaling factor.
    const double scalingFactor = ScalingManager::Get().GetScaleFactor();

    // Draw the text.
    QFont font = this->font();
    font.setPixelSize(m_fontSize * scalingFactor);
    painter.setFont(font);
    QPen pen;
    pen.setColor(m_fontColor);
    painter.setPen(pen);
    QString textQStr = QString::fromStdString(m_text);
    painter.drawText(s_TEXT_OFFSET_X * scalingFactor, s_TEXT_OFFSET_Y * scalingFactor, textQStr);

    // Calculate new points using the current scale factor.
    QPoint scaledPoints[s_NUMBER_OF_VERTICES];
    for (int i = 0; i < s_NUMBER_OF_VERTICES; i++)
    {
        scaledPoints[i].setX(m_vertices[i].x() * scalingFactor);
        scaledPoints[i].setY(m_vertices[i].y() * scalingFactor);
    }

    // Calculate text width.
    QFontMetrics fm(font);
    int endOfText = fm.width(textQStr) + s_TEXT_OFFSET_X * scalingFactor;

    // Position the paint object.
    painter.translate(endOfText, -s_CENTER_UP_ARROW * scalingFactor);

    // Rotate the paint object to generate an up arrow.
    painter.rotate(180);

    // Also translate the paint object towards the bottom of the rect.
    painter.translate(0, -3);
    painter.translate(-s_ARROW_X_OFFSET * scalingFactor, (-m_size - s_CENTER_DOWN_ARROW) * scalingFactor);

    // Create the upward arrow.
    QPolygon polygon;
    polygon << scaledPoints[0]
            << scaledPoints[1]
            << scaledPoints[2]
            << scaledPoints[0];

    // Draw filled arrow.
    QPainterPath path;
    path.addPolygon(polygon);
    painter.fillPath(path, QBrush(Qt::white));

    // Restore the painter object.
    painter.restore();

    // End the painter.
    painter.end();
}

void rgModePushButton::keyPressEvent(QKeyEvent* pEvent)
{
    // Process the enter key.
    if ((pEvent->key() == Qt::Key_Enter) || (pEvent->key() == Qt::Key_Return))
    {
        emit clicked(false);
    }
    else
    {
        QPushButton::keyPressEvent(pEvent);
    }
}

void rgModePushButton::CreateVertices()
{
    // Generate the vertices from the size input.
    // Vertex zero is at the top left.
    m_vertices[0].setX(s_ARROW_SIZE *.8);
    m_vertices[0].setY(s_ARROW_SIZE * .6);

    // Vertex 1 is halfway down the bottom side.
    m_vertices[1].setX(s_ARROW_SIZE / 2);
    m_vertices[1].setY(s_ARROW_SIZE * .9);

    // Vertex 2 is the top right.
    m_vertices[2].setX(s_ARROW_SIZE *.2);
    m_vertices[2].setY(s_ARROW_SIZE * .6);
}

void rgModePushButton::SetText(const std::string& text)
{
    m_text = text;

    update();
}

void rgModePushButton::SetColor(const QColor& color)
{
    m_fontColor = color;

    update();
}

void rgModePushButton::SetHoverColor(const QColor& hoverColor)
{
    m_hoverColor = hoverColor;
}

void rgModePushButton::SetNonHoverColor(const QColor& nonHoverColor)
{
    m_nonHoverColor = nonHoverColor;
}
