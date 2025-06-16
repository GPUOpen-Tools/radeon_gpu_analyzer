//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for Titlebar widget in RGA Build view's File Menu.
//=============================================================================

// C++.
#include <sstream>
#include <cassert>

// Qt.
#include <QWidget>
#include <QStackedLayout>
#include <QLabel>
#include <QLineEdit>
#include <QKeyEvent>

// Local
#include "radeon_gpu_analyzer_gui/qt/rg_menu_titlebar.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"

// Fixed height of the project title item.
static const int kProjectTitleItemHeight = 32;

// File menu bar stylesheets.
static const char* kStrFilemenuTitleBarTooltipWidth = "min-width: %1px; width: %2px;";
static const char* kStrFilemenuTitleBarTooltipHeight = "min-height: %1px; height: %2px; max-height: %3px;";

RgMenuTitlebar::RgMenuTitlebar(QWidget* parent) :
    QWidget(parent)
{
    // Create basic layout to hold the background.
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    // Create the background widget.
    QWidget* background = new QWidget();
    background->setObjectName("itemBackground");
    layout->addWidget(background);

    // Create main layout.
    main_layout_ = new QVBoxLayout();
    main_layout_->setContentsMargins(6, 6, 6, 6);
    background->setLayout(main_layout_);

    // Create stacked layout.
    stacked_layout_ = new QStackedLayout();
    main_layout_->addLayout(stacked_layout_);

    // Create label and editor.
    title_label_ = new QLabel();
    title_text_edit_ = new QLineEdit();

    // Add widgets to layout.
    stacked_layout_->addWidget(title_label_);
    stacked_layout_->addWidget(title_text_edit_);

    // Connect the handler for when texting editing is finished.
    bool is_connected = connect(title_text_edit_, &QLineEdit::editingFinished, this, &RgMenuTitlebar::StopEditing);
    assert(is_connected);
    is_connected = connect(title_text_edit_, &QLineEdit::returnPressed, this, &RgMenuTitlebar::HandleReturnPressed);
    assert(is_connected);

    // Force size for this item.
    setMinimumSize(0, kProjectTitleItemHeight);
    setMaximumSize(QWIDGETSIZE_MAX, kProjectTitleItemHeight);

    // Set mouse cursor to pointing hand cursor.
    SetCursor();
}

RgMenuTitlebar::~RgMenuTitlebar()
{
    title_label_->deleteLater();
    title_text_edit_->deleteLater();
    stacked_layout_->deleteLater();
}

void RgMenuTitlebar::SetTitle(const std::string& title)
{
    // Set widget text.
    title_label_->setText(title.c_str());
    title_text_edit_->setText(title.c_str());

    // Refresh the tooltip.
    RefreshTooltip(title.c_str());
}

void RgMenuTitlebar::StartEditing()
{
    // Set the initial line edit text from the title label text.
    std::string label_text = title_label_->text().toStdString();
    label_text = label_text.substr(label_text.find("</b>") + 4);

    title_text_edit_->setText(label_text.c_str());

    // Show the line edit widget.
    stacked_layout_->setCurrentWidget(title_text_edit_);

    // Default focus on the line edit.
    title_text_edit_->setFocus();
    title_text_edit_->setSelection(0, static_cast<int>(title_label_->text().length()));
}

void RgMenuTitlebar::StopEditing()
{
    title_text_edit_->blockSignals(true);

    // Trim the leading and trailing whitespace characters from the project name.
    std::string updated_project_name = title_text_edit_->text().toStdString();
    RgUtils::TrimLeadingAndTrailingWhitespace(updated_project_name, updated_project_name);

    std::string error_message;
    if (RgUtils::IsValidProjectName(updated_project_name, error_message))
    {
        // Check if the text has changed during editing.
        std::string label_text = title_label_->text().toStdString();
        label_text = label_text.substr(label_text.find("</b>") + 4);

        if (label_text.compare(updated_project_name) != 0)
        {
            // Set the new title text from the edited line text.
            std::stringstream updated_title;
            updated_title << RgUtils::GetProjectTitlePrefix(RgProjectAPI::kOpenCL) << updated_project_name;
            title_label_->setText(updated_title.str().c_str());

            // Indicate the title text has been changed.
            emit TitleChanged(updated_project_name);

            // Update the tooltip.
            std::stringstream updated_tool_tip;
            updated_tool_tip << kStrFileMenuProjectTitleTooltipA << updated_project_name;

            // Refresh the tooltip.
            RefreshTooltip(updated_tool_tip.str());
        }

        // Show the title label widget.
        stacked_layout_->setCurrentWidget(title_label_);
    }
    else
    {
        // Notify the user that the project name is illegal.
        std::stringstream msg;
        msg << error_message << " \"";
        msg << updated_project_name << "\".";
        RgUtils::ShowErrorMessageBox(msg.str().c_str());

        // Keep focus on the line edit.
        title_text_edit_->setFocus();
        title_text_edit_->setSelection(0, static_cast<int>(title_label_->text().length()));
    }

    title_text_edit_->blockSignals(false);
}

void RgMenuTitlebar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event != nullptr && event->button() == Qt::LeftButton)
    {
        StartEditing();
    }
}

void RgMenuTitlebar::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {
        // Block signals from the text edit before
        // switching the index on the stacked widget.
        // Not blocking the signals here will cause
        // an editingFinished/StopEditing signal/slot
        // to execute when the text edit loses focus,
        // causing undesired behavior.
        const QSignalBlocker signal_blocker(title_text_edit_);

        // Switch the stacked widget to the title label.
        assert(stacked_layout_ != nullptr);
        if (stacked_layout_ != nullptr)
        {
            stacked_layout_->setCurrentWidget(title_label_);
        }
    }
    else
    {
        // Pass the event onto the base class
        QWidget::keyPressEvent(event);
    }
}

void RgMenuTitlebar::RefreshTooltip(const std::string& updated_tooltip)
{
    // Make full tooltip string.
    std::string tooltip = updated_tooltip + kStrFileMenuProjectTitleTooltipB;

    // Set tooltip.
    QString tooltip_formatted = "<p style='white-space:pre'>";
    tooltip_formatted += QString::fromStdString(tooltip);
    tooltip_formatted += "</p>";

    // Set tool and status tip.
    RgUtils::SetToolAndStatusTip(tooltip_formatted.toStdString(), this);

    // Set status tip since we do not want the formatted string for the status bar.
    RgUtils::SetStatusTip(tooltip, this);
}

void RgMenuTitlebar::HandleReturnPressed()
{
    title_text_edit_->blockSignals(true);

    // When return is pressed to end editing, redirect the focus.
    RgUtils::FocusOnFirstValidAncestor(this);

    title_text_edit_->blockSignals(false);
}

void RgMenuTitlebar::SetCursor()
{
    // Set mouse cursor to pointing hand cursor.
    setCursor(Qt::PointingHandCursor);
}
