// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QApplication>
#include <QAction>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuFileItem.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

rgMenuFileItem::rgMenuFileItem(const std::string& fullFilePath, rgMenu* pParent)
    : rgMenuItem(pParent)
    , m_fullFilepath(fullFilePath)
{
    // Initialize the context menu for right-clicks on the item.
    InitializeContextMenu();

    // Connect signals to slots.
    ConnectSignals();
}

void rgMenuFileItem::ConnectSignals()
{
    // Connect a FocusChanged handler so we know when a filename change is completed.
    bool isConnected = connect(qApp, &QApplication::focusChanged, this, &rgMenuFileItem::HandleFocusChanged);
    assert(isConnected);

    // Connect the item's "Open in file browser" menu item.
    isConnected = connect(m_contextMenuActions.pOpenContainingFolder, &QAction::triggered, this, &rgMenuFileItem::HandleOpenInFileBrowserClicked);
    assert(isConnected);

    // Connect the item's "Rename" menu item.
    isConnected = connect(m_contextMenuActions.pRenameFile, &QAction::triggered, this, &rgMenuFileItem::HandleRenameClicked);
    assert(isConnected);

    // Connect the handler responsible for showing the item's context menu.
    isConnected = connect(this, &QWidget::customContextMenuRequested, this, &rgMenuFileItem::HandleOpenContextMenu);
    assert(isConnected);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
}

const std::string& rgMenuFileItem::GetFilename() const
{
    return m_fullFilepath;
}

void rgMenuFileItem::OpenContextMenu()
{
    QPoint centerPoint(width() / 2, height() / 2);

    // Open the context menu on a default centered point.
    HandleOpenContextMenu(centerPoint);
}

void rgMenuFileItem::ShowRenameControls(bool isRenaming)
{
    QLineEdit* pLineEdit = GetRenameLineEdit();
    QLabel* pItemLabel = GetItemLabel();

    // Disable signals from the file name line edit and the application for now,
    // so updating line edit and setting focus to it do not cause another
    // signal to fire, causing an infinite loop.
    QSignalBlocker signalBlockerLineEdit(pLineEdit);
    QSignalBlocker signalBlockerApplication(qApp);

    // Swap the visibility of the filename label and line edit.
    if (isRenaming)
    {
        pItemLabel->setVisible(false);
        pLineEdit->setText(m_filename.c_str());
        pLineEdit->setVisible(true);

        std::string filenameOnly;
        bool gotFilename = rgUtils::ExtractFileName(m_filename, filenameOnly, false);
        assert(gotFilename);

        // Focus on the widget with the filename selected so the user can start typing immediately.
        pLineEdit->setFocus();
        pLineEdit->setSelection(0, static_cast<int>(filenameOnly.length()));

        // Set cursor to IBeam cursor.
        setCursor(Qt::IBeamCursor);
    }
    else
    {
        pItemLabel->setVisible(true);
        pLineEdit->setVisible(false);

        // Set cursor to Arrow cursor.
        setCursor(Qt::ArrowCursor);
    }
}

void rgMenuFileItem::SetIsSaved(bool isSaved)
{
    // Only refresh if there is a change.
    if (m_isSaved != isSaved)
    {
        m_isSaved = isSaved;
        UpdateFilenameLabelText();
    }
    else
    {
        m_isSaved = isSaved;
    }
}

void rgMenuFileItem::UpdateFilepath(const std::string& newFilepath)
{
    m_fullFilepath = newFilepath;

    if (!newFilepath.empty())
    {
        // Only display the filename in the interface- not the full path to the file.
        bool isOk = rgUtils::ExtractFileName(newFilepath, m_filename);
        assert(isOk);
    }
    else
    {
        // When the full path to the file is cleared, also clear the filename string.
        m_filename.clear();
    }

    // Update the view to display the latest filename.
    UpdateFilenameLabelText();
}

void rgMenuFileItem::HandleEnterPressed()
{
    // Handle the file name change.
    RenameFile();
}

void rgMenuFileItem::HandleFocusChanged(QWidget* pOld, QWidget* pNow)
{
    Q_UNUSED(pNow);

    if (pOld != nullptr)
    {
        // If the control that lost focus was the renaming QLineEdit, finish the item rename.
        if (pOld == GetRenameLineEdit())
        {
            RenameFile();
        }
    }
}

void rgMenuFileItem::HandleOpenInFileBrowserClicked()
{
    std::string fileDirectory;
    bool gotDirectory = rgUtils::ExtractFileDirectory(GetFilename(), fileDirectory);
    assert(gotDirectory);

    if (gotDirectory)
    {
        // Open a system file browser window pointing to the given directory.
        rgUtils::OpenFolderInFileBrowser(fileDirectory);
    }
}

void rgMenuFileItem::HandleRenameClicked()
{
    // Show the file item renaming controls.
    ShowRenameControls(true);
}

void rgMenuFileItem::HandleOpenContextMenu(const QPoint& widgetClickPosition)
{
    // Only open the context menu for file items that aren't empty.
    if (!m_fullFilepath.empty())
    {
        // Convert the widget's local click position to the global screen position.
        const QPoint clickPoint = mapToGlobal(widgetClickPosition);

        // Open the context menu at the user's click position.
        m_pContextMenu->exec(clickPoint);
    }
}

void rgMenuFileItem::InitializeContextMenu()
{
    // Create the context menu instance.
    m_pContextMenu = new QMenu(this);

    // Set the cursor for the context menu.
    m_pContextMenu->setCursor(Qt::PointingHandCursor);

    // Create the menu items to insert into the context menu.
    m_contextMenuActions.pOpenContainingFolder = new QAction(STR_FILE_CONTEXT_MENU_OPEN_CONTAINING_FOLDER, this);
    m_pContextMenu->addAction(m_contextMenuActions.pOpenContainingFolder);

    // Add a separator between the current menu items.
    m_pContextMenu->addSeparator();

    // Create the rename action and add it to the menu.
    m_contextMenuActions.pRenameFile = new QAction(STR_FILE_CONTEXT_MENU_RENAME_FILE, this);
    m_pContextMenu->addAction(m_contextMenuActions.pRenameFile);

    // Create the remove action and add it to the menu.
    m_contextMenuActions.pRemoveFile = new QAction(STR_FILE_CONTEXT_MENU_REMOVE_FILE, this);
    m_pContextMenu->addAction(m_contextMenuActions.pRemoveFile);
}

bool rgMenuFileItem::RenameFile()
{
    bool isFileRenamed = false;

    if (!m_isEscapePressed)
    {
        QLineEdit* pLineEdit = GetRenameLineEdit();

        // The new filename is whatever the user left in the renaming QLineEdit.
        const std::string newFilename = pLineEdit->text().toStdString();

        // Only attempt a rename if the user has altered the filename string.
        bool filenameChanged = m_filename.compare(newFilename) != 0;

        // If the current filename differs from what the user left in the QLineEdit, update the filename.
        if (filenameChanged)
        {
            // The renamed file will live in the same location as the old one.
            std::string fileFolderPath;
            bool gotFolder = rgUtils::ExtractFileDirectory(m_fullFilepath, fileFolderPath);
            assert(gotFolder);

            // Generate the full path to where the new file lives.
            std::string newFilepath;
            rgUtils::AppendFileNameToPath(fileFolderPath, newFilename, newFilepath);

            // Disable signals from the file name line edit and the application for now,
            // so updating line edit and setting focus to it do not cause another
            // signal to fire, causing an infinite loop.
            QSignalBlocker signalBlockerLineEdit(pLineEdit);
            QSignalBlocker signalBlockerApplication(qApp);
            if (rgUtils::IsValidFileName(newFilename))
            {
                if (!rgUtils::IsFileExists(newFilepath))
                {
                    // Rename the file on disk.
                    rgUtils::RenameFile(m_fullFilepath, newFilepath);

                    // Signal to the file menu that the file path has changed.
                    emit FileRenamed(m_fullFilepath, newFilepath);

                    // Update the file path so the item will display the correct filename in the menu.
                    UpdateFilepath(newFilepath);

                    // The file on disk was successfully renamed.
                    isFileRenamed = true;
                }
                else
                {
                    // Show an error message stating that the rename failed because a
                    // file with the same name already exists in the same location.
                    std::stringstream msg;
                    msg << STR_ERR_CANNOT_RENAME_FILE_A;
                    msg << newFilename;
                    msg << STR_ERR_CANNOT_RENAME_FILE_B_ALREADY_EXISTS;

                    rgUtils::ShowErrorMessageBox(msg.str().c_str(), this);
                }
            }
            else
            {
                std::stringstream msg;
                // Show an error message stating that the rename failed because
                // the given name is invalid.
                if (newFilename.empty())
                {
                    msg << STR_ERR_CANNOT_RENAME_FILE_BLANK_FILENAME;
                }
                else
                {
                    msg << STR_ERR_CANNOT_RENAME_FILE_A;
                    msg << newFilename;
                    msg << STR_ERR_CANNOT_RENAME_FILE_B_ILLEGAL_FILENAME;
                }

                rgUtils::ShowErrorMessageBox(msg.str().c_str(), this);
            }
        }

        // Re-enable the filename editing controls, since the user gets another chance to attempt a rename.
        if (!isFileRenamed && filenameChanged)
        {
            // Toggle the item back to editing mode, showing the original filename.
            ShowRenameControls(true);
        }
        else
        {
            // Toggle the item back to being read-only, showing the updated filename.
            ShowRenameControls(false);
        }
    }
    else
    {
        m_isEscapePressed = false;
    }
    return isFileRenamed;
}