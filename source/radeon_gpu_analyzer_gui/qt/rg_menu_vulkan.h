//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for RGA Build view's File Menu for Vulkan mode.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_VULKAN_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_VULKAN_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_menu_graphics.h"

class RgMenuVulkan : public RgMenuGraphics
{
    Q_OBJECT

public:
    RgMenuVulkan(QWidget* parent);
    virtual ~RgMenuVulkan() = default;

    // Connect the signals for the view's default menu items.
    virtual void ConnectDefaultItemSignals() override;

    // Connect signals for the menu file item.
    virtual void ConnectMenuFileItemSignals(RgMenuFileItem* menu_item) override;

    // Deselect all menu items.
    virtual void DeselectItems() override;

    // Clear the stage's shader source file.
    void ClearStageSourceFile(RgPipelineStage stage);

    // Set the stage's shader source file.
    // Returns "true" if the file has been added succesfully or "false" otherwise.
    bool SetStageSourceFile(RgPipelineStage stage, const std::string& full_path, RgVulkanInputType file_type, bool is_new_file_item);

    // Replace the stage's shader file with another file.
    // This function should be used to replace a SPIR-V file with its disassembly version and vice versa.
    bool ReplaceStageFile(RgPipelineStage stage, const std::string& new_file_path, RgVulkanInputType file_type);

    // Check to see if any of the buttons are pressed.
    bool IsButtonPressed() const;

    // Update the focus indices.
    void UpdateFocusIndex();

public slots:
    // Handler invoked when the user presses the activate key (Enter).
    // This is used to handle pressing add/create buttons when in focus.
    virtual void HandleActivateItemAction() override;

    // Handler invoked when the user changes the currently selected file item.
    virtual void HandleSelectedFileChanged(RgMenuFileItem* selected) override;

private:
    // Process sub button actions.
    virtual void ProcessSubButtonAction() override;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_VULKAN_H_
