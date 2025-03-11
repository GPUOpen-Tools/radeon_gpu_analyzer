#ifdef _WIN32
// This warning relates to a compiler limitation that restricts the length of type names.
// The warning doesn't impose any risk to the code, so it's disabled for this file.
#pragma warning(disable : 4503)
#endif

// C++.
#include <cassert>
#include <memory>
#include <sstream>
#include <vector>
#include <algorithm>

// Qt.
#include <QAction>
#include <QCheckBox>
#include <QListWidgetItem>

// Infra.
#include "qt_common/custom_widgets/arrow_icon_combo_box.h"
#include "qt_common/utils/common_definitions.h"
#include "qt_common/utils/qt_util.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_hide_list_widget_event_filter.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_resource_usage_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_view_container.h"
#include "radeon_gpu_analyzer_gui/rg_data_types.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_resource_usage_csv_file_parser.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

// A separator used in joining an input source file path, target asic and an entry point name.
// Such a string is used to uniquely identify an entry point with potential for
// name collisions between separate source files and gpus.
static const char kEntrypointKeySeparator = '|';

RgIsaDisassemblyView::RgIsaDisassemblyView(QWidget* parent)
    : QWidget(parent)
{
    ui_.setupUi(this);

    ui_.verticalSplitterWidget->setStretchFactor(0, 1);
    ui_.verticalSplitterWidget->setStretchFactor(1, 0);
    ui_.verticalSplitterWidget->handle(1)->setDisabled(true);

    ui_.rawTextPushButton->hide();

    // Create the isa view widget.
    CreateIsaTreeView(parent);

    // Create the dropdown list used to select the current target GPU.
    CreateTargetGpuListControls();

    // Connect the signals for the disassembly view.
    ConnectSignals();

    // Connect the signals for the tree view view.
    ConnectIsaTreeViewSignals();

    // Set mouse pointer to pointing hand cursor.
    SetCursor();

    // Block recursively repolishing child tables in the disassembly view.
    ui_.disassemblyHostWidget->setProperty(kIsRepolishingBlocked, true);

    // Create Full kernel Name Label.
    CreateKernelNameLabel();

    // Set the focus proxy of the view maximize button to be the title bar.
    // This will cause the frame border to change to black when the maximize button loses focus.
    // This is because the view title bar already handles its own loss of focus event.
    ui_.viewMaximizeButton->setFocusProxy(ui_.viewTitlebar);
}

RgIsaDisassemblyView::~RgIsaDisassemblyView()
{
}

void RgIsaDisassemblyView::ClearBuildOutput()
{
    auto start = disassembly_view_input_files_map_.begin();
    auto end   = disassembly_view_input_files_map_.end();
    for (auto iter = start; iter != end; ++iter)
    {
        // Mark Entry for evition from the cache.        
        RgIsaItemModel::EntryData entry_data{};
        entry_data.isa_file_path  = iter->second.first;
        entry_data.vgpr_file_path = iter->second.second;
        entry_data.operation      = RgIsaItemModel::EntryData::Operation::kEvictData;
        rg_isa_item_model_->UpdateData(&entry_data);
    }

    disassembly_view_input_files_map_.clear();

    gpu_resource_usage_views_.clear();
    resource_usage_text_.clear();

    current_disassembly_view_data_key_.clear();
}

void RgIsaDisassemblyView::GetResourceUsageTextBounds(QRect& tab_bounds) const
{
    QFontMetrics font_metrics(resource_usage_font_);
    tab_bounds = font_metrics.boundingRect(resource_usage_text_.c_str());
}

bool RgIsaDisassemblyView::IsEmpty() const
{
    return disassembly_view_input_files_map_.empty();
}

void RgIsaDisassemblyView::RemoveInputFileEntries(const std::string& input_file_path)
{
    // Clean up disassembly tables associated with the given file.
    DestroyDisassemblyViewDataForFile(input_file_path);

    // Clean up resource usage views associated with the given file.
    DestroyResourceUsageViewsForFile(input_file_path);
}

void RgIsaDisassemblyView::HandleInputFileSelectedLineChanged(const std::string& target_gpu,
                                                              const std::string& input_file_path,
                                                              std::string&       entry_name,
                                                              int                line_index)
{
    HandleSelectedEntrypointChanged(target_gpu, input_file_path, entry_name);

    emit InputSourceHighlightedLineChanged(line_index);
}

void RgIsaDisassemblyView::HandleSelectedEntrypointChanged(const std::string& target_gpu,
                                                           const std::string& input_file_path,
                                                           const std::string& selected_entrypoint_name)
{
    // Generate a key string used to identify a named entry point within a given input source file.
    std::string entrypoint_name_key = GenerateEntrypointKey(input_file_path, target_gpu, selected_entrypoint_name);

    if (current_disassembly_view_data_key_ != entrypoint_name_key)
    {
        // Search the map to find the relevant view data associated with the given entrypoint.
        auto view_data_iter = disassembly_view_input_files_map_.find(entrypoint_name_key);
        if (view_data_iter != disassembly_view_input_files_map_.end())
        {
            // Set the data model associated with the given entrypoint.
            const auto&               input_file_pair = view_data_iter->second;
            RgIsaItemModel::EntryData entry_data{};
            entry_data.target_gpu     = target_gpu;
            entry_data.isa_file_path  = input_file_pair.first;
            entry_data.vgpr_file_path = input_file_pair.second;
            entry_data.operation      = RgIsaItemModel::EntryData::Operation::kLoadData;
            if (rg_isa_widget_ && rg_isa_item_model_)
            {
                rg_isa_item_model_->UpdateData(&entry_data);

                rg_isa_widget_->UpdateSpannedColumns();

                const int max_line_number = rg_isa_item_model_->GetLineCount() > 0 ? rg_isa_item_model_->GetLineCount() - 1 : 0;
                rg_isa_widget_->SetGoToLineValidatorLineCount(max_line_number);

                rg_isa_widget_->Search();

                rg_isa_widget_->ClearHistory();

                // Don't bother updating the max vgpr indices until the expand/collapse state is set.
                rg_isa_tree_view_->blockSignals(true);

                rg_isa_widget_->ExpandCollapseAll();

                rg_isa_tree_view_->blockSignals(false);

                std::vector<QModelIndex> source_indices;
                rg_isa_item_model_->GetMaxVgprPressureIndices(source_indices);
                std::set<QModelIndex> source_indices_set(source_indices.begin(), source_indices.end());

                rg_isa_tree_view_->SetHotSpotLineNumbers(source_indices_set);
            }

            // Update current key for the isa disassembly view data.
            current_disassembly_view_data_key_ = entrypoint_name_key;
        }
    }

    // Get a reference to the map of input file path to the entry point names map.
    InputToEntrypointViews& input_file_to_entrypoint_map = gpu_resource_usage_views_[target_gpu];

    // Use the input file path to get a reference to a map of entry point names to resource usage views.
    EntrypointToResourcesView& entrypoint_map = input_file_to_entrypoint_map[input_file_path];

    // Search the map to find the resource usage view associated with the given entrypoint.
    auto resource_view_iter = entrypoint_map.find(selected_entrypoint_name);
    if (resource_view_iter != entrypoint_map.end())
    {
        // Display the resource usage view associated with the given entrypoint.
        RgResourceUsageView* resource_usage_view = resource_view_iter->second;
        ui_.resourceUsageHostStackedWidget->setCurrentWidget(resource_usage_view);
    }
}

void RgIsaDisassemblyView::EnableShowMaxVgprContextOption() const
{
    emit EnableShowMaxVgprOptionSignal(IsMaxVgprColumnVisible());
}

void RgIsaDisassemblyView::SetFocusOnGoToLineWidget()
{
    if (ui_.disassemblyHostWidget != nullptr && rg_isa_widget_ != nullptr)
    {
        auto current_widget = ui_.disassemblyHostWidget->currentWidget();
        if (current_widget == rg_isa_widget_)
        {
            rg_isa_widget_->SetFocusOnGoToLineWidget();
        }
    }
}

void RgIsaDisassemblyView::SetFocusOnSearchWidget()
{
    if (ui_.disassemblyHostWidget != nullptr && rg_isa_widget_ != nullptr)
    {
        auto current_widget = ui_.disassemblyHostWidget->currentWidget();
        if (current_widget == rg_isa_widget_)
        {
            rg_isa_widget_->SetFocusOnSearchWidget();
        }
    }
}

RgIsaTreeView* RgIsaDisassemblyView::GetTreeView() const
{
    return rg_isa_tree_view_;
}

bool RgIsaDisassemblyView::IsLineCorrelationSupported() const
{
    return false;
}

void RgIsaDisassemblyView::HandleTargetGpuChanged()
{
    if (ui_.targetGpuPushButton != nullptr)
    {
        std::string newTargetGpu = ui_.targetGpuPushButton->SelectedText().toStdString();

        // Strip the GPU name from the architecture if needed.
        std::string strippedGpuName;
        size_t      bracketPos = newTargetGpu.find("(");
        if (bracketPos != std::string::npos && newTargetGpu.size() > 2)
        {
            strippedGpuName = newTargetGpu.substr(0, bracketPos - 1);
        }
        else
        {
            strippedGpuName = newTargetGpu;
        }

        // Strip gfx notation if needed.
        size_t slashPos = strippedGpuName.find("/");
        if (slashPos != std::string::npos)
        {
            strippedGpuName = strippedGpuName.substr(slashPos + 1);
        }

        // Emit a signal with the name of the target GPU to switch to.
        emit SelectedTargetGpuChanged(strippedGpuName);
    }

    // Set focus to disassembly view.
    setFocus();
}


void RgIsaDisassemblyView::CreateIsaTreeView(QWidget* disassembly_view_parent)
{
    // Get global config to see which columns should be visible initially.
    RgConfigManager&                  configManager   = RgConfigManager::Instance();
    std::shared_ptr<RgGlobalSettings> global_settings = configManager.GetGlobalConfig();

    auto              visible_disassembly_view_columns = global_settings->visible_disassembly_view_columns;
    std::vector<bool> column_visiblity                 = {true};
    for (auto column : visible_disassembly_view_columns)
    {
        column_visiblity.push_back(column);
    }

    rg_isa_widget_ = new IsaWidget(ui_.disassemblyHostWidget);

    rg_isa_proxy_model_ = new RgIsaProxyModel(rg_isa_widget_, column_visiblity);

    rg_isa_item_model_ = new RgIsaItemModel(rg_isa_widget_);

    rg_isa_tree_view_ = new RgIsaTreeView(rg_isa_widget_, this);

    // Pass a pointer to the disassembly view's parent widget to the nav widget so it can render its combo box correctly.
    rg_isa_widget_->SetModelAndView(disassembly_view_parent, rg_isa_item_model_, rg_isa_tree_view_, rg_isa_proxy_model_);

    rg_isa_item_model_->SetFixedFont(rg_isa_tree_view_->font(), rg_isa_tree_view_);
    
    ui_.disassemblyHostWidget->addWidget(rg_isa_widget_);
    ui_.disassemblyHostWidget->setCurrentWidget(rg_isa_widget_);
}

void RgIsaDisassemblyView::ConnectSignals()
{
    // Connect the handler to give focus to frame on view maximize button click.
    bool is_connected = connect(ui_.viewMaximizeButton, &QPushButton::clicked, this, &RgIsaDisassemblyView::HandleDisassemblyViewClicked);
    assert(is_connected);

    // Connect the handler to give focus to frame on target GPUs list widget's gain of focus.
    is_connected = connect(ui_.targetGpuPushButton, &ArrowIconComboBox::FocusInEvent, this, &RgIsaDisassemblyView::HandleListWidgetFocusInEvent);
    assert(is_connected);

    // Connect the handler to give focus to frame on target GPUs list widget's loss of focus.
    is_connected = connect(ui_.targetGpuPushButton, &ArrowIconComboBox::FocusOutEvent, this, &RgIsaDisassemblyView::HandleListWidgetFocusOutEvent);
    assert(is_connected);

    // Connect the handler to give focus to frame on columns push button click.
    is_connected = connect(ui_.rawTextPushButton, &QPushButton::clicked, this, &RgIsaDisassemblyView::HandleRawTextButtonClicked);
    assert(is_connected);

    // Connect the handler to give focus to frame on columns push button click.
    is_connected = connect(ui_.rawTextPushButton, &QPushButton::clicked, this, &RgIsaDisassemblyView::HandleDisassemblyViewClicked);
    assert(is_connected);

    // Connect the handler to give focus to frame on target GPUs push button click.
    is_connected = connect(ui_.targetGpuPushButton, &QPushButton::clicked, this, &RgIsaDisassemblyView::HandleDisassemblyViewClicked);
    assert(is_connected);

    // Connect the handler to give focus to frame on target GPUs push button loss of focus.
    is_connected = connect(ui_.targetGpuPushButton, &ArrowIconComboBox::FocusOutEvent, this, &RgIsaDisassemblyView::HandleListWidgetFocusOutEvent);
    assert(is_connected);

    // Select next GPU device action.
    select_next_gpu_target_ = new QAction(this);
    select_next_gpu_target_->setShortcutContext(Qt::ApplicationShortcut);
    select_next_gpu_target_->setShortcut(QKeySequence(kDisassemblyViewHotkeyGpuSelection));
    addAction(select_next_gpu_target_);

    // Connect the handler to process the hot key press.
    is_connected = connect(select_next_gpu_target_, &QAction::triggered, this, &RgIsaDisassemblyView::HandleSelectNextGPUTargetAction);
    assert(is_connected);

    // Select next max VGPR line.
    select_next_max_vgpr_line_ = new QAction(this);
    select_next_max_vgpr_line_->setShortcutContext(Qt::ApplicationShortcut);
    select_next_max_vgpr_line_->setShortcut(QKeySequence(kDisassemblyViewHotKeyNextMaxVgprLine));
    addAction(select_next_max_vgpr_line_);

    // Connect the handler to process the hot key press.
    is_connected = connect(select_next_max_vgpr_line_, &QAction::triggered, this, &RgIsaDisassemblyView::ShowNextMaxVgprClickedSignal);
    assert(is_connected);

    // Select previous max VGPR line.
    select_previous_max_vgpr_line_ = new QAction(this);
    select_previous_max_vgpr_line_->setShortcutContext(Qt::ApplicationShortcut);
    select_previous_max_vgpr_line_->setShortcut(QKeySequence(kDisassemblyViewHotKeyPreviousMaxVgprLine));
    addAction(select_previous_max_vgpr_line_);

    // Connect the handler to process the hot key press.
    is_connected = connect(select_previous_max_vgpr_line_, &QAction::triggered, this, &RgIsaDisassemblyView::ShowPrevMaxVgprClickedSignal);
    assert(is_connected);
}

void RgIsaDisassemblyView::ConnectIsaTreeViewSignals()
{
    // Connect the handler to ope disassembly file in explorer.
    bool is_connected =
        connect(rg_isa_tree_view_, &RgIsaTreeView::OpenDisassemblyInFileBrowserSignal, this, &RgIsaDisassemblyView::HandleOpenDisassemblyInFileBrowser);
    assert(is_connected);

    // Connect the enable show max VGPR options signal.
    is_connected = connect(rg_isa_proxy_model_, &RgIsaProxyModel::EnableShowMaxVgprOptionSignal, this, &RgIsaDisassemblyView::EnableShowMaxVgprOptionSignal);
    assert(is_connected);

    is_connected = connect(this, &RgIsaDisassemblyView::EnableShowMaxVgprOptionSignal, rg_isa_tree_view_, &RgIsaTreeView::HandleEnableShowMaxVgprOptionSignal);
    assert(is_connected);
}

void RgIsaDisassemblyView::CreateKernelNameLabel()
{
    ui_.horizontalSpacer_2->changeSize(0, 0);
    HandleSetKernelNameLabel(false);
}

void RgIsaDisassemblyView::CreateTargetGpuListControls()
{
    // Setup the list widget used to select the current target GPU.
    ui_.targetGpuPushButton->InitSingleSelect(this, "Target GPU", false);

    // Connect the signal used to handle a change in the selected target GPU.
    [[maybe_unused]] bool is_connected =
        connect(ui_.targetGpuPushButton, &ArrowIconComboBox::SelectionChanged, this, &RgIsaDisassemblyView::HandleTargetGpuChanged);
    assert(is_connected);
}

void RgIsaDisassemblyView::HandleOpenGpuListWidget()
{
    ui_.targetGpuPushButton->clicked();
}

void RgIsaDisassemblyView::PopulateTargetGpuList(const RgBuildOutputsMap& build_output)
{
    // Get a mapping of the compute capability to architecture.
    std::map<std::string, std::string> compute_capability_to_arch;
    bool                               has_arch_mapping = RgUtils::GetComputeCapabilityToArchMapping(compute_capability_to_arch);

    // Block signals to stop updates when each new GPU is added to the list.
    ui_.targetGpuPushButton->blockSignals(true);

    ui_.targetGpuPushButton->ClearItems();

    std::vector<std::string> targets;

    for (auto target_gpu_iter = build_output.rbegin(); target_gpu_iter != build_output.rend(); ++target_gpu_iter)
    {
        if (target_gpu_iter->second != nullptr)
        {
            // Add each target GPU from the build outputs to the dropdown list.
            auto target_gpu = target_gpu_iter->first;

            // Construct the presented name.
            std::stringstream presented_name;

            // If applicable, prepend the gfx notation (for example, "gfx802/Tonga" for "Tonga").
            std::string gfx_notation;
            bool        has_gfx_notation = RgUtils::GetGfxNotation(target_gpu, gfx_notation);
            if (has_gfx_notation && !gfx_notation.empty())
            {
                presented_name << gfx_notation << "/";
            }
            presented_name << target_gpu;

            // If we have a mapping, let's construct a name that includes
            // the GPU architecture as well: <compute capability> (<architecture>).
            if (has_arch_mapping)
            {
                auto iter = compute_capability_to_arch.find(target_gpu);
                if (iter != compute_capability_to_arch.end())
                {
                    presented_name << " (" << iter->second << ")";
                }
            }

            // Add the name to the list.
            targets.push_back(presented_name.str());
        }
    }
    assert(!targets.empty());

    // Sort and choose the latest target.
    std::sort(targets.begin(), targets.end(), [&](const std::string& a, const std::string& b) {
        const char* kGfxNotationToken = "gfx";
        bool        ret               = true;
        size_t      sz_a              = a.find(kGfxNotationToken);
        size_t      sz_b              = b.find(kGfxNotationToken);
        if (sz_a == std::string::npos && sz_b == std::string::npos)
        {
            // Neither name is in gfx-notation, compare using standard string logic.
            ret = a.compare(b) < 0;
        }
        else if (!(sz_a != std::string::npos && sz_b != std::string::npos))
        {
            // Only one name has the gfx notation, assume that it is a newer generation.
            ret = (sz_b != std::string::npos);
        }
        else
        {
            // Both names are in gfx notation, compare according to the number.
            std::vector<std::string> split_a;
            std::vector<std::string> split_b;
            RgUtils::splitString(a, 'x', split_a);
            RgUtils::splitString(b, 'x', split_b);
            assert(split_a.size() > 1);
            assert(split_b.size() > 1);
            if (split_a.size() > 1 && split_b.size() > 1)
            {
                try
                {
                    int numA = std::stoi(split_a[1], nullptr, 16);
                    int numB = std::stoi(split_b[1], nullptr, 16);
                    ret      = ((numB - numA) > 0);
                }
                catch (...)
                {
                    ret = false;
                }
            }
        }
        return !ret;
    });

    for (const std::string& str : targets)
    {
        ui_.targetGpuPushButton->AddItem(str.c_str());
    }

    // Switch to the first target GPU.
    ui_.targetGpuPushButton->SetSelectedRow(0);

    // Re-enable signals emitted from the target GPU list.
    ui_.targetGpuPushButton->blockSignals(false);

    // Set the default target GPU.
    HandleTargetGpuChanged();
}

bool RgIsaDisassemblyView::PopulateDisassemblyEntries(const GpuToEntryVector& gpu_to_disassembly_csv_entries)
{
    bool ret = true;

    if (!gpu_to_disassembly_csv_entries.empty())
    {
        // Step through each GPU and insert a new RgIsaDisassemblyTabView into a new tab.
        for (auto gpu_entry_iter = gpu_to_disassembly_csv_entries.begin(); gpu_entry_iter != gpu_to_disassembly_csv_entries.end(); ++gpu_entry_iter)
        {
            const std::string&                gpu_name    = gpu_entry_iter->first;
            const std::vector<RgEntryOutput>& gpu_entries = gpu_entry_iter->second;
            for (const RgEntryOutput& entry : gpu_entries)
            {
                OutputFileTypeFinder output_file_type_searcher(RgCliOutputFileType::kIsaDisassemblyCsv);
                auto                 csv_file_iter         = std::find_if(entry.outputs.begin(), entry.outputs.end(), output_file_type_searcher);
                output_file_type_searcher.target_file_type = RgCliOutputFileType::kLiveRegisterAnalysisReport;
                auto live_Vgprs_file_iter                  = std::find_if(entry.outputs.begin(), entry.outputs.end(), output_file_type_searcher);
                if (csv_file_iter != entry.outputs.end() && live_Vgprs_file_iter != entry.outputs.end())
                {
                    // Get the path to the disassembly CSV file to load into the model.
                    std::string disassembly_csv_file_path = csv_file_iter->file_path;

                    // Get the path to the live Vgprs file to load into the model.
                    const std::string& live_vgprs_file_path = live_Vgprs_file_iter->file_path;

                    // Generate a key string used to identify a named entry point within a given input source file.
                    std::string entrypoint_name_key = GenerateEntrypointKey(entry.input_file_path, gpu_name, entry.entrypoint_name);

                    // Associate the kernel name with the data for the disassembly view.
                    disassembly_view_input_files_map_[entrypoint_name_key] = {disassembly_csv_file_path, live_vgprs_file_path};
                }
            }
        }
    }

    if (disassembly_view_input_files_map_.empty())
    {
        ret = false;
    }

    return ret;
}

bool RgIsaDisassemblyView::PopulateResourceUsageEntries(const GpuToEntryVector& gpu_to_resource_usage_csv_entries)
{
    bool is_load_failed = false;

    if (!gpu_to_resource_usage_csv_entries.empty())
    {
        // Step through each GPU and insert a new RgResourceView underneath the disassembly table.
        for (auto gpu_entry_iter = gpu_to_resource_usage_csv_entries.begin(); gpu_entry_iter != gpu_to_resource_usage_csv_entries.end(); ++gpu_entry_iter)
        {
            const std::string&                gpu_name    = gpu_entry_iter->first;
            const std::vector<RgEntryOutput>& gpu_entries = gpu_entry_iter->second;

            // Create a resource usage disassembly table for each entry, and add it to the layout.
            // Only a single entry point table will be visible at a time, and the user can switch between the current entry.
            for (const RgEntryOutput& entry : gpu_entries)
            {
                OutputFileTypeFinder outputFileTypeSearcher(RgCliOutputFileType::kHwResourceUsageFile);
                auto                 csv_file_iter = std::find_if(entry.outputs.begin(), entry.outputs.end(), outputFileTypeSearcher);
                if (csv_file_iter != entry.outputs.end())
                {
                    // Create a CSV parser to read the resource usage file.
                    RgResourceUsageCsvFileParser resource_usage_file_parser(csv_file_iter->file_path);

                    // Attempt to parse the file.
                    std::string parse_error_string;
                    bool        parsed_successfully = resource_usage_file_parser.Parse(parse_error_string);
                    if (parsed_successfully)
                    {
                        // Extract the parsed data, and populate the resource usage view.
                        const RgResourceUsageData& resource_usage_data = resource_usage_file_parser.GetData();
                        RgResourceUsageView*       resource_usage_view = new RgResourceUsageView();
                        resource_usage_view->PopulateView(resource_usage_data);
                        resource_usage_text_ = resource_usage_view->GetResourceUsageText();
                        resource_usage_font_ = resource_usage_view->GetResourceUsageFont();

                        // Connect resource usage view signals.
                        ConnectResourceUsageViewSignals(resource_usage_view);

                        // Add the new resource usage view to the host widget.
                        ui_.resourceUsageHostStackedWidget->addWidget(resource_usage_view);
                        ui_.resourceUsageHostStackedWidget->setContentsMargins(0, 10, 0, 0);

                        // Get a reference to the entry point views associated with the parsed device.
                        InputToEntrypointViews& input_file_to_entrypoint_map = gpu_resource_usage_views_[gpu_name];

                        // Get a reference to the resource views map for the source file.
                        EntrypointToResourcesView& entrypoint_map = input_file_to_entrypoint_map[entry.input_file_path];

                        // Associate the entrypoint's name with the new RgResourceView.
                        entrypoint_map[entry.entrypoint_name] = resource_usage_view;
                    }
                    else
                    {
                        is_load_failed = true;
                    }
                }
            }
        }
    }

    return !is_load_failed;
}

void RgIsaDisassemblyView::ConnectResourceUsageViewSignals(RgResourceUsageView* resource_usage_view)
{
    // Connect to the resource usage view's mouse press event.
    bool is_connected =
        connect(resource_usage_view, &RgResourceUsageView::ResourceUsageViewClickedSignal, this, &RgIsaDisassemblyView::HandleDisassemblyViewClicked);
    assert(is_connected);

    // Connect to the resource usage view's focus out event.
    is_connected = connect(
        resource_usage_view, &RgResourceUsageView::ResourceUsageViewFocusOutEventSignal, this, &RgIsaDisassemblyView::HandleResourceUsageViewFocusOutEvent);
    assert(is_connected);
}

void RgIsaDisassemblyView::HandleSetKernelNameLabel(bool show, const std::string& setLabelText)
{
    if (show)
    {
        if (!setLabelText.empty())
        {
            std::stringstream ss;
            ss << "Kernel Name: ";
            ss << setLabelText;
            ui_.kernelNameLabel->setText(ss.str().c_str());
        }
        ui_.kernelNameLabel->show();
    }
    else
    {
        ui_.kernelNameLabel->hide();
    }
}

void RgIsaDisassemblyView::HandleRawTextButtonClicked()
{
}

void RgIsaDisassemblyView::SetCheckBoxFocusProxies(QCheckBox* check_box)
{
    assert(check_box != nullptr);

    check_box->setFocusProxy(ui_.frame);
}

void RgIsaDisassemblyView::DestroyDisassemblyViewDataForFile(const std::string& input_file_path)
{
    // Keep a list of entry data that should be destroyed after removing the input file.
    std::vector<std::string> view_data_to_remove;
    
    // Step through each entry and try to remove the entries associated with the given input file.
    
    auto start = disassembly_view_input_files_map_.begin();
    auto end   = disassembly_view_input_files_map_.end();
    for (auto iter = start; iter != end; ++iter)
    {
        // Search the map for entries to remove.
        std::string file_path, gpu, entry_name;
        DecodeEntrypointKey(iter->first, file_path, gpu, entry_name);
        if (file_path == input_file_path)
        {
            view_data_to_remove.push_back(iter->first);
        }
    }

    // Destroy all tabs that were marked for destruction.
    for (const auto& entrypoint_name_key : view_data_to_remove)
    {
        auto iter = disassembly_view_input_files_map_.find(entrypoint_name_key);
        if (iter != disassembly_view_input_files_map_.end())
        {
            // Mark Entry for evition from the cache.
            RgIsaItemModel::EntryData entry_data{};
            entry_data.isa_file_path  = iter->second.first;
            entry_data.vgpr_file_path = iter->second.second;
            entry_data.operation      = RgIsaItemModel::EntryData::Operation::kEvictData;
            rg_isa_item_model_->UpdateData(&entry_data);

            disassembly_view_input_files_map_.erase(iter);
        }
    }
}

void RgIsaDisassemblyView::DestroyResourceUsageViewsForFile(const std::string& input_file_path)
{
    // Destroy resource views for all GPUs.
    for (auto gpu_iter = gpu_resource_usage_views_.begin(); gpu_iter != gpu_resource_usage_views_.end(); ++gpu_iter)
    {
        // Find all resource views related to the given input file.
        InputToEntrypointViews& input_file_to_views_iter            = gpu_iter->second;
        auto                    entrypoint_resource_usageviews_iter = input_file_to_views_iter.find(input_file_path);
        if (entrypoint_resource_usageviews_iter != input_file_to_views_iter.end())
        {
            // Step through each entrypoint's resource usage view, and destroy it.
            EntrypointToResourcesView& entrypoint_resource_views = entrypoint_resource_usageviews_iter->second;
            for (auto entrypoint_iter = entrypoint_resource_views.begin(); entrypoint_iter != entrypoint_resource_views.end(); ++entrypoint_iter)
            {
                // Destroy the resource usage view.
                RgResourceUsageView* resource_usage_view = entrypoint_iter->second;

                // Remove the resource usage widget from the view.
                ui_.resourceUsageHostStackedWidget->removeWidget(resource_usage_view);
            }

            input_file_to_views_iter.erase(entrypoint_resource_usageviews_iter);
        }
    }
}

void RgIsaDisassemblyView::SetCurrentResourceUsageView(RgResourceUsageView* resource_usage_view)
{
    // Set the current widget in the stack.
    ui_.resourceUsageHostStackedWidget->setCurrentWidget(resource_usage_view);

    // Use the given resource usage view as the focus proxy for this view.
    setFocusProxy(resource_usage_view);
}

void RgIsaDisassemblyView::SetCursor()
{
    ui_.rawTextPushButton->setCursor(Qt::PointingHandCursor);
    ui_.targetGpuPushButton->setCursor(Qt::PointingHandCursor);
    ui_.viewMaximizeButton->setCursor(Qt::PointingHandCursor);
}

std::string RgIsaDisassemblyView::GenerateEntrypointKey(const std::string& file_path, const std::string& asic, const std::string& entrypoint_name) const
{
    // In some cases it may not be possible to identify a given entry point by name when multiple
    // entrypoints use the same name. Return a unique key based on the input filename and the
    // entry point name, so that each entry point can be identified correctly.
    return file_path + kEntrypointKeySeparator + asic + kEntrypointKeySeparator + entrypoint_name;
}

bool RgIsaDisassemblyView::DecodeEntrypointKey(const std::string& entrypoint_key,
                                               std::string&       file_path,
                                               std::string&       asic,
                                               std::string&       entrypoint_name) const
{
    bool ret = false;

    // Attempt to split the given entry point key string into source file path and entry point name strings.
    std::vector<std::string> file_path_and_entrypoint_name_list;
    RgUtils::splitString(entrypoint_key, kEntrypointKeySeparator, file_path_and_entrypoint_name_list);

    // Verify that only 2 tokens are found. One for the source file path, and another for entry point name.
    size_t token_count = file_path_and_entrypoint_name_list.size();
    assert(token_count == 3);
    if (token_count == 3)
    {
        file_path       = file_path_and_entrypoint_name_list[0];
        asic            = file_path_and_entrypoint_name_list[1];
        entrypoint_name = file_path_and_entrypoint_name_list[2];
        ret             = true;
    }

    return ret;
}

void RgIsaDisassemblyView::HandleDisassemblyViewClicked()
{
    // Emit a signal to indicate that disassembly view was clicked.
    emit DisassemblyViewClicked();

    // Highlight the frame in the correct API color (give it focus).
    SetBorderStylesheet(true);

    // Remove the button focus in the file menu.
    emit RemoveFileMenuButtonFocus();
}

void RgIsaDisassemblyView::HandleDisassemblyTabViewLostFocus()
{
    // Set the border color to black.
    SetBorderStylesheet(false);

    // Remove the button focus in the file menu.
    emit RemoveFileMenuButtonFocus();
}

void RgIsaDisassemblyView::HandleResourceUsageViewFocusOutEvent()
{
    // Set the border color to black.
    SetBorderStylesheet(false);

    // Remove the button focus in the file menu.
    emit RemoveFileMenuButtonFocus();
}

void RgIsaDisassemblyView::HandleTitlebarClickedEvent(QMouseEvent* event)
{
    Q_UNUSED(event);

    // Emit a signal to indicate that disassembly view was clicked.
    emit DisassemblyViewClicked();

    // Highlight the frame in the correct API color (give it focus).
    SetBorderStylesheet(true);

    // Remove the button focus in the file menu.
    emit RemoveFileMenuButtonFocus();
}

void RgIsaDisassemblyView::HandleListWidgetFocusInEvent()
{
    // Emit a signal to indicate that disassembly view was clicked.
    emit DisassemblyViewClicked();

    // Highlight the frame in the correct API color (give it focus).
    SetBorderStylesheet(true);

    // Remove the button focus in the file menu.
    emit RemoveFileMenuButtonFocus();
}

void RgIsaDisassemblyView::HandleListWidgetFocusOutEvent()
{
    // Set the border color to black.
    SetBorderStylesheet(false);
}

void RgIsaDisassemblyView::HandleFocusOutEvent()
{
    // Set the border color to black.
    SetBorderStylesheet(false);

    // Remove the button focus in the file menu.
    emit RemoveFileMenuButtonFocus();
}

void RgIsaDisassemblyView::HandleFocusTargetGpuPushButton()
{
    ui_.targetGpuPushButton->clicked(false);
}

void RgIsaDisassemblyView::HandleOpenDisassemblyInFileBrowser()
{
    // Search the map to find the relevant view data associated with the current entrypoint.
    auto view_data_iter = disassembly_view_input_files_map_.find(current_disassembly_view_data_key_);
    if (view_data_iter != disassembly_view_input_files_map_.end())
    {
        // Set the data model associated with the given entrypoint.
        const auto& disassembly_file_path = view_data_iter->second.first;
        // Use the path to the loaded CSV file to figure out which folder to open.
        std::string build_output_directory;
        bool        is_ok = RgUtils::ExtractFileDirectory(disassembly_file_path, build_output_directory);
        assert(is_ok);
        if (is_ok)
        {
            // Open the directory in the system's file browser.
            RgUtils::OpenFolderInFileBrowser(build_output_directory);
        }
    }
}

void RgIsaDisassemblyView::HandleFocusRawTextDisassemblyPushButton()
{
    ui_.rawTextPushButton->clicked(false);
}

void RgIsaDisassemblyView::ConnectTitleBarDoubleClick(const RgViewContainer* disassembly_view_container)
{
    assert(disassembly_view_container != nullptr);
    if (disassembly_view_container != nullptr)
    {
        [[maybe_unused]] bool is_connected = connect(ui_.viewTitlebar,
                                                     &RgIsaDisassemblyViewTitlebar::ViewTitleBarDoubleClickedSignal,
                                                     disassembly_view_container,
                                                     &RgViewContainer::MaximizeButtonClicked);
        assert(is_connected);
    }
}

bool RgIsaDisassemblyView::ReplaceInputFilePath(const std::string& old_file_path, const std::string& new_file_path)
{
    bool result = true;

    // Replace the file path in the resource usage map.
    if (result)
    {
        for (auto& gpu_and_resource_usage : gpu_resource_usage_views_)
        {
            auto& file_and_resource_usage = gpu_and_resource_usage.second;
            auto  it                      = file_and_resource_usage.find(old_file_path);
            if ((result = (it != file_and_resource_usage.end())) == true)
            {
                std::map<std::string, RgResourceUsageView*> resUsageView = it->second;
                file_and_resource_usage.erase(old_file_path);
                file_and_resource_usage[new_file_path] = resUsageView;
            }
        }
    }

    return result;
}

void RgIsaDisassemblyView::HandleSelectNextGPUTargetAction()
{
    int current_row = ui_.targetGpuPushButton->CurrentRow();

    if (current_row < ui_.targetGpuPushButton->RowCount() - 1)
    {
        current_row++;
    }
    else
    {
        current_row = 0;
    }

    ui_.targetGpuPushButton->SetSelectedRow(current_row);
}

bool RgIsaDisassemblyView::IsMaxVgprColumnVisible() const
{
    bool is_visible = false;
    if (rg_isa_proxy_model_ != nullptr)
    {
        is_visible = rg_isa_proxy_model_->IsVgprColumnVisible();
    }
    return is_visible;
}
