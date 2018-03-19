// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgFileMenuItem.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgFileMenu.h>

rgFileMenuItem::rgFileMenuItem(rgFileMenu* pParent) :
    QWidget(pParent),
    m_pParentMenu(pParent)
{
    setAttribute(Qt::WA_LayoutUsesWidgetRect);
}

rgFileMenu* rgFileMenuItem::GetParentMenu() const
{
    return m_pParentMenu;
}