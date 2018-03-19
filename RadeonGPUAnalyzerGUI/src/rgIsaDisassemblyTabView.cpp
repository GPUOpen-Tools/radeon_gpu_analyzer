// C++.
#include <cassert>
#include <sstream>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgIsaDisassemblyTabView.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgIsaDisassemblyTableView.h>
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/include/rgDefinitions.h>

rgIsaDisassemblyTabView::rgIsaDisassemblyTabView(QWidget* pParent) :
    QWidget(pParent)
{
    ui.setupUi(this);
}

void rgIsaDisassemblyTabView::ClearCorrelationHighlight()
{
    if (m_pCurrentTable != nullptr)
    {
        // Passing an invalid line index to the table model will clear the correlated line.
        m_pCurrentTable->UpdateCorrelatedSourceFileLine(kInvalidCorrelationLineIndex);
    }
}

bool rgIsaDisassemblyTabView::GetCurrentEntrypoint(std::string& currentEntrypoint) const
{
    bool ret = false;

    auto tableIter = m_disassemblyTableViewToEntrypoint.find(m_pCurrentTable);
    if (tableIter != m_disassemblyTableViewToEntrypoint.end())
    {
        currentEntrypoint = tableIter->second;
        ret = true;
    }

    return ret;
}

int rgIsaDisassemblyTabView::GetTableCount() const
{
    return static_cast<int>(m_entrypointDisassemblyTableViews.size());
}

bool rgIsaDisassemblyTabView::PopulateEntries(const std::vector<rgEntryOutput>& disassembledEntries)
{
    bool ret = true;

    // Create a disassembly table for each entry, and add it to the layout.
    // Only a single entrypoint table will be visible at a time, and the user can switch between the current entry.
    for (const rgEntryOutput& entry : disassembledEntries)
    {
        OutputFileTypeFinder outputFileTypeSearcher(rgCliOutputFileType::IsaDisassemblyCsv);
        auto csvFileIter = std::find_if(entry.m_outputs.begin(), entry.m_outputs.end(), outputFileTypeSearcher);
        if (csvFileIter != entry.m_outputs.end())
        {
            // Create a new disassembly table view for each entry point.
            rgIsaDisassemblyTableView* pTableView = new rgIsaDisassemblyTableView(this);

            // Get the path to the disassembly CSV file to load into the table.
            const std::string& disassemblyCsvFilePath = csvFileIter->m_filePath;

            // Set the file path for the table.
            pTableView->SetDisassemblyFilePath(disassemblyCsvFilePath);

            // Connect signals for the new table view.
            ConnectTableViewSignals(pTableView);

            // Add the table to the tab's grid.
            ui.tableGrid->addWidget(pTableView);

            // Register the table with the scaling manager.
            ScalingManager::Get().RegisterObject(pTableView);

            // Hide the table initially, as only a single table can be shown at once.
            pTableView->hide();

            // Associate the kernel name with the table showing the disassembly.
            m_entrypointDisassemblyTableViews[entry.m_kernelName] = pTableView;

            // Also associate the disassembly table with the entrypoint it's displaying data for.
            m_disassemblyTableViewToEntrypoint[pTableView] = entry.m_kernelName;

            // Add the new table to the list of tables associated with the input file.
            std::vector<rgIsaDisassemblyTableView*>& tableList = m_inputFileToIsaTableList[entry.m_inputFilePath];
            tableList.push_back(pTableView);
        }
    }

    return ret;
}

void rgIsaDisassemblyTabView::RemoveInputFileEntries(const std::string& inputFilePath)
{
    auto resultsIter = m_inputFileToIsaTableList.find(inputFilePath);
    if (resultsIter != m_inputFileToIsaTableList.end())
    {
        // We found a list of table views that are used to display disassembly for the given input file.
        std::vector<rgIsaDisassemblyTableView*>& entryTables = resultsIter->second;

        // Step through each table and destroy it.
        for (rgIsaDisassemblyTableView* pTableView : entryTables)
        {
            // Remove the references to the table from the entrypoint name to ISA table map.
            RemoveEntrypointTable(pTableView);

            // Remove the table from the view.
            ui.tableGrid->removeWidget(pTableView);
        }

        // Remove the entry from the input file path to ISA table map.
        auto filePathToTableIter = m_inputFileToIsaTableList.find(inputFilePath);
        if (filePathToTableIter != m_inputFileToIsaTableList.end())
        {
            m_inputFileToIsaTableList.erase(filePathToTableIter);
        }
    }
}

void rgIsaDisassemblyTabView::SwitchToEntryPoint(const std::string& entryName)
{
    // Find the disassembly table corresponding to the given entrypoint name.
    auto entryIter = m_entrypointDisassemblyTableViews.find(entryName);
    if (entryIter != m_entrypointDisassemblyTableViews.end())
    {
        rgIsaDisassemblyTableView* pTableView = entryIter->second;
        assert(pTableView != nullptr);

        if (pTableView != nullptr && pTableView != m_pCurrentTable)
        {
            // Do not load the table if it was loaded already.
            bool isTableCached = pTableView->IsDisassemblyLoaded();
            bool isTableLoaded = false;

            if (!isTableCached)
            {
                // Attempt to load the table.
                // Get the disassembly file path.
                const std::string& disassemblyFilePath = pTableView->GetDisassemblyFilePath();
                assert(!disassemblyFilePath.empty());

                if (!disassemblyFilePath.empty())
                {
                    isTableLoaded = pTableView->LoadDisassembly(disassemblyFilePath);
                    if (!isTableLoaded)
                    {
                        // Tell the user that the disassembly file failed to load.
                        std::stringstream errorString;
                        errorString << STR_ERR_CANNOT_LOAD_DISASSEMBLY_CSV_FILE;
                        errorString << disassemblyFilePath;
                        rgUtils::ShowErrorMessageBox(errorString.str().c_str());
                    }
                }

            }

            // If the table was loaded, either previously or just now, update the UI.
            assert(isTableCached || isTableLoaded);
            if (isTableCached || isTableLoaded)
            {
                // Hide the currently-visible disassembly table.
                if (m_pCurrentTable != nullptr)
                {
                    m_pCurrentTable->hide();
                }

                // Show the disassembly table that we're switching to.
                pTableView->show();
                m_pCurrentTable = pTableView;

                // Use the current table view as the focus proxy for this view.
                setFocusProxy(pTableView);

                // Filter the table we are about to show, without resizing
                // since the table is inheriting the filter from its predecessor.
                pTableView->UpdateFilteredTable();
            }
        }
    }
}

void rgIsaDisassemblyTabView::UpdateCorrelatedSourceFileLine(const std::string& inputFilePath, int lineNumber, std::string& entryName)
{
    if (!entryName.empty())
    {
        // Switch to view the disassembly table for the named entrypoint.
        SwitchToEntryPoint(entryName);
    }

    // Find the table used to present the given entrypoint's disassembly.
    auto entrypointToTableIter = m_entrypointDisassemblyTableViews.find(entryName);
    if (entrypointToTableIter != m_entrypointDisassemblyTableViews.end())
    {
        // Update the currently selected source line number.
        rgIsaDisassemblyTableView* pEntrypointTable = entrypointToTableIter->second;
        pEntrypointTable->UpdateCorrelatedSourceFileLine(lineNumber);
    }
}

bool rgIsaDisassemblyTabView::IsSourceLineCorrelatedForEntry(const std::string& entryName, int lineNumber)
{
    bool  ret = false;
    const auto& isaTableView = m_entrypointDisassemblyTableViews.find(entryName);
    if (isaTableView != m_entrypointDisassemblyTableViews.end())
    {
        ret = isaTableView->second->IsSourceLineCorrelated(lineNumber);
    }
    return ret;
}

void rgIsaDisassemblyTabView::HandleColumnVisibilityFilterStateChanged()
{
    // Only filter the current table.
    if (m_pCurrentTable != nullptr)
    {
        m_pCurrentTable->UpdateFilteredTable();

        // Resize the current table after changing column visibility.
        m_pCurrentTable->RequestTableResize();
    }
}

void rgIsaDisassemblyTabView::ConnectTableViewSignals(rgIsaDisassemblyTableView* pTableView)
{
    // Connect a forwarding signal used to handle when the input source file's correlation highlight line is updated.
    bool isConnected = connect(pTableView, &rgIsaDisassemblyTableView::InputSourceHighlightedLineChanged, this, &rgIsaDisassemblyTabView::InputSourceHighlightedLineChanged);
    assert(isConnected);

    // Connect the disassembly table resized handler.
    isConnected = connect(pTableView, &rgIsaDisassemblyTableView::DisassemblyTableWidthResizeRequested, this, &rgIsaDisassemblyTabView::DisassemblyTableWidthResizeRequested);
    assert(isConnected);
}

void rgIsaDisassemblyTabView::RemoveEntrypointTable(rgIsaDisassemblyTableView* pTableView)
{
    // Set the current table to null- it's going to be removed from the view.
    if (m_pCurrentTable == pTableView)
    {
        m_pCurrentTable = nullptr;
    }

    // Step through the map of entrypoint to table, and erase the one that matches the incoming table being removed.
    auto tablesStartIter = m_entrypointDisassemblyTableViews.begin();
    auto tablesEndIter = m_entrypointDisassemblyTableViews.end();
    for (auto tableIter = tablesStartIter; tableIter != tablesEndIter; ++tableIter)
    {
        if (tableIter->second == pTableView)
        {
            m_entrypointDisassemblyTableViews.erase(tableIter);
            break;
        }
    }

    // Find and remove the given table from the table to entrypoint name map.
    auto tableIter = m_disassemblyTableViewToEntrypoint.find(pTableView);
    if (tableIter != m_disassemblyTableViewToEntrypoint.end())
    {
        m_disassemblyTableViewToEntrypoint.erase(tableIter);
    }
}
