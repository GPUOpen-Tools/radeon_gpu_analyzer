#pragma once

#include <QtWidgets/QDialog>
#include <QtWidgets/QItemDelegate>
#include "ui_rgUnsavedItemsDialog.h"

// Delegate for the rgUnsavedFileDialog which draws the list of unsaved files with truncated text.
class rgUnsavedFileItemDelegate : public QItemDelegate
{
public:
    // Default constructor.
    rgUnsavedFileItemDelegate(QWidget* pParent = nullptr) : QItemDelegate(pParent) {};

    // Drawing operation for text items in the unsaved file dialog.
    virtual void drawDisplay(QPainter* pPainter, const QStyleOptionViewItem& option, const QRect& rect, const QString& text) const override;

    // Size hint for text items in the unsaved file dialog.
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    // Empty implementation here because we don't want to draw the focus indication.
    virtual void drawFocus(QPainter* pPainter, const QStyleOptionViewItem& option, const QRect& rect) const override {}
};

class rgUnsavedItemsDialog : public QDialog
{
    Q_OBJECT

public:

    // Enum for dialog result (indicates which button was pressed).
    enum UnsavedFileDialogResult
    {
        // Ensure cancel is same behavior as rejection (clicking X on window).
        Cancel = QDialog::Rejected,
        Yes,
        No,
    };

    rgUnsavedItemsDialog(QWidget* pParent = nullptr);
    ~rgUnsavedItemsDialog();

    // Add a file to the unsaved file list.
    void AddFile(QString filename);

    // Add multiple files to the unsaved file list.
    void AddFiles(QStringList filenames);

private:
    // Connect the signals.
    void ConnectSignals();

    // Delegate for drawing list items in the file list widget.
    rgUnsavedFileItemDelegate* m_pItemDelegate = nullptr;

protected:
    Ui::rgUnsavedItemsDialog ui;
};