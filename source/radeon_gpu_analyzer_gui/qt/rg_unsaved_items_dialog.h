#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_UNSAVED_ITEMS_DIALOG_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_UNSAVED_ITEMS_DIALOG_H_

#include <QtWidgets/QDialog>
#include <QtWidgets/QItemDelegate>
#include "ui_rg_unsaved_items_dialog.h"

// Delegate for the RgUnsavedFileDialog which draws the list of unsaved files with truncated text.
class RgUnsavedFileItemDelegate : public QItemDelegate
{
public:
    // Default constructor.
    RgUnsavedFileItemDelegate(QWidget* parent = nullptr) : QItemDelegate(parent) {};

    // Drawing operation for text items in the unsaved file dialog.
    virtual void drawDisplay(QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect, const QString& text) const override;

    // Size hint for text items in the unsaved file dialog.
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    // Empty implementation here because we don't want to draw the focus indication.
    virtual void drawFocus(QPainter* painter, const QStyleOptionViewItem& option, const QRect& rect) const override
    {
        Q_UNUSED(painter);
        Q_UNUSED(option);
        Q_UNUSED(rect);
    }
};

class RgUnsavedItemsDialog : public QDialog
{
    Q_OBJECT

public:

    // Enum for dialog result (indicates which button was pressed).
    enum UnsavedFileDialogResult
    {
        // Ensure cancel is same behavior as rejection (clicking X on window).
        kCancel = QDialog::Rejected,
        kYes,
        kNo,
    };

    RgUnsavedItemsDialog(QWidget* parent = nullptr);
    ~RgUnsavedItemsDialog();

    // Add a file to the unsaved file list.
    void AddFile(QString filename);

    // Add multiple files to the unsaved file list.
    void AddFiles(QStringList filenames);

private:
    // Connect the signals.
    void ConnectSignals();

    // Delegate for drawing list items in the file list widget.
    RgUnsavedFileItemDelegate* item_delegate_ = nullptr;

protected:
    Ui::RgUnsavedItemsDialog ui_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_UNSAVED_ITEMS_DIALOG_H_
