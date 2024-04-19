#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_VIEW_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_VIEW_H_

// C++.
#include <memory>

// Qt.
#include <QWidget>
#include <QPlainTextEdit>

// Local.
#include "source/radeon_gpu_analyzer_gui/rg_data_types.h"
#include "ui_rg_isa_disassembly_view.h"

// Forward declarations.
class ArrowIconWidget;
class ListWidget;
enum class RgIsaDisassemblyTableColumns;
class RgIsaDisassemblyTabView;
class RgResourceUsageView;
class RgViewContainer;
namespace Ui
{
    class RgIsaDisassemblyView;
}

// A class responsible for displaying ISA code for multiple GPU architectures.
class RgIsaDisassemblyView : public QWidget
{
    Q_OBJECT

public:
    explicit RgIsaDisassemblyView(QWidget* parent = nullptr);
    virtual ~RgIsaDisassemblyView();

    // Populate the disassembly view using the given clone and build outputs.
    virtual bool PopulateBuildOutput(const std::shared_ptr<RgProjectClone> project_clone, const RgBuildOutputsMap& build_outputs) = 0;

    // Clear all existing build outputs loaded in the view.
    void ClearBuildOutput();

    // Retrieve the bounding rectangle for the resource usage text.
    void GetResourceUsageTextBounds(QRect& text_bounds) const;

    // Is the view currently empty?
    bool IsEmpty() const;

    // Remove the disassembly for the given input file.
    void RemoveInputFileEntries(const std::string& input_file_path);

    // Checks if given source line is present in line correlation table for current entry point.
    bool IsLineCorrelatedInEntry(const std::string& input_file_path, const std::string& target_gpu, const std::string& entrypoint, int src_line) const;

    // Connect the title bar's double click signal.
    void ConnectTitleBarDoubleClick(const RgViewContainer* disassembly_view_container);

    // Replace the input file path in the ISA disassembly & resource tables with another file.
    // This function should be used if:
    // 1. a shader SPIR-V file is replaced with its disassembly version and vice versa (Vulkan mode only).
    // 2. a source file is renamed (all modes).
    bool ReplaceInputFilePath(const std::string& old_file_path, const std::string& new_file_path);

    // Indicate if the max VGPR column is currently visible.
    bool IsMaxVgprColumnVisible() const;

    // Enable/disable the show max VGPR context menu option.
    void EnableShowMaxVgprContextOption() const;

signals:
    // A signal emitted when the input source file's highlighted correlation line should be updated.
    void InputSourceHighlightedLineChanged(int line_number);

    // A signal emitted when the user has changed the disassembly table's column visibility.
    void DisassemblyColumnVisibilityUpdated();

    // A signal emitted when the user changes the selected entry point for a given input file.
    void SelectedEntrypointChanged(const std::string& input_file_path, const std::string& selected_entrypoint_name);

    // A signal emitted when the requested width of the disassembly table is changed.
    void DisassemblyTableWidthResizeRequested(int table_width);

    // A signal emitted when the target GPU is changed.
    void SelectedTargetGpuChanged(const std::string& target_gpu);

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
    void UpdateCurrentSubWidget(DisassemblyViewSubWidgets sub_widget);

    // A signal to focus the cli output window.
    void FocusCliOutputWindow();

    // A signal to focus the source window.
    void FocusSourceWindow();

    // A signal to switch disassembly view size.
    void SwitchDisassemblyContainerSize();

    //  A signal to show/hide maximum VGPRs.
    void ShowMaximumVgprClickedSignal();

    // A signal to enable/disable the Edit->Go to next maximum live VGPR line option.
    void EnableShowMaxVgprOptionSignal(bool is_enabled);

public slots:
    // Handler invoked when the user changes the selected line in the input source file.
    void HandleInputFileSelectedLineChanged(const std::string& target_gpu, const std::string& input_file_path, std::string& entry_name, int line_index);

    // Handler invoked when the user changes the selected entry point for a given input file.
    void HandleSelectedEntrypointChanged(const std::string& target_gpu, const std::string& input_file_path, const std::string& selected_entrypoint_name);

    // Show/hide Kernel Name Label.
    void HandleSetKernelNameLabel(bool show, const std::string& setTextLabel = "");

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
    void HandleTitlebarClickedEvent(QMouseEvent* event);

    // Handler invoked when the user clicks outside of the resource view.
    void HandleResourceUsageViewFocusOutEvent();

    // Handler invoked when the user clicks the Target GPU dropdown arrow button.
    void HandleTargetGpuArrowClicked(bool clicked);

    // Handler invoked when the user changes the selected target GPU.
    void HandleTargetGpuChanged(int current_index);

    // Handler invoked when the list widget gains focus.
    void HandleListWidgetFocusInEvent();

    // Handler invoked when the list widget loses focus.
    void HandleListWidgetFocusOutEvent();

    // Handler to process select GPU target hot key.
    void HandleSelectNextGPUTargetAction();

    // Handler to process select next max VGPR hot key.
    void HandleSelectNextMaxVgprLineAction();

    // Handler to process select previous max VGPR hot key.
    void HandleSelectPreviousMaxVgprLineAction();

    // Handler to focus the target GPUs push button.
    void HandleFocusTargetGpuPushButton();

    // Handler to open the column list widget.
    void HandleOpenColumnListWidget();

    // Handler to open the GPU list widget.
    void HandleOpenGpuListWidget();

    // Handler invoked when disassembly view tab widget tab is changed.
    void HandleCurrentTabChanged(int index);

protected:
    // A map that associates an GPU name to a list of program build outputs.
    typedef std::map<std::string, std::vector<RgEntryOutput>> GpuToEntryVector;

    // A type of map that associates an entry point name with a resource usage view for the entrypoint.
    typedef std::map<std::string, RgResourceUsageView*> EntrypointToResourcesView;

    // A map of full input file path to a map of RgResourceUsageViews for the file.
    typedef std::map<std::string, EntrypointToResourcesView> InputToEntrypointViews;

    // A type of map that associates an entry point name with a livereg analysis view for the entrypoint.
    typedef std::map<std::string, QPlainTextEdit*> EntryPointToLiveregAnalysisViews;

    // A map of full input file path to a map of QPlainTextEdit for the file.
    typedef std::map<std::string, EntryPointToLiveregAnalysisViews> InputToEntrypointLiveregViews;

    // Remove all items from the given list widget.
    void ClearListWidget(ListWidget* &list_widget);

    // Connect signals for a new disassembly tab view.
    void ConnectDisassemblyTabViewSignals(RgIsaDisassemblyTabView* entry_view);

    // Connect the signals for the disassembly view.
    void ConnectSignals();

    // Create the lable responsible for displaying full kernel name in the disassembly table.
    void CreateKernelNameLabel();

    // Create the controls responsible for picking the visible columns in the disassembly table.
    void CreateColumnVisibilityControls();

    // Create the controls responsible for picking the currently selected target GPU.
    void CreateTargetGpuListControls();

    // Get the name of the given disassembly column as a string.
    std::string GetDisassemblyColumnName(RgIsaDisassemblyTableColumns column) const;

    // Retrieve a RgIsaDisassemblyTabView based on the given GPU name.
    RgIsaDisassemblyTabView* GetTargetGpuTabWidgetByTabName(const std::string& gpu_family_name) const;

    // Populate the target GPU list widget.
    void PopulateTargetGpuList(const RgBuildOutputsMap& build_output);

    // Populate the disassembly view with the given build outputs.
    bool PopulateDisassemblyEntries(const GpuToEntryVector& gpu_to_disassembly_csv_entries);

    // Populate the resource usage view with the given build outputs.
    bool PopulateResourceUsageEntries(const GpuToEntryVector& gpu_to_resource_usage_csv_entries);

    // Connect resource usage view signals.
    void ConnectResourceUsageViewSignals(RgResourceUsageView * resource_usage_view);

    // Populate the names in the column visibility list.
    void PopulateColumnVisibilityList();

    // Clean up all disassembly views related to the given input source file.
    void DestroyDisassemblyViewsForFile(const std::string& input_file_path);

    // Clean up all resource usage views related to the given input source file.
    void DestroyResourceUsageViewsForFile(const std::string& input_file_path);

    // Set focus proxies for list widget check boxes to the frame.
    void SetCheckBoxFocusProxies(const ListWidget* list_widget) const;

    // Set font sizes for list widget push buttons.
    void SetFontSizes();

    // Set the currently active resource usage view.
    void SetCurrentResourceUsageView(RgResourceUsageView* resource_usage_view);

    // Set the currently active disassembly GPU tab view.
    void SetCurrentTargetGpuTabView(RgIsaDisassemblyTabView* tab_view);

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // Set the border stylesheet.
    virtual void SetBorderStylesheet(bool is_selected) = 0;

    // Set the current target GPU to display disassembly for.
    void SetTargetGpu(const std::string& target_gpu);

    // Update the "All" checkbox text color to grey or black.
    void UpdateAllCheckBoxText();

    // A map of GPU to the views showing disassembly for multiple kernel entries.
    std::map<std::string, RgIsaDisassemblyTabView*> gpu_tab_views_;

    // A map of GPU name to a map of an input file's entry point resource usage views.
    std::map<std::string, InputToEntrypointViews> gpu_resource_usage_views_;

    // A map of GPU name to a map of an input file's livereg analysis views.
    std::map<std::string, InputToEntrypointLiveregViews> gpu_livereg_analysis_views_;

    // The current target GPU tab being viewed.
    RgIsaDisassemblyTabView* current_tab_view_ = nullptr;

    // The widget used to display all columns available for display in the disassembly table.
    ListWidget* disassembly_columns_list_widget_ = nullptr;

    // A custom event filter for the disassembly columns list widget.
    QObject* disassembly_columns_list_event_filter_ = nullptr;

    // The list widget used to select the current target GPU.
    ListWidget* target_gpus_list_widget_ = nullptr;

    // A custom event filter responsible for hiding the target GPU dropdown list.
    QObject* target_gpus_list_event_filter_ = nullptr;

    // The resource usage text.
    std::string resource_usage_text_;

    // The resource usage font.
    QFont resource_usage_font_;

    // Select next GPU action.
    QAction* select_next_gpu_target_ = nullptr;

    // Select the next max VGPR line.
    QAction* select_next_max_vgpr_line_ = nullptr;

    // Select the previous max VGPR line.
    QAction* select_previous_max_vgpr_line_ = nullptr;

    // The tab key action.
    QAction* tab_key_action_ = nullptr;

    // The shift+tab key action.
    QAction* shift_tab_key_action_ = nullptr;

    // The interface responsible for presenting disassembly results for multiple GPUs.
    Ui::RgIsaDisassemblyView ui_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_VIEW_H_
