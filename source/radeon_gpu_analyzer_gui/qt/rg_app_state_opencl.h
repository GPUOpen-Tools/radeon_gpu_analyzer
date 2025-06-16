//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for a OpenCL-mode-specific implementation of the RgAppState.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_APP_STATE_OPENCL_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_APP_STATE_OPENCL_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_app_state.h"

// Forward declarations.
class RgBuildViewOpencl;
class RgStartTabOpencl;
class QAction;

// An OpenCL-mode-specific implementation of the RgAppState.
class RgAppStateOpencl : public RgAppState
{
    Q_OBJECT

public:
    RgAppStateOpencl()          = default;
    virtual ~RgAppStateOpencl() = default;

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

    // Reset the view after a build is completed or cancelled.
    virtual void ResetViewStateAfterBuild() override;

    // Open the specified files in build view.
    void OpenFilesInBuildView(const QStringList& file_paths) override;

    // Get the application stylesheet for this app state.
    virtual void GetApplicationStylesheet(std::vector<std::string>& stylesheet_file_names) override;

    // Get the main window stylesheet for this state.
    virtual void GetMainWindowStylesheet(std::vector<std::string>& stylesheet_file_names) override;

    // Get the stylesheet for the application settings.
    virtual std::string GetGlobalSettingsViewStylesheet() const override;

    // Get the stylesheet for the build settings.
    virtual std::string GetBuildSettingsViewStylesheet() const override;

private slots:
    // A handler invoked when the "Create new CL file" signal is emitted.
    void HandleCreateNewCLFile();

    // A handler invoked when the "Open existing CL file" signal is emitted.
    void HandleOpenExistingCLFile();

protected:
    // Create API-specific file actions.
    virtual void CreateApiSpecificFileActions(QMenu* menu_bar) override;

    // Create the AppState's internal RgBuildView instance.
    virtual void CreateBuildView() override;

    // Connect file menu actions.
    virtual void ConnectFileMenuActions() override;

private:
    // The start tab used in OpenCL mode.
    RgStartTabOpencl* start_tab_ = nullptr;

    // The build view used in Vulkan mode.
    RgBuildViewOpencl* build_view_ = nullptr;

    // An action responsible for creating a new CL source file.
    QAction* new_file_action_ = nullptr;

    // The action used to open a source file.
    QAction* open_file_action_ = nullptr;
};
#endif  // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_APP_STATE_OPENCL_H_
