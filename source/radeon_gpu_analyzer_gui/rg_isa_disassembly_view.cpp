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
#include "QtCommon/CustomWidgets/ArrowIconWidget.h"
#include "QtCommon/CustomWidgets/ListWidget.h"
#include "QtCommon/Scaling/ScalingManager.h"
#include "QtCommon/Util/CommonDefinitions.h"
#include "QtCommon/Util/QtUtil.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_hide_list_widget_event_filter.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_table_model.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_table_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_tab_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_resource_usage_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_view_container.h"
#include "radeon_gpu_analyzer_gui/rg_data_types.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_resource_usage_csv_file_parser.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

// Names of special widgets within the disassembly view.
static const char* kStrDisassemblyColumnVisibilityList = "DisassemblyColumnVisibilityList";
static const char* kStrDisassemblyColumnListItemCheckbox = "ListWidgetCheckBox";
static const char* kStrDisassemblyColumnListItemAllCheckbox = "ListWidgetAllCheckBox";

// Object name associated with the target GPU dropdown list.
static const char* kStrDisassemblyTargetGpuList = "TargetGpuList";

// Columns push button font size.
const int kPushButtonFontSize = 11;

RgIsaDisassemblyView::RgIsaDisassemblyView(QWidget* parent) :
    QWidget(parent)
{
    ui_.setupUi(this);

    ui_.verticalSplitterWidget->setStretchFactor(0, 1);
    ui_.verticalSplitterWidget->setStretchFactor(1, 0);
    ui_.verticalSplitterWidget->handle(1)->setDisabled(true);

    // Create the widget used to control column visibility.
    CreateColumnVisibilityControls();

    // Fill the column visibility dropdown with all of the possible column names.
    PopulateColumnVisibilityList();

    // Create the dropdown list used to select the current target GPU.
    CreateTargetGpuListControls();

    // Connect the signals for the disassembly view.
    ConnectSignals();

    // Set mouse pointer to pointing hand cursor.
    SetCursor();

    // Block recursively repolishing child tables in the disassembly view.
    ui_.disassemblyTableHostWidget->setProperty(kIsRepolishingBlocked, true);

    // Set the push button font sizes.
    SetFontSizes();

    // Set the focus proxy of the view maximize button to be the title bar.
    // This will cause the frame border to change to black when the maximize button loses focus.
    // This is because the view title bar already handles its own loss of focus event.
    ui_.viewMaximizeButton->setFocusProxy(ui_.viewTitlebar);
}

RgIsaDisassemblyView::~RgIsaDisassemblyView()
{
    // Remove the column dropdown focus event filters if they exist.
    if (disassembly_columns_list_widget_ != nullptr && disassembly_columns_list_event_filter_ != nullptr)
    {
        disassembly_columns_list_widget_->removeEventFilter(disassembly_columns_list_event_filter_);
        qApp->removeEventFilter(disassembly_columns_list_event_filter_);
    }

    // Remove the GPU dropdown event filter.
    if (target_gpus_list_widget_ != nullptr && target_gpus_list_event_filter_ != nullptr)
    {
        target_gpus_list_widget_->removeEventFilter(target_gpus_list_event_filter_);
        qApp->removeEventFilter(target_gpus_list_event_filter_);
    }
}

void RgIsaDisassemblyView::ClearBuildOutput()
{
    if (!gpu_tab_views_.empty())
    {
        // Destroy all existing GPU tabs.
        auto first_gpu_tab = gpu_tab_views_.begin();
        auto last_gpu_tab = gpu_tab_views_.end();
        for (auto gpu_tab_iter = first_gpu_tab; gpu_tab_iter != last_gpu_tab; ++gpu_tab_iter)
        {
            ui_.disassemblyTableHostWidget->removeWidget(gpu_tab_iter->second);
        }
    }

    // Clear all state data in the view.
    gpu_tab_views_.clear();
    gpu_resource_usage_views_.clear();
    resource_usage_text_.clear();
}

void RgIsaDisassemblyView::GetResourceUsageTextBounds(QRect& tab_bounds) const
{
    QFontMetrics font_metrics(resource_usage_font_);
    tab_bounds = font_metrics.boundingRect(resource_usage_text_.c_str());
}

bool RgIsaDisassemblyView::IsEmpty() const
{
    return gpu_tab_views_.empty();
}

void RgIsaDisassemblyView::RemoveInputFileEntries(const std::string& input_file_path)
{
    // Clean up disassembly tables associated with the given file.
    DestroyDisassemblyViewsForFile(input_file_path);

    // Clean up resource usage views associated with the given file.
    DestroyResourceUsageViewsForFile(input_file_path);
}

void RgIsaDisassemblyView::HandleInputFileSelectedLineChanged(const std::string& target_gpu, const std::string& input_file_path, std::string& entry_name, int line_index)
{
    // Get the currently active stacked view.
    RgIsaDisassemblyTabView* current_tab_view = GetTargetGpuTabWidgetByTabName(target_gpu);

    if (current_tab_view != nullptr && current_tab_view->GetTableCount() > 0)
    {
        HandleSelectedEntrypointChanged(target_gpu, input_file_path, entry_name);
        current_tab_view->UpdateCorrelatedSourceFileLine(input_file_path, line_index, entry_name);
    }
}

void RgIsaDisassemblyView::HandleSelectedEntrypointChanged(const std::string& target_gpu, const std::string& input_file_path, const std::string& selected_entrypoint_name)
{
    // Get the currently active stacked view.
    RgIsaDisassemblyTabView* target_tab_view = GetTargetGpuTabWidgetByTabName(target_gpu);

    // Switch the target GPU tab if necessary.
    if (target_tab_view != current_tab_view_)
    {
        SetCurrentTargetGpuTabView(target_tab_view);
    }

    if (target_tab_view != nullptr)
    {
        // Switch the table to show the disassembly for the given entrypoint.
        target_tab_view->SwitchToEntryPoint(input_file_path, selected_entrypoint_name);
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

void RgIsaDisassemblyView::HandleColumnVisibilityButtonClicked(bool /*clicked*/)
{
    // Make the list widget appear and process user selection from the list widget.
    bool visible = disassembly_columns_list_widget_->isVisible();
    if (visible == true)
    {
        disassembly_columns_list_widget_->hide();

        // Change the up arrow to a down arrow.
        ui_.columnVisibilityArrowPushButton->SetDirection(ArrowIconWidget::Direction::DownArrow);
    }
    else
    {
        // Update the check box values in case they were changed in global settings.
        RgConfigManager& config_manager = RgConfigManager::Instance();
        std::shared_ptr<RgGlobalSettings> global_settings = config_manager.GetGlobalConfig();
        ListWidget::SetColumnVisibilityCheckboxes(disassembly_columns_list_widget_, global_settings->visible_disassembly_view_columns);

        // Compute where to place the combo box relative to where the arrow button is.
        QWidget* widget = ui_.columnVisibilityArrowPushButton;
        disassembly_columns_list_widget_->show();
        disassembly_columns_list_widget_->setFocus();
        QRect rect = widget->geometry();
        QPoint pos(0, 0);
        pos = widget->mapTo(this, pos);
        pos.setY(pos.y() + rect.height());
        int height = QtCommon::QtUtil::GetListWidgetHeight(disassembly_columns_list_widget_);
        int width = QtCommon::QtUtil::GetListWidgetWidth(disassembly_columns_list_widget_);
        disassembly_columns_list_widget_->setGeometry(pos.x(), pos.y(), width + s_CHECK_BOX_WIDTH, height);

        // Change the down arrow to an up arrow.
        ui_.columnVisibilityArrowPushButton->SetDirection(ArrowIconWidget::Direction::UpArrow);
    }
}

void RgIsaDisassemblyView::HandleColumnVisibilityComboBoxItemClicked(const QString& text, const bool checked)
{
    RgConfigManager& config_manager = RgConfigManager::Instance();
    std::shared_ptr<RgGlobalSettings> global_settings = config_manager.GetGlobalConfig();

    const int first_column = RgIsaDisassemblyTableModel::GetTableColumnIndex(RgIsaDisassemblyTableColumns::kAddress);
    const int last_column = RgIsaDisassemblyTableModel::GetTableColumnIndex(RgIsaDisassemblyTableColumns::kCount);

    // Get the current checked state of the UI.
    // This will include changes from the check/uncheck that triggered this callback.
    std::vector<bool> column_visibility = ListWidget::GetColumnVisibilityCheckboxes(disassembly_columns_list_widget_);

    // Make sure at least one check box is still checked.
    bool is_at_least_one_checked = std::any_of(column_visibility.begin() + first_column, column_visibility.begin() + last_column, [](bool b) { return b == true; });

    if (checked || is_at_least_one_checked)
    {
        // If the user checked the "All" option, Step through each column and set to visible.
        if (text.compare(kStrDisassemblyTableColumnAll) == 0 && (checked == true))
        {
            for (int column_index = first_column; column_index < last_column; ++column_index)
            {
                column_visibility[column_index] = checked;
            }

            // Update the state of the dropdown check boxes to reflect that all options are checked.
            ListWidget::SetColumnVisibilityCheckboxes(disassembly_columns_list_widget_, column_visibility);
        }

        // Save the changes.
        config_manager.Instance().SetDisassemblyColumnVisibility(column_visibility);
        config_manager.SaveGlobalConfigFile();
    }
    else
    {
        // The user tried to uncheck the last check box, but at least one box
        // MUST be checked, so find that item in the ListWidget, and set it back to checked.
        for (int row = 0; row < disassembly_columns_list_widget_->count(); row++)
        {
            QListWidgetItem* item = disassembly_columns_list_widget_->item(row);
            QCheckBox* check_box = (QCheckBox*)disassembly_columns_list_widget_->itemWidget(item);
            QString check_box_text = check_box->text();
            if (check_box_text.compare(text) == 0)
            {
                QCheckBox* check_box = (QCheckBox*)disassembly_columns_list_widget_->itemWidget(item);
                check_box->setChecked(true);
            }
        }
    }

    // See if the "All" box needs checking/unchecking.
    ListWidget::UpdateAllCheckbox(disassembly_columns_list_widget_);

    // Update the "All" checkbox text color to grey or black.
    UpdateAllCheckBoxText();
}

void RgIsaDisassemblyView::HandleColumnVisibilityFilterStateChanged(bool checked)
{
    // Figure out the sender and process appropriately.
    QObject* sender = QObject::sender();
    assert(sender != nullptr);

    // Find out which entry caused the signal.
    QWidget* item = qobject_cast<QWidget*>(sender);
    assert(item != nullptr);

    QCheckBox* check_box = qobject_cast<QCheckBox*>(sender);
    assert(check_box != nullptr);

    // Process the click.
    HandleColumnVisibilityComboBoxItemClicked(check_box->text(), checked);

    // Emit a signal to trigger a refresh of the disassembly table's filter.
    emit DisassemblyColumnVisibilityUpdated();
}

void RgIsaDisassemblyView::HandleTargetGpuArrowClicked(bool clicked)
{
    // Make the list widget appear and process user selection from the list widget.
    bool visible = target_gpus_list_widget_->isVisible();
    if (visible == true)
    {
        target_gpus_list_widget_->hide();

        // Change the up arrow to a down arrow.
        ui_.targetGpuPushButton->SetDirection(ArrowIconWidget::Direction::DownArrow);
    }
    else
    {
        // Compute where to place the combo box relative to where the arrow button is.
        QWidget* widget = ui_.targetGpuPushButton;
        target_gpus_list_widget_->show();
        target_gpus_list_widget_->setFocus();
        target_gpus_list_widget_->setCursor(Qt::PointingHandCursor);
        QRect rect = widget->geometry();
        QPoint pos(0, 0);
        pos = widget->mapTo(this, pos);
        pos.setY(pos.y() + rect.height());
        int height = QtCommon::QtUtil::GetListWidgetHeight(target_gpus_list_widget_);
        int width = QtCommon::QtUtil::GetListWidgetWidth(target_gpus_list_widget_);
        target_gpus_list_widget_->setGeometry(pos.x(), pos.y(), width + s_CHECK_BOX_WIDTH, height);

        // Change the down arrow to an up arrow.
        ui_.targetGpuPushButton->SetDirection(ArrowIconWidget::Direction::UpArrow);
    }
}

void RgIsaDisassemblyView::HandleTargetGpuChanged(int current_index)
{
    assert(target_gpus_list_widget_ != nullptr);
    if (target_gpus_list_widget_ != nullptr)
    {
        auto target_gpu_item = target_gpus_list_widget_->item(current_index);
        assert(target_gpu_item != nullptr);
        if (target_gpu_item != nullptr)
        {
            // Change the target GPU if it differs from the current target GPU.
            std::string current_target_gpu = ui_.targetGpuPushButton->text().toStdString();
            std::string new_target_gpu = target_gpu_item->text().toStdString();
            if (current_target_gpu.compare(new_target_gpu) != 0)
            {
                // Use the dropdown list's selection model to change the currently selected target GPU.
                QItemSelectionModel* selection_model = target_gpus_list_widget_->selectionModel();
                assert(selection_model != nullptr);
                if (selection_model != nullptr)
                {
                    // Select the new target GPU within the dropdown list widget.
                    QAbstractItemModel* list_model = target_gpus_list_widget_->model();

                    assert(list_model != nullptr);
                    if (list_model != nullptr)
                    {
                        QModelIndex model_index = list_model->index(current_index, 0);
                        selection_model->setCurrentIndex(model_index, QItemSelectionModel::SelectionFlag::Select);
                    }
                }

                // Change the target GPU to the newly selected item.
                SetTargetGpu(new_target_gpu);

                // Strip the GPU name from the architecture if needed.
                std::string stripped_gpu_name;
                size_t bracket_pos = new_target_gpu.find("(");
                if (bracket_pos != std::string::npos && new_target_gpu.size() > 2)
                {
                    stripped_gpu_name = new_target_gpu.substr(0, bracket_pos - 1);
                }
                else
                {
                    stripped_gpu_name = new_target_gpu;
                }

                // Strip gfx notation if needed.
                size_t slash_pos = stripped_gpu_name.find("/");
                if (slash_pos != std::string::npos)
                {
                    stripped_gpu_name = stripped_gpu_name.substr(slash_pos + 1);
                }

                // Emit a signal with the name of the target GPU to switch to.
                emit SelectedTargetGpuChanged(stripped_gpu_name);
            }
        }
    }

    // Set focus to disassembly view.
    setFocus();
}

void RgIsaDisassemblyView::ClearListWidget(ListWidget* &list_widget)
{
    assert(list_widget != nullptr);

    // Disconnect slot/signal connection for each check box
    for (int row = 0; row < list_widget->count(); row++)
    {
        QListWidgetItem* item = list_widget->item(row);
        QCheckBox* check_box = (QCheckBox*)list_widget->itemWidget(item);

        if (list_widget->objectName().compare(kStrDisassemblyColumnVisibilityList) == 0)
        {
            bool is_disconnected = disconnect(check_box, &QCheckBox::clicked, this, &RgIsaDisassemblyView::HandleColumnVisibilityFilterStateChanged);
            assert(is_disconnected);
        }
        else
        {
            assert(false);
        }
    }

    // Clear the list widget. This also deletes each item.
    list_widget->clear();
}

void RgIsaDisassemblyView::ConnectDisassemblyTabViewSignals(RgIsaDisassemblyTabView* entry_view)
{
    // Connect the new disassembly GPU tab's source input file highlight line changed signal.
    bool is_connected = connect(entry_view, &RgIsaDisassemblyTabView::InputSourceHighlightedLineChanged, this, &RgIsaDisassemblyView::InputSourceHighlightedLineChanged);
    assert(is_connected);

    // Connect the disassembly table's resized event handler.
    is_connected = connect(entry_view, &RgIsaDisassemblyTabView::DisassemblyTableWidthResizeRequested, this, &RgIsaDisassemblyView::DisassemblyTableWidthResizeRequested);
    assert(is_connected);

    // Connect the disassembly view's column visibility updated signal.
    is_connected = connect(this, &RgIsaDisassemblyView::DisassemblyColumnVisibilityUpdated, entry_view, &RgIsaDisassemblyTabView::HandleColumnVisibilityFilterStateChanged);
    assert(is_connected);

    // Connect the disassembly view's set frame border red signal.
    is_connected = connect(entry_view, &RgIsaDisassemblyTabView::FrameFocusInSignal, this, &RgIsaDisassemblyView::HandleDisassemblyTabViewClicked);
    assert(is_connected);

    // Connect the disassembly view's set frame border black signal.
    is_connected = connect(entry_view, &RgIsaDisassemblyTabView::FrameFocusOutSignal, this, &RgIsaDisassemblyView::HandleDisassemblyTabViewLostFocus);
    assert(is_connected);

    // Connect the disassembly view's title bar's set frame border red signal.
    is_connected = connect(ui_.viewTitlebar, &RgIsaDisassemblyViewTitlebar::FrameFocusInSignal, this, &RgIsaDisassemblyView::HandleDisassemblyTabViewClicked);
    assert(is_connected);

    // Connect the disassembly view's enable scroll bar signal.
    is_connected = connect(this, &RgIsaDisassemblyView::EnableScrollbarSignals, entry_view, &RgIsaDisassemblyTabView::EnableScrollbarSignals);
    assert(is_connected);

    // Connect the disassembly view's disable scroll bar signal.
    is_connected = connect(this, &RgIsaDisassemblyView::DisableScrollbarSignals, entry_view, &RgIsaDisassemblyTabView::DisableScrollbarSignals);
    assert(is_connected);

    // Connect the disassembly table's target GPU push button focus in signal.
    is_connected = connect(entry_view, &RgIsaDisassemblyTabView::FocusTargetGpuPushButton, this, &RgIsaDisassemblyView::HandleFocusTargetGpuPushButton);
    assert(is_connected);

    // Connect the disassembly table's switch disassembly view size signal.
    is_connected = connect(entry_view, &RgIsaDisassemblyTabView::SwitchDisassemblyContainerSize, this, &RgIsaDisassemblyView::SwitchDisassemblyContainerSize);
    assert(is_connected);

    // Connect the disassembly table's columns push button focus in signal.
    is_connected = connect(entry_view, &RgIsaDisassemblyTabView::FocusColumnPushButton, this, &RgIsaDisassemblyView::HandleFocusColumnsPushButton);
    assert(is_connected);

    // Connect the disassembly table's cli output window focus in signal.
    is_connected = connect(entry_view, &RgIsaDisassemblyTabView::FocusSourceWindow, this, &RgIsaDisassemblyView::FocusSourceWindow);
    assert(is_connected);

    // Connect the disassembly view's update current sub widget signal.
    is_connected = connect(this, &RgIsaDisassemblyView::UpdateCurrentSubWidget, entry_view, &RgIsaDisassemblyTabView::UpdateCurrentSubWidget);
    assert(is_connected);
}

void RgIsaDisassemblyView::ConnectSignals()
{
    // Connect the column visibility selector arrow button.
    bool is_connected = connect(ui_.columnVisibilityArrowPushButton, &QPushButton::clicked, this, &RgIsaDisassemblyView::HandleColumnVisibilityButtonClicked);
    assert(is_connected);

    // Connect the handler to show/hide the target GPU list when the arrow button is clicked.
    is_connected = connect(ui_.targetGpuPushButton, &QPushButton::clicked, this, &RgIsaDisassemblyView::HandleTargetGpuArrowClicked);
    assert(is_connected);

    // Connect the handler to give focus to frame on view maximize button click.
    is_connected = connect(ui_.viewMaximizeButton, &QPushButton::clicked, this, &RgIsaDisassemblyView::HandleDisassemblyTabViewClicked);
    assert(is_connected);

    // Connect the handler to give focus to frame on disassembly column list widget's gain of focus.
    is_connected = connect(disassembly_columns_list_widget_, &ListWidget::FocusInEvent, this, &RgIsaDisassemblyView::HandleListWidgetFocusInEvent);
    assert(is_connected);

    // Connect the handler to remove focus from frame on disassembly column list widget's loss of focus.
    is_connected = connect(disassembly_columns_list_widget_, &ListWidget::FocusOutEvent, this, &RgIsaDisassemblyView::HandleListWidgetFocusOutEvent);
    assert(is_connected);

    // Connect the handler to give focus to frame on target GPUs list widget's gain of focus.
    is_connected = connect(target_gpus_list_widget_, &ListWidget::FocusInEvent, this, &RgIsaDisassemblyView::HandleListWidgetFocusInEvent);
    assert(is_connected);

    // Connect the handler to give focus to frame on target GPUs list widget's loss of focus.
    is_connected = connect(target_gpus_list_widget_, &ListWidget::FocusOutEvent, this, &RgIsaDisassemblyView::HandleListWidgetFocusOutEvent);
    assert(is_connected);

    // Connect the handler to give focus to frame on columns push button click.
    is_connected = connect(ui_.columnVisibilityArrowPushButton, &QPushButton::clicked, this, &RgIsaDisassemblyView::HandleDisassemblyTabViewClicked);
    assert(is_connected);

    // Connect the handler to give focus to frame on target GPUs push button click.
    is_connected = connect(ui_.targetGpuPushButton, &QPushButton::clicked, this, &RgIsaDisassemblyView::HandleDisassemblyTabViewClicked);
    assert(is_connected);

    // Connect the handler to remove focus from frame on columns push button loss of focus.
    is_connected = connect(ui_.columnVisibilityArrowPushButton, &ArrowIconWidget::FocusOutEvent, this, &RgIsaDisassemblyView::HandleListWidgetFocusOutEvent);
    assert(is_connected);

    // Connect the handler to give focus to frame on target GPUs push button loss of focus.
    is_connected = connect(ui_.targetGpuPushButton, &ArrowIconWidget::FocusOutEvent, this, &RgIsaDisassemblyView::HandleListWidgetFocusOutEvent);
    assert(is_connected);

    // Select next GPU device action.
    select_next_gpu_target_ = new QAction(this);
    select_next_gpu_target_->setShortcutContext(Qt::ApplicationShortcut);
    select_next_gpu_target_->setShortcut(QKeySequence(kDisassemblyViewHotkeyGpuSelection));
    addAction(select_next_gpu_target_);

    is_connected = connect(select_next_gpu_target_, &QAction::triggered, this, &RgIsaDisassemblyView::HandleSelectNextGPUTargetAction);
    assert(is_connected);
}

void RgIsaDisassemblyView::CreateColumnVisibilityControls()
{
    // Setup the list widget that opens when the user clicks the column visibility arrow.
    RgUtils::SetupComboList(this, disassembly_columns_list_widget_, ui_.columnVisibilityArrowPushButton, disassembly_columns_list_event_filter_, false);
    disassembly_columns_list_widget_->setObjectName(kStrDisassemblyColumnVisibilityList);

    // Handle the open gpu list widget signal and,
    // the update current sub widget signal from the hide list widget event filter object.
    if (disassembly_columns_list_event_filter_ != nullptr)
    {
        RgHideListWidgetEventFilter* event_filter = static_cast<RgHideListWidgetEventFilter*>(disassembly_columns_list_event_filter_);
        if (event_filter != nullptr)
        {
            bool is_connected = connect(event_filter, &RgHideListWidgetEventFilter::OpenGpuListWidget, this, &RgIsaDisassemblyView::HandleOpenGpuListWidget);
            assert(is_connected);

            is_connected = connect(event_filter, &RgHideListWidgetEventFilter::UpdateCurrentSubWidget, this, &RgIsaDisassemblyView::UpdateCurrentSubWidget);
            assert(is_connected);

            is_connected = connect(event_filter, &RgHideListWidgetEventFilter::FocusCliOutputWindow, this, &RgIsaDisassemblyView::FocusCliOutputWindow);
            assert(is_connected);
        }
    }

    // Update scale factor for widgets.
    QFont font = ui_.columnVisibilityArrowPushButton->font();
    double scale_factor = ScalingManager::Get().GetScaleFactor();
    font.setPointSize(kPushButtonFontSize * scale_factor);
    disassembly_columns_list_widget_->setStyleSheet(s_LIST_WIDGET_STYLE.arg(font.pointSize()));

    // Reset the current selection in the column visibility list.
    disassembly_columns_list_widget_->setCurrentRow(0);
}

void RgIsaDisassemblyView::HandleOpenColumnListWidget()
{
    if (disassembly_columns_list_widget_ != nullptr)
    {
        ui_.columnVisibilityArrowPushButton->clicked();
    }
}

void RgIsaDisassemblyView::CreateTargetGpuListControls()
{
    // Setup the list widget used to select the current target GPU.
    RgUtils::SetupComboList(this, target_gpus_list_widget_, ui_.targetGpuPushButton, target_gpus_list_event_filter_, false);
    target_gpus_list_widget_->setObjectName(kStrDisassemblyTargetGpuList);

    if (target_gpus_list_event_filter_ != nullptr)
    {
        RgHideListWidgetEventFilter* event_filter = static_cast<RgHideListWidgetEventFilter*>(target_gpus_list_event_filter_);
        if (event_filter != nullptr)
        {
            bool is_connected = connect(event_filter, &RgHideListWidgetEventFilter::OpenColumnListWidget, this, &RgIsaDisassemblyView::HandleOpenColumnListWidget);
            assert(is_connected);

            is_connected = connect(event_filter, &RgHideListWidgetEventFilter::UpdateCurrentSubWidget, this, &RgIsaDisassemblyView::UpdateCurrentSubWidget);
            assert(is_connected);
        }
    }

    // Update scale factor for widgets.
    QFont font = ui_.targetGpuPushButton->font();
    double scale_factor = ScalingManager::Get().GetScaleFactor();
    font.setPointSize(kPushButtonFontSize * scale_factor);
    target_gpus_list_widget_->setStyleSheet(s_LIST_WIDGET_STYLE.arg(font.pointSize()));

    // Reset the current selection in the target GPU dropdown list.
    target_gpus_list_widget_->setCurrentRow(0);

    // Connect the signal used to handle a change in the selected target GPU.
    bool is_connected = connect(target_gpus_list_widget_, &QListWidget::currentRowChanged, this, &RgIsaDisassemblyView::HandleTargetGpuChanged);
    assert(is_connected);
}

void RgIsaDisassemblyView::HandleOpenGpuListWidget()
{
    if (target_gpus_list_widget_ != nullptr)
    {
        ui_.targetGpuPushButton->clicked();
    }
}

std::string RgIsaDisassemblyView::GetDisassemblyColumnName(RgIsaDisassemblyTableColumns column) const
{
    std::string result;

    static const std::map<RgIsaDisassemblyTableColumns, std::string> kColumnNameMap =
    {
        { RgIsaDisassemblyTableColumns::kAddress,        kStrDisassemblyTableColumnAddress },
        { RgIsaDisassemblyTableColumns::kOpcode,         kStrDisassemblyTableColumnOpcode },
        { RgIsaDisassemblyTableColumns::kOperands,       kStrDisassemblyTableColumnOperands },
        { RgIsaDisassemblyTableColumns::kFunctionalUnit, kStrDisassemblyTableColumnFunctionalUnit },
        { RgIsaDisassemblyTableColumns::kCycles,         kStrDisassemblyTableColumnCycles },
        { RgIsaDisassemblyTableColumns::kBinaryEncoding, kStrDisassemblyTableColumnBinaryEncoding },
    };

    auto column_name_iter = kColumnNameMap.find(column);
    if (column_name_iter != kColumnNameMap.end())
    {
        result = column_name_iter->second;
    }
    else
    {
        // The incoming column doesn't have a name string mapped to it.
        assert(false);
    }

    return result;
}

RgIsaDisassemblyTabView* RgIsaDisassemblyView::GetTargetGpuTabWidgetByTabName(const std::string& tab_text) const
{
    RgIsaDisassemblyTabView* result = nullptr;

    // If a matching view exists return it.
    auto gpu_tab_iter = gpu_tab_views_.find(tab_text);
    if (gpu_tab_iter != gpu_tab_views_.end())
    {
        result = gpu_tab_iter->second;
    }

    return result;
}

void RgIsaDisassemblyView::PopulateTargetGpuList(const RgBuildOutputsMap& build_output)
{
    // Get a mapping of the compute capability to architecture.
    std::map<std::string, std::string> compute_capability_to_arch;
    bool has_arch_mapping = RgUtils::GetComputeCapabilityToArchMapping(compute_capability_to_arch);

    // Block signals to stop updates when each new GPU is added to the list.
    target_gpus_list_widget_->blockSignals(true);

    target_gpus_list_widget_->clear();

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
            bool has_gfx_notation = RgUtils::GetGfxNotation(target_gpu, gfx_notation);
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
    std::sort(targets.begin(),
        targets.end(), [&](const std::string& a, const std::string& b)
    {
        const char* kGfxNotationToken = "gfx";
        bool ret = true;
        size_t sz_a = a.find(kGfxNotationToken);
        size_t sz_b = b.find(kGfxNotationToken);
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
                     ret = ((numB - numA) > 0);
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
        target_gpus_list_widget_->addItem(str.c_str());
    }

    // Switch to the first target GPU.
    HandleTargetGpuChanged(0);

    // Re-enable signals emitted from the target GPU list.
    target_gpus_list_widget_->blockSignals(false);
}

bool RgIsaDisassemblyView::PopulateDisassemblyEntries(const GpuToEntryVector& gpu_to_disassembly_csv_entries)
{
    bool ret = true;

    if (!gpu_to_disassembly_csv_entries.empty())
    {
        // Step through each GPU and insert a new RgIsaDisassemblyTabView into a new tab.
        for (auto gpu_entry_iter = gpu_to_disassembly_csv_entries.begin(); gpu_entry_iter != gpu_to_disassembly_csv_entries.end(); ++gpu_entry_iter)
        {
            const std::string& gpu_name = gpu_entry_iter->first;
            const std::vector<RgEntryOutput>& gpu_entries = gpu_entry_iter->second;

            // Does a tab already exist for the target GPU we're loading results for?
            RgIsaDisassemblyTabView* entry_view = GetTargetGpuTabWidgetByTabName(gpu_name);

            // Create a new tab for the target GPU, and add a new disassembly table viewer.
            if (entry_view == nullptr)
            {
                // Create a new entry view for each unique GPU.
                entry_view = new RgIsaDisassemblyTabView();

                // Connect signals for the new tab.
                ConnectDisassemblyTabViewSignals(entry_view);

                // Add the new entry view to the map.
                gpu_tab_views_[gpu_name] = entry_view;

                // Add the new disassembly table as a tab page in the array of GPU results.
                ui_.disassemblyTableHostWidget->addWidget(entry_view);
            }

            // Send the CSV file paths to the GPU-specific entry viewer.
            bool is_table_populated = entry_view->PopulateEntries(gpu_entries);

            // Verify that the table was populated correctly.
            assert(is_table_populated);
            if (!is_table_populated)
            {
                ret = false;
            }
        }
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
            const std::string& gpu_name = gpu_entry_iter->first;
            const std::vector<RgEntryOutput>& gpu_entries = gpu_entry_iter->second;

            // Create a resource usage disassembly table for each entry, and add it to the layout.
            // Only a single entry point table will be visible at a time, and the user can switch between the current entry.
            for (const RgEntryOutput& entry : gpu_entries)
            {
                OutputFileTypeFinder outputFileTypeSearcher(RgCliOutputFileType::kHwResourceUsageFile);
                auto csv_file_iter = std::find_if(entry.outputs.begin(), entry.outputs.end(), outputFileTypeSearcher);
                if (csv_file_iter != entry.outputs.end())
                {
                    // Create a CSV parser to read the resource usage file.
                    RgResourceUsageCsvFileParser resource_usage_file_parser(csv_file_iter->file_path);

                    // Attempt to parse the file.
                    std::string parse_error_string;
                    bool parsed_successfully = resource_usage_file_parser.Parse(parse_error_string);
                    if (parsed_successfully)
                    {
                        // Extract the parsed data, and populate the resource usage view.
                        const RgResourceUsageData& resource_usage_data = resource_usage_file_parser.GetData();
                        RgResourceUsageView* resource_usage_view = new RgResourceUsageView();
                        resource_usage_view->PopulateView(resource_usage_data);
                        resource_usage_text_ = resource_usage_view->GetResourceUsageText();
                        resource_usage_font_ = resource_usage_view->GetResourceUsageFont();

                        // Register the resource usage view with the scaling manager.
                        ScalingManager::Get().RegisterObject(resource_usage_view);

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

void RgIsaDisassemblyView::ConnectResourceUsageViewSignals(RgResourceUsageView * resource_usage_view)
{
    // Connect to the resource usage view's mouse press event.
    bool is_connected = connect(resource_usage_view, &RgResourceUsageView::ResourceUsageViewClickedSignal, this, &RgIsaDisassemblyView::HandleDisassemblyTabViewClicked);
    assert(is_connected);

    // Connect to the resource usage view's focus out event.
    is_connected = connect(resource_usage_view, &RgResourceUsageView::ResourceUsageViewFocusOutEventSignal, this, &RgIsaDisassemblyView::HandleResourceUsageViewFocusOutEvent);
    assert(is_connected);
}

void RgIsaDisassemblyView::PopulateColumnVisibilityList()
{
    // Set up the function pointer responsible for handling column visibility filter state change.
    using std::placeholders::_1;
    std::function<void(bool)> slot_function_pointer = std::bind(&RgIsaDisassemblyView::HandleColumnVisibilityFilterStateChanged, this, _1);

    // Remove the existing items first
    ClearListWidget(disassembly_columns_list_widget_);

    // Add the "All" entry
    ListWidget::AddListWidgetCheckboxItem(kStrDisassemblyTableColumnAll, disassembly_columns_list_widget_, slot_function_pointer, this, kStrDisassemblyColumnVisibilityList, kStrDisassemblyColumnListItemAllCheckbox);

    // Loop through each column enum member.
    int start_column = RgIsaDisassemblyTableModel::GetTableColumnIndex(RgIsaDisassemblyTableColumns::kAddress);
    int end_column   = RgIsaDisassemblyTableModel::GetTableColumnIndex(RgIsaDisassemblyTableColumns::kCount);

    // Add an item for each column in the table.
    for (int column_index = start_column; column_index < end_column; ++column_index)
    {
        // Add an item for each possible column in the table.
        std::string column_name = GetDisassemblyColumnName(static_cast<RgIsaDisassemblyTableColumns>(column_index));
        ListWidget::AddListWidgetCheckboxItem(column_name.c_str(), disassembly_columns_list_widget_, slot_function_pointer, this, kStrDisassemblyColumnVisibilityList, kStrDisassemblyColumnListItemCheckbox);
    }

    // Populate the check box items by reading the global settings.
    RgConfigManager& config_manager = RgConfigManager::Instance();
    std::shared_ptr<RgGlobalSettings> global_settings = config_manager.GetGlobalConfig();
    ListWidget::SetColumnVisibilityCheckboxes(disassembly_columns_list_widget_, global_settings->visible_disassembly_view_columns);

    // Update the "All" checkbox text color to grey or black.
    UpdateAllCheckBoxText();

    // Set list widget's check box's focus proxy to be the frame.
    SetCheckBoxFocusProxies(disassembly_columns_list_widget_);
}

void RgIsaDisassemblyView::SetCheckBoxFocusProxies(const ListWidget* list_widget) const
{
    for (int i = 0; i < list_widget->count(); i++)
    {
        QListWidgetItem* item = list_widget->item(i);
        assert(item != nullptr);

        QCheckBox* check_box = qobject_cast<QCheckBox*>(list_widget->itemWidget(item));
        assert(check_box != nullptr);

        check_box->setFocusProxy(ui_.frame);
    }
}

void RgIsaDisassemblyView::UpdateAllCheckBoxText()
{
    bool are_all_items_checked = true;

    // Check to see if all of the boxes are checked.
    for (int i = 1; i < disassembly_columns_list_widget_->count(); i++)
    {
        QListWidgetItem* item = disassembly_columns_list_widget_->item(i);
        assert(item != nullptr);

        QCheckBox* check_box = qobject_cast<QCheckBox*>(disassembly_columns_list_widget_->itemWidget(item));
        assert(check_box != nullptr);

        if (check_box->checkState() == Qt::CheckState::Unchecked)
        {
            are_all_items_checked = false;
            break;
        }
    }

    // If all boxes are checked, update the text color of the All check box.
    QListWidgetItem* item = disassembly_columns_list_widget_->item(0);
    if (item != nullptr)
    {
        QCheckBox* check_box = qobject_cast<QCheckBox*>(disassembly_columns_list_widget_->itemWidget(item));
        if (check_box != nullptr)
        {
            if (are_all_items_checked)
            {
                check_box->setStyleSheet("QCheckBox#ListWidgetAllCheckBox {color: grey;}");
            }
            else
            {
                check_box->setStyleSheet("QCheckBox#ListWidgetAllCheckBox {color: black;}");
            }
        }
    }
}

void RgIsaDisassemblyView::DestroyDisassemblyViewsForFile(const std::string& input_file_path)
{
    // Keep a list of tabs that should be destroyed after removing the input file.
    std::vector<std::string> gpu_tabs_to_remove;

    // Step through each GPU tab and try to remove the entries associated with the given input file.
    auto start_tab = gpu_tab_views_.begin();
    auto end_tab = gpu_tab_views_.end();
    for (auto tab_iter = start_tab; tab_iter != end_tab; ++tab_iter)
    {
        // Search the tab for entries to remove.
        RgIsaDisassemblyTabView* gpu_tab = tab_iter->second;
        assert(gpu_tab != nullptr);
        if (gpu_tab != nullptr)
        {
            // Attempt to remove entries associated with the input file from each GPU tab.
            gpu_tab->RemoveInputFileEntries(input_file_path);

            // Does the GPU tab have any tables left in it? If not, destroy the tab too.
            int num_tables_in_tab = gpu_tab->GetTableCount();
            if (num_tables_in_tab == 0)
            {
                const std::string& gpu_name = tab_iter->first;
                gpu_tabs_to_remove.push_back(gpu_name);
            }
        }
    }

    // Destroy all tabs that were marked for destruction.
    for (auto gpu_tab : gpu_tabs_to_remove)
    {
        auto tab_iter = gpu_tab_views_.find(gpu_tab);
        if (tab_iter != gpu_tab_views_.end())
        {
            // Destroy the GPU tab.
            RgIsaDisassemblyTabView* gpu_tab_view = tab_iter->second;

            // Remove the GPU tab from the view.
            ui_.disassemblyTableHostWidget->removeWidget(gpu_tab_view);

            // Remove the GPU from the view.
            gpu_tab_views_.erase(tab_iter);
        }
    }
}

void RgIsaDisassemblyView::DestroyResourceUsageViewsForFile(const std::string& input_file_path)
{
    // Destroy resource views for all GPUs.
    for (auto gpu_iter = gpu_resource_usage_views_.begin(); gpu_iter != gpu_resource_usage_views_.end(); ++gpu_iter)
    {
        // Find all resource views related to the given input file.
        InputToEntrypointViews& input_file_to_views_iter = gpu_iter->second;
        auto entrypoint_resource_usageviews_iter = input_file_to_views_iter.find(input_file_path);
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

void RgIsaDisassemblyView::SetCurrentTargetGpuTabView(RgIsaDisassemblyTabView* tab_view)
{
    // Set the current widget in the stack.
    ui_.disassemblyTableHostWidget->setCurrentWidget(tab_view);

    // Store the current target GPU tab being viewed.
    current_tab_view_ = tab_view;

    // Use the current tab view as the focus proxy for this view.
    setFocusProxy(tab_view);
}

void RgIsaDisassemblyView::SetCursor()
{
    ui_.columnVisibilityArrowPushButton->setCursor(Qt::PointingHandCursor);
    ui_.targetGpuPushButton->setCursor(Qt::PointingHandCursor);
    ui_.viewMaximizeButton->setCursor(Qt::PointingHandCursor);
}

void RgIsaDisassemblyView::SetTargetGpu(const std::string& target_gpu)
{
    static const int kArrowWidgetExtraWidth = 30;

    // Update the button text.
    ui_.targetGpuPushButton->setText(target_gpu.c_str());

    // Measure the width of the Target GPU text, and add extra space to account for the width of the arrow.
    int scaled_arrow_width = static_cast<int>(kArrowWidgetExtraWidth * ScalingManager::Get().GetScaleFactor());
    int text_width = QtCommon::QtUtil::GetTextWidth(ui_.targetGpuPushButton->font(), target_gpu.c_str());
    ui_.targetGpuPushButton->setMinimumWidth(scaled_arrow_width + text_width);
}

void RgIsaDisassemblyView::SetFontSizes()
{
    // Set column visibility push button font.
    ArrowIconWidget* arrow_widget = dynamic_cast<ArrowIconWidget*>(ui_.columnVisibilityArrowPushButton);
    if (arrow_widget != nullptr)
    {
        arrow_widget->SetFontSize(kPushButtonFontSize);
    }

    // Set ISA list push button font.
    arrow_widget = dynamic_cast<ArrowIconWidget*>(ui_.targetGpuPushButton);
    if (arrow_widget != nullptr)
    {
        arrow_widget->SetFontSize(kPushButtonFontSize);
    }
}

bool RgIsaDisassemblyView::IsLineCorrelatedInEntry(const std::string& input_file_path, const std::string& target_gpu, const std::string& entrypoint, int src_line) const
{
    bool  ret = false;

    RgIsaDisassemblyTabView* target_gpu_tab = GetTargetGpuTabWidgetByTabName(target_gpu.c_str());

    assert(target_gpu_tab != nullptr);
    if (target_gpu_tab != nullptr)
    {
        ret = target_gpu_tab->IsSourceLineCorrelatedForEntry(input_file_path, entrypoint, src_line);
    }

    return ret;
}

void RgIsaDisassemblyView::HandleDisassemblyTabViewClicked()
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

void RgIsaDisassemblyView::HandleFocusColumnsPushButton()
{
    ui_.columnVisibilityArrowPushButton->clicked(false);
}

void RgIsaDisassemblyView::ConnectTitleBarDoubleClick(const RgViewContainer* disassembly_view_container)
{
    assert(disassembly_view_container != nullptr);
    if (disassembly_view_container != nullptr)
    {
        bool is_connected = connect(ui_.viewTitlebar, &RgIsaDisassemblyViewTitlebar::ViewTitleBarDoubleClickedSignal, disassembly_view_container, &RgViewContainer::MaximizeButtonClicked);
        assert(is_connected);
    }
}

bool RgIsaDisassemblyView::ReplaceInputFilePath(const std::string& old_file_path, const std::string& new_file_path)
{
    bool result = true;

    // Replace the file path in all disassembly tab views for all devices.
    for (auto& gpu_and_tab_view : gpu_tab_views_)
    {
        RgIsaDisassemblyTabView* tab_view = gpu_and_tab_view.second;
        if (!tab_view->ReplaceInputFilePath(old_file_path, new_file_path))
        {
            result = false;
            break;
        }
    }

    // Replace the file path in the resource usage map.
    if (result)
    {
        for (auto& gpu_and_resource_usage : gpu_resource_usage_views_)
        {
            auto& file_and_resource_usage = gpu_and_resource_usage.second;
            auto it = file_and_resource_usage.find(old_file_path);
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
    int current_row = target_gpus_list_widget_->currentRow();

    if (current_row < target_gpus_list_widget_->count() - 1)
    {
        current_row++;
    }
    else
    {
        current_row = 0;
    }
    target_gpus_list_widget_->setCurrentRow(current_row);
}
