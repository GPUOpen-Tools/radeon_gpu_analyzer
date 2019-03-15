// C++.
#include <sstream>
#include <cassert>

// Qt.
#include <QMimeData>
#include <QDragEnterEvent>
#include <QPushButton>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuPipelineStateItem.h>

static const char* s_BUTTON_FOCUS_IN_STYLESHEET_GRAPHICS = "QPushButton { background: rgb(253,255,174); border-style: solid; border-width: 2px; border-color: rgb(135, 20, 16);}";
static const char* s_BUTTON_FOCUS_OUT_STYLESHEET = "QPushButton { margin: 1px; background: rgb(214, 214, 214);}";
static const char* s_STR_DEFAULT_PIPELINE_FILE_EXTENSION_GRAPHICS = "gpso";
static const char* s_STR_DEFAULT_PIPELINE_FILE_EXTENSION_COMPUTE = "cpso";
static const char* s_FILE_EXTENSION_DELIMITER = ".";

rgMenuPipelineStateItem::rgMenuPipelineStateItem(rgPipelineType pipelineType, rgMenu* pParent)
    : m_pipelineType(pipelineType),
    rgMenuItem(pParent)
{
    ui.setupUi(this);

    // Set the status bar tip.
    this->setStatusTip(STR_GRAPHICS_MENU_PIPELINE_STATE_TOOLTIP);

    // Set the tool tip.
    this->setToolTip(STR_GRAPHICS_MENU_PIPELINE_STATE_TOOLTIP);

    // Set the mouse cursor to arrow cursor.
    SetCursor(Qt::PointingHandCursor);

    // Connect the file menu signals.
    ConnectSignals();

    // Enable drag and drop.
    setAcceptDrops(true);
}

void rgMenuPipelineStateItem::ConnectSignals()
{
    // Connect the "Pipeline State" button to a handler to convert the parameter.
    bool isConnected = connect(ui.pipelineStateButton, &QPushButton::clicked, this, &rgMenuPipelineStateItem::HandlePipelineStateButtonClicked);
    assert(isConnected);
}

void rgMenuPipelineStateItem::HandlePipelineStateButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    emit PipelineStateButtonClicked(this);
}

void rgMenuPipelineStateItem::SetItemText(const std::string& itemText)
{
    ui.pipelineStateButton->setText(itemText.c_str());
}

QPushButton* rgMenuPipelineStateItem::GetPipelineStateButton() const
{
    return ui.pipelineStateButton;
}

void rgMenuPipelineStateItem::GotFocus()
{
    // Set arrow cursor so it doesn't appear that the user can click on the button again.
    ui.pipelineStateButton->setCursor(Qt::ArrowCursor);

    // Set stylesheet.
    ui.pipelineStateButton->setStyleSheet(s_BUTTON_FOCUS_IN_STYLESHEET_GRAPHICS);
}

void rgMenuPipelineStateItem::LostFocus()
{
    // Set pointing hand cursor so it looks like the user can click on it.
    ui.pipelineStateButton->setCursor(Qt::PointingHandCursor);

    // Set stylesheet.
    ui.pipelineStateButton->setStyleSheet(s_BUTTON_FOCUS_OUT_STYLESHEET);
}

void rgMenuPipelineStateItem::SetCursor(const QCursor& cursor)
{
    // Set the mouse cursor to the specified type.
    ui.pipelineStateButton->setCursor(cursor);
}

void rgMenuPipelineStateItem::SetCurrent(bool isCurrent)
{
    m_current = isCurrent;

    if (m_current)
    {
        GotFocus();
    }
    else
    {
        LostFocus();
    }
}

bool rgMenuPipelineStateItem::IsCurrent() const
{
    return m_current;
}

void rgMenuPipelineStateItem::ClickMenuItem() const
{
    QPushButton* pPipelineStateButton = GetPipelineStateButton();
    assert(pPipelineStateButton != nullptr);
    if (pPipelineStateButton != nullptr)
    {
        pPipelineStateButton->click();
    }
}

void rgMenuPipelineStateItem::dragEnterEvent(QDragEnterEvent* pEvent)
{
    assert(pEvent != nullptr);
    if (pEvent != nullptr)
    {
        const QMimeData* pMimeData = pEvent->mimeData();
        assert(pMimeData != nullptr);
        if (pMimeData != nullptr)
        {
            const int numFiles = pMimeData->urls().size();

            // Make sure the drop data has only one file url, and is a valid file.
            if (pMimeData->hasUrls() && (numFiles == 1))
            {
                // Check to make sure the file is valid.
                QUrl url = pMimeData->urls().at(0);

                // Verify we have the correct file for the current pipeline type.
                bool validFile = false;
                QString extension;
                const QString filePath = url.toLocalFile();
                QStringList nameExtension = filePath.split(s_FILE_EXTENSION_DELIMITER);
                assert(nameExtension.size() == 2);
                if (nameExtension.size() == 2)
                {
                    extension = filePath.split(".").at(1);
                    if (m_pipelineType == rgPipelineType::Graphics && extension.compare(s_STR_DEFAULT_PIPELINE_FILE_EXTENSION_GRAPHICS) == 0)
                    {
                        validFile = true;
                    }
                    else if (m_pipelineType == rgPipelineType::Compute && extension.compare(s_STR_DEFAULT_PIPELINE_FILE_EXTENSION_COMPUTE) == 0)
                    {
                        validFile = true;
                    }

                    if (url.isLocalFile() && validFile)
                    {
                        // Accept the action, making it so we receive a dropEvent when the items are released.
                        pEvent->setDropAction(Qt::DropAction::CopyAction);
                        pEvent->accept();

                        // Change the item's background color.
                        SetCurrent(true);
                    }
                    else
                    {
                        pEvent->ignore();
                    }
                }
                else
                {
                    pEvent->ignore();
                }
            }
            else
            {
                pEvent->ignore();
            }
        }
    }
}

void rgMenuPipelineStateItem::dropEvent(QDropEvent* pEvent)
{
    assert(pEvent != nullptr);
    if (pEvent != nullptr)
    {
        const QMimeData* pMimeData = pEvent->mimeData();
        assert(pMimeData != nullptr);
        if (pMimeData != nullptr)
        {
            // Make sure the drop data has a file.
            if (pMimeData->hasUrls())
            {
                // Check to make sure the file is valid.
                QUrl url = pMimeData->urls().at(0);
                if (url.isLocalFile())
                {
                    // Get the file path.
                    std::string filePath = url.toLocalFile().toStdString();

                    // Click the button.
                    HandlePipelineStateButtonClicked(false);

                    // Emit a signal to open an existing PSO file.
                    emit DragAndDropExistingFile(filePath);
                }
            }
            else
            {
                pEvent->ignore();
            }
        }
    }
}

void rgMenuPipelineStateItem::dragLeaveEvent(QDragLeaveEvent* pEvent)
{
    // Change the item's background color.
    SetCurrent(false);
}