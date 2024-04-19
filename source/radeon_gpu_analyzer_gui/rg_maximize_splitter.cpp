// C++.
#include <assert.h>

// Qt.
#include <QSplitter>
#include <QStyle>
#include <QVariant>
#include <QWidget>

// Infra.
#include "QtCommon/Scaling/ScalingManager.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_maximize_splitter.h"
#include "radeon_gpu_analyzer_gui/qt/rg_view_container.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"

RgMaximizeSplitter::RgMaximizeSplitter(QWidget* parent) :
    QSplitter(parent)
{
}

void RgMaximizeSplitter::AddMaximizableWidget(RgViewContainer* view_container)
{
    if (view_container != nullptr)
    {
        ScalingManager& scaling_manager = ScalingManager::Get();

        // Ensure the container is scaling registered.
        scaling_manager.RegisterObject(view_container);

        // Add the container to the list.
        maximize_containers_.push_back(view_container);

        // Handle the corner button press on the maximization container.
        bool is_connected = connect(view_container, &RgViewContainer::MaximizeButtonClicked, this, &RgMaximizeSplitter::HandleCornerButtonClicked);
        assert(is_connected);

        // Add the container to the splitter.
        addWidget(view_container);

        view_container->SetMaximizedState(false);
    }
}

QWidget* RgMaximizeSplitter::GetMaximizedWidget() const
{
    return maximized_widget_;
}

void RgMaximizeSplitter::MaximizeWidget(QWidget* widget)
{
    // Signal the view container to have the maximum width.
    emit ViewMaximized();

    // Give the border focus.
    emit FrameInFocusSignal();

    // If this splitter has a maximization container for the given widget, maximize the container instead.
    for (RgViewContainer* container : maximize_containers_)
    {
        assert(container != nullptr);
        if (container != nullptr)
        {
            if (container->GetMainWidget() == widget)
            {
                widget = container;
                break;
            }
        }
    }

    // Confirm the widget is a child of this splitter.
    if (widget != nullptr && widget->parentWidget() == this)
    {
        // Ensure the widget is visible.
        widget->show();

        // Hide all other widgets in this splitter.
        for (QObject* child : children())
        {
            QWidget* child_widget = static_cast<QWidget*>(child);

            if (child_widget != widget)
            {
                child_widget->hide();
            }
        }

        // Handle cases when this splitter's parent is an RgMaximizeSplitter.
        RgMaximizeSplitter* parent_splitter = qobject_cast<RgMaximizeSplitter*>(parentWidget());
        if (parent_splitter != nullptr)
        {
            // Maximize this splitter within the parent splitter. This will recursively propagate
            // through all nested RgMaximizeSplitters.
            parent_splitter->MaximizeWidget(this);
        }

        // Remember the maximized widget.
        maximized_widget_ = widget;
    }
}

void RgMaximizeSplitter::Restore()
{
    // Signal the view container to have minimum required width.
    emit ViewRestored();

    if (maximized_widget_ != nullptr)
    {
        // Show all widgets in this splitter.
        for (QObject* child : children())
        {
            QWidget* child_widget = static_cast<QWidget*>(child);
            child_widget->show();

            // If this widget is an RgViewContainer,
            // set the hidden state to false.
            RgViewContainer* view_container = qobject_cast<RgViewContainer*>(child_widget);
            if(view_container != nullptr)
            {
                view_container->SetHiddenState(false);
            }
        }

        // Handle cases when this splitter's parent is an RgMaximizeSplitter.
        RgMaximizeSplitter* parent_splitter = qobject_cast<RgMaximizeSplitter*>(parentWidget());
        if (parent_splitter != nullptr)
        {
            // Restore the parent splitter. This will recursively propagate through all nested RgMaximizeSplitters.
            parent_splitter->Restore();
        }

        // Clear the maximized widget.
        maximized_widget_ = nullptr;
    }
}

void RgMaximizeSplitter::HandleCornerButtonClicked()
{
    // Get RgViewContainer which sent the signal.
    RgViewContainer* container = qobject_cast<RgViewContainer*>(sender());

    // This should always be true, no other object type should be connected to this slot.
    bool isMaximizeContainer = (container != nullptr);
    assert(isMaximizeContainer);

    if (isMaximizeContainer && container->IsMaximizable())
    {
        if (maximized_widget_ != container)
        {
            // Maximize the container.
            MaximizeWidget(container);

            container->SetMaximizedState(true);
            container->SetHiddenState(false);

            for (auto it = maximize_containers_.begin(); it != maximize_containers_.end(); it++)
            {
                RgViewContainer* item = *it;
                if (item != container)
                {
                    if (container->objectName().compare(kStrRgIsaDisassemblyViewContainer) == 0)
                    {
                        if (item->objectName().compare(kStrRgSourceViewContainer) == 0)
                        {
                            item->SetHiddenState(true);
                            break;
                        }
                    }
                    else if (container->objectName().compare(kStrRgSourceViewContainer) == 0)
                    {
                        if (item->objectName().compare(kStrRgIsaDisassemblyViewContainer) == 0)
                        {
                            item->SetHiddenState(true);
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

            container->SetMaximizedState(false);
        }
    }
}

void RgMaximizeSplitter::HandleChildDestroyed(QObject* child)
{
    QWidget* destroyed_widget = qobject_cast<QWidget*>(child);
    bool is_widget = (destroyed_widget != nullptr);
    assert(is_widget);

    // This is just a safe guard, it should never be untrue.
    if (is_widget)
    {
        // If the destroyed widget is the maximized one, restore the maximization state.
        if (maximized_widget_ == destroyed_widget)
        {
            Restore();
        }

        // Check if the object is an RgViewContainer.
        RgViewContainer* container = qobject_cast<RgViewContainer*>(destroyed_widget);
        bool is_maximize_container = (container != nullptr);

        // If the object is a container, remove it from the container list.
        if (is_maximize_container)
        {
            for (auto it = maximize_containers_.begin(); it != maximize_containers_.end(); it++)
            {
                if (*it == destroyed_widget)
                {
                    maximize_containers_.erase(it);
                    break;
                }
            }
        }
    }
}
