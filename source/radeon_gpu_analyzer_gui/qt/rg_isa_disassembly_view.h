//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for shader ISA Disassembly view.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_VIEW_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_VIEW_H_

// C++.
#include <memory>

// Qt.
#include <QPlainTextEdit>
#include <QWidget>

// Infra.
#include "qt_common/custom_widgets/arrow_icon_combo_box.h"
#include "qt_isa_gui/widgets/isa_widget.h"
#include "qt_isa_gui/widgets/isa_tree_view.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_raw_text_disassembly_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_item_delegate.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_item_model.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_proxy_model.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_tree_view.h"

#include "radeon_gpu_analyzer_gui/rg_data_types.h"
#include "ui_rg_isa_disassembly_view.h"

// Forward declarations.
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

    // Set focus on to the go to line, line edit widget.
    void SetFocusOnGoToLineWidget();

    // Set focus on to the line edit search widget.
    void SetFocusOnSearchWidget();

    // Get the current tree view in this widget.
    RgIsaTreeView* GetTreeView() const;

    // Check if the current API has line correlation supported.
    virtual bool IsLineCorrelationSupported() const;

    // Set the target gpu label and architecture in the model by finding the target gpu that corresponds to the given input file in the build settings.
    void SetTargetGpuLabel(std::string input_file, std::shared_ptr<RgBuildSettings> build_settings);

signals:
    // A signal emitted when the input source file's highlighted correlation line was updated.
    void InputSourceHighlightedLineChanged(int src_line_number);

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

    //  A signal to show next maximum VGPRs.
    void ShowNextMaxVgprClickedSignal();

    //  A signal to show prev maximum VGPRs.
    void ShowPrevMaxVgprClickedSignal();

    // A signal to enable/disable the Edit->Go to next maximum live VGPR line option.
    void EnableShowMaxVgprOptionSignal(bool is_enabled) const;

public slots:
    // Handler invoked when the user changes the selected line in the input source file.
    void HandleInputFileSelectedLineChanged(const std::string& target_gpu, const std::string& input_file_path, std::string& entry_name, int line_index);

    // Handler invoked when the user changes the selected entry point for a given input file.
    void HandleSelectedEntrypointChanged(const std::string& target_gpu, const std::string& input_file_path, const std::string& selected_entrypoint_name);

    // Show/hide Kernel Name Label.
    void HandleSetKernelNameLabel(bool show, const std::string& setTextLabel = "");

    // Handler invoked when the user clicks on the raw button.
    void HandleRawTextButtonClicked();

    // Handler invoked when the user clicks on the tab view.
    void HandleDisassemblyViewClicked();

    // Handler invoked to color the container frame black.
    void HandleFocusOutEvent();

    // Handler to focus the column push button.
    void HandleFocusRawTextDisassemblyPushButton();

protected slots:

    // Handler invoked when the disassembly view loses focus.
    void HandleDisassemblyTabViewLostFocus();

    // Handler invoked when the user clicks on title bar.
    void HandleTitlebarClickedEvent(QMouseEvent* event);

    // Handler invoked when the user clicks outside of the resource view.
    void HandleResourceUsageViewFocusOutEvent();

    // Handler invoked when the user changes the selected target GPU.
    void HandleTargetGpuChanged();

    // Handler invoked when the list widget gains focus.
    void HandleListWidgetFocusInEvent();

    // Handler invoked when the list widget loses focus.
    void HandleListWidgetFocusOutEvent();

    // Handler to process select GPU target hot key.
    void HandleSelectNextGPUTargetAction();

    // Handler to focus the target GPUs push button.
    void HandleFocusTargetGpuPushButton();

    // Handler to open disassembly file in browser.
    void HandleOpenDisassemblyInFileBrowser();

    // Handler to open the GPU list widget.
    void HandleOpenGpuListWidget();

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

    // Create Isa viewer tree.
    void CreateIsaTreeView(QWidget* disassembly_view_parent);

    // Connect the signals for the disassembly view.
    void ConnectSignals();

    // Connect the signals for the isa tree view.
    void ConnectIsaTreeViewSignals();

    // Create the lable responsible for displaying full kernel name in the disassembly table.
    void CreateKernelNameLabel();

    // Create the controls responsible for picking the currently selected target GPU.
    void CreateTargetGpuListControls();

    // Populate the target GPU list widget.
    void PopulateTargetGpuList(const RgBuildOutputsMap& build_output);

    // Populate the disassembly view with the given build outputs.
    bool PopulateDisassemblyEntries(const GpuToEntryVector& gpu_to_disassembly_csv_entries);

    // Populate the resource usage view with the given build outputs.
    bool PopulateResourceUsageEntries(const GpuToEntryVector& gpu_to_resource_usage_csv_entries);

    // Connect resource usage view signals.
    void ConnectResourceUsageViewSignals(RgResourceUsageView* resource_usage_view);

    // Clean up all disassembly views related to the given input source file.
    void DestroyDisassemblyViewDataForFile(const std::string& input_file_path);

    // Clean up all resource usage views related to the given input source file.
    void DestroyResourceUsageViewsForFile(const std::string& input_file_path);

    // Set focus proxies for check boxes to the frame.
    void SetCheckBoxFocusProxies(QCheckBox* check_box);

    // Set the currently active resource usage view.
    void SetCurrentResourceUsageView(RgResourceUsageView* resource_usage_view);

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // Set the border stylesheet.
    virtual void SetBorderStylesheet(bool is_selected) = 0;

    // Generate a unique key used to identify a source input file and entrypoint. The key consists
    // of the input source file path and entry point name, joined with a pipe.
    std::string GenerateEntrypointKey(const std::string& file_path, const std::string& asic, const std::string& entrypoint_name) const;

    // Decode a filepath/entrypoint key string into separate tokens.
    bool DecodeEntrypointKey(const std::string& entrypoint_key, std::string& file_path, std::string& asic, std::string& entrypoint_name) const;

    // A map of (GPU+inputfile+entryname) to a map of an input file's data.
    std::unordered_map<std::string, std::pair<std::string, std::string>> disassembly_view_input_files_map_;

    // A map of GPU name to a map of an input file's entry point resource usage views.
    std::map<std::string, InputToEntrypointViews> gpu_resource_usage_views_;

    // A map of GPU name to a map of an input file's livereg analysis views.
    std::map<std::string, InputToEntrypointLiveregViews> gpu_livereg_analysis_views_;

    // The current key (GPU+inputfile+entryname) for the data displayed in the isa disassembly view.
    std::string current_disassembly_view_data_key_;

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

    // Isa disassembly qt data model.
    RgIsaProxyModel* rg_isa_proxy_model_ = nullptr;

    // Isa disassembly qt data model.
    RgIsaItemModel* rg_isa_item_model_ = nullptr;

    // Isa disassembly qt view.
    RgIsaTreeView* rg_isa_tree_view_ = nullptr;

    // Shared Isa view widget.
    IsaWidget* rg_isa_widget_ = nullptr;

    // The interface responsible for presenting disassembly results for multiple GPUs.
    Ui::RgIsaDisassemblyView ui_;
};
#endif  // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_VIEW_H_
