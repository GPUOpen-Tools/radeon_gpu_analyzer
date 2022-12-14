// C++.
#include <cassert>
#include <sstream>

// Infra.
#include "QtCommon/Scaling/ScalingManager.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_tab_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_table_view.h"
#include "radeon_gpu_analyzer_gui/rg_data_types.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"

// A separator used in joining an input source file path with an entry point name.
// Such a string is used to uniquely identify an entry point with potential for
// name collisions between separate source files.
static const char kEntrypointKeySeparator = '|';

RgIsaDisassemblyTabView::RgIsaDisassemblyTabView(QWidget* parent) :
    QWidget(parent)
{
    ui_.setupUi(this);
}

void RgIsaDisassemblyTabView::ClearCorrelationHighlight()
{
    if (current_table_ != nullptr)
    {
        // Passing an invalid line index to the table model will clear the correlated line.
        current_table_->UpdateCorrelatedSourceFileLine(kInvalidCorrelationLineIndex);
    }
}

bool RgIsaDisassemblyTabView::GetCurrentEntrypoint(std::string& current_entrypoint) const
{
    bool ret = false;

    auto table_iter = disassembly_table_view_to_entrypoint_.find(current_table_);
    if (table_iter != disassembly_table_view_to_entrypoint_.end())
    {
        current_entrypoint = table_iter->second;
        ret = true;
    }

    return ret;
}

int RgIsaDisassemblyTabView::GetTableCount() const
{
    return static_cast<int>(entry_point_disassembly_table_views_.size());
}

bool RgIsaDisassemblyTabView::PopulateEntries(const std::vector<RgEntryOutput>& disassembled_entries)
{
    bool ret = true;

    // Create a disassembly table for each entry, and add it to the layout.
    // Only a single entry point table will be visible at a time, and the user can switch between the current entry.
    for (const RgEntryOutput& entry : disassembled_entries)
    {
        RgIsaDisassemblyTableView* table_view = nullptr;
        OutputFileTypeFinder output_file_type_searcher(RgCliOutputFileType::kIsaDisassemblyCsv);
        auto csv_file_iter = std::find_if(entry.outputs.begin(), entry.outputs.end(), output_file_type_searcher);
        if (csv_file_iter != entry.outputs.end())
        {
            // Create a new disassembly table view for each entry point.
            table_view = new RgIsaDisassemblyTableView(this);

            // Get the path to the disassembly CSV file to load into the table.
            const std::string& disassembly_csv_file_path = csv_file_iter->file_path;

            // Set the ISA CSV file path for the table.
            table_view->SetDisassemblyFilePath(disassembly_csv_file_path);

            // Connect signals for the new table view.
            ConnectTableViewSignals(table_view);

            // Add the table to the tab's grid.
            ui_.tableGrid->addWidget(table_view);

            // Register the table with the scaling manager.
            ScalingManager::Get().RegisterObject(table_view);

            // Hide the table initially, as only a single table can be shown at once.
            table_view->hide();

            // Generate a key string used to identify a named entry point within a given input source file.
            std::string entrypoint_name_key = GenerateEntrypointKey(entry.input_file_path, entry.entrypoint_name);

            // Associate the kernel name with the table showing the disassembly.
            entry_point_disassembly_table_views_[entrypoint_name_key] = table_view;

            // Also associate the disassembly table with the entry point it's displaying data for.
            disassembly_table_view_to_entrypoint_[table_view] = entrypoint_name_key;

            // Add the new table to the list of tables associated with the input file.
            std::vector<RgIsaDisassemblyTableView*>& table_list = input_file_to_isa_table_list_[entry.input_file_path];
            table_list.push_back(table_view);
        }

        // Find and set the Live Vgprs file.
        output_file_type_searcher.target_file_type = RgCliOutputFileType::kLiveRegisterAnalysisReport;
        auto live_Vgprs_file_iter                  = std::find_if(entry.outputs.begin(), entry.outputs.end(), output_file_type_searcher);
        if (live_Vgprs_file_iter != entry.outputs.end())
        {
            // Get the path to the live Vgprs file to load into the table.
            const std::string& live_vgprs_file_path = live_Vgprs_file_iter->file_path;

            if (table_view == nullptr)
            {
                table_view = new RgIsaDisassemblyTableView(this);
            }

            // Set the Live Vgprs file path for the table.
            table_view->SetLiveVgprsFilePath(live_vgprs_file_path);
        }
    }

    return ret;
}

void RgIsaDisassemblyTabView::RemoveInputFileEntries(const std::string& input_file_path)
{
    auto results_iter = input_file_to_isa_table_list_.find(input_file_path);
    if (results_iter != input_file_to_isa_table_list_.end())
    {
        // We found a list of table views that are used to display disassembly for the given input file.
        std::vector<RgIsaDisassemblyTableView*>& entry_tables = results_iter->second;

        // Step through each table and destroy it.
        for (RgIsaDisassemblyTableView* table_view : entry_tables)
        {
            // Remove the references to the table from the entry point name to ISA table map.
            RemoveEntrypointTable(table_view);

            // Remove the table from the view.
            ui_.tableGrid->removeWidget(table_view);
        }

        // Remove the entry from the input file path to ISA table map.
        auto file_path_to_table_iter = input_file_to_isa_table_list_.find(input_file_path);
        if (file_path_to_table_iter != input_file_to_isa_table_list_.end())
        {
            input_file_to_isa_table_list_.erase(file_path_to_table_iter);
        }
    }
}

void RgIsaDisassemblyTabView::SwitchToEntryPoint(const std::string& input_file_path, const std::string& entrypoint_name)
{
    // Generate a key string used to identify a named entry point within a given input source file.
    std::string entrypoint_name_key = GenerateEntrypointKey(input_file_path, entrypoint_name);

    // Find the disassembly table corresponding to the given entry point name.
    auto entry_iter = entry_point_disassembly_table_views_.find(entrypoint_name_key);
    if (entry_iter != entry_point_disassembly_table_views_.end())
    {
        RgIsaDisassemblyTableView* table_view = entry_iter->second;
        assert(table_view != nullptr);

        if (table_view != nullptr && table_view != current_table_)
        {
            // Do not load the table if it was loaded already.
            bool is_table_cached = table_view->IsDisassemblyLoaded();
            bool is_table_loaded = false;

            if (!is_table_cached)
            {
                // Attempt to load the table.
                // Get the disassembly file path.
                const std::string& disassembly_file_path = table_view->GetDisassemblyFilePath();
                assert(!disassembly_file_path.empty());

                if (!disassembly_file_path.empty())
                {
                    is_table_loaded = table_view->LoadDisassembly(disassembly_file_path);
                    assert(is_table_loaded);
                    if (!is_table_loaded)
                    {
                        // Tell the user that the disassembly file failed to load.
                        std::stringstream error_string;
                        error_string << kStrErrCannotLoadDisassemblyCsvFile;
                        RgUtils::ShowErrorMessageBox(error_string.str().c_str(), this);
                    }
                }

                // Get the live Vgprs file path.
                const std::string& live_vgprs_file_path = table_view->GetLiveVgprsFilePath();
                bool               is_live_vgpr_loaded  = false;
                if (!live_vgprs_file_path.empty())
                {
                    is_live_vgpr_loaded = table_view->LoadLiveVgpr(live_vgprs_file_path);
                    assert(is_live_vgpr_loaded);
                    if (!is_live_vgpr_loaded)
                    {
                        // Tell the user that the live VGPR file failed to load.
                        std::stringstream error_string;
                        error_string << kStrErrCannotLoadLiveVgprFile;
                        RgUtils::ShowErrorMessageBox(error_string.str().c_str(), this);
                    }
                }

                if (is_table_loaded)
                {
                    table_view->InitializeModelData();
                }
            }

            // Hide the currently-visible disassembly table.
            if (current_table_ != nullptr)
            {
                current_table_->hide();
            }

            // Show the disassembly table that we're switching to.
            table_view->show();

            // Update the current table.
            current_table_ = table_view;

            // Use the current table view as the focus proxy for this view.
            setFocusProxy(table_view);

            // Filter the table we are about to show, without resizing
            // since the table is inheriting the filter from its predecessor.
            table_view->UpdateFilteredTable();
        }
    }
}

void RgIsaDisassemblyTabView::UpdateCorrelatedSourceFileLine(const std::string& input_file_path, int line_number, std::string& entry_name)
    {
    if (!entry_name.empty())
    {
        // Switch to view the disassembly table for the named entrypoint.
        SwitchToEntryPoint(input_file_path, entry_name);
    }

    // Find the table used to present the given entrypoint's disassembly.
    std::string entrypoint_name_key = GenerateEntrypointKey(input_file_path, entry_name);
    auto entrypoint_to_table_iter = entry_point_disassembly_table_views_.find(entrypoint_name_key);
    if (entrypoint_to_table_iter != entry_point_disassembly_table_views_.end())
    {
        // Update the currently selected source line number.
        RgIsaDisassemblyTableView* entrypoint_table = entrypoint_to_table_iter->second;
        entrypoint_table->UpdateCorrelatedSourceFileLine(line_number);
    }
}

bool RgIsaDisassemblyTabView::IsSourceLineCorrelatedForEntry(const std::string& input_file_path, const std::string& entry_name, int line_number)
{
    bool  ret = false;

    // Generate a key string used to identify a named entry point within a given input source file.
    std::string entrypoint_name_key = GenerateEntrypointKey(input_file_path, entry_name);

    const auto& isa_table_view = entry_point_disassembly_table_views_.find(entrypoint_name_key);
    if (isa_table_view != entry_point_disassembly_table_views_.end())
    {
        ret = isa_table_view->second->IsSourceLineCorrelated(line_number);
    }
    return ret;
}

bool RgIsaDisassemblyTabView::ReplaceInputFilePath(const std::string& old_file_path, const std::string& new_file_path)
{
    assert(!old_file_path.empty());
    assert(!new_file_path.empty());
    bool ret = (!old_file_path.empty() && !new_file_path.empty());

    if (ret)
    {
        // 1. Replace the file path in the "entry point --> disasm table view" map.
        // There may be multiple matching elements since the keys in this map
        // consist of the file path and the entry name: KEY = "file-path|entry-name".
        std::vector<std::map<std::string, RgIsaDisassemblyTableView*>::iterator> nodes_to_change;
        for (auto it = entry_point_disassembly_table_views_.begin(); it != entry_point_disassembly_table_views_.end(); ++it)
        {
            std::string file_path, entry_name;
            DecodeEntrypointKey(it->first, file_path, entry_name);
            if (file_path == old_file_path)
            {
                nodes_to_change.push_back(it);
            }
        }

        for (auto it : nodes_to_change)
        {
            std::string file_path, entry_name;
            DecodeEntrypointKey(it->first, file_path, entry_name);
            auto table_view = it->second;
            entry_point_disassembly_table_views_.erase(it);
            std::string new_key = GenerateEntrypointKey(new_file_path, entry_name);
            entry_point_disassembly_table_views_[new_key] = table_view;
        }
    }

    if (ret)
    {
        // 2. Replace the file path in the "disasm table view --> entry point" map.
        for (auto& val : disassembly_table_view_to_entrypoint_)
        {
            std::string file_path, entry_name;
            DecodeEntrypointKey(val.second, file_path, entry_name);
            if (file_path == old_file_path)
            {
                val.second = GenerateEntrypointKey(new_file_path, entry_name);
            }
        }
    }

    if (ret)
    {
        // 3. Replace the file path in the "input file --> disasm table views" map.
        auto it = input_file_to_isa_table_list_.find(old_file_path);
        if ((ret = (it != input_file_to_isa_table_list_.end())) == true)
        {
            auto table_views = it->second;
            input_file_to_isa_table_list_.erase(it);
            input_file_to_isa_table_list_[new_file_path] = table_views;
        }
    }

    assert(ret);

    return ret;
}

void RgIsaDisassemblyTabView::HandleColumnVisibilityFilterStateChanged()
{
    // Only filter the current table.
    if (current_table_ != nullptr)
    {
        current_table_->UpdateFilteredTable();

        // Resize the current table after changing column visibility.
        current_table_->RequestTableResize();
    }
}

void RgIsaDisassemblyTabView::ConnectTableViewSignals(RgIsaDisassemblyTableView* table_view)
{
    // Connect a forwarding signal used to handle when the input source file's correlation highlight line is updated.
    bool is_connected = connect(table_view, &RgIsaDisassemblyTableView::InputSourceHighlightedLineChanged, this, &RgIsaDisassemblyTabView::InputSourceHighlightedLineChanged);
    assert(is_connected);

    // Connect the disassembly table resized handler.
    is_connected = connect(table_view, &RgIsaDisassemblyTableView::DisassemblyTableWidthResizeRequested, this, &RgIsaDisassemblyTabView::DisassemblyTableWidthResizeRequested);
    assert(is_connected);

    // Connect the disassembly table clicked handler.
    is_connected = connect(table_view, &RgIsaDisassemblyTableView::FrameFocusInSignal, this, &RgIsaDisassemblyTabView::FrameFocusInSignal);
    assert(is_connected);

    // Connect a forwarding signal used to handle when the disassembly table loses focus.
    is_connected = connect(table_view, &RgIsaDisassemblyTableView::FrameFocusOutSignal, this, &RgIsaDisassemblyTabView::FrameFocusOutSignal);
    assert(is_connected);

    // Connect the disassembly view's enable scroll bar signal.
    is_connected = connect(this, &RgIsaDisassemblyTabView::EnableScrollbarSignals, table_view, &RgIsaDisassemblyTableView::EnableScrollbarSignals);
    assert(is_connected);

    // Connect the disassembly view's disable scroll bar signal.
    is_connected = connect(this, &RgIsaDisassemblyTabView::DisableScrollbarSignals, table_view, &RgIsaDisassemblyTableView::DisableScrollbarSignals);
    assert(is_connected);

    // Connect the disassembly table's target GPU push button focus in signal.
    is_connected = connect(table_view, &RgIsaDisassemblyTableView::FocusTargetGpuPushButton, this, &RgIsaDisassemblyTabView::FocusTargetGpuPushButton);
    assert(is_connected);

    // Connect the disassembly table's switch disassembly view size signal.
    is_connected = connect(table_view, &RgIsaDisassemblyTableView::SwitchDisassemblyContainerSize, this, &RgIsaDisassemblyTabView::SwitchDisassemblyContainerSize);
    assert(is_connected);

    // Connect the disassembly table's columns push button focus in signal.
    is_connected = connect(table_view, &RgIsaDisassemblyTableView::FocusColumnPushButton, this, &RgIsaDisassemblyTabView::FocusColumnPushButton);
    assert(is_connected);

    // Connect the disassembly table's cli output window focus in signal.
    is_connected = connect(table_view, &RgIsaDisassemblyTableView::FocusCliOutputWindow, this, &RgIsaDisassemblyTabView::FocusCliOutputWindow);
    assert(is_connected);

    // Connect the disassembly table's source window focus in signal.
    is_connected = connect(table_view, &RgIsaDisassemblyTableView::FocusSourceWindow, this, &RgIsaDisassemblyTabView::FocusSourceWindow);
    assert(is_connected);

    // Connect the disassembly tab view's update current sub widget signal.
    is_connected = connect(this, &RgIsaDisassemblyTabView::UpdateCurrentSubWidget, table_view, &RgIsaDisassemblyTableView::UpdateCurrentSubWidget);
    assert(is_connected);

    // Connect the disassembly table's focus cli output window signal.
    is_connected = connect(table_view, &RgIsaDisassemblyTableView::FocusCliOutputWindow, this, &RgIsaDisassemblyTabView::FocusCliOutputWindow);
    assert(is_connected);
}

void RgIsaDisassemblyTabView::HandleShowNextMaxVgprSignal()
{
    if (current_table_ != nullptr)
    {
        current_table_->HandleShowNextMaxVgprSignal();
    }
}

bool RgIsaDisassemblyTabView::IsShowCurrentMaxVgprEnabled()
{
    return current_table_->IsShowCurrentMaxVgprEnabled();
}

void RgIsaDisassemblyTabView::HandleShowPreviousMaxVgprSignal()
    {
    if (current_table_ != nullptr)
    {
        current_table_->HandleShowPreviousMaxVgprSignal();
    }
}

std::string RgIsaDisassemblyTabView::GenerateEntrypointKey(const std::string& file_path, const std::string& entrypoint_name) const
{
    // In some cases it may not be possible to identify a given entry point by name when multiple
    // entrypoints use the same name. Return a unique key based on the input filename and the
    // entry point name, so that each entry point can be identified correctly.
    return file_path + kEntrypointKeySeparator + entrypoint_name;
}

bool RgIsaDisassemblyTabView::DecodeEntrypointKey(const std::string& entrypoint_key, std::string& file_path, std::string& entrypoint_name) const
{
    bool ret = false;

    // Attempt to split the given entry point key string into source file path and entry point name strings.
    std::vector<std::string> file_path_and_entrypoint_name_list;
    RgUtils::splitString(entrypoint_key, kEntrypointKeySeparator, file_path_and_entrypoint_name_list);

    // Verify that only 2 tokens are found. One for the source file path, and another for entry point name.
    size_t token_count = file_path_and_entrypoint_name_list.size();
    assert(token_count == 2);
    if (token_count == 2)
    {
        file_path = file_path_and_entrypoint_name_list[0];
        entrypoint_name = file_path_and_entrypoint_name_list[1];
        ret = true;
    }

    return ret;
}

void RgIsaDisassemblyTabView::RemoveEntrypointTable(RgIsaDisassemblyTableView* table_view)
{
    // Set the current table to null- it's going to be removed from the view.
    if (current_table_ == table_view)
    {
        current_table_ = nullptr;
    }

    // Step through the map of entry point to table, and erase the one that matches the incoming table being removed.
    auto tables_start_iter = entry_point_disassembly_table_views_.begin();
    auto tables_end_iter = entry_point_disassembly_table_views_.end();
    for (auto table_iter = tables_start_iter; table_iter != tables_end_iter; ++table_iter)
    {
        if (table_iter->second == table_view)
        {
            entry_point_disassembly_table_views_.erase(table_iter);
            break;
        }
    }

    // Find and remove the given table from the table to entry point name map.
    auto table_iter = disassembly_table_view_to_entrypoint_.find(table_view);
    if (table_iter != disassembly_table_view_to_entrypoint_.end())
    {
        disassembly_table_view_to_entrypoint_.erase(table_iter);
    }
}

RgIsaDisassemblyTableView* RgIsaDisassemblyTabView::GetCurrentTableView() const
{
    return current_table_;
}
