// C++.
#include <cassert>

// Qt.
#include <QCheckBox>
#include <QMainWindow>

// Infra.
#include "qt_common/utils/common_definitions.h"
#include "qt_common/utils/qt_util.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_editor_widget_enum.h"
#include "radeon_gpu_analyzer_gui/qt/rg_hide_list_widget_event_filter.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"

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
}

uint32_t RgPipelineStateEditorWidgetEnum::GetValue() const
{
    uint32_t result = 0;

    if (is_bit_flags_enum_)
    {
        // Combine all flags into a single integer.
        for (int i = 0; i < ui_.enumComboPushButton->RowCount(); i++)
        {
            QListWidgetItem* item = ui_.enumComboPushButton->FindItem(i);
            assert(item != nullptr);
            if (item != nullptr)
            {
                if (ui_.enumComboPushButton->IsChecked(i))
                {
                    uint32_t current_flag = enumerators_[i].value;
                    result |= current_flag;
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

    for (const RgEnumNameValuePair& current_enum : enumerators)
    {
        // Add an item for each possible enum.
        // If this item needs a check box, make the item checkable.
        if (is_bit_flags_enum_)
        {
            ui_.enumComboPushButton->AddCheckboxItem(current_enum.name.c_str(), QVariant(), false, false);
        }
        else
        {
            ui_.enumComboPushButton->AddItem(current_enum.name.c_str(), QVariant());
        }
    }
}

void RgPipelineStateEditorWidgetEnum::GetFlagBitsString(std::string& flag_bits)
{
    uint32_t flags_value = 0;

    for (int i = 0; i < ui_.enumComboPushButton->RowCount(); i++)
    {
        bool is_checked = ui_.enumComboPushButton->IsChecked(i);
        if (is_checked)
        {
            uint32_t current_flag = enumerators_[i].value;
            flags_value |= current_flag;
        }
    }

    // Return a string that displays individual flag bits.
    int num_enumerators = static_cast<int>(enumerators_.size());
    QString output_flag_bits = "(" + QString::number(flags_value) + ") " + QString::number(flags_value, 2).rightJustified(num_enumerators, '0');
    flag_bits = output_flag_bits.toStdString();
}

void RgPipelineStateEditorWidgetEnum::GetTooltipString(std::string& tooltip_text)
{
    // Bitwise OR together all the checked values and display that number for push button text.
    QString tooltip;
    for (int i = 0; i < ui_.enumComboPushButton->RowCount(); i++)
    {
        bool is_checked = ui_.enumComboPushButton->IsChecked(i);
        if (is_checked)
        {
            // Append a bitwise OR pipe between each enumerator.
            if (!tooltip.isEmpty())
            {
                tooltip += " | ";
            }

            QListWidgetItem* item = ui_.enumComboPushButton->FindItem(i);
            assert(item != nullptr);
            if (item != nullptr)
            {
                const QString& enum_string = item->text();
                tooltip += enum_string;
            }
        }
    }

    // Return the tooltip text used to show each enumerator name.
    tooltip_text = tooltip.toStdString();
}

void RgPipelineStateEditorWidgetEnum::HandleUpdateEnumButtonText(const QString& text, bool checked)
{
    Q_UNUSED(text);
    Q_UNUSED(checked);

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
            QListWidgetItem* enum_item = ui_.enumComboPushButton->FindItem(enumerator_index);
            assert(enum_item != nullptr);
            if (enum_item != nullptr)
            {
                bool is_checked = (value & enumerators_[enumerator_index].value) == enumerators_[enumerator_index].value;

                ui_.enumComboPushButton->SetChecked(enumerator_index, is_checked);
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
            QListWidgetItem* enum_item  = ui_.enumComboPushButton->FindItem(item_index);
            assert(enum_item != nullptr);
            if (enum_item != nullptr)
            {
                // Set the push button's text to the item with the given value.
                int enum_item_index = enumerator_iter - enumerators_.begin();
                assert(enum_item != nullptr);
                if (enum_item != nullptr)
                {
                    QListWidgetItem* item        = ui_.enumComboPushButton->FindItem(enum_item_index);
                    QString          enum_string = ui_.enumComboPushButton->ItemText(item);
                    UpdateSelectedEnum(enum_string.toStdString());
                }
            }
        }
    }
}

void RgPipelineStateEditorWidgetEnum::ConnectSignals()
{
    // Connect the arrow widget button focus in handler.
    bool is_connected = connect(ui_.enumComboPushButton, &ArrowIconComboBox::FocusInEvent, this, &RgPipelineStateEditorWidget::FocusInSignal);
    assert(is_connected);

    // Connect the arrow icon combo box check box changed handler.
    is_connected = connect(ui_.enumComboPushButton, &ArrowIconComboBox::CheckboxChanged, this, &RgPipelineStateEditorWidgetEnum::HandleFlagCheckStateChanged);
    assert(is_connected);

    // Connect the signal used to handle a change in the selected enum.
    is_connected = connect(ui_.enumComboPushButton, &ArrowIconComboBox::SelectionChanged, this, &RgPipelineStateEditorWidgetEnum::HandleSelectionChanged);
    assert(is_connected);
}

void RgPipelineStateEditorWidgetEnum::UpdateSelectedEnum(const std::string& new_value)
{
    static const int kArrowWidgetExtraWidth = 100;

    // Update the button text.
    ui_.enumComboPushButton->setText(new_value.c_str());
}

void RgPipelineStateEditorWidgetEnum::CreateEnumControls()
{
    // Limit the maximum height of the popup ListWidget.
    // A vertical scrollbar will automatically be inserted if it's needed.
    static const int kMaxListHeight = 250;

    // Setup the list widget that opens when the user clicks the enum dropdown.
    if (is_bit_flags_enum_)
    {
        ui_.enumComboPushButton->InitMultiSelect(parent_, "Value");
    }
    else
    {
        ui_.enumComboPushButton->InitSingleSelect(parent_, "Value", false);
    }

    // Set a unique object name for the enum combo push button.
    QString enum_combo_push_button_object_name = kStrEnumListPushButton + QString::number(enum_combo_push_button_counter_);
    ui_.enumComboPushButton->setObjectName(enum_combo_push_button_object_name);

    // Bump up the enum combo push button counter.
    UpdateEnumPushButtonCounter();

}

void RgPipelineStateEditorWidgetEnum::HandleFlagCheckStateChanged(QCheckBox* check_box)
{
    // Process the click.
    if (check_box != nullptr)
    {
        HandleUpdateEnumButtonText(check_box->text(), check_box->isChecked());
    }
}

void RgPipelineStateEditorWidgetEnum::HandleSelectionChanged()
{
    // Update the current index member variable.
    current_index_ = ui_.enumComboPushButton->CurrentRow();

    // Change the push button to the newly selected item.
    UpdateSelectedEnum(ui_.enumComboPushButton->SelectedText().toStdString());

    emit EditingFinished();
}

void RgPipelineStateEditorWidgetEnum::UpdateEnumPushButtonCounter()
{
    enum_combo_push_button_counter_++;
}

int RgPipelineStateEditorWidgetEnum::GetEnumPushButtonCounter()
{
    return enum_combo_push_button_counter_;
}
