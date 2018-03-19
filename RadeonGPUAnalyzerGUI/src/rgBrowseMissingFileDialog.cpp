// C++.
#include <cassert>

// Qt.
#include <QWidget>
#include <QDialog>
#include <QSignalMapper>
#include <QPainter>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgBrowseMissingFileDialog.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/include/rgDefinitions.h>

rgBrowseMissingFileDialog::rgBrowseMissingFileDialog(rgProjectAPI api, QWidget* pParent)
    : QDialog(pParent)
    , m_projectAPI(api)
{
    // Setup the UI.
    ui.setupUi(this);

    // Workaround to avoid browsing.
    ui.browsePushButton->setVisible(false);
    ui.cancelPushButton->setVisible(false);

    // Connect the signals.
    ConnectSignals();

    // Create item delegate for the list widget.
    m_pItemDelegate = new rgMissingFileItemDelegate();
    bool isDelegateValid = (m_pItemDelegate != nullptr);
    assert(isDelegateValid);

    if (isDelegateValid)
    {
        // Set custom delegate for the list widget.
        ui.fileListWidget->setItemDelegate(m_pItemDelegate);
    }

    // Disable selection of items.
    ui.fileListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // Disable the browse button until the user selects a row to update.
    ToggleBrowseButtonEnabled(false);

    // Disable the help button in the title bar.
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

rgBrowseMissingFileDialog::~rgBrowseMissingFileDialog()
{
    delete m_pItemDelegate;
}

void rgBrowseMissingFileDialog::ConnectSignals()
{
    // File list's selected row changed handler.
    QItemSelectionModel* pFileListSelectionModel = ui.fileListWidget->selectionModel();
    bool isConnected = connect(pFileListSelectionModel, &QItemSelectionModel::selectionChanged, this, &rgBrowseMissingFileDialog::HandleSelectedFilePathChanged);
    assert(isConnected);

    // Browse button.
    isConnected = connect(ui.browsePushButton, &QPushButton::clicked, this, &rgBrowseMissingFileDialog::HandleBrowseButtonClicked);
    assert(isConnected);

    // Create a signal mapper to map the button clicks to the done(int) slot
    // with appropriate result values.
    QSignalMapper* pButtonSignalMapper = new QSignalMapper(this);

    // Note: Using the SIGNAL/SLOT syntax below to simplify use of QSignalMapper.
    // OK button.
    isConnected = connect(ui.okPushButton, SIGNAL(clicked()), pButtonSignalMapper, SLOT(map()));
    assert(isConnected);
    pButtonSignalMapper->setMapping(ui.okPushButton, MissingFileDialogResult::OK);

    // Cancel button.
    isConnected = connect(ui.cancelPushButton, SIGNAL(clicked()), pButtonSignalMapper, SLOT(map()));
    assert(isConnected);
    pButtonSignalMapper->setMapping(ui.cancelPushButton, MissingFileDialogResult::Cancel);

    // Signal mapper.
    isConnected = connect(pButtonSignalMapper, SIGNAL(mapped(int)), this, SLOT(done(int)));
    assert(isConnected);

    isConnected = connect(this, &QDialog::rejected, this, &rgBrowseMissingFileDialog::HandleRejected);
    assert(isConnected);
}

void rgBrowseMissingFileDialog::ToggleBrowseButtonEnabled(bool isEnabled)
{
    ui.browsePushButton->setEnabled(isEnabled);
}

void rgBrowseMissingFileDialog::AddFile(const std::string filename)
{
    ui.fileListWidget->addItem(filename.c_str());
}

const std::map<std::string, std::string>& rgBrowseMissingFileDialog::GetUpdateFilePathsMap() const
{
    return m_updatedPathMap;
}

void rgBrowseMissingFileDialog::HandleBrowseButtonClicked()
{
    // Use the selection model to determine which path the user wants to update.
    QItemSelectionModel* pSelectionModel = ui.fileListWidget->selectionModel();
    assert(pSelectionModel != nullptr);
    if (pSelectionModel != nullptr)
    {
        QModelIndex selectedRowIndex = pSelectionModel->currentIndex();
        int rowIndex = selectedRowIndex.row();

        BrowseFilePathForRow(rowIndex);
    }
}

void rgBrowseMissingFileDialog::BrowseFilePathForRow(int rowIndex)
{
    QModelIndex selectedRowIndex = ui.fileListWidget->model()->index(rowIndex, 0);
    bool isIndexValid = selectedRowIndex.isValid();
    assert(isIndexValid);

    if (isIndexValid)
    {
        QVariant rowData = selectedRowIndex.data(Qt::DisplayRole);

        // Extract the existing path to the missing file.
        const std::string originalFilePath = rowData.toString().toStdString();
        std::string updatedFilePath = originalFilePath;

        // Show a file browser dialog so the user can update the path to the missing file.
        bool filePathUpdated = rgUtils::OpenFileDialog(this, m_projectAPI, updatedFilePath);

        // Did the user provide a valid path to replace the missing file's path?
        if (filePathUpdated)
        {
            bool updatedFileExists = rgUtils::IsFileExists(updatedFilePath);
            assert(updatedFileExists);

            if (updatedFileExists)
            {
                // Update the map with the new file paths.
                m_updatedPathMap[originalFilePath] = updatedFilePath;

                // Update the list row to display the new updated file path.
                ui.fileListWidget->model()->setData(selectedRowIndex, updatedFilePath.c_str());
            }
        }
    }
}

void rgBrowseMissingFileDialog::HandleSelectedFilePathChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    // Toggle the enabledness of the Browse button if necessary.
    bool enableBrowse = false;
    if (!selected.empty())
    {
        QModelIndexList selectedIndices = selected.indexes();
        if (!selectedIndices.empty())
        {
            QModelIndex selectedRowIndex = selectedIndices[0];
            if (selectedRowIndex.isValid())
            {
                // The browse button should be clickable if the user has selected a valid row.
                enableBrowse = true;
            }
        }
    }

    ToggleBrowseButtonEnabled(enableBrowse);
}

void rgBrowseMissingFileDialog::HandleSelectedRowDoubleClicked(QListWidgetItem* pItem)
{
    int rowIndex = ui.fileListWidget->row(pItem);

    BrowseFilePathForRow(rowIndex);
}

void rgBrowseMissingFileDialog::HandleRejected()
{
    setResult(MissingFileDialogResult::Cancel);
}

void rgMissingFileItemDelegate::drawDisplay(QPainter* pPainter, const QStyleOptionViewItem& option, const QRect &rect, const QString& text) const
{
    bool isPainterValid = (pPainter != nullptr);
    assert(isPainterValid);

    if (isPainterValid)
    {
        // Truncate string so it fits within rect.
        QString truncatedString = rgUtils::TruncateString(text.toStdString(), gs_TEXT_TRUNCATE_LENGTH_FRONT,
            gs_TEXT_TRUNCATE_LENGTH_BACK, rect.width(), pPainter->font(), rgUtils::EXPAND_BACK).c_str();

        // Draw text within rect.
        pPainter->drawText(rect, Qt::AlignVCenter, truncatedString);
    }
}

QSize rgMissingFileItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // Use standard size hint implementation, but with a fixed width of 0 (width will be determined by view width).
    QSize adjustedHint = QItemDelegate::sizeHint(option, index);
    adjustedHint.setWidth(0);

    return adjustedHint;
}