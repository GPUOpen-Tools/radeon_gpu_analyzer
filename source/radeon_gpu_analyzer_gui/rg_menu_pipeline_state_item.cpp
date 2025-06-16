//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for Pipeline state item widget in RGA Build view's File Menu.
//=============================================================================

// C++.
#include <sstream>
#include <cassert>

// Qt.
#include <QMimeData>
#include <QDragEnterEvent>
#include <QPushButton>

// QtCommon.
#include "qt_common/utils/qt_util.h"

// Local.
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/qt/rg_menu_pipeline_state_item.h"

// Stylesheet for the when the menu buttons have focus.
static const char* kStrButtonFocusInStylesheetGraphics =
    "QPushButton { background: palette(highlight); border-style: solid; border-width: 2px; border-color: rgb(135, 20, 16);}";

// Stylesheet for the when the menu buttons do not have focus.
static const char* kStrButtonFocusOutStylesheet = "QPushButton { margin: 1px; }";

static const char* kStrDefaultPipelineFileExtensionGraphicsLocal = "gpso";
static const char* kStrDefaultPipelineFileExtensionComputeLocal  = "cpso";

RgMenuPipelineStateItem::RgMenuPipelineStateItem(RgPipelineType pipeline_type, RgMenu* parent)
    : pipeline_type_(pipeline_type)
    , RgMenuItem(parent)
{
    ui_.setupUi(this);

    // Set the status bar tip.
    this->setStatusTip(kStrGraphicsMenuPipelineStateTooltip);

    // Set the tool tip.
    this->setToolTip(kStrGraphicsMenuPipelineStateTooltip);

    ColorThemeType color_theme = QtCommon::QtUtils::ColorTheme::Get().GetColorTheme();

    if (color_theme == kColorThemeTypeDark)
    {
        ui_.pipelineStateButton->setIcon(QIcon(":/icons/state_settings_cube_dark_mode.svg"));
    }

    // Set the mouse cursor to arrow cursor.
    SetCursor(Qt::PointingHandCursor);

    // Connect the file menu signals.
    ConnectSignals();

    // Enable drag and drop.
    setAcceptDrops(true);
}

void RgMenuPipelineStateItem::ConnectSignals()
{
    // Connect the "Pipeline State" button to a handler to convert the parameter.
    [[maybe_unused]] bool is_connected =
        connect(ui_.pipelineStateButton, &QPushButton::clicked, this, &RgMenuPipelineStateItem::HandlePipelineStateButtonClicked);
    assert(is_connected);
}

void RgMenuPipelineStateItem::HandlePipelineStateButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    emit PipelineStateButtonClicked(this);

    // Disable the pipeline state menu item in Build menu.
    emit EnablePipelineMenuItem(false);

    // Enable the build settings menu item in Build menu.
    emit EnableBuildSettingsMenuItem(true);
}

void RgMenuPipelineStateItem::SetItemText(const std::string& item_text)
{
    ui_.pipelineStateButton->setText(item_text.c_str());
}

QPushButton* RgMenuPipelineStateItem::GetPipelineStateButton() const
{
    return ui_.pipelineStateButton;
}

void RgMenuPipelineStateItem::GotFocus()
{
    // Set arrow cursor so it doesn't appear that the user can click on the button again.
    ui_.pipelineStateButton->setCursor(Qt::ArrowCursor);

    // Set stylesheet.
    ui_.pipelineStateButton->setStyleSheet(kStrButtonFocusInStylesheetGraphics);
}

void RgMenuPipelineStateItem::LostFocus()
{
    // Set pointing hand cursor so it looks like the user can click on it.
    ui_.pipelineStateButton->setCursor(Qt::PointingHandCursor);

    // Set stylesheet.
    ui_.pipelineStateButton->setStyleSheet(kStrButtonFocusOutStylesheet);
}

void RgMenuPipelineStateItem::SetCursor(const QCursor& cursor)
{
    // Set the mouse cursor to the specified type.
    ui_.pipelineStateButton->setCursor(cursor);
}

void RgMenuPipelineStateItem::SetCurrent(bool is_current)
{
    current_ = is_current;

    if (current_)
    {
        GotFocus();
    }
    else
    {
        LostFocus();
    }
}

bool RgMenuPipelineStateItem::IsCurrent() const
{
    return current_;
}

void RgMenuPipelineStateItem::ClickMenuItem() const
{
    QPushButton* pipeline_state_button = GetPipelineStateButton();
    assert(pipeline_state_button != nullptr);
    if (pipeline_state_button != nullptr)
    {
        pipeline_state_button->click();
    }
}

void RgMenuPipelineStateItem::dragEnterEvent(QDragEnterEvent* event)
{
    assert(event != nullptr);
    if (event != nullptr)
    {
        const QMimeData* mime_data = event->mimeData();
        assert(mime_data != nullptr);
        if (mime_data != nullptr)
        {
            const int num_files = mime_data->urls().size();

            // Make sure the drop data has only one file url, and is a valid file.
            if (mime_data->hasUrls() && (num_files == 1))
            {
                // Check to make sure the file is valid.
                QUrl url = mime_data->urls().at(0);

                // Verify we have the correct file for the current pipeline type.
                bool          valid_file = false;
                QString       extension;
                const QString file_path     = url.toLocalFile();
                QStringList   num_extension = file_path.split(kStrFileExtensionDelimiter);
                assert(num_extension.size() == 2);
                if (num_extension.size() == 2)
                {
                    extension = file_path.split(".").at(1);
                    if (pipeline_type_ == RgPipelineType::kGraphics && extension.compare(kStrDefaultPipelineFileExtensionGraphicsLocal) == 0)
                    {
                        valid_file = true;
                    }
                    else if (pipeline_type_ == RgPipelineType::kCompute && extension.compare(kStrDefaultPipelineFileExtensionComputeLocal) == 0)
                    {
                        valid_file = true;
                    }

                    if (url.isLocalFile() && valid_file)
                    {
                        // Accept the action, making it so we receive a dropEvent when the items are released.
                        event->setDropAction(Qt::DropAction::CopyAction);
                        event->accept();

                        // Change the item's background color.
                        SetCurrent(true);
                    }
                    else
                    {
                        event->ignore();
                    }
                }
                else
                {
                    event->ignore();
                }
            }
            else
            {
                event->ignore();
            }
        }
    }
}

void RgMenuPipelineStateItem::dropEvent(QDropEvent* event)
{
    assert(event != nullptr);
    if (event != nullptr)
    {
        const QMimeData* mime_data = event->mimeData();
        assert(mime_data != nullptr);
        if (mime_data != nullptr)
        {
            // Make sure the drop data has a file.
            if (mime_data->hasUrls())
            {
                // Check to make sure the file is valid.
                QUrl url = mime_data->urls().at(0);
                if (url.isLocalFile())
                {
                    // Get the file path.
                    std::string file_path = url.toLocalFile().toStdString();

                    // Click the button.
                    HandlePipelineStateButtonClicked(false);

                    // Emit a signal to open an existing PSO file.
                    emit DragAndDropExistingFile(file_path);
                }
            }
            else
            {
                event->ignore();
            }
        }
    }
}

void RgMenuPipelineStateItem::dragLeaveEvent(QDragLeaveEvent* event)
{
    Q_UNUSED(event);

    // Change the item's background color.
    SetCurrent(false);
}
