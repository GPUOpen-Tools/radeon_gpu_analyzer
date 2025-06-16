//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for a Vulkan-mode-specific implementation of the RgAppState.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_APP_STATE_VULKAN_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_APP_STATE_VULKAN_H_

// C++
#include <memory>

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_app_state.h"

// Forward declarations.
class RgBuildViewVulkan;
class RgStartTabVulkan;
class QAction;
struct RgProject;

// A Vulkan-mode-specific implementation of the RgAppState.
class RgAppStateVulkan : public RgAppStateGraphics
{
    Q_OBJECT

public:
    RgAppStateVulkan()          = default;
    virtual ~RgAppStateVulkan() = default;

    // Reset the RgBuildView instance used to display the current project.
    virtual void ResetBuildView() override;

    // Connect signals with the RgBuildView.
    virtual void ConnectBuildViewSignals(RgBuildView* build_view) override;

    // Get the start tab instance.
    virtual RgStartTab* GetStartTab() override;

    // Get the state's RgBuildView instance.
    virtual RgBuildView* GetBuildView() override;

    // Set the start tab instance.
    virtual void SetStartTab(RgStartTab* start_tab) override;

    // Cleanup the state before destroying it.
    virtual void Cleanup(QMenu* menu_bar) override;

    // Update the view when a project build is started.
    virtual void HandleProjectBuildStarted() override;

    // Reset the view after a build is completed or canceled.
    virtual void ResetViewStateAfterBuild() override {}

    // Open the specified files in build view.
    void OpenFilesInBuildView(const QStringList& file_paths) override;

    // Get the application stylesheet for this app state.
    virtual void GetApplicationStylesheet(std::vector<std::string>& stylesheet_file_names) override;

    // Get the main window stylesheet for this state.
    virtual void GetMainWindowStylesheet(std::vector<std::string>& stylesheet_file_names) override;

    // Handle switching to the pipeline state view.
    virtual void HandlePipelineStateEvent() override;

    // Get the global application settings view stylesheet for this state.
    virtual std::string GetGlobalSettingsViewStylesheet() const override;

    // Get the build application settings view stylesheet for this state.
    virtual std::string GetBuildSettingsViewStylesheet() const override;

public slots:
    void HandleProjectLoaded(std::shared_ptr<RgProject>);

private slots:
    // A handler invoked when the "Create new graphics pipeline" signal is emitted.
    void HandleCreateNewGraphicsPipeline();

    // A handler invoked when the "Create new compute pipeline" signal is emitted.
    void HandleCreateNewComputePipeline();

protected:
    // Create API-specific file actions.
    virtual void CreateApiSpecificFileActions(QMenu* menu_bar) override;

    // Create the AppState's internal RgBuildView instance.
    virtual void CreateBuildView() override;

    // Connect file menu actions.
    virtual void ConnectFileMenuActions() override;

private:
    // The start tab used in OpenCL mode.
    RgStartTabVulkan* start_tab_ = nullptr;

    // The build view used in Vulkan mode.
    RgBuildViewVulkan* build_view_ = nullptr;

    // An action responsible for creating a new graphics pipeline.
    QAction* new_graphics_pipeline_action_ = nullptr;

    // An action responsible for creating a new compute pipeline.
    QAction* new_compute_pipeline_action_ = nullptr;
};
#endif  // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_APP_STATE_VULKAN_H_
