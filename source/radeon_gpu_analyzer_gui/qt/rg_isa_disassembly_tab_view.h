#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_TAB_VIEW_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_TAB_VIEW_H_

// Qt.
#include <QWidget>

// Local.
#include "source/radeon_gpu_analyzer_gui/rg_data_types.h"
#include "ui_rg_isa_disassembly_tab_view.h"

// Forward declarations.
struct RgEntryOutput;
class RgIsaDisassemblyTableView;
namespace Ui
{
    class RgIsaDisassemblyTabView;
}

// A widget used to present disassembled instructions for multiple entrypoints in a kernel.
// Within this view all disassembled instructions target the same ASIC.
class RgIsaDisassemblyTabView : public QWidget
{
    Q_OBJECT

public:
    explicit RgIsaDisassemblyTabView(QWidget* parent = nullptr);
    virtual ~RgIsaDisassemblyTabView() = default;

    // Clear out the existing correlated highlight lines from the disassembly table.
    void ClearCorrelationHighlight();

    // Get the name of the entry point for the currently displayed table.
    bool GetCurrentEntrypoint(std::string& current_entrypoint) const;

    // Retrieve the number of disassembly tables being displayed within this ASIC tab.
    int GetTableCount() const;

    // Populate the disassembly tab with a separate table for each kernel entrypoint.
    bool PopulateEntries(const std::vector<RgEntryOutput>& disassembled_entry_csv_file_paths);

    // Remove ISA tables associated with the given input file path.
    void RemoveInputFileEntries(const std::string& input_file_path);

    // Switch the view to display the disassembled instructions for the given entrypoint.
    void SwitchToEntryPoint(const std::string& input_file_path, const std::string& entrypoint_name);

    // Update the input source file's line number used when correlating with ISA instruction lines.
    void UpdateCorrelatedSourceFileLine(const std::string& input_file_path, int line_number, std::string& entrypoint_name);

    // Checks if given line number is present in the correlation table for given entry.
    bool IsSourceLineCorrelatedForEntry(const std::string& input_file_path, const std::string& entry_name, int line_number);

    // Replace the input file path in the Table List map.
    // This function should be used when replacing a shader SPIR-V file with its disassembly version and vice versa.
    bool ReplaceInputFilePath(const std::string& old_file_path, const std::string& new_file_path);

signals:
    // A signal emitted containing the index of the correlated line in the high level source input file.
    void InputSourceHighlightedLineChanged(int line_number);

    // A signal emitted when the requested width of the disassembly table is changed.
    void DisassemblyTableWidthResizeRequested(int table_width);

    // A signal emitted when the disassembly tab view is clicked.
    void FrameFocusInSignal();

    // A signal emitted when the disassembly tab view loses focus.
    void FrameFocusOutSignal();

    // A signal to disable table scroll bar signals.
    void DisableScrollbarSignals();

    // A signal to enable table scroll bar signals.
    void EnableScrollbarSignals();

    // A signal to indicate that the target GPU push button gained the focus.
    void FocusTargetGpuPushButton();

    // A signal to indicate that the column push button gained the focus.
    void FocusColumnPushButton();

    // A signal to update the current sub widget.
    void UpdateCurrentSubWidget(DisassemblyViewSubWidgets sub_widget);

    // A signal to focus the cli output window.
    void FocusCliOutputWindow();

    // A signal to focus the source window.
    void FocusSourceWindow();

    // A signal to switch disassembly view size.
    void SwitchDisassemblyContainerSize();

public slots:
    // A handler invoked when the user changes the visible columns in the disassembly table.
    void HandleColumnVisibilityFilterStateChanged();

protected:
    // Connect signals for a new ISA table view instance.
    void ConnectTableViewSignals(RgIsaDisassemblyTableView* table_view);

    // Generate a unique key used to identify a source input file and entrypoint. The key consists
    // of the input source file path and entry point name, joined with a pipe.
    std::string GenerateEntrypointKey(const std::string& file_path, const std::string& entrypoint_name) const;

    // Decode a filepath/entrypoint key string into separate tokens.
    bool DecodeEntrypointKey(const std::string& entrypoint_key, std::string& file_path, std::string& entrypoint_name) const;

    // Remove the given table from the disassembly tab.
    void RemoveEntrypointTable(RgIsaDisassemblyTableView* table_view);

    // A map of entry point name key to disassembly view.
    std::map<std::string, RgIsaDisassemblyTableView*> entry_point_disassembly_table_views_;

    // A map of disassembly view to entry point name.
    std::map<RgIsaDisassemblyTableView*, std::string> disassembly_table_view_to_entrypoint_;

    // A map of input file path to a list of disassembly tables generated from the file.
    std::map<std::string, std::vector<RgIsaDisassemblyTableView*>> input_file_to_isa_table_list_;

    // A pointer to the disassembly table currently being viewed.
    RgIsaDisassemblyTableView* current_table_ = nullptr;

    // The view holding the disassembly tables for multiple entries.
    Ui::RgIsaDisassemblyTabView ui_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_TAB_VIEW_H_
