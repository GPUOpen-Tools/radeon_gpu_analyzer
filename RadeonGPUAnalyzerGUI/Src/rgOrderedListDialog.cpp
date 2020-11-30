// C++.
#include <cassert>

// Qt.
#include <QAction>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgOrderedListDialog.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

rgOrderedListDialog::rgOrderedListDialog(const char* pDelimiter, QWidget* pParent) :
    QDialog(pParent),
    m_pDelimiter(pDelimiter)
{
    const int WIDGET_FIXED_WIDTH = 600;
    const int WIDGET_FIXED_HEIGHT = 150;

    // Verify that the given delimiter is valid.
    assert(m_pDelimiter != nullptr);

    // Setup the UI.
    ui.setupUi(this);

    // Set the background to white.
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    // Set the window icon.
    setWindowIcon(QIcon(":/icons/rgaIcon.png"));

    // Set the size of the window.
    QSize size;
    size.setWidth(WIDGET_FIXED_WIDTH * ScalingManager::Get().GetScaleFactor());
    size.setHeight(WIDGET_FIXED_HEIGHT * ScalingManager::Get().GetScaleFactor());
    setMinimumSize(size);

    // Disable the help button in the title bar.
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Connect the signals.
    ConnectSignals();

    // Set the mouse cursor to the pointing hand cursor for various widgets.
    SetCursor();

    // Set the button fonts.
    SetButtonFonts();

    // Set push button shortcuts.
    SetButtonShortcuts();

    // Start off with various buttons disabled.
    ui.moveUpPushButton->setEnabled(false);
    ui.moveDownPushButton->setEnabled(false);
    ui.editPushButton->setEnabled(false);
}

void rgOrderedListDialog::SetListItems(const QString& entries)
{
    // Clear any existing data.
    m_itemsList.clear();
    ui.itemsList->clear();

    if (!entries.isEmpty())
    {
        // Process the incoming text.
        QStringList entryList = entries.split(m_pDelimiter);

        // Remove trailing and leading whitespace, and then only add non-empty strings.
        // This could happen if the user manually typed invalid entries.
        for (int i = 0; i < entryList.count(); ++i)
        {
            QString item = entryList[i].trimmed();

            if (item != "")
            {
                m_itemsList.append(item);
            }
        }

        // Update the list widget.
        UpdateListWidget();
    }

    // Insert an empty item to the list widget and select it.
    InsertBlankItem();
}

void rgOrderedListDialog::ConnectSignals()
{
    // Cancel button.
    bool isConnected = connect(ui.cancelPushButton, &QPushButton::clicked, this, &rgOrderedListDialog::HandleExit);
    assert(isConnected);

    // Add new entry button.
    isConnected = connect(this->ui.newPushButton, &QPushButton::clicked, this, &rgOrderedListDialog::HandleNewButtonClick);
    assert(isConnected);

    // Edit button.
    isConnected = connect(this->ui.editPushButton, &QPushButton::clicked, this, &rgOrderedListDialog::HandleEditButtonClick);
    assert(isConnected);

    // "OK" button.
    isConnected = connect(this->ui.okPushButton, &QPushButton::clicked, this, &rgOrderedListDialog::HandleOKButtonClick);
    assert(isConnected);

    // "Move up" button.
    isConnected = connect(this->ui.moveUpPushButton, &QPushButton::clicked, this, &rgOrderedListDialog::HandleMoveUpButtonClick);
    assert(isConnected);

    // "Move down" button.
    isConnected = connect(this->ui.moveDownPushButton, &QPushButton::clicked, this, &rgOrderedListDialog::HandleMoveDownButtonClick);
    assert(isConnected);

    // "Delete" button.
    isConnected = connect(this->ui.deletePushButton, &QPushButton::clicked, this, &rgOrderedListDialog::HandleDeleteButtonClick);
    assert(isConnected);

    // Items tree widget.
    isConnected = connect(ui.itemsList, &QListWidget::itemChanged, this, &rgOrderedListDialog::HandleListItemChanged);
    assert(isConnected);

    isConnected = connect(ui.itemsList, &QListWidget::itemSelectionChanged, this, &rgOrderedListDialog::HandleListItemSelectionChanged);
    assert(isConnected);
}

void rgOrderedListDialog::SetCursor()
{
    // Set the cursor to pointing hand cursor.
    ui.cancelPushButton->setCursor(Qt::PointingHandCursor);
    ui.deletePushButton->setCursor(Qt::PointingHandCursor);
    ui.moveDownPushButton->setCursor(Qt::PointingHandCursor);
    ui.moveUpPushButton->setCursor(Qt::PointingHandCursor);
    ui.newPushButton->setCursor(Qt::PointingHandCursor);
    ui.okPushButton->setCursor(Qt::PointingHandCursor);
}

void rgOrderedListDialog::SetButtonFonts()
{
    // Create font.
    QFont font = ui.deletePushButton->font();
    font.setPointSize(gs_BUTTON_POINT_FONT_SIZE);

    // Update font size for all the buttons.
    ui.cancelPushButton->setFont(font);
    ui.deletePushButton->setFont(font);
    ui.moveDownPushButton->setFont(font);
    ui.moveUpPushButton->setFont(font);
    ui.newPushButton->setFont(font);
    ui.okPushButton->setFont(font);
}

void rgOrderedListDialog::HandleExit(bool /* checked */)
{
    // Clear any existing data.
    m_itemsList.clear();
    ui.itemsList->clear();

    close();
}

void rgOrderedListDialog::HandleOKButtonClick(bool /* checked */)
{
    // Remove any empty entries.
    m_itemsList.removeAll("");

    // Emit a signal to indicate "OK" button clicked so the data can be saved.
    emit OKButtonClicked(m_itemsList);

    // Close the dialog.
    close();
}

void rgOrderedListDialog::UpdateListWidget()
{
    // Clear any existing data.
    ui.itemsList->clear();

    int counter = 0;

    // Update the list widget.
    foreach(auto entry, m_itemsList)
    {
        QListWidgetItem* pItem = new QListWidgetItem;
        pItem->setText(entry);
        pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
        ui.itemsList->insertItem(counter, pItem);
        counter++;
    }
}


void rgOrderedListDialog::HandleMoveDownButtonClick(bool /* checked */)
{
    // Get the current index.
    int currentIndex = ui.itemsList->currentRow();

    // Do not do anything if this is the last item.
    if (currentIndex < ui.itemsList->count() - 1)
    {
        // If the next item is empty, do not do anything.
        QString nextItem = ui.itemsList->item(currentIndex + 1)->text();
        if (!nextItem.isEmpty())
        {
            QListWidgetItem* pCurrentItem = ui.itemsList->takeItem(currentIndex);
            if (pCurrentItem != nullptr)
            {
                ui.itemsList->insertItem(currentIndex + 1, pCurrentItem);
                ui.itemsList->setCurrentRow(currentIndex + 1);

                // Update local data with this change.
                m_itemsList.swap(currentIndex, currentIndex + 1);
            }
        }
    }

    // Update buttons.
    UpdateButtons();
}

void rgOrderedListDialog::HandleMoveUpButtonClick(bool /* checked */)
{
    // Get the current index.
    int currentIndex = ui.itemsList->currentRow();

    // Make sure the index is not out of bounds.
    if (currentIndex < ui.itemsList->count())
    {
        // Do not do anything if this is the first item, or the text field is empty.
        if (currentIndex > 0 && !ui.itemsList->item(currentIndex)->text().isEmpty())
        {
            QListWidgetItem* pCurrentItem = ui.itemsList->takeItem(currentIndex);
            if (pCurrentItem != nullptr)
            {
                ui.itemsList->insertItem(currentIndex - 1, pCurrentItem);
                ui.itemsList->setCurrentRow(currentIndex - 1);

                // Update local data with this change.
                m_itemsList.swap(currentIndex - 1, currentIndex);
            }
        }
    }

    // Update buttons.
    UpdateButtons();
}

void rgOrderedListDialog::HandleDeleteButtonClick(bool /* checked */)
{
    // Display a confirmation message box.
    bool isConfirmation = rgUtils::ShowConfirmationMessageBox(STR_INCLUDE_DIR_DIALOG_DELETE_BOX_TITLE, STR_INCLUDE_DIR_DIALOG_DELETE_BOX_MESSAGE, this);

    if (isConfirmation)
    {
        // Remove the selected item from the list widget.
        int currentIndex = ui.itemsList->currentRow();
        QListWidgetItem* pCurrentItem = ui.itemsList->takeItem(currentIndex);

        // Remove the selected item from the string list.
        if (pCurrentItem != nullptr)
        {
            m_itemsList.removeAt(currentIndex);

            // Update buttons.
            UpdateButtons();
        }

        // If this was the last item in the list widget,
        // add a blank place holder.
        if (ui.itemsList->count() == 0)
        {
            // Insert an empty item to the list widget and highlight it.
            InsertBlankItem();
        }
    }
}

void rgOrderedListDialog::SetButtonShortcuts()
{
    // Add button keyboard shortcut.
    m_pAddAction = new QAction(this);
    m_pAddAction->setShortcut(QKeySequence(Qt::Key_Alt | Qt::Key_N));

    bool isConnected = connect(m_pAddAction, &QAction::triggered, this, &rgOrderedListDialog::HandleNewButtonClick);
    assert(isConnected);

    // Delete button keyboard shortcut.
    m_pDeleteAction = new QAction(this);
    m_pDeleteAction->setShortcut(QKeySequence(Qt::Key_Alt | Qt::Key_D));

    isConnected = connect(m_pDeleteAction, &QAction::triggered, this, &rgOrderedListDialog::HandleDeleteButtonClick);
    assert(isConnected);

    // "Move up" button shortcut.
    m_pMoveUpAction = new QAction(this);
    m_pMoveUpAction->setShortcut(QKeySequence(Qt::Key_Alt | Qt::Key_U));

    isConnected = connect(m_pMoveUpAction, &QAction::triggered, this, &rgOrderedListDialog::HandleMoveUpButtonClick);
    assert(isConnected);

    // "Move down" button shortcut.
    m_pMoveDownAction = new QAction(this);
    m_pMoveDownAction->setShortcut(QKeySequence(Qt::Key_Alt | Qt::Key_O));

    isConnected = connect(m_pMoveDownAction, &QAction::triggered, this, &rgOrderedListDialog::HandleMoveDownButtonClick);
    assert(isConnected);
}

void rgOrderedListDialog::HandleListItemChanged(QListWidgetItem* pItem)
{
    OnListItemChanged(pItem);
}

void rgOrderedListDialog::HandleNewButtonClick(bool /* checked */)
{
    // Get the last item in the list widget.
    QListWidgetItem* pItem = nullptr;
    if (ui.itemsList->count() > 0)
    {
        pItem = ui.itemsList->item(ui.itemsList->count() - 1);
    }
    if (pItem != nullptr && pItem->text().isEmpty())
    {
        QListWidgetItem* pItem = ui.itemsList->item(ui.itemsList->count() - 1);
        ui.itemsList->setCurrentItem(pItem);
        ui.itemsList->editItem(pItem);
    }
    else
    {
        QListWidgetItem* pItem = new QListWidgetItem;
        pItem->setText("");
        pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
        ui.itemsList->setCurrentItem(pItem);
        ui.itemsList->insertItem(ui.itemsList->count(), pItem);
        ui.itemsList->editItem(pItem);
    }

    // Update buttons.
    UpdateButtons();
}

void rgOrderedListDialog::HandleEditButtonClick(bool /* checked */)
{
    // Get the current row.
    const int row = ui.itemsList->currentRow();

    // Make the current row editable.
    QListWidgetItem* pItem = ui.itemsList->item(row);
    ui.itemsList->editItem(pItem);
}

void rgOrderedListDialog::UpdateButtons()
{
    // Enable/disable move up/down buttons.
    if (ShouldDisableMoveUpDownButtons())
    {
        ui.moveUpPushButton->setEnabled(false);
        ui.moveDownPushButton->setEnabled(false);
    }
    else
    {
        // Get the current row.
        const int currentRow = ui.itemsList->currentRow();

        // Set both up/down buttons to disabled for now.
        ui.moveUpPushButton->setEnabled(false);
        ui.moveDownPushButton->setEnabled(false);

        // Figure out which up/down buttons to enable.
        if (currentRow > 0)
        {
            ui.moveUpPushButton->setEnabled(true);
        }
        if (currentRow >= 0 && currentRow < m_itemsList.count() - 1)
        {
            ui.moveDownPushButton->setEnabled(true);
        }
    }

    // Enable/Disable the Edit and Delete buttons.
    if (ui.itemsList->count() == 0)
    {
        ui.editPushButton->setEnabled(false);
        ui.deletePushButton->setEnabled(false);
    }
    else
    {
        ui.editPushButton->setEnabled(true);
        ui.deletePushButton->setEnabled(true);
    }
}

bool rgOrderedListDialog::ShouldDisableMoveUpDownButtons()
{
    bool shouldDisableButtons = false;

    if (ui.itemsList->count() < 2)
    {
        shouldDisableButtons = true;
    }
    else if (ui.itemsList->count() == 2)
    {
        if (ui.itemsList->item(1)->text().isEmpty())
        {
            shouldDisableButtons = true;
        }
    }
    else if (ui.itemsList->currentRow() >= 0 &&
             ui.itemsList->item(ui.itemsList->currentRow())->text().isEmpty())
    {
        shouldDisableButtons = true;
    }

    return shouldDisableButtons;
}

void rgOrderedListDialog::UpdateToolTips()
{
    for (int i = 0; i < ui.itemsList->count(); i++)
    {
        QListWidgetItem* pItem = ui.itemsList->item(i);
        if (pItem != nullptr)
        {
            pItem->setToolTip(pItem->text());
        }
    }
}

void rgOrderedListDialog::InsertBlankItem()
{
    // Create a blank item and insert it in the list widget.
    QListWidgetItem* pItem = new QListWidgetItem();
    pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
    ui.itemsList->addItem(pItem);
    ui.itemsList->setCurrentItem(pItem);
    ui.itemsList->setFocus();
}

void rgOrderedListDialog::HandleListItemSelectionChanged()
{
    UpdateButtons();
}
