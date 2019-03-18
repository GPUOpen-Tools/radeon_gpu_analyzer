// C++.
#include <cassert>
#include <sstream>
#include <algorithm>

// Qt.
#include <QFileDialog>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIncludeDirectoriesView.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

rgIncludeDirectoriesView::rgIncludeDirectoriesView(const char* pDelimiter, QWidget* pParent) :
    rgOrderedListDialog(pDelimiter, pParent)
{
    // Initialize the browse button that gets inserted into the view.
    InitializeBrowseButton();

    // Set the window title.
    setWindowTitle(STR_INCLUDE_DIR_DIALOG_SELECT_INCLUDE_DIRS);

    // Connect the signals.
    ConnectSignals();

    // Set the mouse cursor to the pointing hand cursor for various widgets.
    SetCursor();

    // Set the button fonts.
    SetButtonFonts();

    // Update various buttons.
    UpdateButtons();
}

void rgIncludeDirectoriesView::ConnectSignals()
{
    assert(m_pBrowsePushButton != nullptr);

    // Browse new include directory button.
    bool isConnected = connect(m_pBrowsePushButton, &QPushButton::clicked, this, &rgIncludeDirectoriesView::HandleIncludeFileLocationBrowseButtonClick);
    assert(isConnected);
}

void rgIncludeDirectoriesView::InitializeBrowseButton()
{
    // Create a new button to browse for new directories.
    m_pBrowsePushButton = new QPushButton(this);
    m_pBrowsePushButton->setText(STR_INCLUDE_DIR_DIALOG_BROWSE_BUTTON);

    assert(m_pBrowsePushButton != nullptr);
    if (m_pBrowsePushButton != nullptr)
    {
        // This index specifies where the browse button will get inserted in the vertical layout of buttons in the dialog.
        static const int BROWSE_BUTTON_INSERTION_INDEX = 2;

        // Add the new browse button to the view.
        QVBoxLayout* pVerticalButtonsLayout = ui.verticalPushButtonsLayout;
        assert(pVerticalButtonsLayout != nullptr);
        if (pVerticalButtonsLayout != nullptr)
        {
            pVerticalButtonsLayout->insertWidget(BROWSE_BUTTON_INSERTION_INDEX, m_pBrowsePushButton);
            ScalingManager::Get().RegisterObject(m_pBrowsePushButton);
        }
    }
}

void rgIncludeDirectoriesView::SetCursor()
{
    assert(m_pBrowsePushButton != nullptr);
    if (m_pBrowsePushButton != nullptr)
    {
        // Set the cursor to pointing hand cursor on the Browse button.
        m_pBrowsePushButton->setCursor(Qt::PointingHandCursor);
    }
}

void rgIncludeDirectoriesView::SetButtonFonts()
{
    // Create a font based on other QPushButtons in within the view.
    QFont font = ui.deletePushButton->font();
    font.setPointSize(gs_BUTTON_POINT_FONT_SIZE);

    assert(m_pBrowsePushButton != nullptr);
    if (m_pBrowsePushButton != nullptr)
    {
        // Update font size for all the buttons.
        m_pBrowsePushButton->setFont(font);
    }
}

void rgIncludeDirectoriesView::HandleIncludeFileLocationBrowseButtonClick(bool /* checked */)
{
    // Create a file chooser dialog.
    QFileDialog fileDialog;

    // Get the last entry from the directories list and open the dialog there.
    std::string latestPath = rgConfigManager::Instance().GetLastSelectedFolder();

    // If the user has an item selected, use that as starting path.
    // Otherwise use the last item in the list since it is probably most recent.
    QListWidgetItem* pItem = ui.itemsList->currentItem();
    if (pItem != nullptr && !pItem->text().isEmpty() && QDir(pItem->text()).exists())
    {
        latestPath = pItem->text().toStdString();
    }
    else if (m_itemsList.size() > 0)
    {
        latestPath = m_itemsList.at(m_itemsList.size() - 1).toStdString();
    }

    bool shouldShowFindDirectoryDialog = true;

    while (shouldShowFindDirectoryDialog)
    {
        QString selectedDirectory = QFileDialog::getExistingDirectory(this, tr(STR_INCLUDE_DIR_DIALOG_SELECT_DIR_TITLE),
            latestPath.c_str(),
            QFileDialog::ShowDirsOnly);

        // If the user did not select an entry, don't update anything, just exit the loop.
        if (selectedDirectory.isEmpty())
        {
            shouldShowFindDirectoryDialog = false;
        }
        else
        {
            // If not a duplicate selection, update the list widget.
            if (!m_itemsList.contains(selectedDirectory))
            {
                QListWidgetItem* pItem = ui.itemsList->currentItem();
                if (pItem == nullptr)
                {
                    // There was no selected item,
                    // so make sure there is a final entry that is empty, and edit that.
                    if (ui.itemsList->count() > 0)
                    {
                        pItem = ui.itemsList->item(ui.itemsList->count() - 1);
                    }

                    if (pItem == nullptr || !pItem->text().isEmpty())
                    {
                        // Add an empty row to the dialog
                        InsertBlankItem();
                        assert(ui.itemsList->count() > 0);

                        // Use the empty row for the new item
                        pItem = ui.itemsList->item(ui.itemsList->count() - 1);
                    }
                }

                assert(pItem != nullptr);
                pItem->setText(selectedDirectory);

                // If the last item in the UI is no longer empty, add another item.
                pItem = ui.itemsList->item(ui.itemsList->count() - 1);
                if (pItem == nullptr || !pItem->text().isEmpty())
                {
                    // Add an empty row to the dialog
                    InsertBlankItem();
                }

                // Don't show the find directory dialog again.
                shouldShowFindDirectoryDialog = false;
            }
            else
            {
                // Display an error message when a duplicate directory is selected.
                rgUtils::ShowErrorMessageBox(STR_INCLUDE_DIR_DIALOG_DIR_ALREADY_SELECTED, this);

                // Make the user try again.
                shouldShowFindDirectoryDialog = true;
            }
        }
    }

    // Update move buttons.
    UpdateButtons();
}

void rgIncludeDirectoriesView::OnListItemChanged(QListWidgetItem* pItem)
{
    // Block signals from the list widget.
    ui.itemsList->blockSignals(true);

    m_editingInvalidEntry = false;

    // Process the newly-entered data.
    if (pItem != nullptr)
    {
        QString newDirectory = pItem->text();

        bool directoryExists = QDir(newDirectory).exists();
        bool directoryDuplicate = m_itemsList.contains(newDirectory);
        bool directoryValueEmpty = newDirectory.isEmpty();

        int itemRow = ui.itemsList->row(pItem);

        if (directoryValueEmpty && itemRow != ui.itemsList->count() - 1)
        {
            // The user has emptied out the entry, so delete it.
            // Simulate a click on the delete button to remove the entry from the UI.
            ui.deletePushButton->click();
        }
        else
        {
            // If the new directory exists, and it is not a duplicate entry, update local data.
            if (!directoryExists || directoryDuplicate)
            {
                // Display an error message box.
                if (!directoryExists)
                {
                    rgUtils::ShowErrorMessageBox(STR_INCLUDE_DIR_DIALOG_DIR_DOES_NOT_EXIST, this);
                }
                else if (directoryDuplicate)
                {
                    rgUtils::ShowErrorMessageBox(STR_INCLUDE_DIR_DIALOG_DIR_ALREADY_SELECTED, this);
                }

                m_editingInvalidEntry = true;
            }

            // Update local data.
            if (itemRow < m_itemsList.count())
            {
                m_itemsList[itemRow] = newDirectory;
            }
            else
            {
                m_itemsList.append(newDirectory);
            }
        }

        // Update tool tips.
        UpdateToolTips();

        // Unblock signals from the list widget.
        ui.itemsList->blockSignals(false);

        // Update buttons.
        UpdateButtons();
    }
}
