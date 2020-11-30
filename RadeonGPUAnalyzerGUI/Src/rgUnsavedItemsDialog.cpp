// C++.
#include <cassert>

// Qt.
#include <QWidget>
#include <QDialog>
#include <QPainter>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgUnsavedItemsDialog.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>

rgUnsavedItemsDialog::rgUnsavedItemsDialog(QWidget *pParent)
    : QDialog(pParent)
{
    // Setup the UI.
    ui.setupUi(this);

    // Disable the help button in the titlebar.
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Connect the signals.
    ConnectSignals();

    // Create item delegate for the list widget.
    m_pItemDelegate = new rgUnsavedFileItemDelegate();
    bool isDelegateValid = (m_pItemDelegate != nullptr);
    assert(isDelegateValid);

    if (isDelegateValid)
    {
        // Set custom delegate for the list widget.
        ui.fileListWidget->setItemDelegate(m_pItemDelegate);
    }

    // Disable selection of items.
    ui.fileListWidget->setSelectionMode(QAbstractItemView::NoSelection);

    // Do not allow the file list widget to have focus.
    ui.fileListWidget->setFocusPolicy(Qt::FocusPolicy::NoFocus);

    // Set default focus to "Yes" button.
    ui.yesPushButton->setFocus();

    // Set the tab order.
    setTabOrder(ui.yesPushButton, ui.noPushButton);
    setTabOrder(ui.noPushButton, ui.cancelPushButton);
    setTabOrder(ui.cancelPushButton, ui.yesPushButton);
}

rgUnsavedItemsDialog::~rgUnsavedItemsDialog()
{
    if (m_pItemDelegate != nullptr)
    {
        delete m_pItemDelegate;
    }
}

void rgUnsavedItemsDialog::ConnectSignals()
{
    // Yes button.
    bool isConnected = connect(ui.yesPushButton, &QPushButton::clicked, [=] { done(UnsavedFileDialogResult::Yes); });
    assert(isConnected);

    // No button.
    isConnected = connect(ui.noPushButton, &QPushButton::clicked, [=] { done(UnsavedFileDialogResult::No); });
    assert(isConnected);

    // Cancel button.
    isConnected = connect(ui.cancelPushButton, &QPushButton::clicked, [=] { done(UnsavedFileDialogResult::Cancel); });
    assert(isConnected);
}

void rgUnsavedItemsDialog::AddFile(QString filename)
{
    ui.fileListWidget->addItem(filename);
}

void rgUnsavedItemsDialog::AddFiles(QStringList filenames)
{
    foreach(const QString& filename, filenames)
    {
        AddFile(filename);
    }
}

void rgUnsavedFileItemDelegate::drawDisplay(QPainter* pPainter, const QStyleOptionViewItem& option, const QRect &rect, const QString& text) const
{
    bool isPainterValid = (pPainter != nullptr);
    assert(isPainterValid);

    if (isPainterValid)
    {
        // Truncate string so it fits within rect.
        QString truncatedString = rgUtils::TruncateString(text.toStdString(), gs_TEXT_TRUNCATE_LENGTH_FRONT,
            gs_TEXT_TRUNCATE_LENGTH_BACK, rect.width(), pPainter->font(), rgUtils::EXPAND_BACK).c_str();

        // Draw text within rect.
        pPainter->drawText(rect, Qt::AlignVCenter, truncatedString);
    }
}

QSize rgUnsavedFileItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // Use standard size hint implementation, but with a fixed width of 0 (width will be determined by view width).
    QSize adjustedHint = QItemDelegate::sizeHint(option, index);
    adjustedHint.setWidth(0);

    return adjustedHint;
}


