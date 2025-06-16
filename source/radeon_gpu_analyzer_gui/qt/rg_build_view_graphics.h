//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for Build View class for Graphics APIs.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_VIEW_GRAPHICS_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_VIEW_GRAPHICS_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_build_view.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_menu_graphics.h"

// Qt.
#include <QAction>
#include <QMenu>

// Forward declarations.
class RgPipelineStateModel;
class RgPipelineStateView;

// A special RgBuildView base class used in Graphics API modes.
class RgBuildViewGraphics : public RgBuildView
{
    Q_OBJECT

public:
    explicit RgBuildViewGraphics(RgProjectAPI api, QWidget* parent = nullptr);
    virtual ~RgBuildViewGraphics() = default;

    // Get the graphics file menu (a.k.a pipeline menu, left panel).
    virtual RgMenuGraphics* GetGraphicsFileMenu() = 0;

signals:
    // A signal emitted when a new PSO file has been loaded into the PSO editor.
    void PsoFileLoaded();

public slots:
    // Handler invoked when the Pipeline State button is clicked.
    void HandlePipelineStateSettingsMenuButtonClicked();

    // Handler invoked when the state editor is resized.
    void HandleStateEditorResized();

protected slots:
    // Handler invoked when the find widget should be toggled.
    void HandleFindWidgetVisibilityToggled();

    // Handler invoked when a new pipeline state file needs to be loaded.
    void HandleLoadPipelineStateFile(const std::string& file_path);

    // A handler invoked when the pipeline state file should be saved.
    virtual bool HandlePipelineStateFileSaved() = 0;

    // A handler invoked when a pipeline state file should be loaded.
    virtual void HandlePipelineStateFileLoaded() = 0;

    // Handler invoked when the state editor is hidden.
    void HandleStateEditorHidden();

protected:
    // Get the pipeline state model instance.
    virtual RgPipelineStateModel* GetPipelineStateModel() = 0;

    // Load the pipeline state file from the given file path.
    virtual bool LoadPipelineStateFile(const std::string& pipeline_state_file_path) = 0;

    // Handle graphics-mode-specific RgBuildView switching.
    virtual void HandleModeSpecificEditMode(EditMode now_mode) override;

    // Helper function used to toggle the visibility of the find widget.
    virtual void ToggleFindWidgetVisibility(bool is_visible) override;

    // Recompute the location and geometry of the RgFindTextWidget.
    virtual void UpdateFindWidgetGeometry() override;

    // Connect signals to the PSO editor's find widget.
    void ConnectPsoFindSignals();

    // The index of the pipeline state being edited.
    int pipeline_state_index_;

    // A view used to edit the pipeline state configuration.
    RgPipelineStateView* pipeline_state_view_ = nullptr;

    // The frame used to hold the pipeline state editor in a border.
    QFrame* pso_editor_frame_ = nullptr;

    // A find widget used for PSO editor.
    RgFindTextWidget* pso_find_widget_ = nullptr;

    // Context menu for adding/creating file.
    QMenu* add_create_context_menu_ = nullptr;

    // Open existing file action.
    QAction* action_add_existing_file_ = nullptr;

    // Create new file action.
    QAction* action_create_new_file_ = nullptr;

private:
    // Open the Pipeline State settings view within the RgBuildView.
    void OpenPipelineStateSettingsView();
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_BUILD_VIEW_GRAPHICS_H_
