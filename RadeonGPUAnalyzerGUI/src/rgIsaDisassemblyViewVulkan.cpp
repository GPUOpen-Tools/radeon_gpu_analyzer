// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyTabView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyViewVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>

rgIsaDisassemblyViewVulkan::rgIsaDisassemblyViewVulkan(QWidget* pParent)
    : rgIsaDisassemblyViewGraphics(pParent)
{
}

void rgIsaDisassemblyViewVulkan::SetBorderStylesheet()
{
    ui.frame->setStyleSheet(STR_DISASSEMBLY_FRAME_BORDER_RED_STYLESHEET);
}