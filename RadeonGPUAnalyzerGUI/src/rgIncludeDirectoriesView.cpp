// C++.
#include <cassert>
#include <sstream>
#include <algorithm>

// Qt.
#include <QFileDialog>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgIncludeDirectoriesView.h>
#include <RadeonGPUAnalyzerGUI/include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>

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

    // Set push button shortcuts.
    SetButtonShortcuts();

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

    // Get the latest entry from the directories list and open the dialog there.
    std::string latestPath = rgConfigManager::GetLastSelectedFolder();
    if (m_itemsList.size() > 0)
    {
        latestPath = m_itemsList.at(m_itemsList.size() - 1).toStdString();
    }

    QString selectedDirectory = QFileDialog::getExistingDirectory(this, tr(STR_INCLUDE_DIR_DIALOG_SELECT_DIR_TITLE),
                                                                  latestPath.c_str(),
                                                                  QFileDialog::ShowDirsOnly);

    // If the user selected an entry, process it.
    if (!selectedDirectory.isEmpty())
    {
        // If not a duplicate selection, update the list widget.
        if (!m_itemsList.contains(selectedDirectory))
        {
            QListWidgetItem* pItem = nullptr;
            if (ui.itemsList->count() > 0)
            {
                pItem = ui.itemsList->item(ui.itemsList->count() - 1);
            }
            if (pItem != nullptr && pItem->text().isEmpty())
            {
                QListWidgetItem* pItem = ui.itemsList->item(ui.itemsList->count() - 1);
                pItem->setText(selectedDirectory);
                ui.itemsList->setCurrentItem(pItem);
            }
            else
            {
                QListWidgetItem* pItem = new QListWidgetItem;
                pItem->setText(selectedDirectory);
                pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
                ui.itemsList->setCurrentItem(pItem);
                ui.itemsList->insertItem(ui.itemsList->count(), pItem);
            }

            // Update move buttons.
            UpdateButtons();
        }
        else
        {
            // Display an error message when a duplicate directory is selected.
            rgUtils::ShowErrorMessageBox(STR_INCLUDE_DIR_DIALOG_DIR_ALREADY_SELECTED, this);
        }
    }
}

void rgIncludeDirectoriesView::OnListItemChanged(QListWidgetItem* pItem)
{
    // Block signals from the list widget.
    ui.itemsList->blockSignals(true);

    // Process the newly-entered data.
    if (pItem != nullptr)
    {
        QString newDirectory = pItem->text();

        bool directoryExists = QDir(newDirectory).exists();
        bool directoryDuplicate = m_itemsList.contains(newDirectory);
        bool directoryValueEmpty = newDirectory.isEmpty();
        // If the new directory exists, and it is not a duplicate entry, update local data.
        if (directoryExists && !directoryDuplicate && !directoryValueEmpty)
        {
            // Update local data.
            m_itemsList << newDirectory;

            // Update tool tips.
            UpdateToolTips();

            m_editingInvalidEntry = false;
        }
        else
        {
            if (!m_editingInvalidEntry)
            {
                // Update local data.
                m_itemsList << newDirectory;
            }

            // Display an error message box.
            if (!directoryExists)
            {
                rgUtils::ShowErrorMessageBox(STR_INCLUDE_DIR_DIALOG_DIR_DOES_NOT_EXIST, this);
            }
            else if (directoryDuplicate && !newDirectory.isEmpty())
            {
                rgUtils::ShowErrorMessageBox(STR_INCLUDE_DIR_DIALOG_DIR_ALREADY_SELECTED, this);
            }

            m_editingInvalidEntry = true;
        }

        // Unblock signals from the list widget.
        ui.itemsList->blockSignals(false);

        // Update buttons.
        UpdateButtons();
    }
}

void rgIncludeDirectoriesView::SetButtonShortcuts()
{
    // Browse button keyboard shortcut.
    m_pBrowseAction = new QAction(this);
    m_pBrowseAction->setShortcut(QKeySequence(Qt::Key_Alt | Qt::Key_B));

    bool isConnected = connect(m_pBrowseAction, &QAction::triggered, this, &rgIncludeDirectoriesView::HandleIncludeFileLocationBrowseButtonClick);
    assert(isConnected);
}
