// C++.
#include <cassert>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_menu_entry_point_tree.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"

// Qt.
#include <QApplication>
#include <QClipboard>
#include <QMouseEvent>

RgMenuEntryPointTree::RgMenuEntryPointTree(QWidget* parent) :
    QTreeView(parent)
{
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);

    // Initialize the context menu.
    InitializeContextMenu();

    // Connect the signals.
    ConnectSignals();

    // Set context menu policy.
    this->setContextMenuPolicy(Qt::CustomContextMenu);
}

void RgMenuEntryPointTree::mouseMoveEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
}

void RgMenuEntryPointTree::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event);

    assert(event != nullptr);
    if (event != nullptr)
    {
        // Do not highlight the item if the right mouse button is clicked.
        if (event->buttons() != Qt::RightButton)
        {
            QTreeView::mousePressEvent(event);
        }
    }
}

void RgMenuEntryPointTree::ConnectSignals()
{
    // Connect the handler responsible for showing the item's context menu.
    bool is_connected = connect(this, &QTreeView::customContextMenuRequested, this, &RgMenuEntryPointTree::HandleOpenContextMenu);
    assert(is_connected);

    // Connect the item's "Copy" menu item.
    is_connected = connect(copy_kernel_name_action_, &QAction::triggered, this, &RgMenuEntryPointTree::HandleCopyKernelNameSelection);
    assert(is_connected);
}

void RgMenuEntryPointTree::AdjustTreeSize(QStandardItemModel* model)
{
    assert(model != nullptr);
    if (model != nullptr)
    {
        // Calculate the entry point table view height.
        int height = (rowHeight(model->index(0, 0))) * model->rowCount() + 2;

        // Update the entry point table view height.
        setMinimumHeight(height);
        setMaximumHeight(height);
    }
}

void RgMenuEntryPointTree::InitializeContextMenu()
{
    // Create the context menu instance.
    context_menu_ = new QMenu(this);

    // Create the menu items to insert into the context menu.
    copy_kernel_name_action_ = new QAction(kStrFileContextMenuCopyFileName, this);
    context_menu_->addAction(copy_kernel_name_action_);
}

void RgMenuEntryPointTree::HandleOpenContextMenu(const QPoint& widget_click_position)
{
    // Get and save the kernel name clicked on.
    QModelIndex model_index = this->indexAt(widget_click_position);
    kernel_name_ = model_index.data().toString();

    // Convert the widget's local click position to the global screen position.
    const QPoint click_point = mapToGlobal(widget_click_position);

    // Open the context menu at the user's click position.
    context_menu_->exec(click_point);
}

void RgMenuEntryPointTree::HandleCopyKernelNameSelection()
{
    // Copy the name of the file to clipboard.
    QClipboard* clip_board = QApplication::clipboard();
    assert(clip_board != nullptr);
    if (clip_board != nullptr)
    {
        clip_board->setText(kernel_name_);
    }
}
