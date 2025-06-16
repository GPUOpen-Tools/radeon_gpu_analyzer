//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for RgEditorElementEnum class.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_EDITOR_ELEMENT_ENUM_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_EDITOR_ELEMENT_ENUM_H_

// Qt.
#include <QVariant>

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_editor_element.h"

// Forward declarations.
class RgPipelineStateEditorWidgetEnum;

// A structure containing the name string of an enumerator as well as the value.
struct RgEnumNameValuePair
{
    // The enumerator member name.
    std::string name;

    // The enum member value.
    uint32_t value;
};

// Surround the given value in quotes.
#define QUOTE(x) #x

// Define a name/value pair for an enumeration member.
#define ENUM_VALUE(x) RgEnumNameValuePair { QUOTE(x), static_cast<uint32_t>(x) }

// A vector of enumeration name/value pairs.
typedef std::vector<RgEnumNameValuePair> RgEnumValuesVector;

// A predicate used to find an enumerator with a specific value.
struct RgEnumeratorSearcher
{
    RgEnumeratorSearcher(uint32_t value) : value_to_search_for(value) { }

    // Check if the given name/value pair matches the value being searched for.
    bool operator()(const RgEnumNameValuePair& enumeratorPair) const
    {
        return enumeratorPair.value == value_to_search_for;
    }

    // The enumerator value to match against.
    uint32_t value_to_search_for;
};

// An editable enumeration value.
class RgEditorElementEnum : public RgEditorElement
{
    Q_OBJECT

public:
    RgEditorElementEnum(QWidget* parent, const std::string& member_name, const RgEnumValuesVector& enumerators, uint32_t* value, bool is_bit_flags = false);
    virtual ~RgEditorElementEnum() = default;

    // Get the data held within this item.
    virtual QVariant Data(int column) const override;

    // Retrieve the editor widget used by the row.
    virtual RgPipelineStateEditorWidget* GetEditorWidget() override;

    // Retrieve a vector of possible enumerators to set the value to.
    const RgEnumValuesVector& GetEnumerators() const;

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
    RgEnumValuesVector enumerators_;

    // The current enum value.
    uint32_t* value_ = nullptr;

    // The widget used to edit enumeration values.
    RgPipelineStateEditorWidgetEnum* editor_widget_ = nullptr;

    // Is the enumeration a of bit flags?
    bool is_bit_flags_enum_ = false;

    // The parent object.
    QWidget* parent_ = nullptr;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_EDITOR_ELEMENT_ENUM_H_
