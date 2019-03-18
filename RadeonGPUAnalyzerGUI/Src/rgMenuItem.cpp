// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuItem.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenu.h>

rgMenuItem::rgMenuItem(rgMenu* pParent) :
    QWidget(pParent),
    m_pParentMenu(pParent)
{
    setAttribute(Qt::WA_LayoutUsesWidgetRect);
}

rgMenu* rgMenuItem::GetParentMenu() const
{
    return m_pParentMenu;
}