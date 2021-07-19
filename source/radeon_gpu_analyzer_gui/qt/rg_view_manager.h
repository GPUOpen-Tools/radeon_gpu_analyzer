#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_VIEW_MANAGER_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_VIEW_MANAGER_H_

// Qt.
#include <QObject>

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_view_container.h"

class RgViewManager : public QObject
{
    Q_OBJECT

public:
    // An enumeration to indicate the view type.
    enum RgViewManagerViewContainerIndex
    {
        kFileMenu,
        kSourceView,
        kDisassemblyView,
        kBuildOutputView,
        kCount
    };

    // An enumeration to keep track of current view
    // when either PSO editor or build settings is shown.
    enum RgCurrentFocusedIndex
    {
        kFileMenuCurrent,
        kBuildSettingsViewCurrent,
        kPsoEditorViewCurrent,
        kBuildOutputViewCurrent
    };

    RgViewManager(QWidget* parent);
    virtual ~RgViewManager();

    // Add a view to the view container vector.
    // The boolean is_active is used for two purposes:
    // 1. To skip a container while navigating through the containers using Ctrl+TAB.
    // 2. To find an inactive parent container to assign focus to for a child widget
    //    when this child widget does not have a parent in the active container list.
    void AddView(RgViewContainer* view_container, bool is_active = true);

    // Add a view to the view container vector at the specified index.
    void AddView(RgViewContainer* view_container, bool is_active, int index);

    // Set the focus to disassembly view.
    void SetDisassemblyViewFocus();

    // Set the focus to output window.
    void SetOutputWindowFocus();

    // Set the focus to source window.
    void SetSourceWindowFocus();

    // Transfer focus to the next view in the array of view containers.
    void FocusNextView();

    // Transfer focus to the previous view in the array of view containers.
    void FocusPrevView();

    // Set/reset source view current boolean.
    void SetIsSourceViewCurrent(bool value);

    // Set/reset build settings view current boolean.
    void SetIsBuildSettingsViewCurrent(bool value);

    // Set/reset PSO Editor view current boolean.
    void SetIsPsoEditorViewCurrent(bool value);

    // Switch the container size.
    void SwitchContainerSize();

    // Set the currently focused view.
    void SetCurrentFocusedView(RgCurrentFocusedIndex current_focus_index);

signals:
    // A signal to indicate the frame gaining the focus.
    void FrameFocusInSignal();

    // A signal to indicate the frame losing the focus.
    void FrameFocusOutSignal();

    // A signal to indicate the build settings widget getting the focus.
    void BuildSettingsWidgetFocusInSignal();

    // A signal to indicate the build settings widget losing the focus.
    void BuildSettingsWidgetFocusOutSignal();

    // A signal to indicate the PSO Editor widget getting the focus.
    void PsoEditorWidgetFocusInSignal();

    // A signal to indicate the PSO Editor widget losing the focus.
    void PsoEditorWidgetFocusOutSignal();

private:
    void ApplyViewFocus();
    void ClearFocusedView();
    void CreateActions();
    int GetFocusIndex(RgViewManagerViewContainerIndex view_container_index);
    void SetFocusedView(RgViewContainer* container);
    void SetFocusedViewIndex(int index);

    // View container lists.
    std::vector<RgViewContainer*> view_containers_;
    std::vector<RgViewContainer*> inactive_view_containers_;

    // Focused view.
    RgViewContainer* focus_view_container_ = nullptr;
    int focus_view_index_;

    // Parent widget (needed so actions can be added to it).
    QWidget* parent_ = nullptr;

    // Focus navigation actions.
    QAction* focus_next_view_action_ = nullptr;
    QAction* focus_prev_view_action_ = nullptr;

    // Indicate the type of view.
    bool is_source_view_current_ = true;

    // Indicate if build settings view is current.
    bool is_build_settings_view_current_ = false;

    // Indicate if PSO Editor view is current.
    bool is_pso_editor_view_current_ = false;

    // Keep track of focused window.
    RgCurrentFocusedIndex current_focused_view_ = RgCurrentFocusedIndex::kBuildSettingsViewCurrent;

private slots:
    // Handler invoked when the application is about to quit.
    void HandleApplicationAboutToQuit();

    // Handlers for focus navigation actions.
    void HandleFocusNextViewAction();
    void HandleFocusPrevViewAction();

    // Handler for when the qt focused object changes.
    void HandleFocusObjectChanged(QObject* object);
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_VIEW_MANAGER_H_
