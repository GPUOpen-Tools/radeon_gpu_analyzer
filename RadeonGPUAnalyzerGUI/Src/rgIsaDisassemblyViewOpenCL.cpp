// C++.
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyViewOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>

rgIsaDisassemblyViewOpenCL::rgIsaDisassemblyViewOpenCL(QWidget* pParent)
    : rgIsaDisassemblyView(pParent)
{
}

bool rgIsaDisassemblyViewOpenCL::PopulateBuildOutput(const std::shared_ptr<rgProjectClone> pProjectClone, const rgBuildOutputsMap& buildOutputs)
{
    bool ret = false;

    assert(pProjectClone != nullptr);
    if (pProjectClone != nullptr)
    {
        std::vector<rgSourceFileInfo>& projectSourceFiles = pProjectClone->m_sourceFiles;

        // Build artifacts may contain disassembly for source files that are no longer
        // in the project, so provide a list of files to load, along with the current build output.
        ret = PopulateDisassemblyView(projectSourceFiles, buildOutputs);
    }

    return ret;
}

void rgIsaDisassemblyViewOpenCL::SetBorderStylesheet()
{
    ui.frame->setStyleSheet(STR_DISASSEMBLY_FRAME_BORDER_GREEN_STYLESHEET);
}

bool rgIsaDisassemblyViewOpenCL::PopulateDisassemblyView(const std::vector<rgSourceFileInfo>& sourceFiles, const rgBuildOutputsMap& buildOutput)
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
            std::shared_ptr<rgCliBuildOutputOpenCL> pGpuBuildOutputOpenCL =
                std::dynamic_pointer_cast<rgCliBuildOutputOpenCL>(pGpuBuildOutput);

            // Step through the outputs map, and load the disassembly data for each input file.
            for (auto outputIter = pGpuBuildOutputOpenCL->m_perFileOutput.begin(); outputIter != pGpuBuildOutputOpenCL->m_perFileOutput.end(); ++outputIter)
            {
                const std::string& sourceFilePath = outputIter->first;

                // Only load build outputs for files in the given list of source files.
                rgSourceFilePathSearcher sourceFileSearcher(sourceFilePath);
                auto sourceFileIter = std::find_if(sourceFiles.begin(), sourceFiles.end(), sourceFileSearcher);
                if (sourceFileIter != sourceFiles.end())
                {
                    // Get the list of outputs for the input file.
                    rgFileOutputs& fileOutputs = outputIter->second;
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