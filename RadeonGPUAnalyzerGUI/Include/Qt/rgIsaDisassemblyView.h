#pragma once

// C++.
#include <memory>

// Qt.
#include <QWidget>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>
#include <ui_rgIsaDisassemblyView.h>

// Forward declarations.
class ArrowIconWidget;
class ListWidget;
enum class rgIsaDisassemblyTableColumns;
class rgIsaDisassemblyTabView;
class rgResourceUsageView;
class rgViewContainer;
namespace Ui
{
    class rgIsaDisassemblyView;
}

// A class responsible for displaying ISA code for multiple GPU architectures.
class rgIsaDisassemblyView : public QWidget
{
    Q_OBJECT

public:
    explicit rgIsaDisassemblyView(QWidget* pParent = nullptr);
    virtual ~rgIsaDisassemblyView();

    // Populate the disassembly view using the given clone and build outputs.
    virtual bool PopulateBuildOutput(const std::shared_ptr<rgProjectClone> pProjectClone, const rgBuildOutputsMap& buildOutputs) = 0;

    // Clear all existing build outputs loaded in the view.
    void ClearBuildOutput();

    // Retrieve the bounding rectangle for the resource usage text.
    void GetResourceUsageTextBounds(QRect& textBounds) const;

    // Is the view currently empty?
    bool IsEmpty() const;

    // Remove the disassembly for the given input file.
    void RemoveInputFileEntries(const std::string& inputFilePath);

    // Checks if given source line is present in line correlation table for current entry point.
    bool IsLineCorrelatedInEntry(const std::string& inputFilePath, const std::string& targetGpu, const std::string& entrypoint, int srcLine) const;

    // Connect the title bar's double click signal.
    void ConnectTitleBarDoubleClick(const rgViewContainer* pDisassemblyViewContainer);

    // Replace the input file path in the ISA disassembly & resource tables with another file.
    // This function should be used if:
    // 1. a shader SPIR-V file is replaced with its disassembly version and vice versa (Vulkan mode only).
    // 2. a source file is renamed (all modes).
    bool ReplaceInputFilePath(const std::string& oldFilePath, const std::string& newFilePath);

signals:
    // A signal emitted when the input source file's highlighted correlation line should be updated.
    void InputSourceHighlightedLineChanged(int lineNumber);

    // A signal emitted when the user has changed the disassembly table's column visibility.
    void DisassemblyColumnVisibilityUpdated();

    // Handler invoked when the user changes the selected entry point for a given input file.
    void SelectedEntrypointChanged(const std::string& inputFilePath, const std::string& selectedEntrypointName);

    // A signal emitted when the requested width of the disassembly table is changed.
    void DisassemblyTableWidthResizeRequested(int tableWidth);

    // A signal emitted when the target GPU is changed.
    void SelectedTargetGpuChanged(const std::string& targetGpu);

    // A signal to disable table scroll bar signals.
    void DisableScrollbarSignals();

    // A signal to enable table scroll bar signals.
    void EnableScrollbarSignals();

    // A signal to remove focus from file menu sub buttons.
    void RemoveFileMenuButtonFocus();

    // A signal to indicate that the user clicked on disassembly view.
    void DisassemblyViewClicked();

    // A signal to focus the next view.
    void FocusNextView();

    // A signal to focus the previous view.
    void FocusPrevView();

    // A signal to focus the disassembly view.
    void FocusDisassemblyView();

    // A signal to update the current sub widget.
    void UpdateCurrentSubWidget(DisassemblyViewSubWidgets subWidget);

    // A signal to focus the cli output window.
    void FocusCliOutputWindow();

    // A signal to focus the source window.
    void FocusSourceWindow();

    // A signal to switch disassembly view size.
    void SwitchDisassemblyContainerSize();

public slots:
    // Handler invoked when the user changes the selected line in the input source file.
    void HandleInputFileSelectedLineChanged(const std::string& targetGpu, const std::string& inputFilePath, std::string& entryName, int lineIndex);

    // Handler invoked when the user changes the selected entry point for a given input file.
    void HandleSelectedEntrypointChanged(const std::string& targetGpu, const std::string& inputFilePath, const std::string& selectedEntrypointName);

    // Handler invoked when the user clicks on the tab view.
    void HandleDisassemblyTabViewClicked();

    // Handler invoked to color the container frame black.
    void HandleFocusOutEvent();

    // Handler to focus the column push button.
    void HandleFocusColumnsPushButton();

protected slots:
    // Handler invoked when the user clicks the column visibility arrow.
    void HandleColumnVisibilityButtonClicked(bool clicked);

    // Handler invoked when the user clicks an item in the column visibility list.
    void HandleColumnVisibilityComboBoxItemClicked(const QString& text, const bool checked);

    // Handler invoked when a check box's state is changed.
    void HandleColumnVisibilityFilterStateChanged(bool checked);

    // Handler invoked when the disassembly view loses focus.
    void HandleDisassemblyTabViewLostFocus();

    // Handler invoked when the user clicks on title bar.
    void HandleTitlebarClickedEvent(QMouseEvent* pEvent);

    // Handler invoked when the user clicks outside of the resource view.
    void HandleResourceUsageViewFocusOutEvent();

    // Handler invoked when the user clicks the Target GPU dropdown arrow button.
    void HandleTargetGpuArrowClicked(bool clicked);

    // Handler invoked when the user changes the selected target GPU.
    void HandleTargetGpuChanged(int currentIndex);

    // Handler invoked when the list widget gains focus.
    void HandleListWidgetFocusInEvent();

    // Handler invoked when the list widget loses focus.
    void HandleListWidgetFocusOutEvent();

    // Handler to process select GPU target hot key.
    void HandleSelectNextGPUTargetAction();

    // Handler to focus the target GPUs push button.
    void HandleFocusTargetGpuPushButton();

    // Handler to open the column list widget.
    void HandleOpenColumnListWidget();

    // Handler to open the GPU list widget.
    void HandleOpenGpuListWidget();

protected:
    // A map that associates an GPU name to a list of program build outputs.
    typedef std::map<std::string, std::vector<rgEntryOutput>> GpuToEntryVector;

    // A type of map that associates an entry point name with a resource usage view for the entrypoint.
    typedef std::map<std::string, rgResourceUsageView*> EntrypointToResourcesView;

    // A map of full input file path to a map of rgResourceUsageViews for the file.
    typedef std::map<std::string, EntrypointToResourcesView> InputToEntrypointViews;

    // Remove all items from the given list widget.
    void ClearListWidget(ListWidget* &pListWidget);

    // Connect signals for a new disassembly tab view.
    void ConnectDisassemblyTabViewSignals(rgIsaDisassemblyTabView* pEntryView);

    // Connect the signals for the disassembly view.
    void ConnectSignals();

    // Create the controls responsible for picking the visible columns in the disassembly table.
    void CreateColumnVisibilityControls();

    // Create the controls responsible for picking the currently selected target GPU.
    void CreateTargetGpuListControls();

    // Get the name of the given disassembly column as a string.
    std::string GetDisassemblyColumnName(rgIsaDisassemblyTableColumns column) const;

    // Retrieve a rgIsaDisassemblyTabView based on the given GPU name.
    rgIsaDisassemblyTabView* GetTargetGpuTabWidgetByTabName(const std::string& gpuFamilyName) const;

    // Populate the target GPU list widget.
    void PopulateTargetGpuList(const rgBuildOutputsMap& buildOutput);

    // Populate the disassembly view with the given build outputs.
    bool PopulateDisassemblyEntries(const GpuToEntryVector& gpuToDisassemblyCsvEntries);

    // Populate the resource usage view with the given build outputs.
    bool PopulateResourceUsageEntries(const GpuToEntryVector& gpuToResourceUsageCsvEntries);

    // Connect resource usage view signals.
    void ConnectResourceUsageViewSignals(rgResourceUsageView * pResourceUsageView);

    // Populate the names in the column visibility list.
    void PopulateColumnVisibilityList();

    // Clean up all disassembly views related to the given input source file.
    void DestroyDisassemblyViewsForFile(const std::string& inputFilePath);

    // Clean up all resource usage views related to the given input source file.
    void DestroyResourceUsageViewsForFile(const std::string& inputFilePath);

    // Set focus proxies for list widget check boxes to the frame.
    void SetCheckBoxFocusProxies(const ListWidget* pListWidget) const;

    // Set font sizes for list widget push buttons.
    void SetFontSizes();

    // Set the currently active resource usage view.
    void SetCurrentResourceUsageView(rgResourceUsageView* pResourceUsageView);

    // Set the currently active disassembly GPU tab view.
    void SetCurrentTargetGpuTabView(rgIsaDisassemblyTabView* pTabView);

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // Set the border stylesheet.
    virtual void SetBorderStylesheet() = 0;

    // Set the current target GPU to display disassembly for.
    void SetTargetGpu(const std::string& targetGpu);

    // Update the "All" checkbox text color to grey or black.
    void UpdateAllCheckBoxText();

    // A map of GPU to the views showing disassembly for multiple kernel entries.
    std::map<std::string, rgIsaDisassemblyTabView*> m_gpuTabViews;

    // A map of GPU name to a map of an input file's entry point resource usage views.
    std::map<std::string, InputToEntrypointViews> m_gpuResourceUsageViews;

    // The current target GPU tab being viewed.
    rgIsaDisassemblyTabView* m_pCurrentTabView = nullptr;

    // The widget used to display all columns available for display in the disassembly table.
    ListWidget* m_pDisassemblyColumnsListWidget = nullptr;

    // A custom event filter for the disassembly columns list widget.
    QObject* m_pDisassemblyColumnsListEventFilter = nullptr;

    // The list widget used to select the current target GPU.
    ListWidget* m_pTargetGpusListWidget = nullptr;

    // A custom event filter responsible for hiding the target GPU dropdown list.
    QObject* m_pTargetGpusListEventFilter = nullptr;

    // The resource usage text.
    std::string m_resourceUsageText;

    // The resource usage font.
    QFont m_resourceUsageFont;

    // Select next GPU action.
    QAction* m_pSelectNextGPUTarget = nullptr;

    // The tab key action.
    QAction* m_pTabKeyAction = nullptr;

    // The shift+tab key action.
    QAction* m_pShiftTabKeyAction = nullptr;

    // The interface responsible for presenting disassembly results for multiple GPUs.
    Ui::rgIsaDisassemblyView ui;
};