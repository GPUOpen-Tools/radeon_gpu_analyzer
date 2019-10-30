#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyView.h>

// A class responsible for displaying ISA code for multiple GPU architectures.
class rgIsaDisassemblyViewOpenCL : public rgIsaDisassemblyView
{
    Q_OBJECT

public:
    rgIsaDisassemblyViewOpenCL(QWidget* pParent);
    virtual ~rgIsaDisassemblyViewOpenCL() = default;

    // Populate the disassembly view using the given clone and build outputs.
    virtual bool PopulateBuildOutput(const std::shared_ptr<rgProjectClone> pProjectClone, const rgBuildOutputsMap& buildOutputs) override;

protected:
    // Populate the disassembly view with the given CLI build output.
    bool PopulateDisassemblyView(const std::vector<rgSourceFileInfo>& sourceFiles, const rgBuildOutputsMap& buildOutput);
};