// C++.
#include <cassert>

// Qt.
#include <QCheckBox>

// Infra.
#include "QtCommon/CustomWidgets/ArrowIconWidget.h"
#include "QtCommon/CustomWidgets/ListWidget.h"
#include "QtCommon/Util/CommonDefinitions.h"
#include "QtCommon/Util/QtUtil.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_editor_widget_enum.h"
#include "radeon_gpu_analyzer_gui/qt/rg_hide_list_widget_event_filter.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

// Names of special widgets.
static const char* kStrEnumList = "EnumList";
static const char* kStrEnumListItemCheckbox = "EnumListWidgetCheckBox";
static const char* kStrEnumListPushButton = "ListPushButton";

// Enumeration push button font size.
const int kPushButtonFontSize = 11;

// Initialize the static enum push button count.
int RgPipelineStateEditorWidgetEnum::enum_combo_push_button_counter_ = 0;

RgPipelineStateEditorWidgetEnum::RgPipelineStateEditorWidgetEnum(bool is_bit_flags_enum, QWidget* parent)
    : is_bit_flags_enum_(is_bit_flags_enum)
    , parent_(parent)
    , RgPipelineStateEditorWidget(parent)
{
    ui_.setupUi(this);

    // Create the enum drop down.
    CreateEnumControls();

    // Set the dropdown arrow button as the focus proxy widget.
    setFocusProxy(ui_.enumComboPushButton);

    // Connect internal signals.
    ConnectSignals();
}

RgPipelineStateEditorWidgetEnum::~RgPipelineStateEditorWidgetEnum()
{
    // Remove the event filter from the main window.
    if (enum_list_widget_ != nullptr)
    {
        qApp->removeEventFilter(enum_list_event_filter_);
    }
}

uint32_t RgPipelineStateEditorWidgetEnum::GetValue() const
{
    uint32_t result = 0;

    if (is_bit_flags_enum_)
    {
        assert(enum_list_widget_ != nullptr);
        if (enum_list_widget_ != nullptr)
        {
            // Combine all flags into a single integer.
            for (int i = 0; i < enum_list_widget_->count(); i++)
            {
                QListWidgetItem* item = enum_list_widget_->item(i);
                assert(item != nullptr);
                if (item != nullptr)
                {
                    QCheckBox* check_box = qobject_cast<QCheckBox*>(enum_list_widget_->itemWidget(item));
                    assert(check_box != nullptr);
                    if (check_box != nullptr)
                    {
                        if (check_box->checkState() == Qt::CheckState::Checked)
                        {
                            uint32_t current_flag = enumerators_[i].value;
                            result |= current_flag;
                        }
                    }
                }
            }
        }
    }
    else
    {
        // Read the push button text and return the appropriate value for it.
        assert(current_index_ >= 0 && current_index_ < enumerators_.size());
        if (current_index_ >= 0 && current_index_ < enumerators_.size())
        {
            result = enumerators_[current_index_].value;
        }
    }

    return result;
}

void RgPipelineStateEditorWidgetEnum::SetEnumerators(const RgEnumValuesVector& enumerators)
{
    enumerators_ = enumerators;

    // Set up the function pointer responsible for handling column visibility filter state change.
    using std::placeholders::_1;
    std::function<void(bool)> slot_function_pointer = std::bind(&RgPipelineStateEditorWidgetEnum::HandleFlagCheckStateChanged, this, _1);

    assert(enum_list_widget_ != nullptr);
    if (enum_list_widget_ != nullptr)
    {
        // Add an item for each column in the table.
        for (const RgEnumNameValuePair& current_enum : enumerators)
        {
            // Add an item for each possible enum.
            // If this item needs a check box, make the item checkable.
            if (is_bit_flags_enum_)
            {
                ListWidget::AddListWidgetCheckboxItem(current_enum.name.c_str(), enum_list_widget_, slot_function_pointer, this, enum_list_widget_->objectName(), kStrEnumListItemCheckbox);
            }
            else
            {
                enum_list_widget_->addItem(current_enum.name.c_str());
            }
        }
    }
}

void RgPipelineStateEditorWidgetEnum::GetFlagBitsString(std::string& flag_bits)
{
    uint32_t current_value = GetValue();

    uint32_t flagsValue = 0;
    assert(enum_list_widget_ != nullptr);
    if (enum_list_widget_ != nullptr)
    {
        int item_count = enum_list_widget_->count();

        // Bitwise OR together all the checked values and display that number for push button text.
        for (int i = 0; i < enumerators_.size(); ++i)
        {
            bool is_checked = (enumerators_[i].value & current_value) == enumerators_[i].value;
            if (is_checked)
            {
                assert(i >= 0 && i < item_count);
                if (i >= 0 && i < item_count)
                {
                    QListWidgetItem* item = enum_list_widget_->item(i);
                    assert(item != nullptr);
                    if (item != nullptr)
                    {
                        QCheckBox* check_box_item = static_cast<QCheckBox*>(enum_list_widget_->itemWidget(item));
                        assert(check_box_item != nullptr);
                        if (check_box_item != nullptr)
                        {
                            uint32_t current_flag = enumerators_[i].value;
                            flagsValue |= current_flag;
                        }
                    }
                }
            }
        }
    }

    // Return a string that displays individual flag bits.
    int num_enumerators = static_cast<int>(enumerators_.size());
    QString output_flag_bits = "(" + QString::number(flagsValue) + ") " + QString::number(flagsValue, 2).rightJustified(num_enumerators, '0');
    flag_bits = output_flag_bits.toStdString();
}

void RgPipelineStateEditorWidgetEnum::GetTooltipString(std::string& tooltip_text)
{
    uint32_t current_value = GetValue();

    // Bitwise OR together all the checked values and display that number for push button text.
    QString tooltip;
    assert(enum_list_widget_ != nullptr);
    if (enum_list_widget_ != nullptr)
    {
        int item_count = enum_list_widget_->count();
        for (int i = 0; i < enumerators_.size(); ++i)
        {
            bool is_checked = (enumerators_[i].value & current_value) == enumerators_[i].value;
            if (is_checked)
            {
                assert(i >= 0 && i < item_count);
                if (i >= 0 && i < item_count)
                {
                    // Append a bitwise OR pipe between each enumerator.
                    if (!tooltip.isEmpty())
                    {
                        tooltip += " | ";
                    }

                    QListWidgetItem* item = enum_list_widget_->item(i);
                    assert(item != nullptr);
                    if (item != nullptr)
                    {
                        QCheckBox* checkbox_item = static_cast<QCheckBox*>(enum_list_widget_->itemWidget(item));
                        assert(checkbox_item != nullptr);
                        if (checkbox_item != nullptr)
                        {
                            const QString& enum_string = checkbox_item->text();
                            tooltip += enum_string;
                        }
                    }
                }
            }
        }
    }

    // Return the tooltip text used to show each enumerator name.
    tooltip_text = tooltip.toStdString();
}

void RgPipelineStateEditorWidgetEnum::HandleUpdateEnumButtonText(const QString& text, bool checked)
{
    std::string tooltip_text;
    GetTooltipString(tooltip_text);

    std::string flag_bits;
    GetFlagBitsString(flag_bits);

    // Set the tooltip to display all the checked values as a text.
    ui_.enumComboPushButton->setToolTip(tooltip_text.c_str());

    // Set the flag bits string.
    UpdateSelectedEnum(flag_bits);

    // The user altered the current value- signal to the parent row that editing is finished.
    emit EditingFinished();
}

void RgPipelineStateEditorWidgetEnum::SetValue(uint32_t value)
{
    if (is_bit_flags_enum_)
    {
        for (int enumerator_index = 0; enumerator_index < static_cast<int>(enumerators_.size()); ++enumerator_index)
        {
            // Set the push button's text to the item with the given value.
            auto enum_item = enum_list_widget_->item(enumerator_index);
            assert(enum_item != nullptr);
            if (enum_item != nullptr)
            {
                bool is_checked = (value & enumerators_[enumerator_index].value) == enumerators_[enumerator_index].value;

                QCheckBox* checkbox = qobject_cast<QCheckBox*>(enum_list_widget_->itemWidget(enum_item));
                assert(checkbox != nullptr);
                if (checkbox != nullptr)
                {
                    checkbox->setCheckState(is_checked ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
                }
            }
        }

        // Set the tooltip to display all the checked values as a text.
        std::string tooltip_text;
        GetTooltipString(tooltip_text);
        ui_.enumComboPushButton->setToolTip(tooltip_text.c_str());

        // Set the flag bits string.
        std::string flag_bits;
        GetFlagBitsString(flag_bits);
        UpdateSelectedEnum(flag_bits);
    }
    else
    {
        // Find the given value in the list of enumerators, and set it as the
        // current value in the combo box containing all enumerator options.
        RgEnumeratorSearcher searcher(value);
        auto enumerator_iter = std::find_if(enumerators_.begin(), enumerators_.end(), searcher);
        if (enumerator_iter != enumerators_.end())
        {
            // Set the push button's text to the item with the given value.
            int item_index = enumerator_iter - enumerators_.begin();
            auto enum_item = enum_list_widget_->item(item_index);
            assert(enum_item != nullptr);
            if (enum_item != nullptr)
            {
                // Set the push button's text to the item with the given value.
                int item_index = enumerator_iter - enumerators_.begin();
                auto enum_item = enum_list_widget_->item(item_index);
                assert(enum_item != nullptr);
                if (enum_item != nullptr)
                {
                    std::string enum_string = enum_item->text().toStdString();
                    UpdateSelectedEnum(enum_string);

                    // Update the current index member variable.
                    SetSelectedListRow(item_index);
                }
            }
        }

        // Update the button text with the selected entry.
        auto enum_item = enum_list_widget_->item(current_index_);
        assert(enum_item != nullptr);
        if (enum_item != nullptr)
        {
            std::string enum_string = enum_item->text().toStdString();
            UpdateSelectedEnum(enum_string);
        }
    }
}

void RgPipelineStateEditorWidgetEnum::ConnectSignals()
{
    // Connect the enum arrow widget button clicked handler.
    bool is_connected = connect(this->ui_.enumComboPushButton, &QPushButton::clicked, this, &RgPipelineStateEditorWidgetEnum::HandleEnumPushButtonClick);
    assert(is_connected);

    // Connect the arrow widget button focus in handler.
    is_connected = connect(ui_.enumComboPushButton, &ArrowIconWidget::FocusInEvent, this, &RgPipelineStateEditorWidget::FocusInSignal);
    assert(is_connected);

    // Connect the signal used to handle a change in the selected enum.
    is_connected = connect(enum_list_widget_, &QListWidget::currentRowChanged, this, &RgPipelineStateEditorWidgetEnum::HandleEnumChanged);
    assert(is_connected);

    // Connect the handler to close list widget on application's loss of focus.
    is_connected = connect(qApp, &QGuiApplication::applicationStateChanged, this, &RgPipelineStateEditorWidgetEnum::HandleApplicationFocusOutEvent);
    assert(is_connected);
}

void RgPipelineStateEditorWidgetEnum::HandleHotKeyPressedSignal()
{
    HideListWidget();
}

void RgPipelineStateEditorWidgetEnum::HideListWidget()
{
    assert(enum_list_widget_ != nullptr);
    if (enum_list_widget_ != nullptr)
    {
        // List widget push button lost focus so hide the list widget.
        if (!enum_list_widget_->isHidden())
        {
            enum_list_widget_->hide();

            // Change the up arrow to a down arrow.
            ui_.enumComboPushButton->SetDirection(ArrowIconWidget::Direction::DownArrow);

            // Emit the list widget status signal.
            emit EnumListWidgetStatusSignal(false);
        }
    }
}

void RgPipelineStateEditorWidgetEnum::HandleApplicationFocusOutEvent(Qt::ApplicationState state)
{
    assert(enum_list_widget_ != nullptr);
    if (enum_list_widget_ != nullptr && state != Qt::ApplicationState::ApplicationActive)
    {
        HideListWidget();
    }
}

void RgPipelineStateEditorWidgetEnum::HandleEnumChanged(int current_index)
{
    assert(enum_list_widget_ != nullptr);
    if (enum_list_widget_ != nullptr)
    {
        auto enum_item = enum_list_widget_->item(current_index);
        assert(enum_item != nullptr);
        if (enum_item != nullptr)
        {
            // Change the enum value if it differs from the current enum value.
            std::string current_enum = ui_.enumComboPushButton->text().toStdString();
            std::string new_enum = enum_item->text().toStdString();
            if (current_enum.compare(new_enum) != 0)
            {
                // Use the dropdown list's selection model to change the currently selected enum.
                QItemSelectionModel* selection_model = enum_list_widget_->selectionModel();
                assert(selection_model != nullptr);
                if (selection_model != nullptr)
                {
                    // Select the new enum within the dropdown list widget.
                    QAbstractItemModel* list_model = enum_list_widget_->model();

                    assert(list_model != nullptr);
                    if (list_model != nullptr)
                    {
                        QModelIndex model_index = list_model->index(current_index, 0);
                        selection_model->setCurrentIndex(model_index, QItemSelectionModel::SelectionFlag::Select);
                    }
                }

                // Change the push button to the newly selected item.
                UpdateSelectedEnum(new_enum);

                // Update the current index member variable.
                current_index_ = current_index;

                emit EditingFinished();
            }
        }
    }
}

void RgPipelineStateEditorWidgetEnum::UpdateSelectedEnum(const std::string& new_value)
{
    static const int kArrowWidgetExtraWidth = 100;

    // Update the button text.
    ui_.enumComboPushButton->setText(new_value.c_str());

    // Measure the width of the enum text, and add extra space to account for the width of the arrow.
    int scaled_arrow_width = static_cast<int>(kArrowWidgetExtraWidth * ScalingManager::Get().GetScaleFactor());
    int text_width = QtCommon::QtUtil::GetTextWidth(ui_.enumComboPushButton->font(), new_value.c_str());
    ui_.enumComboPushButton->setMinimumWidth(scaled_arrow_width + text_width);
}

void RgPipelineStateEditorWidgetEnum::HandleEnumPushButtonClick(bool /* checked */)
{
    // Make the list widget appear and process user selection from the list widget.
    bool visible = enum_list_widget_->isVisible();
    if (visible)
    {
        enum_list_widget_->hide();

        // Change the up arrow to a down arrow.
        ui_.enumComboPushButton->SetDirection(ArrowIconWidget::Direction::DownArrow);

        // Emit the list widget status signal.
        emit EnumListWidgetStatusSignal(false);
    }
    else
    {
        // Compute where to place the combo box relative to where the arrow button is.
        QWidget* widget = ui_.enumComboPushButton;
        QRect rect = widget->geometry();
        QPoint pos(0, 0);
        QMainWindow* main_window = qobject_cast<QMainWindow*>(QApplication::activeWindow());
        pos = widget->mapTo(main_window, pos);
        pos.setY(pos.y() + rect.height());
        int height = QtCommon::QtUtil::GetListWidgetHeight(enum_list_widget_);
        int width = QtCommon::QtUtil::GetListWidgetWidth(enum_list_widget_);
        enum_list_widget_->setGeometry(pos.x(), pos.y(), width + s_CHECK_BOX_WIDTH, height);
        enum_list_widget_->show();

        // Change the down arrow to an up arrow.
        ui_.enumComboPushButton->SetDirection(ArrowIconWidget::Direction::UpArrow);

        // Emit the list widget status signal.
        emit EnumListWidgetStatusSignal(true);
    }
}

void RgPipelineStateEditorWidgetEnum::SetSelectedListRow(int row_index)
{
    // Update the current index member variable.
    current_index_ = row_index;

    // Reset the current selection in the enum list.
    enum_list_widget_->setCurrentRow(current_index_);
}

void RgPipelineStateEditorWidgetEnum::CreateEnumControls()
{
    // Limit the maximum height of the popup ListWidget.
    // A vertical scrollbar will automatically be inserted if it's needed.
    static const int kMaxListHeight = 250;

    // Setup the list widget that opens when the user clicks the enum arrow.
    QMainWindow* main_window = qobject_cast<QMainWindow*>(QApplication::activeWindow());
    RgUtils::SetupComboList(parent_, enum_list_widget_, ui_.enumComboPushButton, enum_list_event_filter_, false);
    enum_list_widget_->setMaximumHeight(kMaxListHeight);
    enum_list_widget_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // Set a unique object name for the enum combo push button.
    QString enum_combo_push_button_object_name = kStrEnumListPushButton + QString::number(enum_combo_push_button_counter_);
    ui_.enumComboPushButton->setObjectName(enum_combo_push_button_object_name);

    // Add the enum combo push button object to the list widget event filter.
    RgHideListWidgetEventFilter* event_filter = qobject_cast<RgHideListWidgetEventFilter*>(enum_list_event_filter_);
    assert(event_filter != nullptr);
    if (event_filter != nullptr)
    {
        event_filter->AddObjectName(enum_combo_push_button_object_name);

        // Connect to event filter's list widget status signal.
        bool is_connected = connect(event_filter, &RgHideListWidgetEventFilter::EnumListWidgetStatusSignal, this, &RgPipelineStateEditorWidgetEnum::EnumListWidgetStatusSignal);
        assert(is_connected);
    }

    // Bump up the enum combo push button counter.
    UpdateEnumPushButtonCounter();

    // Update scale factor for widgets.
    QFont font = ui_.enumComboPushButton->font();
    double scale_factor = ScalingManager::Get().GetScaleFactor();
    font.setPointSize(kPushButtonFontSize * scale_factor);
    enum_list_widget_->setStyleSheet(s_LIST_WIDGET_STYLE.arg(font.pointSize()));

    SetSelectedListRow(0);

    // Set the list cursor to pointing hand cursor.
    enum_list_widget_->setCursor(Qt::PointingHandCursor);
}

void RgPipelineStateEditorWidgetEnum::HandleFlagCheckStateChanged(bool checked)
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
    if (check_box != nullptr)
    {
        HandleUpdateEnumButtonText(check_box->text(), checked);
    }
}

void RgPipelineStateEditorWidgetEnum::UpdateEnumPushButtonCounter()
{
    enum_combo_push_button_counter_++;
}

int RgPipelineStateEditorWidgetEnum::GetEnumPushButtonCounter()
{
    return enum_combo_push_button_counter_;
}
