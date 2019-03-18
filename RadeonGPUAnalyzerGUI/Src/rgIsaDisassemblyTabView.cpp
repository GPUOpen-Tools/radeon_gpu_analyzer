// C++.
#include <cassert>
#include <sstream>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyTabView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyTableView.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>

// A separator used in joining an input source file path with an entry point name.
// Such a string is used to uniquely identify an entry point with potential for
// name collisions between separate source files.
static const char s_ENTRYPOINT_KEY_SEPARATOR = '|';

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
    // Only a single entry point table will be visible at a time, and the user can switch between the current entry.
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

            // Generate a key string used to identify a named entry point within a given input source file.
            std::string entrypointNameKey = GenerateEntrypointKey(entry.m_inputFilePath, entry.m_entrypointName);

            // Associate the kernel name with the table showing the disassembly.
            m_entrypointDisassemblyTableViews[entrypointNameKey] = pTableView;

            // Also associate the disassembly table with the entry point it's displaying data for.
            m_disassemblyTableViewToEntrypoint[pTableView] = entrypointNameKey;

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
            // Remove the references to the table from the entry point name to ISA table map.
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

void rgIsaDisassemblyTabView::SwitchToEntryPoint(const std::string& inputFilePath, const std::string& entrypointName)
{
    // Generate a key string used to identify a named entry point within a given input source file.
    std::string entrypointNameKey = GenerateEntrypointKey(inputFilePath, entrypointName);

    // Find the disassembly table corresponding to the given entry point name.
    auto entryIter = m_entrypointDisassemblyTableViews.find(entrypointNameKey);
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
                        rgUtils::ShowErrorMessageBox(errorString.str().c_str(), this);
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
        SwitchToEntryPoint(inputFilePath, entryName);
    }

    // Find the table used to present the given entrypoint's disassembly.
    std::string entrypointNameKey = GenerateEntrypointKey(inputFilePath, entryName);
    auto entrypointToTableIter = m_entrypointDisassemblyTableViews.find(entrypointNameKey);
    if (entrypointToTableIter != m_entrypointDisassemblyTableViews.end())
    {
        // Update the currently selected source line number.
        rgIsaDisassemblyTableView* pEntrypointTable = entrypointToTableIter->second;
        pEntrypointTable->UpdateCorrelatedSourceFileLine(lineNumber);
    }
}

bool rgIsaDisassemblyTabView::IsSourceLineCorrelatedForEntry(const std::string& inputFilePath, const std::string& entryName, int lineNumber)
{
    bool  ret = false;

    // Generate a key string used to identify a named entry point within a given input source file.
    std::string entrypointNameKey = GenerateEntrypointKey(inputFilePath, entryName);

    const auto& isaTableView = m_entrypointDisassemblyTableViews.find(entrypointNameKey);
    if (isaTableView != m_entrypointDisassemblyTableViews.end())
    {
        ret = isaTableView->second->IsSourceLineCorrelated(lineNumber);
    }
    return ret;
}

bool rgIsaDisassemblyTabView::ReplaceInputFilePath(const std::string& oldFilePath, const std::string& newFilePath)
{
    assert(!oldFilePath.empty());
    assert(!newFilePath.empty());
    bool ret = (!oldFilePath.empty() && !newFilePath.empty());

    if (ret)
    {
        // 1. Replace the file path in the "entry point --> disasm table view" map.
        // There may be multiple matching elements since the keys in this map
        // consist of the file path and the entry name: KEY = "file-path|entry-name".
        std::vector<std::map<std::string, rgIsaDisassemblyTableView*>::iterator> nodesToChange;
        for (auto it = m_entrypointDisassemblyTableViews.begin(); it != m_entrypointDisassemblyTableViews.end(); ++it)
        {
            std::string filePath, entryName;
            DecodeEntrypointKey(it->first, filePath, entryName);
            if (filePath == oldFilePath)
            {
                nodesToChange.push_back(it);
            }
        }

        for (auto it : nodesToChange)
        {
            std::string filePath, entryName;
            DecodeEntrypointKey(it->first, filePath, entryName);
            auto pTableView = it->second;
            m_entrypointDisassemblyTableViews.erase(it);
            std::string newKey = GenerateEntrypointKey(newFilePath, entryName);
            m_entrypointDisassemblyTableViews[newKey] = pTableView;
        }
    }

    if (ret)
    {
        // 2. Replace the file path in the "disasm table view --> entry point" map.
        for (auto& val : m_disassemblyTableViewToEntrypoint)
        {
            std::string filePath, entryName;
            DecodeEntrypointKey(val.second, filePath, entryName);
            if (filePath == oldFilePath)
            {
                val.second = GenerateEntrypointKey(newFilePath, entryName);
            }
        }
    }

    if (ret)
    {
        // 3. Replace the file path in the "input file --> disasm table views" map.
        auto it = m_inputFileToIsaTableList.find(oldFilePath);
        if ((ret = (it != m_inputFileToIsaTableList.end())) == true)
        {
            auto tableViews = it->second;
            m_inputFileToIsaTableList.erase(it);
            m_inputFileToIsaTableList[newFilePath] = tableViews;
        }
    }

    assert(ret);

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

    // Connect the disassembly table clicked handler.
    isConnected = connect(pTableView, &rgIsaDisassemblyTableView::FrameFocusInSignal, this, &rgIsaDisassemblyTabView::FrameFocusInSignal);
    assert(isConnected);

    // Connect a forwarding signal used to handle when the disassembly table loses focus.
    isConnected = connect(pTableView, &rgIsaDisassemblyTableView::FrameFocusOutSignal, this, &rgIsaDisassemblyTabView::FrameFocusOutSignal);
    assert(isConnected);

    // Connect the disassembly view's enable scroll bar signal.
    isConnected = connect(this, &rgIsaDisassemblyTabView::EnableScrollbarSignals, pTableView, &rgIsaDisassemblyTableView::EnableScrollbarSignals);
    assert(isConnected);

    // Connect the disassembly view's disable scroll bar signal.
    isConnected = connect(this, &rgIsaDisassemblyTabView::DisableScrollbarSignals, pTableView, &rgIsaDisassemblyTableView::DisableScrollbarSignals);
    assert(isConnected);

    // Connect the disassembly table's target GPU push button focus in signal.
    isConnected = connect(pTableView, &rgIsaDisassemblyTableView::FocusTargetGpuPushButton, this, &rgIsaDisassemblyTabView::FocusTargetGpuPushButton);
    assert(isConnected);

    // Connect the disassembly table's switch disassembly view size signal.
    isConnected = connect(pTableView, &rgIsaDisassemblyTableView::SwitchDisassemblyContainerSize, this, &rgIsaDisassemblyTabView::SwitchDisassemblyContainerSize);
    assert(isConnected);

    // Connect the disassembly table's columns push button focus in signal.
    isConnected = connect(pTableView, &rgIsaDisassemblyTableView::FocusColumnPushButton, this, &rgIsaDisassemblyTabView::FocusColumnPushButton);
    assert(isConnected);

    // Connect the disassembly table's cli output window focus in signal.
    isConnected = connect(pTableView, &rgIsaDisassemblyTableView::FocusCliOutputWindow, this, &rgIsaDisassemblyTabView::FocusCliOutputWindow);
    assert(isConnected);

    // Connect the disassembly table's source window focus in signal.
    isConnected = connect(pTableView, &rgIsaDisassemblyTableView::FocusSourceWindow, this, &rgIsaDisassemblyTabView::FocusSourceWindow);
    assert(isConnected);

    // Connect the disassembly tab view's update current sub widget signal.
    isConnected = connect(this, &rgIsaDisassemblyTabView::UpdateCurrentSubWidget, pTableView, &rgIsaDisassemblyTableView::UpdateCurrentSubWidget);
    assert(isConnected);

    // Connect the disassembly table's focus cli output window signal.
    isConnected = connect(pTableView, &rgIsaDisassemblyTableView::FocusCliOutputWindow, this, &rgIsaDisassemblyTabView::FocusCliOutputWindow);
    assert(isConnected);

}

std::string rgIsaDisassemblyTabView::GenerateEntrypointKey(const std::string& filePath, const std::string& entrypointName) const
{
    // In some cases it may not be possible to identify a given entry point by name when multiple
    // entrypoints use the same name. Return a unique key based on the input filename and the
    // entry point name, so that each entry point can be identified correctly.
    return filePath + s_ENTRYPOINT_KEY_SEPARATOR + entrypointName;
}

bool rgIsaDisassemblyTabView::DecodeEntrypointKey(const std::string& entrypointKey, std::string& filePath, std::string& entrypointName) const
{
    bool ret = false;

    // Attempt to split the given entry point key string into source file path and entry point name strings.
    std::vector<std::string> filePathAndEntrypointNameList;
    rgUtils::splitString(entrypointKey, s_ENTRYPOINT_KEY_SEPARATOR, filePathAndEntrypointNameList);

    // Verify that only 2 tokens are found. One for the source file path, and another for entry point name.
    size_t tokenCount = filePathAndEntrypointNameList.size();
    assert(tokenCount == 2);
    if (tokenCount == 2)
    {
        filePath = filePathAndEntrypointNameList[0];
        entrypointName = filePathAndEntrypointNameList[1];
        ret = true;
    }

    return ret;
}

void rgIsaDisassemblyTabView::RemoveEntrypointTable(rgIsaDisassemblyTableView* pTableView)
{
    // Set the current table to null- it's going to be removed from the view.
    if (m_pCurrentTable == pTableView)
    {
        m_pCurrentTable = nullptr;
    }

    // Step through the map of entry point to table, and erase the one that matches the incoming table being removed.
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

    // Find and remove the given table from the table to entry point name map.
    auto tableIter = m_disassemblyTableViewToEntrypoint.find(pTableView);
    if (tableIter != m_disassemblyTableViewToEntrypoint.end())
    {
        m_disassemblyTableViewToEntrypoint.erase(tableIter);
    }
}
