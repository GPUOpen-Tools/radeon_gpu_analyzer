// C++.
#include <cassert>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuEntryPointTree.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>

// Qt.
#include <QApplication>
#include <QClipboard>
#include <QMouseEvent>

rgMenuEntryPointTree::rgMenuEntryPointTree(QWidget* pParent) :
    QTreeView(pParent)
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

void rgMenuEntryPointTree::mouseMoveEvent(QMouseEvent* pEvent)
{
}

void rgMenuEntryPointTree::mousePressEvent(QMouseEvent* pEvent)
{
    assert(pEvent != nullptr);
    if (pEvent != nullptr)
    {
        // Do not highlight the item if the right mouse button is clicked.
        if (pEvent->buttons() != Qt::RightButton)
        {
            QTreeView::mousePressEvent(pEvent);
        }
    }
}

void rgMenuEntryPointTree::ConnectSignals()
{
    // Connect the handler responsible for showing the item's context menu.
    bool isConnected = connect(this, &QTreeView::customContextMenuRequested, this, &rgMenuEntryPointTree::HandleOpenContextMenu);
    assert(isConnected);

    // Connect the item's "Copy" menu item.
    isConnected = connect(m_pCopyKernelNameAction, &QAction::triggered, this, &rgMenuEntryPointTree::HandleCopyKernelNameSelection);
    assert(isConnected);
}

void rgMenuEntryPointTree::AdjustTreeSize(QStandardItemModel* pModel)
{
    assert(pModel != nullptr);
    if (pModel != nullptr)
    {
        // Calculate the entry point table view height.
        int height = (rowHeight(pModel->index(0, 0))) * pModel->rowCount() + 2;

        // Update the entry point table view height.
        setMinimumHeight(height);
        setMaximumHeight(height);
    }
}

void rgMenuEntryPointTree::InitializeContextMenu()
{
    // Create the context menu instance.
    m_pContextMenu = new QMenu(this);

    // Create the menu items to insert into the context menu.
    m_pCopyKernelNameAction = new QAction(STR_FILE_CONTEXT_MENU_COPY_FILE_NAME, this);
    m_pContextMenu->addAction(m_pCopyKernelNameAction);
}

void rgMenuEntryPointTree::HandleOpenContextMenu(const QPoint& widgetClickPosition)
{
    // Get and save the kernel name clicked on.
    QModelIndex modelIndex = this->indexAt(widgetClickPosition);
    m_kernelName = modelIndex.data().toString();

    // Convert the widget's local click position to the global screen position.
    const QPoint clickPoint = mapToGlobal(widgetClickPosition);

    // Open the context menu at the user's click position.
    m_pContextMenu->exec(clickPoint);
}

void rgMenuEntryPointTree::HandleCopyKernelNameSelection()
{
    // Copy the name of the file to clipboard.
    QClipboard* pClipboard = QApplication::clipboard();
    assert(pClipboard != nullptr);
    if (pClipboard != nullptr)
    {
        pClipboard->setText(m_kernelName);
    }
}
