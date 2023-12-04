#pragma once

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_app_state.h"

// Forward declarations.
class RgBuildViewBinary;
class RgStartTabBinary;
class QAction;

// A Binary-mode-specific implementation of the RgAppState.
class RgAppStateBinary : public RgAppStateAnalysis
{
    Q_OBJECT

public:
    RgAppStateBinary()          = default;
    virtual ~RgAppStateBinary() = default;

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

    // A handler invoked when the "Load Code objeect Binary existing  file" signal is emitted.
    void HandleLoadCodeObjectBinary();

protected:
    // Create API-specific file actions.
    virtual void CreateApiSpecificFileActions(QMenu* menu_bar) override;

    // Create the AppState's internal RgBuildView instance.
    virtual void CreateBuildView() override;

    // Connect file menu actions.
    virtual void ConnectFileMenuActions() override;

private:
    // The start tab used in OpenCL mode.
    RgStartTabBinary* start_tab_ = nullptr;

    // The build view used in Vulkan mode.
    RgBuildViewBinary* build_view_ = nullptr;

    // The action used to open a codeobj file.
    QAction* open_bin_file_action_ = nullptr;
};
