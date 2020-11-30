#pragma once

// Qt.
#include <QVariant>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElement.h>

// Forward declarations.
class rgPipelineStateEditorWidgetEnum;

// A structure containing the name string of an enumerator as well as the value.
struct rgEnumNameValuePair
{
    // The enumerator member name.
    std::string m_name;

    // The enum member value.
    uint32_t m_value;
};

// Surround the given value in quotes.
#define QUOTE(x) #x

// Define a name/value pair for an enumeration member.
#define ENUM_VALUE(x) rgEnumNameValuePair { QUOTE(x), static_cast<uint32_t>(x) }

// A vector of enumeration name/value pairs.
typedef std::vector<rgEnumNameValuePair> rgEnumValuesVector;

// A predicate used to find an enumerator with a specific value.
struct rgEnumeratorSearcher
{
    rgEnumeratorSearcher(uint32_t value) : m_valueToSearchFor(value) { }

    // Check if the given name/value pair matches the value being searched for.
    bool operator()(const rgEnumNameValuePair& enumeratorPair) const
    {
        return enumeratorPair.m_value == m_valueToSearchFor;
    }

    // The enumerator value to match against.
    uint32_t m_valueToSearchFor;
};

// An editable enumeration value.
class rgEditorElementEnum : public rgEditorElement
{
    Q_OBJECT

public:
    rgEditorElementEnum(QWidget* pParent, const std::string& memberName, const rgEnumValuesVector& enumerators, uint32_t* pValue, bool isBitFlags = false);
    virtual ~rgEditorElementEnum() = default;

    // Get the data held within this item.
    virtual QVariant Data(int column) const override;

    // Retrieve the editor widget used by the row.
    virtual rgPipelineStateEditorWidget* GetEditorWidget() override;

    // Retrieve a vector of possible enumerators to set the value to.
    const rgEnumValuesVector& GetEnumerators() const;

    // Get the current value of the element.
    uint32_t GetValue() const;

    // Set the current value of the element.
    void SetValue(uint32_t value);

signals:
    // A signal to indicate change of view.
    void HotKeyPressedSignal();

protected:
    // A handler invoked when the user has changed the editable value.
    virtual void ValueChangedHandler() override;

private:
    // A vector of the enumeration values that are possible to set in this element.
    rgEnumValuesVector m_enumerators;

    // The current enum value.
    uint32_t* m_pValue = nullptr;

    // The widget used to edit enumeration values.
    rgPipelineStateEditorWidgetEnum* m_pEditorWidget = nullptr;

    // Is the enumeration a of bit flags?
    bool m_isBitFlagsEnum = false;

    // The parent object.
    QWidget* m_pParent = nullptr;
};