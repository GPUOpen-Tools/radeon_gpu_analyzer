#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuGraphics.h>

// Qt.
#include <QAction>
#include <QMenu>

// Forward declarations.
class rgPipelineStateModel;
class rgPipelineStateView;

// A special rgBuildView base class used in Graphics API modes.
class rgBuildViewGraphics : public rgBuildView
{
    Q_OBJECT

public:
    explicit rgBuildViewGraphics(rgProjectAPI api, QWidget* pParent = nullptr);
    virtual ~rgBuildViewGraphics() = default;

    // Get the graphics file menu (a.k.a pipeline menu, left panel).
    virtual rgMenuGraphics* GetGraphicsFileMenu() = 0;

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
    void HandleLoadPipelineStateFile(const std::string& filePath);

    // A handler invoked when the pipeline state file should be saved.
    virtual bool HandlePipelineStateFileSaved() = 0;

    // A handler invoked when a pipeline state file should be loaded.
    virtual void HandlePipelineStateFileLoaded() = 0;

    // Handler invoked when the state editor is hidden.
    void HandleStateEditorHidden();

protected:
    // Get the pipeline state model instance.
    virtual rgPipelineStateModel* GetPipelineStateModel() = 0;

    // Load the pipeline state file from the given file path.
    virtual bool LoadPipelineStateFile(const std::string& pipelineStateFilePath) = 0;

    // Handle graphics-mode-specific rgBuildView switching.
    virtual void HandleModeSpecificEditMode(EditMode newMode) override;

    // Helper function used to toggle the visibility of the find widget.
    virtual void ToggleFindWidgetVisibility(bool isVisible) override;

    // Recompute the location and geometry of the rgFindTextWidget.
    virtual void UpdateFindWidgetGeometry() override;

    // Connect signals to the PSO editor's find widget.
    void ConnectPSOFindSignals();

    // The index of the pipeline state being edited.
    int m_pipelineStateIndex;

    // A view used to edit the pipeline state configuration.
    rgPipelineStateView* m_pPipelineStateView = nullptr;

    // The frame used to hold the pipeline state editor in a border.
    QFrame* m_pPsoEditorFrame = nullptr;

    // A find widget used for PSO editor.
    rgFindTextWidget* m_pPsoFindWidget = nullptr;

    // Context menu for adding/creating file.
    QMenu* m_pAddCreateContextMenu = nullptr;

    // Open existing file action.
    QAction *m_pActionAddExistingFile = nullptr;

    // Create new file action.
    QAction *m_pActionCreateNewFile = nullptr;

private:
    // Open the Pipeline State settings view within the rgBuildView.
    void OpenPipelineStateSettingsView();
};