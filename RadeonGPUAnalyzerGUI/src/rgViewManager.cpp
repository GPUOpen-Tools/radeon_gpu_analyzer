
// C++.
#include <cassert>

// Qt.
#include <QAction>
#include <QApplication>

// Local
#include <RadeonGPUAnalyzerGUI/include/qt/rgViewManager.h>
#include <RadeonGPUAnalyzerGUI/include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>

rgViewManager::rgViewManager(QWidget* pParent) :
    m_pParent(pParent),
    m_focusViewIndex(-1)
{
    CreateActions();

    // Handler for when the focus object changes.
    bool isConnected = connect(qApp, &QGuiApplication::focusObjectChanged, this, &rgViewManager::HandleFocusObjectChanged);
    assert(isConnected);

    // Handler for when the application is about to quit.
    isConnected = connect(qApp, &QCoreApplication::aboutToQuit, this, &rgViewManager::HandleApplicationAboutToQuit);
    assert(isConnected);
}

rgViewManager::~rgViewManager()
{

}

void rgViewManager::CreateActions()
{
    // Focus next view action.
    m_pFocusNextViewAction = new QAction(this);
    m_pFocusNextViewAction->setShortcutContext(Qt::ApplicationShortcut);
    m_pFocusNextViewAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_NEXT_VIEW));
    m_pParent->addAction(m_pFocusNextViewAction);

    bool isConnected = connect(m_pFocusNextViewAction, &QAction::triggered, this, &rgViewManager::HandleFocusNextViewAction);
    assert(isConnected);

    // Focus previous view action.
    m_pFocusPrevViewAction = new QAction(this);
    m_pFocusPrevViewAction->setShortcutContext(Qt::ApplicationShortcut);
    m_pFocusPrevViewAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_PREVIOUS_VIEW));
    m_pParent->addAction(m_pFocusPrevViewAction);

    isConnected = connect(m_pFocusPrevViewAction, &QAction::triggered, this, &rgViewManager::HandleFocusPrevViewAction);
    assert(isConnected);
}

void rgViewManager::AddView(rgViewContainer* pViewContainer, bool isActive)
{
    if (isActive)
    {
        m_viewContainers.push_back(pViewContainer);
    }
    else
    {
        m_inactiveViewContainers.push_back(pViewContainer);
    }
}

void rgViewManager::FocusNextView()
{
    // Increment focus index.
    int newFocusIndex = m_focusViewIndex + 1;
    if (newFocusIndex >= m_viewContainers.size())
    {
        newFocusIndex = 0;
    }

    // Change the view focus index.
    SetFocusedViewIndex(newFocusIndex);

    // Apply the focus change.
    ApplyViewFocus();
}

void rgViewManager::FocusPrevView()
{
    // Decrement focus index.
    int newFocusIndex = m_focusViewIndex - 1;
    if (newFocusIndex < 0)
    {
        newFocusIndex = static_cast<int>(m_viewContainers.size()) - 1;
    }

    // Change the view focus index.
    SetFocusedViewIndex(newFocusIndex);

    // Apply the focus change.
    ApplyViewFocus();
}

void rgViewManager::SetFocusedView(rgViewContainer* pViewContainer)
{
    if (m_pFocusViewContainer != nullptr)
    {
        // Get old focused view container.
        rgViewContainer* pOldFocusContainer = m_pFocusViewContainer;

        // Set focus state.
        pOldFocusContainer->SetFocusedState(false);
    }

    // Set new focused view container.
    m_pFocusViewContainer = pViewContainer;

    // Set focus state.
    m_pFocusViewContainer->SetFocusedState(true);
}

void rgViewManager::SetFocusedViewIndex(int index)
{
    if (index >= 0 && index < m_viewContainers.size())
    {
        // Set focus index.
        m_focusViewIndex = index;

        // Get container at the focus index.
        rgViewContainer* newViewContainer = m_viewContainers[m_focusViewIndex];

        // Set the focused view.
        SetFocusedView(newViewContainer);
    }
}

void rgViewManager::ClearFocusedView()
{
    // Clear focused state of currently focused view container.
    if (m_pFocusViewContainer != nullptr)
    {
        m_pFocusViewContainer->SetFocusedState(false);
    }

    // Invalidate focus index.
    m_focusViewIndex = -1;
    m_pFocusViewContainer = nullptr;
}

void rgViewManager::ApplyViewFocus()
{
    // Focus in on the widget.
    if (m_pFocusViewContainer != nullptr)
    {
        // Get widget to focus on
        QWidget* pFocusWidget = m_pFocusViewContainer->GetMainWidget();

        if (pFocusWidget != nullptr)
        {
            pFocusWidget->setFocus();
        }
    }
}

void rgViewManager::HandleApplicationAboutToQuit()
{
    // Remove all references to existing containers so widgets aren't re-polished during shutdown.
    m_viewContainers.clear();
    m_inactiveViewContainers.clear();
}

void rgViewManager::HandleFocusNextViewAction()
{
    FocusNextView();
}

void rgViewManager::HandleFocusPrevViewAction()
{
    FocusPrevView();
}

int FindAncestorContainerIndex(const QWidget* pWidget, const std::vector<rgViewContainer*>& containerList)
{
    int ret = -1;

    // Search the list to find a container that is an ancestor of the given widget.
    for (int i = 0; i < containerList.size(); i++)
    {
        rgViewContainer* pViewContainer = containerList[i];

        // If the focus widget is a child of a view container, give that container view focus.
        if (pViewContainer->isAncestorOf(pWidget))
        {
            ret = i;
            break;
        }
    }

    return ret;
}

void rgViewManager::HandleFocusObjectChanged(QObject* pObject)
{
    QWidget* pWidget = qobject_cast<QWidget*>(pObject);

    if (pWidget != nullptr)
    {
        // Find appropriate view container to switch view focus to.
        int focusIndex = FindAncestorContainerIndex(pWidget, m_viewContainers);

        if (focusIndex >= 0)
        {
            SetFocusedViewIndex(focusIndex);
        }
        else
        {
            // If no container in the main list exists, check the inactive container list.
            int inactiveIndex = FindAncestorContainerIndex(pWidget, m_inactiveViewContainers);

            if (inactiveIndex >= 0)
            {
                // Get the view from the inactive view list.
                rgViewContainer* pView = m_inactiveViewContainers[inactiveIndex];

                // Set the focused view.
                SetFocusedView(pView);
            }
            else
            {
                ClearFocusedView();
            }
        }
    }
    else
    {
        ClearFocusedView();
    }
}
