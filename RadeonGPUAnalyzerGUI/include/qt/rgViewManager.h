#pragma once

// Qt.
#include <QObject>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgViewContainer.h>

class rgViewManager : public QObject
{
    Q_OBJECT

public:
    rgViewManager(QWidget* pParent);
    virtual ~rgViewManager();

    void AddView(rgViewContainer* pViewContainer, bool isActive = true);

    // Transfer focus to the next view in the array of view containers.
    void FocusNextView();

    // Transfer focus to the previous view in the array of view containers.
    void FocusPrevView();

private:
    void CreateActions();
    void SetFocusedView(rgViewContainer* pContainer);
    void SetFocusedViewIndex(int index);
    void ClearFocusedView();
    void ApplyViewFocus();

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

private slots:
    // Handler invoked when the application is about to quit.
    void HandleApplicationAboutToQuit();

    // Handlers for focus navigation actions.
    void HandleFocusNextViewAction();
    void HandleFocusPrevViewAction();

    // Handler for when the qt focused object changes.
    void HandleFocusObjectChanged(QObject* pObject);
};