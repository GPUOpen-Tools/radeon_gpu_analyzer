#pragma once

// Qt.
#include <QWidget>

// Local.
#include <ui_rgIsaDisassemblyTabView.h>

// Forward declarations.
struct rgEntryOutput;
class rgIsaDisassemblyTableView;
namespace Ui
{
    class rgIsaDisassemblyTabView;
}

// A widget used to present disassembled instructions for multiple entrypoints in a kernel.
// Within this view all disassembled instructions target the same ASIC.
class rgIsaDisassemblyTabView : public QWidget
{
    Q_OBJECT

public:
    explicit rgIsaDisassemblyTabView(QWidget* pParent = nullptr);
    virtual ~rgIsaDisassemblyTabView() = default;

    // Clear out the existing correlated highlight lines from the disassembly table.
    void ClearCorrelationHighlight();

    // Get the name of the entrypoint for the currently displayed table.
    bool GetCurrentEntrypoint(std::string& currentEntrypoint) const;

    // Retrieve the number of disassembly tables being displayed within this ASIC tab.
    int GetTableCount() const;

    // Populate the disassembly tab with a separate table for each kernel entrypoint.
    bool PopulateEntries(const std::vector<rgEntryOutput>& disassembledEntryCsvFilePaths);

    // Remove ISA tables associated with the given input file path.
    void RemoveInputFileEntries(const std::string& inputFilePath);

    // Switch the view to display the disassembled instructions for the given entrypoint.
    void SwitchToEntryPoint(const std::string& entryName);

    // Update the input source file's line number used when correlating with ISA instruction lines.
    void UpdateCorrelatedSourceFileLine(const std::string& inputFilePath, int lineNumber, std::string& entrypointName);

    // Checks if given line number is present in the correlation table for given entry.
    bool IsSourceLineCorrelatedForEntry(const std::string& entryName, int lineNumber);

signals:
    // A signal emitted containing the index of the correlated line in the high level source input file.
    void InputSourceHighlightedLineChanged(int lineNumber);

    // A signal emitted when the requested width of the disassembly table is changed.
    void DisassemblyTableWidthResizeRequested(int tableWidth);

public slots:
    // A handler invoked when the user changes the visible columns in the disassembly table.
    void HandleColumnVisibilityFilterStateChanged();

private:
    // Connect signals for a new ISA table view instance.
    void ConnectTableViewSignals(rgIsaDisassemblyTableView* pTableView);

    // Remove the given table from the disassembly tab.
    void RemoveEntrypointTable(rgIsaDisassemblyTableView* pTableView);

    // A map of entrypoint name to disassembly view.
    std::map<std::string, rgIsaDisassemblyTableView*> m_entrypointDisassemblyTableViews;

    // A map of disassembly view to entrypoint name.
    std::map<rgIsaDisassemblyTableView*, std::string> m_disassemblyTableViewToEntrypoint;

    // A map of input file path to a list of disassembly tables generated from the file.
    std::map<std::string, std::vector<rgIsaDisassemblyTableView*>> m_inputFileToIsaTableList;

    // A pointer to the disassembly table currently being viewed.
    rgIsaDisassemblyTableView* m_pCurrentTable = nullptr;

    // The view holding the disassembly tables for multiple entries.
    Ui::rgIsaDisassemblyTabView ui;
};