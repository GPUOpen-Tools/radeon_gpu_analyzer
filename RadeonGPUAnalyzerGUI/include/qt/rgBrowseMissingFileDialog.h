#pragma once

// C++.
#include <memory>

// Qt.
#include <QtWidgets/QDialog>
#include <QtWidgets/QItemDelegate>

// Local.
#include "ui_rgBrowseMissingFileDialog.h"

// Forward declarations.
struct rgProject;
enum rgProjectAPI : char;

// Delegate for the rgBrowseMissingFileDialog which draws the list of missing files with truncated text.
class rgMissingFileItemDelegate : public QItemDelegate
{
public:
    // Default constructor.
    rgMissingFileItemDelegate(QWidget* pParent = nullptr) : QItemDelegate(pParent) {};

    // Drawing operation for text items in the unsaved file dialog.
    virtual void drawDisplay(QPainter* pPainter, const QStyleOptionViewItem& option, const QRect& rect, const QString& text) const override;

    // Size hint for text items in the unsaved file dialog.
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    // Empty implementation here because we don't want to draw the focus indication.
    virtual void drawFocus(QPainter* pPainter, const QStyleOptionViewItem& option, const QRect& rect) const override {}
};

class rgBrowseMissingFileDialog : public QDialog
{
    Q_OBJECT

public:
    // Enum for dialog result (indicates which button was pressed).
    enum MissingFileDialogResult
    {
        OK,
        Cancel
    };

    // Constructor and destructor.
    rgBrowseMissingFileDialog(rgProjectAPI api, QWidget* pParent = nullptr);
    ~rgBrowseMissingFileDialog();

    // Add a file to the unsaved file list.
    void AddFile(const std::string filename);

    // Retrieve the map which associates original filepaths with the new ones chosen by the user.
    const std::map<std::string, std::string>& GetUpdateFilePathsMap() const;

private slots:
    // Handler invoked when the user clicks the Browse button in the dialog.
    void HandleBrowseButtonClicked();

    // Handler invoked when the user changes the selected row in the list of missing files.
    void HandleSelectedFilePathChanged(const QItemSelection &selected, const QItemSelection &deselected);

    // Handler invoked when the user double-clicks on a row in the list of missing files.
    void HandleSelectedRowDoubleClicked(QListWidgetItem* pItem);

    // Handler invoked when the user clicks on the "X" in the title bar to close the dialog box.
    void HandleRejected();

private:
    // Connect the signals.
    void ConnectSignals();

    // Display a file browser to the user, where they can choose to replace the path at the given row.
    void BrowseFilePathForRow(int rowIndex);

    // Toggle the enabledness of the Browse button.
    void ToggleBrowseButtonEnabled(bool isEnabled);

    // A map that associated the original file path with a new one that the user chose to replace it.
    std::map<std::string, std::string> m_updatedPathMap;

    // Delegate for drawing list items in the file list widget.
    rgMissingFileItemDelegate* m_pItemDelegate = nullptr;

    // The project that references missing files.
    rgProjectAPI m_projectAPI;

    // The Qt interface for the dialog.
    Ui::rgBrowseMissingFileDialog ui;
};