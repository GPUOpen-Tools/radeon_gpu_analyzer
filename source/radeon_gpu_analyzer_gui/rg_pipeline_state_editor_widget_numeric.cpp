// C++.
#include <cassert>
#include <limits>
#include <sstream>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_pipeline_state_editor_widget_numeric.h"
#include "radeon_gpu_analyzer_gui/qt/rg_unsigned_int_validator.h"

RgPipelineStateEditorWidgetNumeric::RgPipelineStateEditorWidgetNumeric(QWidget* parent)
    : RgPipelineStateEditorWidget(parent)
{
    ui_.setupUi(this);

    // Set the numeric line edit as the focus proxy widget.
    setFocusProxy(ui_.lineEdit);

    // Connect internal signals.
    ConnectSignals();
}

QVariant RgPipelineStateEditorWidgetNumeric::GetValue() const
{
    return QVariant::fromValue(ui_.lineEdit->text());
}

void RgPipelineStateEditorWidgetNumeric::SetType(RgEditorDataType type)
{
    type_ = type;

    switch (type_)
    {
    case RgEditorDataType::kInt8:
        validator_ = new QIntValidator(std::numeric_limits<int8_t>::min(), std::numeric_limits<int8_t>::max(), this);
        break;
    case RgEditorDataType::kInt16:
        validator_ = new QIntValidator(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), this);
        break;
    case RgEditorDataType::kInt32:
        validator_ = new QIntValidator(std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max(), this);
        break;
    case RgEditorDataType::kUInt8:
        validator_ = new RgUnsignedIntValidator(0, std::numeric_limits<uint8_t>::max(), this);
        break;
    case RgEditorDataType::kUInt16:
        validator_ = new RgUnsignedIntValidator(0, std::numeric_limits<uint16_t>::max(), this);
        break;
    case RgEditorDataType::kUInt32:
        validator_ = new RgUnsignedIntValidator(0, std::numeric_limits<uint32_t>::max(), this);
        break;
    case RgEditorDataType::kFloat:
        validator_ = new QDoubleValidator(std::numeric_limits<float>::min(), std::numeric_limits<float>::max(), 4, this);
        break;
    case RgEditorDataType::kDouble:
        validator_ = new QDoubleValidator(std::numeric_limits<double>::min(), std::numeric_limits<double>::max(), 4, this);
        break;
    default:
        // If we get here, the editor is attempting to edit
        // a value type it doesn't know how to edit.
        assert(false);
        break;
    }

    // Verify that a validator was created and assign it to the line edit control.
    assert(validator_ != nullptr);
    if (validator_ != nullptr)
    {
        ui_.lineEdit->setValidator(validator_);
    }
}

void RgPipelineStateEditorWidgetNumeric::SetValue(QVariant value)
{
    bool is_ok = false;
    switch (type_)
    {
    case RgEditorDataType::kInt8:
    case RgEditorDataType::kInt16:
    case RgEditorDataType::kInt32:
        {
            int integer = value.toInt(&is_ok);
            if (is_ok)
            {
                std::stringstream stream;
                stream << integer;
                ui_.lineEdit->setText(stream.str().c_str());
            }
        }
        break;
    case RgEditorDataType::kUInt8:
    case RgEditorDataType::kUInt16:
    case RgEditorDataType::kUInt32:
        {
            uint unsigned_integer = value.toUInt(&is_ok);
            if (is_ok)
            {
                std::stringstream stream;
                stream << unsigned_integer;
                ui_.lineEdit->setText(stream.str().c_str());
            }
        }
        break;
    case RgEditorDataType::kFloat:
    case RgEditorDataType::kDouble:
        {
            std::stringstream stream;
            stream << value.toDouble();
            ui_.lineEdit->setText(stream.str().c_str());
        }
        break;
    default:
        // If we get here, the editor is attempting to display
        // a value type it doesn't know how to edit.
        assert(false);
        break;
    }
}

void RgPipelineStateEditorWidgetNumeric::ConnectSignals()
{
    // Connect the line editing finished signal.
    bool is_connected = connect(ui_.lineEdit, &QLineEdit::editingFinished, this, &RgPipelineStateEditorWidget::EditingFinished);
    assert(is_connected);

    // Connect the line edit widget focus in signal.
    is_connected = connect(ui_.lineEdit, &RgLineEdit::LineEditFocusInEvent, this, &RgPipelineStateEditorWidget::FocusInSignal);
    assert(is_connected);
}

void RgPipelineStateEditorWidgetNumeric::HighlightSubString(int start_location, const std::string& search_string)
{
    // Update search string location.
    UpdateStringMatchingLocation(start_location, static_cast<int>(search_string.size()), search_string);
    ui_.lineEdit->SetHighlightSubStringData(string_highlight_data_);
    ui_.lineEdit->SetHighlightSubString(true);
    ui_.lineEdit->update();
}

void RgPipelineStateEditorWidgetNumeric::UpdateStringMatchingLocation(int start_location, int length, const std::string& search_string)
{
    bool found = false;

    // Remove any entries that do not match the search string anymore.
    int count = 0;
    std::vector<int> remove_entries;
    for (auto& stringData : string_highlight_data_)
    {
        if (stringData.highlight_string != search_string)
        {
            remove_entries.push_back(count);
        }
        count++;
    }
    for (std::vector<int>::reverse_iterator it = remove_entries.rbegin(); it != remove_entries.rend(); ++it)
    {
        string_highlight_data_.remove(*it);
    }

    // Update existing locations, if any.
    for (auto& string_data : string_highlight_data_)
    {
        if (string_data.start_location == start_location)
        {
            string_data.end_location = start_location + length;
            string_data.highlight_string = search_string;
            found = true;
            break;
        }
    }

    // Create a new entry if a matching entry not found.
    if (!found)
    {
        StringHighlightData string_highlight_data = {};
        string_highlight_data.start_location = start_location;
        string_highlight_data.end_location = start_location + length;
        string_highlight_data.highlight_string = search_string;
        string_highlight_data.highlight_color = Qt::yellow;
        string_highlight_data_.push_back(string_highlight_data);
    }
}
