#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyView.h>

// A class responsible for displaying ISA code for graphics or compute pipelines.
class rgIsaDisassemblyViewGraphics : public rgIsaDisassemblyView
{
    Q_OBJECT

public:
    explicit rgIsaDisassemblyViewGraphics(QWidget* pParent = nullptr);
    virtual ~rgIsaDisassemblyViewGraphics() = default;

    // Populate the disassembly view using the given clone and build outputs.
    virtual bool PopulateBuildOutput(const std::shared_ptr<rgProjectClone> pProjectClone, const rgBuildOutputsMap& buildOutputs) override;

private:
    bool PopulateDisassemblyView(const ShaderInputFileArray& shaderStageArray, const rgBuildOutputsMap& buildOutput);
};