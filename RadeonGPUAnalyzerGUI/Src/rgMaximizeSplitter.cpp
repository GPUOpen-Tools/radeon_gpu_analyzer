// C++.
#include <assert.h>

// Qt.
#include <QSplitter>
#include <QStyle>
#include <QVariant>
#include <QWidget>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMaximizeSplitter.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgViewContainer.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>

rgMaximizeSplitter::rgMaximizeSplitter(QWidget* pParent) :
    QSplitter(pParent)
{
}

void rgMaximizeSplitter::AddMaximizableWidget(rgViewContainer* pViewContainer)
{
    if (pViewContainer != nullptr)
    {
        ScalingManager& scalingManager = ScalingManager::Get();

        // Ensure the container is scaling registered.
        scalingManager.RegisterObject(pViewContainer);

        // Add the container to the list.
        m_maximizeContainers.push_back(pViewContainer);

        // Handle the corner button press on the maximization container.
        bool isConnected = connect(pViewContainer, &rgViewContainer::MaximizeButtonClicked, this, &rgMaximizeSplitter::HandleCornerButtonClicked);
        assert(isConnected);

        // Add the container to the splitter.
        addWidget(pViewContainer);

        pViewContainer->SetMaximizedState(false);
    }
}

QWidget* rgMaximizeSplitter::GetMaximizedWidget() const
{
    return m_pMaximizedWidget;
}

void rgMaximizeSplitter::MaximizeWidget(QWidget* pWidget)
{
    // Signal the view container to have the maximum width.
    emit ViewMaximized();

    // Give the border focus.
    emit FrameInFocusSignal();

    // If this splitter has a maximization container for the given widget, maximize the container instead.
    for (rgViewContainer* pContainer : m_maximizeContainers)
    {
        assert(pContainer != nullptr);
        if (pContainer != nullptr)
        {
            if (pContainer->GetMainWidget() == pWidget)
            {
                pWidget = pContainer;
                break;
            }
        }
    }

    // Confirm the widget is a child of this splitter.
    if (pWidget != nullptr && pWidget->parentWidget() == this)
    {
        // Ensure the widget is visible.
        pWidget->show();

        // Hide all other widgets in this splitter.
        for (QObject* pChild : children())
        {
            QWidget* pChildWidget = static_cast<QWidget*>(pChild);

            if (pChildWidget != pWidget)
            {
                pChildWidget->hide();
            }
        }

        // Handle cases when this splitter's parent is an rgMaximizeSplitter.
        rgMaximizeSplitter* pParentSplitter = qobject_cast<rgMaximizeSplitter*>(parentWidget());
        if (pParentSplitter != nullptr)
        {
            // Maximize this splitter within the parent splitter. This will recursively propagate
            // through all nested rgMaximizeSplitters.
            pParentSplitter->MaximizeWidget(this);
        }

        // Remember the maximized widget.
        m_pMaximizedWidget = pWidget;
    }
}

void rgMaximizeSplitter::Restore()
{
    // Signal the view container to have minimum required width.
    emit ViewRestored();

    if (m_pMaximizedWidget != nullptr)
    {
        // Show all widgets in this splitter.
        for (QObject* pChild : children())
        {
            QWidget* pChildWidget = static_cast<QWidget*>(pChild);
            pChildWidget->show();

            // If this widget is an rgViewContainer,
            // set the hidden state to false.
            rgViewContainer* pViewContainer = qobject_cast<rgViewContainer*>(pChildWidget);
            if(pViewContainer != nullptr)
            {
                pViewContainer->SetHiddenState(false);
            }
        }

        // Handle cases when this splitter's parent is an rgMaximizeSplitter.
        rgMaximizeSplitter* pParentSplitter = qobject_cast<rgMaximizeSplitter*>(parentWidget());
        if (pParentSplitter != nullptr)
        {
            // Restore the parent splitter. This will recursively propagate through all nested rgMaximizeSplitters.
            pParentSplitter->Restore();
        }

        // Clear the maximized widget.
        m_pMaximizedWidget = nullptr;
    }
}

void rgMaximizeSplitter::HandleCornerButtonClicked()
{
    // Get rgViewContainer which sent the signal.
    rgViewContainer* pContainer = qobject_cast<rgViewContainer*>(sender());

    // This should always be true, no other object type should be connected to this slot.
    bool isMaximizeContainer = (pContainer != nullptr);
    assert(isMaximizeContainer);

    if (isMaximizeContainer && pContainer->IsMaximizable())
    {
        if (m_pMaximizedWidget != pContainer)
        {
            // Maximize the container.
            MaximizeWidget(pContainer);

            pContainer->SetMaximizedState(true);
            pContainer->SetHiddenState(false);

            for (auto it = m_maximizeContainers.begin(); it != m_maximizeContainers.end(); it++)
            {
                rgViewContainer* pItem = *it;
                if (pItem != pContainer)
                {
                    if (pContainer->objectName().compare(STR_RG_ISA_DISASSEMBLY_VIEW_CONTAINER) == 0)
                    {
                        if (pItem->objectName().compare(STR_RG_SOURCE_VIEW_CONTAINER) == 0)
                        {
                            pItem->SetHiddenState(true);
                            break;
                        }
                    }
                    else if (pContainer->objectName().compare(STR_RG_SOURCE_VIEW_CONTAINER) == 0)
                    {
                        if (pItem->objectName().compare(STR_RG_ISA_DISASSEMBLY_VIEW_CONTAINER) == 0)
                        {
                            pItem->SetHiddenState(true);
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            // Restore to default layout.
            Restore();

            pContainer->SetMaximizedState(false);
        }
    }
}

void rgMaximizeSplitter::HandleChildDestroyed(QObject* pChild)
{
    QWidget* pDestroyedWidget = qobject_cast<QWidget*>(pChild);
    bool isWidget = (pDestroyedWidget != nullptr);
    assert(isWidget);

    // This is just a safe guard, it should never be untrue.
    if (isWidget)
    {
        // If the destroyed widget is the maximized one, restore the maximization state.
        if (m_pMaximizedWidget == pDestroyedWidget)
        {
            Restore();
        }

        // Check if the object is an rgViewContainer.
        rgViewContainer* pContainer = qobject_cast<rgViewContainer*>(pDestroyedWidget);
        bool isMaximizeContainer = (pContainer != nullptr);

        // If the object is a container, remove it from the container list.
        if (isMaximizeContainer)
        {
            for (auto it = m_maximizeContainers.begin(); it != m_maximizeContainers.end(); it++)
            {
                rgViewContainer* pContainer = *it;
                if (pContainer == pDestroyedWidget)
                {
                    m_maximizeContainers.erase(it);
                    break;
                }
            }
        }
    }
}