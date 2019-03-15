#pragma once

// Qt.
#include <QTreeView>
#include <QStandardItemModel>
#include <QMenu>

class rgMenuEntryPointTree : public QTreeView
{
    Q_OBJECT

public:
    explicit rgMenuEntryPointTree(QWidget* pParent = nullptr);
    virtual ~rgMenuEntryPointTree() = default;
    void AdjustTreeSize(QStandardItemModel* pModel);

protected:
    // Re-implement mouse move event.
    virtual void mouseMoveEvent(QMouseEvent* pEvent) override;

    // Re-implement mouse press event.
    virtual void mousePressEvent(QMouseEvent* pEvent) override;

private:
    // Connect the signals.
    void ConnectSignals();

    // Handler invoked when the user clicks the kernel's "Copy" context menu itme.
    void HandleCopyKernelNameSelection();

    // Handler invoked when the user wants to open the context menu for the item.
    void HandleOpenContextMenu(const QPoint& localClickPosition);

    // Initialize the context menu.
    void InitializeContextMenu();

    // The context menu to display when the user right-clicks on a kernel name.
    QMenu* m_pContextMenu = nullptr;

    // The context menu item used to copy the kernel name.
    QAction* m_pCopyKernelNameAction = nullptr;

    // The kernel name clicked on.
    QString m_kernelName;
};