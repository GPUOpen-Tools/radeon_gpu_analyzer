
// C++.
#include <cassert>

// Qt.
#include <QAction>
#include <QApplication>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_build_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_view_manager.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

RgViewManager::RgViewManager(QWidget* parent) :
    parent_(parent),
    focus_view_index_(-1)
{
    CreateActions();

    // Handler for when the focus object changes.
    bool is_connected = connect(qApp, &QGuiApplication::focusObjectChanged, this, &RgViewManager::HandleFocusObjectChanged);
    assert(is_connected);

    // Handler for when the application is about to quit.
    is_connected = connect(qApp, &QCoreApplication::aboutToQuit, this, &RgViewManager::HandleApplicationAboutToQuit);
    assert(is_connected);
}

RgViewManager::~RgViewManager()
{

}

void RgViewManager::CreateActions()
{
    // Focus next view action.
    focus_next_view_action_ = new QAction(this);
    focus_next_view_action_->setShortcutContext(Qt::ApplicationShortcut);
    focus_next_view_action_->setShortcut(QKeySequence(kActionHotkeyNextView));
    assert(parent_ != nullptr);
    if (parent_ != nullptr)
    {
        parent_->addAction(focus_next_view_action_);
    }

    bool is_connected = connect(focus_next_view_action_, &QAction::triggered, this, &RgViewManager::HandleFocusNextViewAction);
    assert(is_connected);

    // Focus previous view action.
    focus_prev_view_action_ = new QAction(this);
    focus_prev_view_action_->setShortcutContext(Qt::ApplicationShortcut);
    focus_prev_view_action_->setShortcut(QKeySequence(kActionHotkeyPreviousView));
    assert(parent_ != nullptr);
    if (parent_ != nullptr)
    {
        parent_->addAction(focus_prev_view_action_);
    }

    is_connected = connect(focus_prev_view_action_, &QAction::triggered, this, &RgViewManager::HandleFocusPrevViewAction);
    assert(is_connected);
}

void RgViewManager::AddView(RgViewContainer* view_container, bool is_active)
{
    if (is_active)
    {
        view_containers_.push_back(view_container);
    }
    else
    {
        inactive_view_containers_.push_back(view_container);
    }
}

void RgViewManager::AddView(RgViewContainer* view_container, bool is_active, int index)
{
    assert(index >= RgViewManagerViewContainerIndex::kFileMenu);
    assert(index < RgViewManagerViewContainerIndex::kCount);

    if (index >= RgViewManagerViewContainerIndex::kFileMenu && index < RgViewManagerViewContainerIndex::kCount)
    {
        if (is_active)
        {
            view_containers_.insert(view_containers_.begin() + index, view_container);
        }
        else
        {
            inactive_view_containers_.insert(inactive_view_containers_.begin() + index, view_container);
        }
    }
}

void RgViewManager::FocusNextView()
{
    if (focus_view_index_ >= 0)
    {
        // If the current view is output window, and it is
        // currently maximized, do not process this action.
        RgViewContainer* view_container = view_containers_.at(focus_view_index_);
        bool focus_next_view = !(view_container->IsInMaximizedState() && (view_container->objectName().compare(kStrRgBuildOutputViewContainer) == 0));
        if (focus_next_view)
        {
            // Increment focus index.
            int new_focus_index = focus_view_index_ + 1;

            if (new_focus_index >= view_containers_.size())
            {
                new_focus_index = 0;
            }

            if (!is_source_view_current_)
            {
                if (current_focused_view_ == RgCurrentFocusedIndex::kFileMenuCurrent)
                {
                    // Set the focus to the hidden disassembly view to remove
                    // focus from the file menu.
                    new_focus_index = GetFocusIndex(RgViewManagerViewContainerIndex::kDisassemblyView);

                    // Change the view focus index.
                    SetFocusedViewIndex(new_focus_index);

                    // Apply the focus change.
                    ApplyViewFocus();

                    if (is_build_settings_view_current_)
                    {
                        emit BuildSettingsWidgetFocusInSignal();
                        current_focused_view_ = RgCurrentFocusedIndex::kBuildSettingsViewCurrent;
                    }
                    else if (is_pso_editor_view_current_)
                    {
                        emit PsoEditorWidgetFocusInSignal();
                        current_focused_view_ = RgCurrentFocusedIndex::kPsoEditorViewCurrent;
                    }
                    else
                    {
                        // Should not get here.
                        assert(false);
                    }
                }
                else if (current_focused_view_ == RgCurrentFocusedIndex::kBuildOutputViewCurrent)
                {
                    emit BuildSettingsWidgetFocusOutSignal();
                    emit PsoEditorWidgetFocusOutSignal();
                    new_focus_index = GetFocusIndex(RgViewManagerViewContainerIndex::kFileMenu);
                    current_focused_view_ = RgCurrentFocusedIndex::kFileMenuCurrent;

                    // Change the view focus index.
                    SetFocusedViewIndex(new_focus_index);

                    // Apply the focus change.
                    ApplyViewFocus();
                }
                else if ((current_focused_view_ == RgCurrentFocusedIndex::kBuildSettingsViewCurrent) ||
                    (current_focused_view_ == RgCurrentFocusedIndex::kPsoEditorViewCurrent))
                {
                    emit BuildSettingsWidgetFocusOutSignal();
                    emit PsoEditorWidgetFocusOutSignal();
                    new_focus_index = GetFocusIndex(RgViewManagerViewContainerIndex::kBuildOutputView);
                    current_focused_view_ = RgCurrentFocusedIndex::kBuildOutputViewCurrent;

                    // Change the view focus index.
                    SetFocusedViewIndex(new_focus_index);

                    // Apply the focus change.
                    ApplyViewFocus();
                }
            }
            else
            {
                view_container = view_containers_.at(new_focus_index);
                while (view_container->IsInHiddenState() && new_focus_index < view_containers_.size())
                {
                    new_focus_index++;
                    if (new_focus_index >= view_containers_.size())
                    {
                        new_focus_index = 0;
                    }
                    view_container = view_containers_.at(new_focus_index);
                }

                // Change the view focus index.
                SetFocusedViewIndex(new_focus_index);

                // Apply the focus change.
                ApplyViewFocus();
            }
        }
    }
}

void RgViewManager::SwitchContainerSize()
{
    RgViewContainer* view_container = view_containers_.at(focus_view_index_);
    assert(view_container != nullptr);
    if (view_container != nullptr)
    {
        view_container->SwitchContainerSize();
    }
}

void RgViewManager::SetSourceWindowFocus()
{
    // Change the view focus index.
    SetFocusedViewIndex(RgViewManagerViewContainerIndex::kSourceView);

    // Apply the focus change.
    ApplyViewFocus();
}

void RgViewManager::SetOutputWindowFocus()
{
    // Change the view focus index.
    SetFocusedViewIndex(RgViewManagerViewContainerIndex::kBuildOutputView);

    // Apply the focus change.
    ApplyViewFocus();
}

void RgViewManager::SetDisassemblyViewFocus()
{
    // Change the view focus index.
    SetFocusedViewIndex(RgViewManagerViewContainerIndex::kDisassemblyView);

    // Apply the focus change.
    ApplyViewFocus();
}

void RgViewManager::FocusPrevView()
{
    // If the current view is output window, and it is
    // currently maximized, do not process this action.
    if (focus_view_index_ >= 0)
    {
        RgViewContainer* view_container = view_containers_.at(focus_view_index_);
        bool focus_prev_view = !(view_container->IsInMaximizedState() && (view_container->objectName().compare(kStrRgBuildOutputViewContainer) == 0));
        if (focus_prev_view)
        {
            int new_focus_index = focus_view_index_ - 1;

            if (!is_source_view_current_)
            {
                if (current_focused_view_ == RgCurrentFocusedIndex::kFileMenuCurrent)
                {
                    emit BuildSettingsWidgetFocusOutSignal();
                    emit PsoEditorWidgetFocusOutSignal();
                    new_focus_index = GetFocusIndex(RgViewManagerViewContainerIndex::kBuildOutputView);
                    current_focused_view_ = RgCurrentFocusedIndex::kBuildOutputViewCurrent;

                    // Change the view focus index.
                    SetFocusedViewIndex(new_focus_index);

                    // Apply the focus change.
                    ApplyViewFocus();
                }
                else if (current_focused_view_ == RgCurrentFocusedIndex::kBuildOutputViewCurrent)
                {
                    // Set the focus to the hidden disassembly view to remove
                    // focus from the file menu.
                    new_focus_index = GetFocusIndex(RgViewManagerViewContainerIndex::kDisassemblyView);

                    // Change the view focus index.
                    SetFocusedViewIndex(new_focus_index);

                    // Apply the focus change.
                    ApplyViewFocus();

                    if (is_build_settings_view_current_)
                    {
                        emit BuildSettingsWidgetFocusInSignal();
                        current_focused_view_ = RgCurrentFocusedIndex::kBuildSettingsViewCurrent;
                    }
                    else if (is_pso_editor_view_current_)
                    {
                        emit PsoEditorWidgetFocusInSignal();
                        current_focused_view_ = RgCurrentFocusedIndex::kPsoEditorViewCurrent;
                    }
                    else
                    {
                        // Should not get here.
                        assert(false);
                    }
                }
                else if ((current_focused_view_ == RgCurrentFocusedIndex::kBuildSettingsViewCurrent) ||
                    (current_focused_view_ == RgCurrentFocusedIndex::kPsoEditorViewCurrent))
                {
                    emit BuildSettingsWidgetFocusOutSignal();
                    emit PsoEditorWidgetFocusOutSignal();
                    new_focus_index = GetFocusIndex(RgViewManagerViewContainerIndex::kFileMenu);
                    current_focused_view_ = RgCurrentFocusedIndex::kFileMenuCurrent;

                    // Change the view focus index.
                    SetFocusedViewIndex(new_focus_index);

                    // Apply the focus change.
                    ApplyViewFocus();
                }
            }
            else
            {
                // Decrement focus index.
                if (new_focus_index < 0)
                {
                    new_focus_index = static_cast<int>(view_containers_.size()) - 1;
                }

                view_container = view_containers_.at(new_focus_index);
                while (view_container->IsInHiddenState() && new_focus_index < view_containers_.size())
                {
                    new_focus_index--;
                    if (new_focus_index < 0)
                    {
                        new_focus_index = static_cast<int>(view_containers_.size()) - 1;
                    }
                    view_container = view_containers_.at(new_focus_index);
                }

                // Change the view focus index.
                SetFocusedViewIndex(new_focus_index);

                // Apply the focus change.
                ApplyViewFocus();
            }
        }
    }
}

void RgViewManager::SetFocusedView(RgViewContainer* view_container)
{
    if (focus_view_container_ != nullptr)
    {
        // Get old focused view container.
        RgViewContainer* old_focus_container = focus_view_container_;

        // Set focus state.
        old_focus_container->SetFocusedState(false);
    }

    // Set new focused view container.
    focus_view_container_ = view_container;

    // Set focus state.
    assert(focus_view_container_ != nullptr);
    if (focus_view_container_ != nullptr)
    {
        focus_view_container_->SetFocusedState(true);

        if ((focus_view_container_->objectName()).compare(kStrRgIsaDisassemblyViewContainer) != 0)
        {
            // Emit the frame out of focus signal.
            emit FrameFocusOutSignal();
        }
    }
}

void RgViewManager::SetFocusedViewIndex(int index)
{
    if (index >= 0 && index < view_containers_.size())
    {
        // Set focus index.
        focus_view_index_ = index;

        // Get container at the focus index.
        RgViewContainer* new_view_container = view_containers_[focus_view_index_];

        // Set the focused view.
        SetFocusedView(new_view_container);
    }
}

void RgViewManager::ClearFocusedView()
{
    // Clear focused state of currently focused view container.
    // Having this on Linux causes a crash, so leaving it out of Linux build.
#ifndef __linux
    if (focus_view_container_ != nullptr)
    {
        focus_view_container_->SetFocusedState(false);
    }
#endif // __linux

    // Invalidate focus index.
    focus_view_index_ = -1;
    focus_view_container_ = nullptr;
}

void RgViewManager::ApplyViewFocus()
{
    // Focus in on the widget.
    if (focus_view_container_ != nullptr)
    {
        // Get widget to focus on
        QWidget* focus_widget = focus_view_container_->GetMainWidget();

        if (focus_widget != nullptr)
        {
            focus_widget->setFocus();
        }

        // If the widget is disassembly view, also change the frame color.
        RgIsaDisassemblyView* view = qobject_cast<RgIsaDisassemblyView*>(focus_widget);
        if (view != nullptr)
        {
            emit FrameFocusInSignal();
        }
    }
}

void RgViewManager::HandleApplicationAboutToQuit()
{
    // Remove all references to existing containers so widgets aren't re-polished during shutdown.
    view_containers_.clear();
    inactive_view_containers_.clear();
}

void RgViewManager::SetCurrentFocusedView(RgCurrentFocusedIndex current_focus_index)
{
    current_focused_view_ = current_focus_index;
}

void RgViewManager::HandleFocusNextViewAction()
{
    FocusNextView();
}

void RgViewManager::HandleFocusPrevViewAction()
{
    FocusPrevView();
}

int FindAncestorContainerIndex(const QWidget* widget, const std::vector<RgViewContainer*>& container_list)
{
    int ret = -1;

    // Search the list to find a container that is an ancestor of the given widget.
    for (int i = 0; i < container_list.size(); i++)
    {
        RgViewContainer* view_container = container_list[i];

        // If the focus widget is a child of a view container, give that container view focus.
        if (view_container->isAncestorOf(widget))
        {
            ret = i;
            break;
        }
    }

    return ret;
}

void RgViewManager::HandleFocusObjectChanged(QObject* object)
{
    QWidget* widget = qobject_cast<QWidget*>(object);

    if (widget != nullptr)
    {
        // Find appropriate view container to switch view focus to.
        int focus_index = FindAncestorContainerIndex(widget, view_containers_);

        if (focus_index >= 0)
        {
            SetFocusedViewIndex(focus_index);
        }
        else
        {
            // If no container in the main list exists, check the inactive container list.
            int inactive_index = FindAncestorContainerIndex(widget, inactive_view_containers_);

            if (inactive_index >= 0)
            {
                // Get the view from the inactive view list.
                RgViewContainer* view = inactive_view_containers_[inactive_index];

                // Set the focused view.
                SetFocusedView(view);
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

void RgViewManager::SetIsSourceViewCurrent(bool value)
{
    is_source_view_current_ = value;
}

void RgViewManager::SetIsBuildSettingsViewCurrent(bool value)
{
    is_build_settings_view_current_ = value;
}

void RgViewManager::SetIsPsoEditorViewCurrent(bool value)
{
    is_pso_editor_view_current_ = value;
}

int RgViewManager::GetFocusIndex(RgViewManagerViewContainerIndex view_container_index)
{
    int focus_index = 0;

    QString match_container;
    switch (view_container_index)
    {
    case RgViewManagerViewContainerIndex::kBuildOutputView:
        match_container = kStrRgBuildOutputViewContainer;
        break;
    case RgViewManagerViewContainerIndex::kFileMenu:
        match_container = kStrRgFileMenuViewContainer;
        break;
    case RgViewManagerViewContainerIndex::kDisassemblyView:
        match_container = kStrRgIsaDisassemblyViewContainer;
        break;
    case RgViewManagerViewContainerIndex::kSourceView:
        match_container = kStrRgSourceViewContainer;
        break;
    default:
        assert(false);
    }

    // Step through the view container vector and find the matching view container.
    for (const RgViewContainer* view_container : view_containers_)
    {
        QString container_name = view_container->objectName();
        if (container_name.compare(match_container) == 0)
        {
            break;
        }
        focus_index++;
    }

    return focus_index;
}
