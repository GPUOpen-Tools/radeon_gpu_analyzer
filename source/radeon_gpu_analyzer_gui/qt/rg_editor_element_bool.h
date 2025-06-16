//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for RgEditorElementBool class.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_EDITOR_ELEMENT_BOOL_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_EDITOR_ELEMENT_BOOL_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_editor_element.h"

// Forward declarations.
class RgPipelineStateEditorWidgetBool;

class RgEditorElementBool : public RgEditorElement
{
    Q_OBJECT

public:
    RgEditorElementBool(QWidget* parent, const std::string& member_name, uint32_t* value);
    virtual ~RgEditorElementBool() = default;

    // Get the data held within this item.
    virtual QVariant Data(int column) const;

    // Retrieve the editor widget used by the row.
    virtual RgPipelineStateEditorWidget* GetEditorWidget() override;

    // Get the current value of the element.
    bool GetValue() const;

    // Set the current value of the element.
    void SetValue(bool value);

protected:
    // A handler invoked when the user has changed the editable value.
    virtual void ValueChangedHandler() override;

private:
    // The current value of the element.
    uint32_t* value_ = nullptr;

    // The widget used to edit the boolean value.
    RgPipelineStateEditorWidgetBool* editor_widget_ = nullptr;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_EDITOR_ELEMENT_BOOL_H_
