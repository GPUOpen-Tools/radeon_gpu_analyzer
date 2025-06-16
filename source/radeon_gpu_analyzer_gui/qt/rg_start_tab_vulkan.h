//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for a Vulkan-specific implementation of the start tab.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_START_TAB_VULKAN_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_START_TAB_VULKAN_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_start_tab.h"

// A Vulkan-specific implementation of the start tab.
class RgStartTabVulkan : public RgStartTab
{
    Q_OBJECT

public:
    RgStartTabVulkan(QWidget* parent);
    virtual ~RgStartTabVulkan() = default;

signals:
    // Signal emitted when the user clicks the "Create Graphics pipeline" button.
    void CreateGraphicsPipelineEvent();

    // Signal emitted when the user clicks the "Create Compute pipeline" button.
    void CreateComputePipelineEvent();

protected:
    // Apply API-specific string constants to the view object's widgets.
    virtual void ApplyApiStringConstants() override;

    // Get a list of buttons to insert into the start page's "Start" section.
    virtual void GetStartButtons(std::vector<QPushButton*>& start_buttons) override;

private:
    // Initialize the start buttons.
    void InitializeStartButtons();

    // Connect signals for the API-specific start page items.
    void ConnectSignals();

    // A QPushButton used to create a new graphics pipeline project.
    QPushButton* create_graphics_pipeline_push_button_ = nullptr;

    // A QPushButton used to create a new compute pipeline project.
    QPushButton* create_compute_pipeline_push_button_ = nullptr;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_START_TAB_VULKAN_H_
