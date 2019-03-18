#pragma once

// Qt.
#include <QWidget>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyViewGraphics.h>

// A class responsible for displaying ISA code for multiple GPU architectures.
class rgIsaDisassemblyViewVulkan : public rgIsaDisassemblyViewGraphics
{
    Q_OBJECT

public:
    rgIsaDisassemblyViewVulkan(QWidget* pParent);
    virtual ~rgIsaDisassemblyViewVulkan() = default;

protected:
    // Set the border stylesheet.
    virtual void SetBorderStylesheet() override;
};