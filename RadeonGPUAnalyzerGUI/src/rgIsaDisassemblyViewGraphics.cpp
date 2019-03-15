// C++.
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyViewGraphics.h>

rgIsaDisassemblyViewGraphics::rgIsaDisassemblyViewGraphics(QWidget* pParent)
    : rgIsaDisassemblyView(pParent)
{
}

bool rgIsaDisassemblyViewGraphics::PopulateBuildOutput(const std::shared_ptr<rgProjectClone> pProjectClone, const rgBuildOutputsMap& buildOutputs)
{
    bool ret = false;

    std::shared_ptr<rgGraphicsProjectClone> pPipelineClone = std::dynamic_pointer_cast<rgGraphicsProjectClone>(pProjectClone);
    assert(pPipelineClone != nullptr);
    if (pPipelineClone != nullptr)
    {
        const ShaderInputFileArray& shaderStageArray = pPipelineClone->m_pipeline.m_shaderStages;

        // Build artifacts may contain disassembly for shader files that are no longer associated
        // with any stage in the pipeline. Provide the map of the pipeline's current stages, along
        // with the build artifacts that can be loaded.
        ret = PopulateDisassemblyView(shaderStageArray, buildOutputs);
    }

    return ret;
}

bool rgIsaDisassemblyViewGraphics::PopulateDisassemblyView(const ShaderInputFileArray& shaderStageArray, const rgBuildOutputsMap& buildOutput)
{
    bool isProblemFound = false;

    // Iterate through each target GPU's output.
    for (auto gpuOutputsIter = buildOutput.begin(); gpuOutputsIter != buildOutput.end(); ++gpuOutputsIter)
    {
        const std::string& targetGpu = gpuOutputsIter->first;
        std::shared_ptr<rgCliBuildOutput> pGpuBuildOutput = gpuOutputsIter->second;
        bool isValidOutput = pGpuBuildOutput != nullptr;
        assert(isValidOutput);
        if (isValidOutput)
        {
            std::shared_ptr<rgCliBuildOutputPipeline> pGpuBuildOutputPipeline =
                std::dynamic_pointer_cast<rgCliBuildOutputPipeline>(pGpuBuildOutput);

            // Step through the outputs map, and load the disassembly data for each input file.
            for (auto inputFileOutputsIter = pGpuBuildOutputPipeline->m_perFileOutput.begin(); inputFileOutputsIter != pGpuBuildOutputPipeline->m_perFileOutput.end(); ++inputFileOutputsIter)
            {
                const std::string& inputFilePath = inputFileOutputsIter->first;

                // Only load build outputs for files in the given list of source files.
                auto inputFilePathIter = std::find(shaderStageArray.begin(), shaderStageArray.end(), inputFilePath);
                if (inputFilePathIter != shaderStageArray.end())
                {
                    // Get the list of outputs for the input file.
                    rgFileOutputs& fileOutputs = inputFileOutputsIter->second;
                    const std::vector<rgEntryOutput>& fileEntryOutputs = fileOutputs.m_outputs;

                    // Transform the disassembled entries into a map of GPU -> Entries.
                    // This will make populating the UI much simpler.
                    std::map<std::string, std::vector<rgEntryOutput>> gpuToDisassemblyCsvEntries;
                    std::map<std::string, std::vector<rgEntryOutput>> gpuToResourceUsageCsvEntries;
                    for (const rgEntryOutput& entry : fileOutputs.m_outputs)
                    {
                        for (const rgOutputItem& outputs : entry.m_outputs)
                        {
                            if (outputs.m_fileType == IsaDisassemblyCsv)
                            {
                                std::vector<rgEntryOutput>& disassemblyCsvFilePaths = gpuToDisassemblyCsvEntries[targetGpu];
                                disassemblyCsvFilePaths.push_back(entry);
                            }
                            else if (outputs.m_fileType == HwResourceUsageFile)
                            {
                                std::vector<rgEntryOutput>& entryCsvFilePaths = gpuToResourceUsageCsvEntries[targetGpu];
                                entryCsvFilePaths.push_back(entry);
                            }
                        }
                    }


                    // Load the disassembly CSV entries from the build output.
                    bool isDisassemblyDataLoaded = PopulateDisassemblyEntries(gpuToDisassemblyCsvEntries);
                    assert(isDisassemblyDataLoaded);
                    if (!isDisassemblyDataLoaded)
                    {
                        isProblemFound = true;
                    }

                    if (!isProblemFound)
                    {
                        // Load the resource usage CSV entries from the build output.
                        bool isResourceUsageDataLoaded = PopulateResourceUsageEntries(gpuToResourceUsageCsvEntries);
                        assert(isResourceUsageDataLoaded);
                        if (!isResourceUsageDataLoaded)
                        {
                            isProblemFound = true;
                        }
                    }
                }
            }
        }
    }

    // If the disassembly results loaded correctly, add the target GPU to the dropdown.
    if (!isProblemFound)
    {
        // Populate the target GPU dropdown list with the targets from the build output.
        PopulateTargetGpuList(buildOutput);
    }

    return !isProblemFound;
}