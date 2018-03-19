#pragma once

// Local.
#include "ui_rgOrderedListViewDialog.h"
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>

class rgOrderedListDialog : public QDialog
{
    Q_OBJECT

public:
    rgOrderedListDialog(const char* pDelimiter, QWidget* pParent = nullptr);
    virtual ~rgOrderedListDialog() = default;

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
    void HandleListItemChanged(QListWidgetItem* pItem);

    // Handler when an entry is clicked on.
    void HandleListItemClicked(QListWidgetItem* pItem);

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
    virtual void OnListItemChanged(QListWidgetItem* pItem) = 0;

    // Insert a blank item in the list widget.
    void InsertBlankItem();

    // Update the buttons.
    void UpdateButtons();

    // Update the list widget with the latest data.
    void UpdateListWidget();

    // Update tool tips.
    void UpdateToolTips();

    // The list holding selected list items.
    QStringList m_itemsList;

    // The delimiter used to split the ordered list.
    const char* m_pDelimiter = nullptr;

    // The generated interface view object.
    Ui::rgOrderedListViewDialog ui;

    // Indicate whether editing an invalid entry.
    // This means either the directory does not exist
    // or a duplicate entry was entered.
    bool m_editingInvalidEntry = false;

private:
    // Connect the signals.
    void ConnectSignals();

    // Set the button fonts.
    void SetButtonFonts();

    // Set button shortcuts.
    void SetButtonShortcuts();

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // Button keyboard actions.
    QAction* m_pAddAction = nullptr;
    QAction* m_pDeleteAction = nullptr;
    QAction* m_pMoveUpAction = nullptr;
    QAction* m_pMoveDownAction = nullptr;
};