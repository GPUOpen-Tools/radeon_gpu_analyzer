// C++.
#include <cassert>
#include <limits>
#include <sstream>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateEditorWidgetNumeric.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgUnsignedIntValidator.h>

rgPipelineStateEditorWidgetNumeric::rgPipelineStateEditorWidgetNumeric(QWidget* pParent)
    : rgPipelineStateEditorWidget(pParent)
{
    ui.setupUi(this);

    // Set the numeric line edit as the focus proxy widget.
    setFocusProxy(ui.lineEdit);

    // Connect internal signals.
    ConnectSignals();
}

QVariant rgPipelineStateEditorWidgetNumeric::GetValue() const
{
    return QVariant::fromValue(ui.lineEdit->text());
}

void rgPipelineStateEditorWidgetNumeric::SetType(rgEditorDataType type)
{
    m_type = type;

    switch (m_type)
    {
    case rgEditorDataType::Int8:
        m_pValidator = new QIntValidator(std::numeric_limits<int8_t>::min(), std::numeric_limits<int8_t>::max(), this);
        break;
    case rgEditorDataType::Int16:
        m_pValidator = new QIntValidator(std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max(), this);
        break;
    case rgEditorDataType::Int32:
        m_pValidator = new QIntValidator(std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max(), this);
        break;
    case rgEditorDataType::UInt8:
        m_pValidator = new rgUnsignedIntValidator(0, std::numeric_limits<uint8_t>::max(), this);
        break;
    case rgEditorDataType::UInt16:
        m_pValidator = new rgUnsignedIntValidator(0, std::numeric_limits<uint16_t>::max(), this);
        break;
    case rgEditorDataType::UInt32:
        m_pValidator = new rgUnsignedIntValidator(0, std::numeric_limits<uint32_t>::max(), this);
        break;
    case rgEditorDataType::Float:
        m_pValidator = new QDoubleValidator(std::numeric_limits<float>::min(), std::numeric_limits<float>::max(), 4, this);
        break;
    case rgEditorDataType::Double:
        m_pValidator = new QDoubleValidator(std::numeric_limits<double>::min(), std::numeric_limits<double>::max(), 4, this);
        break;
    default:
        // If we get here, the editor is attempting to edit
        // a value type it doesn't know how to edit.
        assert(false);
        break;
    }

    // Verify that a validator was created and assign it to the line edit control.
    assert(m_pValidator != nullptr);
    if (m_pValidator != nullptr)
    {
        ui.lineEdit->setValidator(m_pValidator);
    }
}

void rgPipelineStateEditorWidgetNumeric::SetValue(QVariant value)
{
    bool isOk = false;
    switch (m_type)
    {
    case rgEditorDataType::Int8:
    case rgEditorDataType::Int16:
    case rgEditorDataType::Int32:
        {
            int integer = value.toInt(&isOk);
            if (isOk)
            {
                std::stringstream stream;
                stream << integer;
                ui.lineEdit->setText(stream.str().c_str());
            }
        }
        break;
    case rgEditorDataType::UInt8:
    case rgEditorDataType::UInt16:
    case rgEditorDataType::UInt32:
        {
            uint unsignedInteger = value.toUInt(&isOk);
            if (isOk)
            {
                std::stringstream stream;
                stream << unsignedInteger;
                ui.lineEdit->setText(stream.str().c_str());
            }
        }
        break;
    case rgEditorDataType::Float:
    case rgEditorDataType::Double:
        {
            std::stringstream stream;
            stream << value.toDouble();
            ui.lineEdit->setText(stream.str().c_str());
        }
        break;
    default:
        // If we get here, the editor is attempting to display
        // a value type it doesn't know how to edit.
        assert(false);
        break;
    }
}

void rgPipelineStateEditorWidgetNumeric::ConnectSignals()
{
    // Connect the line editing finished signal.
    bool isConnected = connect(ui.lineEdit, &QLineEdit::editingFinished, this, &rgPipelineStateEditorWidget::EditingFinished);
    assert(isConnected);

    // Connect the line edit widget focus in signal.
    isConnected = connect(ui.lineEdit, &rgLineEdit::LineEditFocusInEvent, this, &rgPipelineStateEditorWidget::FocusInSignal);
    assert(isConnected);
}

void rgPipelineStateEditorWidgetNumeric::HighlightSubString(int startLocation, const std::string& searchString)
{
    // Update search string location.
    UpdateStringMatchingLocation(startLocation, static_cast<int>(searchString.size()), searchString);
    ui.lineEdit->SetHighlightSubStringData(m_stringHighlightData);
    ui.lineEdit->SetHighlightSubString(true);
    ui.lineEdit->update();
}

void rgPipelineStateEditorWidgetNumeric::UpdateStringMatchingLocation(int startLocation, int length, const std::string& searchString)
{
    bool found = false;

    // Remove any entries that do not match the search string anymore.
    int count = 0;
    std::vector<int> removeEntries;
    for (auto& stringData : m_stringHighlightData)
    {
        if (stringData.m_highlightString != searchString)
        {
            removeEntries.push_back(count);
        }
        count++;
    }
    for (std::vector<int>::reverse_iterator it = removeEntries.rbegin(); it != removeEntries.rend(); ++it)
    {
        m_stringHighlightData.remove(*it);
    }

    // Update existing locations, if any.
    for (auto& stringData : m_stringHighlightData)
    {
        if (stringData.m_startLocation == startLocation)
        {
            stringData.m_endLocation = startLocation + length;
            stringData.m_highlightString = searchString;
            found = true;
            break;
        }
    }

    // Create a new entry if a matching entry not found.
    if (!found)
    {
        StringHighlightData stringHighlightData = {};
        stringHighlightData.m_startLocation = startLocation;
        stringHighlightData.m_endLocation = startLocation + length;
        stringHighlightData.m_highlightString = searchString;
        stringHighlightData.m_highlightColor = Qt::yellow;
        m_stringHighlightData.push_back(stringHighlightData);
    }
}