#pragma once

// C++.
#include <memory>

// Qt.
#include <QKeyEvent>
#include <QWidget>

// Local.
#include <ui_rgIsaDisassemblyTableView.h>

// Forward declarations.
class QItemSelection;
class QLabel;
class QMenu;
struct rgCliBuildOutput;
class rgIsaDisassemblyTableModelFilteringModel;
class rgIsaDisassemblyTableModel;
namespace Ui
{
    class rgIsaDisassemblyTableView;
}

// A widget responsible for presenting a table of disassembled ISA instruction information.
class rgIsaDisassemblyTableView : public QWidget
{
    Q_OBJECT

public:
    explicit rgIsaDisassemblyTableView(QWidget* pParent = nullptr);
    virtual ~rgIsaDisassemblyTableView() = default;

    // Does the given source file line contribute to this entry point disassembly?
    bool IsLineInEntrypoint(int lineIndex);

    // Load a disassembly CSV file at the given file path.
    bool LoadDisassembly(const std::string& disassemblyCsvFilePath);

    // Emit a signal causing the disassembly view to be resized to the current table's width.
    void RequestTableResize();

    // Update the input file's selected line number used to correlate ISA lines against.
    void UpdateCorrelatedSourceFileLine(int lineNumber);

    // Update the table's filter model so that it's re-filtered.
    void UpdateFilteredTable();

    // Get the file path of the textual file that is associated with this table.
    std::string GetDisassemblyFilePath() const;

    // Set the file path for the textual file that is associated with this table.
    void SetDisassemblyFilePath(const std::string& disassemblyFilePath);

    // Returns true if the disassembly was already loaded (and is therefore cached).
    bool IsDisassemblyLoaded() const;

    // Is the given source line correlated with some ISA line(s)?
    bool IsSourceLineCorrelated(int lineIndex) const;

protected:
    // Re-implement keyPressEvent.
    virtual void keyPressEvent(QKeyEvent* pEvent) override;

signals:
    // A signal emitted when the user changes the selected ISA line. Parameter is the input file's line number to select.
    void InputSourceHighlightedLineChanged(int lineNumber);

    // A signal emitted when the requested width of the disassembly table is changed.
    void DisassemblyTableWidthResizeRequested(int tableWidth);

    // A signal emitted when the user clicks on the disassembly table.
    void FrameFocusInSignal();

    // A signal emitted when the user clicks outside of the disassembly table.
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
    void UpdateCurrentSubWidget(DisassemblyViewSubWidgets subWidget);

    // A signal to focus the cli output window.
    void FocusCliOutputWindow();

    // A signal to focus the source window.
    void FocusSourceWindow();

    // A signal to switch disassembly view size.
    void SwitchDisassemblyContainerSize();

public slots:
    // A handler invoked when the user clicks a branch operand label link.
    void HandleBranchLinkClicked(const QString& link);

private slots:
    // Handler invoked when the copy item is clicked in the table's context menu.
    void HandleCopyDisassemblyClicked();

    // Handler invoked when the "Open in file browser" button is clicked in the table's context menu.
    void HandleOpenDisassemblyInFileBrowserClicked();

    // Handle updating the disassembly table when current selection has changed.
    void HandleCurrentSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    // Handler invoked when the table's context menu should be opened.
    void HandleOpenContextMenu(const QPoint& widgetClickPosition);

protected:
    // Connect signals used for the right-click context menu.
    void ConnectContextMenuSignals();

    // Connect signals emitted through the table's selection model.
    void ConnectSelectionSignals();

    // Connect signals.
    void ConnectSignals();

    // Determine the begin and end row indices for the current selection.
    bool GetSelectedRowRange(int& minRow, int& maxRow) const;

    // Initialize the context menu attached to the disassembly table.
    void InitializeContextMenu();

    // Add label rows into the left-most column of the disassembly table.
    void InitializeLabelRows();

    // Create new QLabels with clickable links and insert them into the disassembly table.
    void InitializeLinkLabels();

    // Check if the currently selected range is correlated with the same input source line.
    bool IsContiguousCorrelatedRangeSelected(int& correlatedLineIndex) const;

    // Scroll the disassembly table to the given line number.
    void ScrollToLine(int lineNumber);

    // The path to the currently-loaded disassembly CSV file.
    std::string m_disassemblyFilePath;

    // A signal to indicate if the disassembly was already cached in this table.
    bool m_isDisassemblyCached = false;

    // The model containing the disassembly table data.
    rgIsaDisassemblyTableModel* m_pIsaTableModel = nullptr;

    // A column-filtering proxy model connected to the table model containing the data.
    rgIsaDisassemblyTableModelFilteringModel* m_pIsaTableFilteringModel = nullptr;

    // The context menu displayed when the user left-clicks on the disassembly table.
    QMenu* m_pContextMenu = nullptr;

    // The context menu item used to copy the selected disassembly rows.
    QAction* m_pCopySelectedDisassembly = nullptr;

    // The context menu item used to open the current disassembly table's data in the file browser.
    QAction* m_pOpenDisassemblyInFileBrowser = nullptr;

    // The disassembly view's interface.
    Ui::rgIsaDisassemblyTableView ui;
};