// C++.
#include <cassert>

// Qt.
#include <QWidget>
#include <QDialog>
#include <QPainter>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_unsaved_items_dialog.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"

RgUnsavedItemsDialog::RgUnsavedItemsDialog(QWidget *parent)
    : QDialog(parent)
{
    // Setup the UI.
    ui_.setupUi(this);

    // Disable the help button in the titlebar.
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Connect the signals.
    ConnectSignals();

    // Create item delegate for the list widget.
    item_delegate_ = new RgUnsavedFileItemDelegate();
    bool is_delegate_valid = (item_delegate_ != nullptr);
    assert(is_delegate_valid);

    if (is_delegate_valid)
    {
        // Set custom delegate for the list widget.
        ui_.fileListWidget->setItemDelegate(item_delegate_);
    }

    // Disable selection of items.
    ui_.fileListWidget->setSelectionMode(QAbstractItemView::NoSelection);

    // Do not allow the file list widget to have focus.
    ui_.fileListWidget->setFocusPolicy(Qt::FocusPolicy::NoFocus);

    // Set default focus to "Yes" button.
    ui_.yesPushButton->setFocus();

    // Set the tab order.
    setTabOrder(ui_.yesPushButton, ui_.noPushButton);
    setTabOrder(ui_.noPushButton, ui_.cancelPushButton);
    setTabOrder(ui_.cancelPushButton, ui_.yesPushButton);
}

RgUnsavedItemsDialog::~RgUnsavedItemsDialog()
{
    if (item_delegate_ != nullptr)
    {
        delete item_delegate_;
    }
}

void RgUnsavedItemsDialog::ConnectSignals()
{
    // Yes button.
    bool is_connected = connect(ui_.yesPushButton, &QPushButton::clicked, [=] { done(UnsavedFileDialogResult::kYes); });
    assert(is_connected);

    // No button.
    is_connected = connect(ui_.noPushButton, &QPushButton::clicked, [=] { done(UnsavedFileDialogResult::kNo); });
    assert(is_connected);

    // Cancel button.
    is_connected = connect(ui_.cancelPushButton, &QPushButton::clicked, [=] { done(UnsavedFileDialogResult::kCancel); });
    assert(is_connected);
}

void RgUnsavedItemsDialog::AddFile(QString filename)
{
    ui_.fileListWidget->addItem(filename);
}

void RgUnsavedItemsDialog::AddFiles(QStringList filenames)
{
    foreach(const QString& filename, filenames)
    {
        AddFile(filename);
    }
}

void RgUnsavedFileItemDelegate::drawDisplay(QPainter* painter, const QStyleOptionViewItem& option, const QRect &rect, const QString& text) const
{
    Q_UNUSED(option);

    bool is_painter_valid = (painter != nullptr);
    assert(is_painter_valid);

    if (is_painter_valid)
    {
        // Truncate string so it fits within rect.
        QString truncated_string = RgUtils::TruncateString(text.toStdString(), kTextTruncateLengthFront,
            kTextTruncateLengthBack, rect.width(), painter->font(), RgUtils::kExpandBack).c_str();

        // Draw text within rect.
        painter->drawText(rect, Qt::AlignVCenter, truncated_string);
    }
}

QSize RgUnsavedFileItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // Use standard size hint implementation, but with a fixed width of 0 (width will be determined by view width).
    QSize adjusted_hint = QItemDelegate::sizeHint(option, index);
    adjusted_hint.setWidth(0);

    return adjusted_hint;
}

