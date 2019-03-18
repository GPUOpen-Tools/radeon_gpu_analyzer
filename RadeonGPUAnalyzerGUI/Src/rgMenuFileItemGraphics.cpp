// C++.
#include <sstream>
#include <cassert>

// Qt.
#include <QKeyEvent>
#include <QPushButton>
#include <QMimeData>
#include <QMenu>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuFileItemGraphics.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuGraphics.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtilsGraphics.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

static const char* s_STR_SUB_BUTTON_NO_FOCUS_STYLESHEET = "rgMenuFileItemGraphics #addFilePushButton:hover\
{ \
border-style: solid;\
border-width: 2px;\
border-color: rgb(135,20,16)\
}\
rgMenuFileItemGraphics #removeButton:hover\
{\
border-style: solid;\
border-width: 2px;\
border-color: rgb(135,20,16)}";
static const char* s_STR_SUB_BUTTON_IN_FOCUS_STYLESHEET = "border: 2px inset rgb(135, 20, 16);";
static const char* s_STR_FILE_MENU_ITEM_NAME_GRAPHICS = "fileMenuItemGraphics";

rgMenuFileItemGraphics::rgMenuFileItemGraphics(rgMenu* pParent, rgPipelineStage stage)
    : rgMenuFileItem("", pParent)
    , m_stage(stage)
{
    ui.setupUi(this);

    // Disable the renaming controls upon creation.
    ShowRenameControls(false);

    // Set the mouse cursor to pointing hand cursor.
    SetCursor(Qt::PointingHandCursor);

    // Initialize the graphics-specific part of context menu.
    InitializeContextMenuGraphics();

    // Connect the file menu signals.
    ConnectSignals();

    // Show the add/create buttons by default since the stage is empty.
    SetButtonsMode(StageButtonsMode::AddCreate);

    // Initialize all strings within the widget.
    SetStringConstants();

    // Enable drops so the graphics file item can handle opening dropped files.
    setAcceptDrops(true);

    // Set the cursor for this file item to arrow cursor.
    setCursor(Qt::ArrowCursor);

    // Set object name.
    setObjectName(s_STR_FILE_MENU_ITEM_NAME_GRAPHICS);
}

void rgMenuFileItemGraphics::SetCursor(const QCursor& cursor)
{
    // Set the mouse cursor to the specified type.
    ui.addFilePushButton->setCursor(cursor);
    ui.removeFilePushButton->setCursor(cursor);
}

// Set the shader file for the stage.
void rgMenuFileItemGraphics::SetShaderFile(const std::string& filename, rgVulkanInputType fileType)
{
    UpdateFilepath(filename);
    m_fileType = fileType;

    StageButtonsMode mode = !m_fullFilepath.empty() ? StageButtonsMode::Remove : StageButtonsMode::AddCreate;

    // Display the close button to remove the shader from the stage.
    SetButtonsMode(mode);
}

void rgMenuFileItemGraphics::SetButtonsEnabled(bool isEnabled)
{
    // Set the enabled state of the add/create buttons, and the close button.
    ui.addFilePushButton->setEnabled(isEnabled);
    ui.removeFilePushButton->setEnabled(isEnabled);
}

void rgMenuFileItemGraphics::SetButtonsMode(StageButtonsMode buttonMode)
{
    bool isAddCreateVisible = (buttonMode == StageButtonsMode::AddCreate);

    // Set the visibility of the add button.
    ui.addFilePushButton->setVisible(isAddCreateVisible);

    // The close button is visible when a source file has been attached to the stage.
    ui.removeFilePushButton->setVisible(!isAddCreateVisible);
}

void rgMenuFileItemGraphics::HandleAddExistingFileButtonClicked()
{
    // Emit a signal to add an existing file, and provide the target stage.
    emit AddExistingFileButtonClicked(m_stage);
}

void rgMenuFileItemGraphics::HandleCreateSourceFileButtonClicked()
{
    // Emit a signal to add an existing file, and provide the target stage.
    emit CreateSourceFileButtonClicked(m_stage);
}

void rgMenuFileItemGraphics::HandleRemoveSourceFileButtonClicked()
{
    // Emit a signal to remove the source file from the stage.
    emit RemoveSourceFileButtonClicked(m_stage);
}

void rgMenuFileItemGraphics::HandleRestoreOriginalSpvClicked()
{
    emit RestoreOriginalSpvButtonClicked(m_stage);
}

void rgMenuFileItemGraphics::InitializeContextMenuGraphics()
{
    // Create the Restore Original SPIR-V Binary action.
    // (Do not add it to the menu here because it's not always present).
    m_contextMenuActionsGraphics.pRestoreSpv = new QAction(STR_FILE_CONTEXT_MENU_RESTORE_SPV, this);
}

void rgMenuFileItemGraphics::ConnectSignals()
{
    // Connect the "Add existing file" push button clicked signal.
    bool isConnected = connect(ui.addFilePushButton, &QPushButton::clicked, this, &rgMenuFileItemGraphics::HandleAddExistingFileButtonClicked);
    assert(isConnected);

    // Connect the "Remove file" push button clicked signal.
    isConnected = connect(ui.removeFilePushButton, &QPushButton::clicked, this, &rgMenuFileItemGraphics::HandleRemoveSourceFileButtonClicked);
    assert(isConnected);

    // Connect the item's "Remove" context menu item.
    isConnected = connect(m_contextMenuActions.pRemoveFile, &QAction::triggered, this, &rgMenuFileItemGraphics::HandleRemoveSourceFileButtonClicked);
    assert(isConnected);

    // Connect the handler responsible for restoring backup (original) spir-v binary file.
    isConnected = connect(m_contextMenuActionsGraphics.pRestoreSpv, &QAction::triggered, this, &rgMenuFileItemGraphics::HandleRestoreOriginalSpvClicked);
    assert(isConnected);

    // Connect the filename QLineEdit signals, so the user can confirm a rename by pressing Return.
    isConnected = connect(ui.filenameLineEdit, &QLineEdit::returnPressed, this, &rgMenuFileItem::HandleEnterPressed);
    assert(isConnected);
}

void rgMenuFileItemGraphics::AddContextMenuActionRestoreSpv()
{
    assert(m_pContextMenu != nullptr);
    if (m_pContextMenu != nullptr)
    {
        m_contextMenuActionsGraphics.pSeparator = m_pContextMenu->addSeparator();
        m_pContextMenu->addAction(m_contextMenuActionsGraphics.pRestoreSpv);
    }
}

void rgMenuFileItemGraphics::RemoveContextMenuActionRestoreSpv()
{
    assert(m_pContextMenu != nullptr);
    if (m_pContextMenu != nullptr)
    {
        m_pContextMenu->removeAction(m_contextMenuActionsGraphics.pSeparator);
        m_pContextMenu->removeAction(m_contextMenuActionsGraphics.pRestoreSpv);
    }
}

void rgMenuFileItemGraphics::SetStringConstants()
{
    // Create a utility class instance based on the current API. It will be used to display API-specific shader stage names.
    rgProjectAPI currentApi = rgConfigManager::Instance().GetCurrentAPI();
    std::shared_ptr<rgUtilsGraphics> pGraphicsUtil = rgUtilsGraphics::CreateUtility(currentApi);

    assert(pGraphicsUtil != nullptr);
    if (pGraphicsUtil != nullptr)
    {
        // Update the stage item's label. The label displays the stage type, and the source filename
        // for the stage, if one exists.
        UpdateFilenameLabelText();

        // Build the tooltip text for the Add button.
        std::stringstream addButtonTooltipText;
        addButtonTooltipText << STR_GRAPHICS_MENU_SHADER_STAGE_ADD_BUTTON_TOOLTIP_A;
        addButtonTooltipText << pGraphicsUtil->PipelineStageToString(m_stage);
        addButtonTooltipText << STR_GRAPHICS_MENU_SHADER_STAGE_ADD_BUTTON_TOOLTIP_B;

        // Set the status bar tip for the add button.
        ui.addFilePushButton->setStatusTip(addButtonTooltipText.str().c_str());

        // Set the tool tip for the add button.
        ui.addFilePushButton->setToolTip(addButtonTooltipText.str().c_str());

        // Build the tooltip text for the Create button.
        std::stringstream createButtonTooltipText;
        createButtonTooltipText << STR_GRAPHICS_MENU_SHADER_STAGE_CREATE_BUTTON_TOOLTIP_A;
        createButtonTooltipText << pGraphicsUtil->PipelineStageToString(m_stage);
        createButtonTooltipText << STR_GRAPHICS_MENU_SHADER_STAGE_CREATE_BUTTON_TOOLTIP_B;

        // Build the tooltip text for the Delete button.
        std::stringstream removeButtonTooltipText;
        removeButtonTooltipText << STR_GRAPHICS_MENU_SHADER_STAGE_CLOSE_BUTTON_TOOLTIP;

        // Set the status bar tip for the Delete button.
        ui.removeFilePushButton->setStatusTip(removeButtonTooltipText.str().c_str());

        // Set the tool tip for the Delete button.
        ui.removeFilePushButton->setToolTip(removeButtonTooltipText.str().c_str());
    }
}

void rgMenuFileItemGraphics::UpdateFilenameLabelText()
{
    // Create a utility class instance based on the current API. It will be used to display API-specific shader stage names.
    rgProjectAPI currentApi = rgConfigManager::Instance().GetCurrentAPI();
    std::shared_ptr<rgUtilsGraphics> pGraphicsUtil = rgUtilsGraphics::CreateUtility(currentApi);

    assert(pGraphicsUtil != nullptr);
    if (pGraphicsUtil != nullptr)
    {
        std::string stageNameAbbreviation = pGraphicsUtil->PipelineStageToAbbreviation(m_stage);
        if (m_fullFilepath.empty())
        {
            // Set the name of the stage in the item's label.
            ui.stageNameLabel->setText(stageNameAbbreviation.c_str());
        }
        else
        {
            std::string text = m_filename;
            std::string displayText;

            // Determine suffix based on whether or not the file is saved.
            if (!m_isSaved)
            {
                text += STR_UNSAVED_FILE_SUFFIX;
            }

            // Get available space and subtract 2 for the parentheses around the file name,
            // four for the stage name, and 1 for the space after the name.
            const int availableSpace = ui.filenameDisplayLayout->contentsRect().width();
            rgUtils::GetDisplayText(text, displayText, availableSpace - 7, ui.stageNameLabel, gs_TEXT_TRUNCATE_LENGTH_BACK_VULKAN);

            // Construct the filename label for the stage.
            std::stringstream stageSourceText;
            stageSourceText << stageNameAbbreviation;
            stageSourceText << " (";
            stageSourceText << displayText;
            stageSourceText << ")";

            // Set the name of the stage in the item's label, and include the source file name.
            ui.stageNameLabel->setText(stageSourceText.str().c_str());

            // Set the full path as a tooltip.
            this->setToolTip(m_fullFilepath.c_str());
        }
    }
}

QLineEdit* rgMenuFileItemGraphics::GetRenameLineEdit()
{
    return ui.filenameLineEdit;
}

QLabel* rgMenuFileItemGraphics::GetItemLabel()
{
    return ui.stageNameLabel;
}

void rgMenuFileItemGraphics::enterEvent(QEvent* pEvent)
{
    if (!m_fullFilepath.empty())
    {
        ui.removeFilePushButton->show();

        // Change the item color.
        SetHovered(true);
    }
}

void rgMenuFileItemGraphics::leaveEvent(QEvent* pEvent)
{
    if (!m_fullFilepath.empty())
    {
        ui.removeFilePushButton->hide();

        // Change the item color.
        SetHovered(false);
    }
}

void rgMenuFileItemGraphics::mouseDoubleClickEvent(QMouseEvent* pEvent)
{
    // Don't allow renaming a pipeline stage item that's empty.
    if (!m_fullFilepath.empty())
    {
        // On double-click, allow the user to re-name the item's filename.
        ShowRenameControls(true);
    }
}

void rgMenuFileItemGraphics::mousePressEvent(QMouseEvent* pEvent)
{
    // Don't process a pipeline stage click if the stage is empty.
    if (!m_fullFilepath.empty())
    {
        emit MenuItemSelected(this);
    }
}

void rgMenuFileItemGraphics::resizeEvent(QResizeEvent *pEvent)
{
    UpdateFilenameLabelText();
}

void rgMenuFileItemGraphics::showEvent(QShowEvent *pEvent)
{
    UpdateFilenameLabelText();
}

void rgMenuFileItemGraphics::keyPressEvent(QKeyEvent* pEvent)
{
    if (pEvent->key() == Qt::Key_Escape)
    {
        m_isEscapePressed = true;

        // Hide the rename box, and display the labels
        ShowRenameControls(false);
    }
    else
    {
        // Pass the event onto the base class
        QWidget::keyPressEvent(pEvent);
    }
}

void rgMenuFileItemGraphics::SetHovered(bool isHovered)
{
    // Set "hovered" property to be utilized by this widget's stylesheet.
    ui.itemBackground->setProperty(STR_FILE_MENU_PROPERTY_HOVERED, isHovered);

    // Repolish the widget to ensure the style gets updated.
    ui.itemBackground->style()->unpolish(ui.itemBackground);
    ui.itemBackground->style()->polish(ui.itemBackground);
}

void rgMenuFileItemGraphics::SetCurrent(bool isCurrent)
{
    // Set "current" property to be utilized by this widget's stylesheet.
    ui.itemBackground->setProperty(STR_FILE_MENU_PROPERTY_CURRENT, isCurrent);

    // Repolish the widget to ensure the style gets updated.
    ui.itemBackground->style()->unpolish(ui.itemBackground);
    ui.itemBackground->style()->polish(ui.itemBackground);
}

void rgMenuFileItemGraphics::SetStageIsOccupied(bool isOccupied)
{
    // Set "occupied" property to be utilized by this widget's stylesheet.
    ui.itemBackground->setProperty(STR_FILE_MENU_PROPERTY_OCCUPIED, isOccupied);

    // Re-polish the widget to ensure the style gets updated.
    ui.itemBackground->style()->unpolish(ui.itemBackground);
    ui.itemBackground->style()->polish(ui.itemBackground);
}

void rgMenuFileItemGraphics::dragEnterEvent(QDragEnterEvent* pEvent)
{
    // Change the item's background color.
    SetHovered(true);
    SetCurrent(true);

    assert(pEvent != nullptr);
    if (pEvent != nullptr)
    {
        const QMimeData* pMimeData = pEvent->mimeData();
        assert(pMimeData != nullptr);
        if (pMimeData != nullptr)
        {
            const int numFiles = pMimeData->urls().size();

            // Make sure the drop data has only one file url.
            if (pMimeData->hasUrls() && (numFiles == 1))
            {
                // Check to make sure the file is valid.
                QUrl url = pMimeData->urls().at(0);

                // Do not allow the user to use PSO files.
                bool validFile = true;
                QString extension;
                const QString filePath = url.toLocalFile();
                QStringList nameExtension = filePath.split(STR_FILE_EXTENSION_DELIMITER);
                assert(nameExtension.size() == 2);
                if (nameExtension.size() == 2)
                {
                    extension = filePath.split(".").at(1);
                    if (extension.compare(STR_DEFAULT_PIPELINE_FILE_EXTENSION_NAME_GRAPHICS) == 0)
                    {
                        validFile = false;
                    }
                    else if (extension.compare(STR_DEFAULT_PIPELINE_FILE_EXTENSION_NAME_COMPUTE) == 0)
                    {
                        validFile = false;
                    }
                }

                if (url.isLocalFile() && validFile)
                {
                    // @TODO: Validate the file for correct type.
                    // Accept the action, making it so we receive a dropEvent when the items are released.
                    pEvent->setDropAction(Qt::DropAction::CopyAction);
                    pEvent->accept();
                }
                else
                {
                    pEvent->ignore();

                    // Change the item's background color.
                    SetHovered(false);
                    SetCurrent(false);
                }
            }
            else
            {
                pEvent->ignore();

                // Change the item's background color.
                SetHovered(false);
                SetCurrent(false);
            }
        }
    }
}

void rgMenuFileItemGraphics::dragLeaveEvent(QDragLeaveEvent* pEvent)
{
    // Change the item's background color.
    SetHovered(false);
    SetCurrent(false);
}

void rgMenuFileItemGraphics::dragMoveEvent(QDragMoveEvent *event)
{
    // If there is already a file present in this stage,
    // do not allow the user to drop here.
    if (m_fullFilepath.empty())
    {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void rgMenuFileItemGraphics::dropEvent(QDropEvent* pEvent)
{
    const QMimeData* pMimeData = pEvent->mimeData();

    // Make sure the drop data has a file.
    if (pMimeData->hasUrls())
    {
        // Check to make sure the file is valid.
        QUrl url = pMimeData->urls().at(0);
        if (url.isLocalFile())
        {
            // Get the file path.
            std::string filePath = url.toLocalFile().toStdString();

            // Emit a signal to add an existing file, and provide the target stage.
            emit DragAndDropExistingFile(m_stage, filePath);
        }
    }
    else
    {
        pEvent->ignore();
    }
}

void rgMenuFileItemGraphics::SetButtonHighlighted(const GraphicsMenuTabFocusItems button)
{
    switch (button)
    {
    case GraphicsMenuTabFocusItems::RemoveButton:
        ui.removeFilePushButton->show();
        ui.removeFilePushButton->setStyleSheet(s_STR_SUB_BUTTON_IN_FOCUS_STYLESHEET);
        break;
    case GraphicsMenuTabFocusItems::AddExistingFileButton:
        ui.addFilePushButton->setStyleSheet(s_STR_SUB_BUTTON_IN_FOCUS_STYLESHEET);
        break;
    default:
        assert(false);
        break;
    }
}

void rgMenuFileItemGraphics::RemoveSubButtonFocus()
{
    ui.removeFilePushButton->setStyleSheet(s_STR_SUB_BUTTON_NO_FOCUS_STYLESHEET);
    ui.addFilePushButton->setStyleSheet(s_STR_SUB_BUTTON_NO_FOCUS_STYLESHEET);
}

void rgMenuFileItemGraphics::HideRemoveFilePushButton()
{
    ui.removeFilePushButton->hide();
}

void rgMenuFileItemGraphics::ProcessRemoveButtonClick()
{
    ui.removeFilePushButton->click();
}

void rgMenuFileItemGraphics::ProcessAddExistingFileButtonClick()
{
    ui.addFilePushButton->click();
}

rgPipelineStage rgMenuFileItemGraphics::GetStage() const
{
    return m_stage;
}

rgVulkanInputType rgMenuFileItemGraphics::GetFileType() const
{
    return m_fileType;
}
