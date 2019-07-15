
// C++.
#include <cassert>

// Qt.
#include <QAction>
#include <QApplication>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgViewManager.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

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
    assert(m_pParent != nullptr);
    if (m_pParent != nullptr)
    {
        m_pParent->addAction(m_pFocusNextViewAction);
    }

    bool isConnected = connect(m_pFocusNextViewAction, &QAction::triggered, this, &rgViewManager::HandleFocusNextViewAction);
    assert(isConnected);

    // Focus previous view action.
    m_pFocusPrevViewAction = new QAction(this);
    m_pFocusPrevViewAction->setShortcutContext(Qt::ApplicationShortcut);
    m_pFocusPrevViewAction->setShortcut(QKeySequence(gs_ACTION_HOTKEY_PREVIOUS_VIEW));
    assert(m_pParent != nullptr);
    if (m_pParent != nullptr)
    {
        m_pParent->addAction(m_pFocusPrevViewAction);
    }

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

void rgViewManager::AddView(rgViewContainer* pViewContainer, bool isActive, int index)
{
    assert(index >= rgViewManagerViewContainerIndex::FileMenu);
    assert(index < rgViewManagerViewContainerIndex::Count);

    if (index >= rgViewManagerViewContainerIndex::FileMenu && index < rgViewManagerViewContainerIndex::Count)
    {
        if (isActive)
        {
            m_viewContainers.insert(m_viewContainers.begin() + index, pViewContainer);
        }
        else
        {
            m_inactiveViewContainers.insert(m_inactiveViewContainers.begin() + index, pViewContainer);
        }
    }
}

void rgViewManager::FocusNextView()
{
    // If the current view is output window, and it is
    // currently maximized, do not process this action.
    if (m_focusViewIndex >= 0)
    {
        rgViewContainer* pViewContainer = m_viewContainers.at(m_focusViewIndex);
        bool focusNextView = !(pViewContainer->IsInMaximizedState() && (pViewContainer->objectName().compare(STR_RG_BUILD_OUTPUT_VIEW_CONTAINER) == 0));
        if (focusNextView)
        {
            // Increment focus index.
            int newFocusIndex = m_focusViewIndex + 1;

            if (newFocusIndex >= m_viewContainers.size())
            {
                newFocusIndex = 0;
            }

            if (!m_isSourceViewCurrent)
            {
                if (m_currentFocusedView == rgCurrentFocusedIndex::FileMenuCurrent)
                {
                    // Set the focus to the hidden disassembly view to remove
                    // focus from the file menu.
                    newFocusIndex = GetFocusIndex(rgViewManagerViewContainerIndex::DisassemblyView);

                    // Change the view focus index.
                    SetFocusedViewIndex(newFocusIndex);

                    // Apply the focus change.
                    ApplyViewFocus();

                    if (m_isBuildSettingsViewCurrent)
                    {
                        emit BuildSettingsWidgetFocusInSignal();
                        m_currentFocusedView = rgCurrentFocusedIndex::BuildSettingsViewCurrent;
                    }
                    else if (m_isPSOEditorViewCurrent)
                    {
                        emit PSOEditorWidgetFocusInSignal();
                        m_currentFocusedView = rgCurrentFocusedIndex::PSOEditorViewCurrent;
                    }
                    else
                    {
                        // Should not get here.
                        assert(false);
                    }
                }
                else if (m_currentFocusedView == rgCurrentFocusedIndex::BuildOutputViewCurrent)
                {
                    emit BuildSettingsWidgetFocusOutSignal();
                    emit PSOEditorWidgetFocusOutSignal();
                    newFocusIndex = GetFocusIndex(rgViewManagerViewContainerIndex::FileMenu);
                    m_currentFocusedView = rgCurrentFocusedIndex::FileMenuCurrent;

                    // Change the view focus index.
                    SetFocusedViewIndex(newFocusIndex);

                    // Apply the focus change.
                    ApplyViewFocus();
                }
                else if ((m_currentFocusedView == rgCurrentFocusedIndex::BuildSettingsViewCurrent) ||
                    (m_currentFocusedView == rgCurrentFocusedIndex::PSOEditorViewCurrent))
                {
                    emit BuildSettingsWidgetFocusOutSignal();
                    emit PSOEditorWidgetFocusOutSignal();
                    newFocusIndex = GetFocusIndex(rgViewManagerViewContainerIndex::BuildOutputView);
                    m_currentFocusedView = rgCurrentFocusedIndex::BuildOutputViewCurrent;

                    // Change the view focus index.
                    SetFocusedViewIndex(newFocusIndex);

                    // Apply the focus change.
                    ApplyViewFocus();
                }
            }
            else
            {
                rgViewContainer* pViewContainer = m_viewContainers.at(newFocusIndex);
                while (pViewContainer->IsInHiddenState() && newFocusIndex < m_viewContainers.size())
                {
                    newFocusIndex++;
                    if (newFocusIndex >= m_viewContainers.size())
                    {
                        newFocusIndex = 0;
                    }
                    pViewContainer = m_viewContainers.at(newFocusIndex);
                }

                // Change the view focus index.
                SetFocusedViewIndex(newFocusIndex);

                // Apply the focus change.
                ApplyViewFocus();
            }
        }
    }
}

void rgViewManager::SwitchContainerSize()
{
    rgViewContainer* pViewContainer = m_viewContainers.at(m_focusViewIndex);
    assert(pViewContainer != nullptr);
    if (pViewContainer != nullptr)
    {
        pViewContainer->SwitchContainerSize();
    }
}

void rgViewManager::SetSourceWindowFocus()
{
    // Change the view focus index.
    SetFocusedViewIndex(rgViewManagerViewContainerIndex::SourceView);

    // Apply the focus change.
    ApplyViewFocus();
}

void rgViewManager::SetOutputWindowFocus()
{
    // Change the view focus index.
    SetFocusedViewIndex(rgViewManagerViewContainerIndex::BuildOutputView);

    // Apply the focus change.
    ApplyViewFocus();
}

void rgViewManager::SetDisassemblyViewFocus()
{
    // Change the view focus index.
    SetFocusedViewIndex(rgViewManagerViewContainerIndex::DisassemblyView);

    // Apply the focus change.
    ApplyViewFocus();
}

void rgViewManager::FocusPrevView()
{
    // If the current view is output window, and it is
    // currently maximized, do not process this action.
    if (m_focusViewIndex >= 0)
    {
        rgViewContainer* pViewContainer = m_viewContainers.at(m_focusViewIndex);
        bool focusPrevView = !(pViewContainer->IsInMaximizedState() && (pViewContainer->objectName().compare(STR_RG_BUILD_OUTPUT_VIEW_CONTAINER) == 0));
        if (focusPrevView)
        {
            int newFocusIndex = m_focusViewIndex - 1;

            if (!m_isSourceViewCurrent)
            {
                if (m_currentFocusedView == rgCurrentFocusedIndex::FileMenuCurrent)
                {
                    emit BuildSettingsWidgetFocusOutSignal();
                    emit PSOEditorWidgetFocusOutSignal();
                    newFocusIndex = GetFocusIndex(rgViewManagerViewContainerIndex::BuildOutputView);
                    m_currentFocusedView = rgCurrentFocusedIndex::BuildOutputViewCurrent;

                    // Change the view focus index.
                    SetFocusedViewIndex(newFocusIndex);

                    // Apply the focus change.
                    ApplyViewFocus();
                }
                else if (m_currentFocusedView == rgCurrentFocusedIndex::BuildOutputViewCurrent)
                {
                    // Set the focus to the hidden disassembly view to remove
                    // focus from the file menu.
                    newFocusIndex = GetFocusIndex(rgViewManagerViewContainerIndex::DisassemblyView);

                    // Change the view focus index.
                    SetFocusedViewIndex(newFocusIndex);

                    // Apply the focus change.
                    ApplyViewFocus();

                    if (m_isBuildSettingsViewCurrent)
                    {
                        emit BuildSettingsWidgetFocusInSignal();
                        m_currentFocusedView = rgCurrentFocusedIndex::BuildSettingsViewCurrent;
                    }
                    else if (m_isPSOEditorViewCurrent)
                    {
                        emit PSOEditorWidgetFocusInSignal();
                        m_currentFocusedView = rgCurrentFocusedIndex::PSOEditorViewCurrent;
                    }
                    else
                    {
                        // Should not get here.
                        assert(false);
                    }
                }
                else if ((m_currentFocusedView == rgCurrentFocusedIndex::BuildSettingsViewCurrent) ||
                    (m_currentFocusedView == rgCurrentFocusedIndex::PSOEditorViewCurrent))
                {
                    emit BuildSettingsWidgetFocusOutSignal();
                    emit PSOEditorWidgetFocusOutSignal();
                    newFocusIndex = GetFocusIndex(rgViewManagerViewContainerIndex::FileMenu);
                    m_currentFocusedView = rgCurrentFocusedIndex::FileMenuCurrent;

                    // Change the view focus index.
                    SetFocusedViewIndex(newFocusIndex);

                    // Apply the focus change.
                    ApplyViewFocus();
                }
            }
            else
            {
                // Decrement focus index.
                if (newFocusIndex < 0)
                {
                    newFocusIndex = static_cast<int>(m_viewContainers.size()) - 1;
                }

                rgViewContainer* pViewContainer = m_viewContainers.at(newFocusIndex);
                while (pViewContainer->IsInHiddenState() && newFocusIndex < m_viewContainers.size())
                {
                    newFocusIndex--;
                    if (newFocusIndex < 0)
                    {
                        newFocusIndex = static_cast<int>(m_viewContainers.size()) - 1;
                    }
                    pViewContainer = m_viewContainers.at(newFocusIndex);
                }

                // Change the view focus index.
                SetFocusedViewIndex(newFocusIndex);

                // Apply the focus change.
                ApplyViewFocus();
            }
        }
    }
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
    assert(m_pFocusViewContainer != nullptr);
    if (m_pFocusViewContainer != nullptr)
    {
        m_pFocusViewContainer->SetFocusedState(true);

        if ((m_pFocusViewContainer->objectName()).compare(STR_RG_ISA_DISASSEMBLY_VIEW_CONTAINER) != 0)
        {
            // Emit the frame out of focus signal.
            emit FrameFocusOutSignal();
        }
    }
}

void rgViewManager::SetFocusedViewIndex(int index)
{
    if (index >= 0 && index < m_viewContainers.size())
    {
        // Set focus index.
        m_focusViewIndex = index;

        // Get container at the focus index.
        rgViewContainer* pNewViewContainer = m_viewContainers[m_focusViewIndex];

        // Set the focused view.
        SetFocusedView(pNewViewContainer);
    }
}

void rgViewManager::ClearFocusedView()
{
    // Clear focused state of currently focused view container.
    // Having this on Linux causes a crash, so leaving it out of Linux build.
#ifndef __linux
    if (m_pFocusViewContainer != nullptr)
    {
        m_pFocusViewContainer->SetFocusedState(false);
    }
#endif // __linux

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

        // If the widget is disassembly view, also change the frame color.
        rgIsaDisassemblyView* pView = qobject_cast<rgIsaDisassemblyView*>(pFocusWidget);
        if (pView != nullptr)
        {
            emit FrameFocusInSignal();
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

void rgViewManager::SetIsSourceViewCurrent(bool value)
{
    m_isSourceViewCurrent = value;
}

void rgViewManager::SetIsBuildSettingsViewCurrent(bool value)
{
    m_isBuildSettingsViewCurrent = value;
}

void rgViewManager::SetIsPSOEditorViewCurrent(bool value)
{
    m_isPSOEditorViewCurrent = value;
}

int rgViewManager::GetFocusIndex(rgViewManagerViewContainerIndex viewContainerIndex)
{
    int focusIndex = 0;

    QString matchContainer;
    switch (viewContainerIndex)
    {
    case rgViewManagerViewContainerIndex::BuildOutputView:
        matchContainer = STR_RG_BUILD_OUTPUT_VIEW_CONTAINER;
        break;
    case rgViewManagerViewContainerIndex::FileMenu:
        matchContainer = STR_RG_FILE_MENU_VIEW_CONTAINER;
        break;
    case rgViewManagerViewContainerIndex::DisassemblyView:
        matchContainer = STR_RG_ISA_DISASSEMBLY_VIEW_CONTAINER;
        break;
    case rgViewManagerViewContainerIndex::SourceView:
        matchContainer = STR_RG_SOURCE_VIEW_CONTAINER;
        break;
    default:
        assert(false);
    }

    // Step through the view container vector and find the matching view container.
    for (const rgViewContainer* pViewContainer : m_viewContainers)
    {
        QString containerName = pViewContainer->objectName();
        if (containerName.compare(matchContainer) == 0)
        {
            break;
        }
        focusIndex++;
    }

    return focusIndex;
}
