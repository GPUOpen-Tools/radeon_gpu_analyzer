//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for RgEditorElementArrayElementRemove class.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_EDITOR_ELEMENT_ARRAY_ELEMENT_REMOVE_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_EDITOR_ELEMENT_ARRAY_ELEMENT_REMOVE_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_editor_element.h"

// Forward declarations.
class RgEditorElementArrayElementAdd;
class RgPipelineStateEditorWidgetArrayElementRemove;

class RgEditorElementArrayElementRemove : public RgEditorElement
{
    Q_OBJECT

public:
    RgEditorElementArrayElementRemove(QWidget* parent, const std::string& member_name);
    virtual ~RgEditorElementArrayElementRemove() = default;

    // Retrieve the editor widget used by the row.
    virtual RgPipelineStateEditorWidget* GetEditorWidget() override;

    // Update the array root element and child element index.
    void SetElementIndex(RgEditorElementArrayElementAdd* parent_array, int child_index);

private slots:
    // A handler invoked when the delete button is clicked.
    void HandleDeleteButtonClicked();

private:
    // Connect signals to slots.
    void ConnectSignals();

    // The index of the associated child element.
    int child_index_;

    // The array element being resized.
    RgEditorElementArrayElementAdd* array_root_element_ = nullptr;

    // The widget used to remove a single array element.
    RgPipelineStateEditorWidgetArrayElementRemove* editor_widget_ = nullptr;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_EDITOR_ELEMENT_ARRAY_ELEMENT_REMOVE_H_
