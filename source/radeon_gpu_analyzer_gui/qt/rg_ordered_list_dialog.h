#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ORDERED_LIST_DIALOG_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ORDERED_LIST_DIALOG_H_

// Local.
#include "ui_rg_ordered_list_view_dialog.h"
#include "source/radeon_gpu_analyzer_gui/rg_data_types.h"

class RgOrderedListDialog : public QDialog
{
    Q_OBJECT

public:
    RgOrderedListDialog(const char* delimiter, QWidget* parent = nullptr);
    virtual ~RgOrderedListDialog() = default;

    // Initialize the list of entries.
    void SetListItems(const QString& entries);

private slots :
    // Exit the dialog.
    void HandleExit(bool /* checked */);

    // Handler to delete the selected item.
    void HandleDeleteButtonClick(bool /* checked */);

    // Handler to edit the selected entry.
    void HandleEditButtonClick(bool /* checked */);

    // Handler when an entry changes.
    void HandleListItemChanged(QListWidgetItem* item);

    // Handler when an entry is selected, this supports both clicking, and also using the arrow keys to change selection.
    void HandleListItemSelectionChanged();

    // Handler to move the selected item down one row.
    void HandleMoveDownButtonClick(bool /* checked */);

    // Handler to move the selected item up one row.
    void HandleMoveUpButtonClick(bool /* checked */);

    // Handler when the OK button is clicked.
    void HandleOKButtonClick(bool /* checked */);

    // Handler when the New button is clicked.
    void HandleNewButtonClick(bool /* checked */);

signals:
    // Signal emitted when the user clicks the "OK" button to close the dialog.
    void OKButtonClicked(QStringList);

protected:
    // An overridden virtual responsible for determining if an edited list item is valid.
    virtual void OnListItemChanged(QListWidgetItem* item) = 0;

    // Insert a blank item in the list widget.
    void InsertBlankItem();

    // Update the buttons.
    void UpdateButtons();

    // Update the list widget with the latest data.
    void UpdateListWidget();

    // Update tool tips.
    void UpdateToolTips();

    // The list holding selected list items.
    QStringList items_list_;

    // The delimiter used to split the ordered list.
    const char* delimiter_ = nullptr;

    // The generated interface view object.
    Ui::RgOrderedListViewDialog ui_;

    // Indicate whether editing an invalid entry.
    // This means either the directory does not exist
    // or a duplicate entry was entered.
    bool editing_invalid_entry_ = false;

private:
    // Connect the signals.
    void ConnectSignals();

    // Find out if the Move up/down buttons need to be disabled.
    bool ShouldDisableMoveUpDownButtons();

    // Set the button fonts.
    void SetButtonFonts();

    // Set button shortcuts.
    void SetButtonShortcuts();

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // Button keyboard actions.
    QAction* add_action_ = nullptr;
    QAction* delete_action_ = nullptr;
    QAction* move_up_action_ = nullptr;
    QAction* move_down_action_ = nullptr;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ORDERED_LIST_DIALOG_H_
