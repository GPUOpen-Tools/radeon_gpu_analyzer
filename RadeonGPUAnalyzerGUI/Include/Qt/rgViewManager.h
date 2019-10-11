#pragma once

// Qt.
#include <QObject>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgViewContainer.h>

class rgViewManager : public QObject
{
    Q_OBJECT

public:
    // An enumeration to indicate the view type.
    enum rgViewManagerViewContainerIndex
    {
        FileMenu,
        SourceView,
        DisassemblyView,
        BuildOutputView,
        Count
    };

    // An enumeration to keep track of current view
    // when either PSO editor or build settings is shown.
    enum rgCurrentFocusedIndex
    {
        FileMenuCurrent,
        BuildSettingsViewCurrent,
        PSOEditorViewCurrent,
        BuildOutputViewCurrent
    };

    rgViewManager(QWidget* pParent);
    virtual ~rgViewManager();

    // Add a view to the view container vector.
    // The boolean isActive is used for two purposes:
    // 1. To skip a container while navigating through the containers using Ctrl+TAB.
    // 2. To find an inactive parent container to assign focus to for a child widget
    //    when this child widget does not have a parent in the active container list.
    void AddView(rgViewContainer* pViewContainer, bool isActive = true);

    // Add a view to the view container vector at the specified index.
    void AddView(rgViewContainer* pViewContainer, bool isActive, int index);

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
    void SetIsPSOEditorViewCurrent(bool value);

    // Switch the container size.
    void SwitchContainerSize();

    // Set the currently focused view.
    void SetCurrentFocusedView(rgCurrentFocusedIndex rgCurrentFocusIndex);

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
    void PSOEditorWidgetFocusInSignal();

    // A signal to indicate the PSO Editor widget losing the focus.
    void PSOEditorWidgetFocusOutSignal();

private:
    void ApplyViewFocus();
    void ClearFocusedView();
    void CreateActions();
    int GetFocusIndex(rgViewManagerViewContainerIndex viewContainerIndex);
    void SetFocusedView(rgViewContainer* pContainer);
    void SetFocusedViewIndex(int index);

    // View container lists.
    std::vector<rgViewContainer*> m_viewContainers;
    std::vector<rgViewContainer*> m_inactiveViewContainers;

    // Focused view.
    rgViewContainer* m_pFocusViewContainer = nullptr;
    int m_focusViewIndex;

    // Parent widget (needed so actions can be added to it).
    QWidget* m_pParent = nullptr;

    // Focus navigation actions.
    QAction* m_pFocusNextViewAction = nullptr;
    QAction* m_pFocusPrevViewAction = nullptr;

    // Indicate the type of view.
    bool m_isSourceViewCurrent = true;

    // Indicate if build settings view is current.
    bool m_isBuildSettingsViewCurrent = false;

    // Indicate if PSO Editor view is current.
    bool m_isPSOEditorViewCurrent = false;

    // Keep track of focused window.
    rgCurrentFocusedIndex m_currentFocusedView = rgCurrentFocusedIndex::BuildSettingsViewCurrent;

private slots:
    // Handler invoked when the application is about to quit.
    void HandleApplicationAboutToQuit();

    // Handlers for focus navigation actions.
    void HandleFocusNextViewAction();
    void HandleFocusPrevViewAction();

    // Handler for when the qt focused object changes.
    void HandleFocusObjectChanged(QObject* pObject);
};
