// C++.
#include <sstream>
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgScrollArea.h>

rgScrollArea::rgScrollArea(QWidget* pParent) : QScrollArea(pParent)
{
}

void rgScrollArea::mousePressEvent(QMouseEvent* pEvent)
{
    emit ScrollAreaClickedEvent();

    // Pass the event onto the base class.
    QScrollArea::mousePressEvent(pEvent);
}