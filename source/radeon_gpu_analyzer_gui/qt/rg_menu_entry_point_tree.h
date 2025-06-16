//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for RGA File Menu's Tree view.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_ENTRY_POINT_TREE_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_ENTRY_POINT_TREE_H_

// Qt.
#include <QTreeView>
#include <QStandardItemModel>
#include <QMenu>

class RgMenuEntryPointTree : public QTreeView
{
    Q_OBJECT

public:
    explicit RgMenuEntryPointTree(QWidget* parent = nullptr);
    virtual ~RgMenuEntryPointTree() = default;
    void AdjustTreeSize(QStandardItemModel* pModel);

protected:
    // Re-implement mouse move event.
    virtual void mouseMoveEvent(QMouseEvent* event) override;

    // Re-implement mouse press event.
    virtual void mousePressEvent(QMouseEvent* event) override;

private:
    // Connect the signals.
    void ConnectSignals();

    // Handler invoked when the user clicks the kernel's "Copy" context menu item.
    void HandleCopyKernelNameSelection();

    // Handler invoked when the user wants to open the context menu for the item.
    void HandleOpenContextMenu(const QPoint& local_click_position);

    // Initialize the context menu.
    void InitializeContextMenu();

    // The context menu to display when the user right-clicks on a kernel name.
    QMenu* context_menu_ = nullptr;

    // The context menu item used to copy the kernel name.
    QAction* copy_kernel_name_action_ = nullptr;

    // The kernel name clicked on.
    QString kernel_name_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_ENTRY_POINT_TREE_H_
