// C++.
#include <sstream>
#include <cassert>

// Qt.
#include <QKeyEvent>
#include <QPushButton>
#include <QMimeData>
#include <QMenu>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_menu_file_item_graphics.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_graphics.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_utils_graphics.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

static const char* kStrSubButtonNoFocusStylesheet = "rgMenuFileItemGraphics #addFilePushButton:hover\
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
static const char* kStrSubButtonInFocusStylesheet = "border: 2px inset rgb(135, 20, 16);";
static const char* kStrFileMenuItemNameGraphics = "fileMenuItemGraphics";

RgMenuFileItemGraphics::RgMenuFileItemGraphics(RgMenu* parent, RgPipelineStage stage)
    : RgMenuFileItem("", parent)
    , stage_(stage)
{
    ui_.setupUi(this);

    // Disable the renaming controls upon creation.
    ShowRenameControls(false);

    // Set the mouse cursor to pointing hand cursor.
    SetCursor(Qt::PointingHandCursor);

    // Initialize the graphics-specific part of context menu.
    InitializeContextMenuGraphics();

    // Connect the file menu signals.
    ConnectSignals();

    // Show the add/create buttons by default since the stage is empty.
    SetButtonsMode(StageButtonsMode::kAddCreate);

    // Initialize all strings within the widget.
    SetStringConstants();

    // Enable drops so the graphics file item can handle opening dropped files.
    setAcceptDrops(true);

    // Set the cursor for this file item to arrow cursor.
    setCursor(Qt::ArrowCursor);

    // Set object name.
    setObjectName(kStrFileMenuItemNameGraphics);
}

void RgMenuFileItemGraphics::SetCursor(const QCursor& cursor)
{
    // Set the mouse cursor to the specified type.
    ui_.addFilePushButton->setCursor(cursor);
    ui_.removeFilePushButton->setCursor(cursor);
}

// Set the shader file for the stage.
void RgMenuFileItemGraphics::SetShaderFile(const std::string& filename, RgVulkanInputType file_type)
{
    UpdateFilepath(filename);
    file_type_ = file_type;

    StageButtonsMode mode = !full_file_path_.empty() ? StageButtonsMode::kRemove : StageButtonsMode::kAddCreate;

    // Display the close button to remove the shader from the stage.
    SetButtonsMode(mode);
}

void RgMenuFileItemGraphics::SetButtonsEnabled(bool is_enabled)
{
    // Set the enabled state of the add/create buttons, and the close button.
    ui_.addFilePushButton->setEnabled(is_enabled);
    ui_.removeFilePushButton->setEnabled(is_enabled);
}

void RgMenuFileItemGraphics::SetButtonsMode(StageButtonsMode button_mode)
{
    bool is_add_create_visible = (button_mode == StageButtonsMode::kAddCreate);

    // Set the visibility of the add button.
    ui_.addFilePushButton->setVisible(is_add_create_visible);

    // The close button is visible when a source file has been attached to the stage.
    ui_.removeFilePushButton->setVisible(!is_add_create_visible);
}

void RgMenuFileItemGraphics::HandleAddExistingFileButtonClicked()
{
    // Emit a signal to add an existing file, and provide the target stage.
    emit AddExistingFileButtonClicked(stage_);
}

void RgMenuFileItemGraphics::HandleCreateSourceFileButtonClicked()
{
    // Emit a signal to add an existing file, and provide the target stage.
    emit CreateSourceFileButtonClicked(stage_);
}

void RgMenuFileItemGraphics::HandleRemoveSourceFileButtonClicked()
{
    // Emit a signal to remove the source file from the stage.
    emit RemoveSourceFileButtonClicked(stage_);
}

void RgMenuFileItemGraphics::HandleRestoreOriginalSpvClicked()
{
    emit RestoreOriginalSpvButtonClicked(stage_);
}

void RgMenuFileItemGraphics::InitializeContextMenuGraphics()
{
    // Create the Restore Original SPIR-V Binary action.
    // (Do not add it to the menu here because it's not always present).
    context_menu_actions_graphics_.restore_spv = new QAction(kStrFileContextMenuRestoreSpv, this);
}

void RgMenuFileItemGraphics::ConnectSignals()
{
    // Connect the "Add existing file" push button clicked signal.
    bool is_connected = connect(ui_.addFilePushButton, &QPushButton::clicked, this, &RgMenuFileItemGraphics::HandleAddExistingFileButtonClicked);
    assert(is_connected);

    // Connect the "Remove file" push button clicked signal.
    is_connected = connect(ui_.removeFilePushButton, &QPushButton::clicked, this, &RgMenuFileItemGraphics::HandleRemoveSourceFileButtonClicked);
    assert(is_connected);

    // Connect the item's "Remove" context menu item.
    is_connected = connect(context_menu_actions_.remove_file, &QAction::triggered, this, &RgMenuFileItemGraphics::HandleRemoveSourceFileButtonClicked);
    assert(is_connected);

    // Connect the handler responsible for restoring backup (original) spir-v binary file.
    is_connected = connect(context_menu_actions_graphics_.restore_spv, &QAction::triggered, this, &RgMenuFileItemGraphics::HandleRestoreOriginalSpvClicked);
    assert(is_connected);

    // Connect the filename QLineEdit signals, so the user can confirm a rename by pressing Return.
    is_connected = connect(ui_.filenameLineEdit, &QLineEdit::returnPressed, this, &RgMenuFileItem::HandleEnterPressed);
    assert(is_connected);
}

void RgMenuFileItemGraphics::AddContextMenuActionRestoreSpv()
{
    assert(context_menu_ != nullptr);
    if (context_menu_ != nullptr)
    {
        context_menu_actions_graphics_.separator = context_menu_->addSeparator();
        context_menu_->addAction(context_menu_actions_graphics_.restore_spv);
    }
}

void RgMenuFileItemGraphics::RemoveContextMenuActionRestoreSpv()
{
    assert(context_menu_ != nullptr);
    if (context_menu_ != nullptr)
    {
        context_menu_->removeAction(context_menu_actions_graphics_.separator);
        context_menu_->removeAction(context_menu_actions_graphics_.restore_spv);
    }
}

void RgMenuFileItemGraphics::SetStringConstants()
{
    // Create a utility class instance based on the current API. It will be used to display API-specific shader stage names.
    RgProjectAPI current_api = RgConfigManager::Instance().GetCurrentAPI();
    std::shared_ptr<RgUtilsGraphics> graphics_util = RgUtilsGraphics::CreateUtility(current_api);

    assert(graphics_util != nullptr);
    if (graphics_util != nullptr)
    {
        // Update the stage item's label. The label displays the stage type, and the source filename
        // for the stage, if one exists.
        UpdateFilenameLabelText();

        // Build the tooltip text for the Add button.
        std::stringstream add_button_tooltip_text;
        add_button_tooltip_text << kStrGraphicsMenuShaderStageAddButtonTooltipA;
        add_button_tooltip_text << graphics_util->PipelineStageToString(stage_);
        add_button_tooltip_text << kStrGraphicsMenuShaderStageAddButtonTooltipB;

        // Set the status bar tip for the add button.
        ui_.addFilePushButton->setStatusTip(add_button_tooltip_text.str().c_str());

        // Set the tool tip for the add button.
        ui_.addFilePushButton->setToolTip(add_button_tooltip_text.str().c_str());

        // Build the tooltip text for the Create button.
        std::stringstream create_button_tooltip_text;
        create_button_tooltip_text << kStrGraphicsMenuShaderStageCreateButtonTooltipA;
        create_button_tooltip_text << graphics_util->PipelineStageToString(stage_);
        create_button_tooltip_text << kStrGraphicsMenuShaderStageCreateButtonTooltipB;

        // Build the tooltip text for the Delete button.
        std::stringstream remove_button_tooltip_text;
        remove_button_tooltip_text << kStrGraphicsMenuShaderStageCloseButtonTooltip;

        // Set the status bar tip for the Delete button.
        ui_.removeFilePushButton->setStatusTip(remove_button_tooltip_text.str().c_str());

        // Set the tool tip for the Delete button.
        ui_.removeFilePushButton->setToolTip(remove_button_tooltip_text.str().c_str());
    }
}

void RgMenuFileItemGraphics::UpdateFilenameLabelText()
{
    // Create a utility class instance based on the current API. It will be used to display API-specific shader stage names.
    RgProjectAPI current_api = RgConfigManager::Instance().GetCurrentAPI();
    std::shared_ptr<RgUtilsGraphics> graphics_util = RgUtilsGraphics::CreateUtility(current_api);

    assert(graphics_util != nullptr);
    if (graphics_util != nullptr)
    {
        std::string stage_name_abbreviation = graphics_util->PipelineStageToAbbreviation(stage_);
        if (full_file_path_.empty())
        {
            // Set the name of the stage in the item's label.
            ui_.stageNameLabel->setText(stage_name_abbreviation.c_str());
        }
        else
        {
            std::string text = filename_;
            std::string display_text;

            // Determine suffix based on whether or not the file is saved.
            if (!is_saved_)
            {
                text += kStrUnsavedFileSuffix;
            }

            // Get available space and subtract 2 for the parentheses around the file name,
            // four for the stage name, and 1 for the space after the name.
            const int available_space = ui_.filenameDisplayLayout->contentsRect().width();
            RgUtils::GetDisplayText(text, display_text, available_space - 7, ui_.stageNameLabel, kTextTruncateLengthBackVulkan);

            // Construct the filename label for the stage.
            std::stringstream stage_source_text;
            stage_source_text << stage_name_abbreviation;
            stage_source_text << " (";
            stage_source_text << display_text;
            stage_source_text << ")";

            // Set the name of the stage in the item's label, and include the source file name.
            ui_.stageNameLabel->setText(stage_source_text.str().c_str());

            // Set the full path as a tooltip.
            this->setToolTip(full_file_path_.c_str());
        }
    }
}

QLineEdit* RgMenuFileItemGraphics::GetRenameLineEdit()
{
    return ui_.filenameLineEdit;
}

QLabel* RgMenuFileItemGraphics::GetItemLabel()
{
    return ui_.stageNameLabel;
}

void RgMenuFileItemGraphics::enterEvent(QEnterEvent* event)
{
    Q_UNUSED(event);

    if (!full_file_path_.empty())
    {
        ui_.removeFilePushButton->show();

        // Change the item color.
        SetHovered(true);
    }
}

void RgMenuFileItemGraphics::leaveEvent(QEvent* event)
{
    Q_UNUSED(event);

    if (!full_file_path_.empty())
    {
        ui_.removeFilePushButton->hide();

        // Change the item color.
        SetHovered(false);
    }
}

void RgMenuFileItemGraphics::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_UNUSED(event);

    // Don't allow renaming a pipeline stage item that's empty.
    if (!full_file_path_.empty())
    {
        // On double-click, allow the user to re-name the item's filename.
        ShowRenameControls(true);
    }
}

void RgMenuFileItemGraphics::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event);

    // Don't process a pipeline stage click if the stage is empty.
    if (!full_file_path_.empty())
    {
        emit MenuItemSelected(this);

        // Emit a signal to enable pipeline state menu item in Build menu.
        emit EnablePipelineMenuItem(true);

        // Emit a signal to enable build settings menu item in Build menu.
        emit EnableBuildSettingsMenuItem(true);
    }
}

void RgMenuFileItemGraphics::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    UpdateFilenameLabelText();
}

void RgMenuFileItemGraphics::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    UpdateFilenameLabelText();
}

void RgMenuFileItemGraphics::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {
        is_escape_pressed_ = true;

        // Hide the rename box, and display the labels
        ShowRenameControls(false);
    }
    else
    {
        // Pass the event onto the base class
        QWidget::keyPressEvent(event);
    }
}

void RgMenuFileItemGraphics::SetHovered(bool is_hovered)
{
    // Set "hovered" property to be utilized by this widget's stylesheet.
    ui_.itemBackground->setProperty(kStrFileMenuPropertyHovered, is_hovered);

    // Repolish the widget to ensure the style gets updated.
    ui_.itemBackground->style()->unpolish(ui_.itemBackground);
    ui_.itemBackground->style()->polish(ui_.itemBackground);
}

void RgMenuFileItemGraphics::SetCurrent(bool is_current)
{
    // Set "current" property to be utilized by this widget's stylesheet.
    ui_.itemBackground->setProperty(kStrFileMenuPropertyCurrent, is_current);

    // Repolish the widget to ensure the style gets updated.
    ui_.itemBackground->style()->unpolish(ui_.itemBackground);
    ui_.itemBackground->style()->polish(ui_.itemBackground);
}

void RgMenuFileItemGraphics::SetStageIsOccupied(bool is_occupied)
{
    // Set "occupied" property to be utilized by this widget's stylesheet.
    ui_.itemBackground->setProperty(kStrFileMenuPropertyOccupied, is_occupied);

    // Re-polish the widget to ensure the style gets updated.
    ui_.itemBackground->style()->unpolish(ui_.itemBackground);
    ui_.itemBackground->style()->polish(ui_.itemBackground);
}

void RgMenuFileItemGraphics::dragEnterEvent(QDragEnterEvent* event)
{
    // Change the item's background color.
    SetHovered(true);
    SetCurrent(true);

    assert(event != nullptr);
    if (event != nullptr)
    {
        const QMimeData* mime_data = event->mimeData();
        assert(mime_data != nullptr);
        if (mime_data != nullptr)
        {
            const int num_files = mime_data->urls().size();

            // Make sure the drop data has only one file url.
            if (mime_data->hasUrls() && (num_files == 1))
            {
                // Check to make sure the file is valid.
                QUrl url = mime_data->urls().at(0);

                // Do not allow the user to use PSO files.
                bool valid_file = true;
                QString extension;
                const QString file_path = url.toLocalFile();
                QStringList name_extension = file_path.split(kStrFileExtensionDelimiter);
                assert(name_extension.size() == 2);
                if (name_extension.size() == 2)
                {
                    extension = file_path.split(".").at(1);
                    if (extension.compare(kStrDefaultPipelineFileExtensionNameGraphics) == 0)
                    {
                        valid_file = false;
                    }
                    else if (extension.compare(kStrDefaultPipelineFileExtensionNameCompute) == 0)
                    {
                        valid_file = false;
                    }
                }

                if (url.isLocalFile() && valid_file)
                {
                    // Accept the action, making it so we receive a dropEvent when the items are released.
                    event->setDropAction(Qt::DropAction::CopyAction);
                    event->accept();
                }
                else
                {
                    event->ignore();

                    // Change the item's background color.
                    SetHovered(false);
                    SetCurrent(false);
                }
            }
            else
            {
                event->ignore();

                // Change the item's background color.
                SetHovered(false);
                SetCurrent(false);
            }
        }
    }
}

void RgMenuFileItemGraphics::dragLeaveEvent(QDragLeaveEvent* event)
{
    Q_UNUSED(event);

    // Change the item's background color.
    SetHovered(false);
    SetCurrent(false);
}

void RgMenuFileItemGraphics::dragMoveEvent(QDragMoveEvent *event)
{
    // If there is already a file present in this stage,
    // do not allow the user to drop here.
    if (full_file_path_.empty())
    {
        event->setDropAction(Qt::CopyAction);
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void RgMenuFileItemGraphics::dropEvent(QDropEvent* event)
{
    const QMimeData* mime_data = event->mimeData();

    // Make sure the drop data has a file.
    if (mime_data->hasUrls())
    {
        // Check to make sure the file is valid.
        QUrl url = mime_data->urls().at(0);
        if (url.isLocalFile())
        {
            // Get the file path.
            std::string file_path = url.toLocalFile().toStdString();

            // Emit a signal to add an existing file, and provide the target stage.
            emit DragAndDropExistingFile(stage_, file_path);
        }
    }
    else
    {
        event->ignore();
    }
}

void RgMenuFileItemGraphics::SetButtonHighlighted(const GraphicsMenuTabFocusItems button)
{
    switch (button)
    {
    case GraphicsMenuTabFocusItems::kRemoveButton:
        ui_.removeFilePushButton->show();
        ui_.removeFilePushButton->setStyleSheet(kStrSubButtonInFocusStylesheet);
        break;
    case GraphicsMenuTabFocusItems::kAddExistingFileButton:
        ui_.addFilePushButton->setStyleSheet(kStrSubButtonInFocusStylesheet);
        break;
    default:
        assert(false);
        break;
    }
}

void RgMenuFileItemGraphics::RemoveSubButtonFocus()
{
    ui_.removeFilePushButton->setStyleSheet(kStrSubButtonNoFocusStylesheet);
    ui_.addFilePushButton->setStyleSheet(kStrSubButtonNoFocusStylesheet);
}

void RgMenuFileItemGraphics::HideRemoveFilePushButton()
{
    ui_.removeFilePushButton->hide();
}

void RgMenuFileItemGraphics::ProcessRemoveButtonClick()
{
    ui_.removeFilePushButton->click();
}

void RgMenuFileItemGraphics::ProcessAddExistingFileButtonClick()
{
    ui_.addFilePushButton->click();
}

RgPipelineStage RgMenuFileItemGraphics::GetStage() const
{
    return stage_;
}

RgVulkanInputType RgMenuFileItemGraphics::GetFileType() const
{
    return file_type_;
}
