//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for RGA start-up dialog.
//=============================================================================

// C++.
#include <assert.h>
#include <QKeyEvent>
#include <QStyleHints>

// QtCommon.
#include "qt_common/utils/qt_util.h"

// Local.
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_binary.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/qt/rg_startup_dialog.h"
#include "ui_rg_startup_dialog.h"

static const int kHorizontalLayoutSpacing = 15;
static const int kLeftAndRightMargin      = 23;
static const int kSpacingAfterLabel       = 10;

static const std::vector<const char*> kDescription = {kStrStartupDialogBinaryDescription,
                                                      kStrStartupDialogOpenclDescription,
                                                      kStrStartupDialogVulkanDescription};

int maxDescriptionWidth = 0;

RgStartupDialog::RgStartupDialog(QWidget* parent)
    : QDialog(parent)
    , selected_api_(static_cast<RgProjectAPI>(static_cast<int>(RgProjectAPI::kApiCount) - 1))
    , should_not_ask_again_(false)
{
    ui_.setupUi(this);

    // Disable the help button in the title bar, and disable resizing of this dialog.
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::MSWindowsFixedSizeDialogHint);

    // Shift the initial focus to the Do Not Ask Me Again check box.
    ui_.DoNotAskMeAgainCheckBox->setFocus();

    // Connect signals.
    ConnectSignals();

    // Populate the API list widget. (Starting with Vulkan, and going backwards).
    for (int api_index = static_cast<int>(RgProjectAPI::kApiCount) - 1; api_index > static_cast<int>(RgProjectAPI::kUnknown); --api_index)
    {
        RgProjectAPI current_api = static_cast<RgProjectAPI>(api_index);
        std::string  api_string;
        RgUtils::ProjectAPIToString(current_api, api_string, false);
        ui_.apiListWidget->addItem(QString::fromStdString(api_string));
    }

    // Select Vulkan selection in the list widget by default.
    ui_.apiListWidget->setCurrentRow(0);
    ui_.apiListWidget->setFocus();

    // Set the description for Vulkan selection.
    ui_.descriptionLabel->setText(kStrStartupDialogVulkanDescription);

    clearFocus();

    // Set the cursor to pointing hand cursor.
    SetCursor();

    // Set description field length.
    SetDescriptionLength();

    // Set button shortcuts.
    ui_.exitPushButton->setShortcut(Qt::AltModifier | Qt::Key_X);
    ui_.startRGAPushButton->setShortcut(Qt::AltModifier | Qt::Key_R);

    //Scale the dialog and the widget inside it.
    ScaleDialog();

    setStyleSheet(
        "QCheckBox::indicator { border: 1px solid palette(text); background-color: rgb(240, 240, 240); width: 12px; height: 12px; } "
        "QCheckBox::indicator:checked { image: url(:/icons/checkmark_black.svg);}");
}

void RgStartupDialog::ConnectSignals() const
{
    // Connect the handler invoked when the user clicks the Exit push button.
    bool is_connected = connect(ui_.exitPushButton, &QPushButton::clicked, this, &RgStartupDialog::HandleExitButtonClicked);
    assert(is_connected);

    // Connect the handler invoked when the user clicks the Start RGA push button.
    is_connected = connect(ui_.startRGAPushButton, &QPushButton::clicked, this, &RgStartupDialog::HandleStartRGAButtonClicked);
    assert(is_connected);

    // Connect the handler invoked when the user clicks an item in the API list widget.
    is_connected = connect(ui_.apiListWidget, &QListWidget::itemClicked, this, &RgStartupDialog::HandleListWidgetItemClicked);
    assert(is_connected);

    // Connect the handler invoked when the user double clicks an item in the API list widget.
    is_connected = connect(ui_.apiListWidget, &QListWidget::itemDoubleClicked, this, &RgStartupDialog::HandleListWidgetItemDoubleClicked);
    assert(is_connected);

    // Connect the handler invoked when the user selects the API by using up/down arrow keys.
    is_connected = connect(ui_.apiListWidget, &QListWidget::currentRowChanged, this, &RgStartupDialog::HandleListWidgetItemSelected);
    assert(is_connected);

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    is_connected = connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, &RgStartupDialog::HandleOsColorSchemeChanged);
    assert(is_connected);
#endif
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
void RgStartupDialog::HandleOsColorSchemeChanged(Qt::ColorScheme color_scheme)
{
    if (RgConfigManager::Instance().GetGlobalConfig()->color_theme != kColorThemeTypeCount)
    {
        return;
    }

    if (color_scheme == Qt::ColorScheme::Unknown)
    {
        return;
    }

    ColorThemeType color_mode = kColorThemeTypeLight;
    if (color_scheme == Qt::ColorScheme::Light)
    {
        color_mode = kColorThemeTypeLight;
    }
    else if (color_scheme == Qt::ColorScheme::Dark)
    {
        color_mode = kColorThemeTypeDark;
    }

    if (color_mode == QtCommon::QtUtils::ColorTheme::Get().GetColorTheme())
    {
        return;
    }

    QtCommon::QtUtils::ColorTheme::Get().SetColorTheme(color_mode);

    qApp->setPalette(QtCommon::QtUtils::ColorTheme::Get().GetCurrentPalette());

    emit QtCommon::QtUtils::ColorTheme::Get().ColorThemeUpdated();
}
#endif

void RgStartupDialog::SetCursor() const
{
    // Set the cursor to pointing hand cursor for various widgets.
    ui_.DoNotAskMeAgainCheckBox->setCursor(Qt::PointingHandCursor);
    ui_.exitPushButton->setCursor(Qt::PointingHandCursor);
    ui_.startRGAPushButton->setCursor(Qt::PointingHandCursor);
    ui_.apiListWidget->setCursor(Qt::PointingHandCursor);
}

RgProjectAPI RgStartupDialog::SelectedApi() const
{
    return selected_api_;
}

bool RgStartupDialog::ShouldNotAskAgain() const
{
    return should_not_ask_again_;
}

void RgStartupDialog::keyPressEvent(QKeyEvent* event)
{
    // Disable the escape key from closing the dialog
    if (event->key() != Qt::Key_Escape)
    {
        QDialog::keyPressEvent(event);
    }
}

void RgStartupDialog::HandleExitButtonClicked(bool /* checked */)
{
    reject();
}

void RgStartupDialog::HandleStartRGAButtonClicked(bool /* checked */)
{
    should_not_ask_again_ = ui_.DoNotAskMeAgainCheckBox->isChecked();

    accept();
}

void RgStartupDialog::HandleListWidgetItemClicked(QListWidgetItem* item)
{
    if (item->text().compare(kStrApiNameOpencl) == 0)
    {
        selected_api_ = RgProjectAPI::kOpenCL;

        // Also update the description.
        ui_.descriptionLabel->setText(kStrStartupDialogOpenclDescription);
    }
    else if (item->text().compare(kStrApiNameVulkan) == 0)
    {
        selected_api_ = RgProjectAPI::kVulkan;

        // Also update the description.
        ui_.descriptionLabel->setText(kStrStartupDialogVulkanDescription);
    }
    else if (item->text().compare(kStrApiNameBinary) == 0)
    {
        selected_api_ = RgProjectAPI::kBinary;

        // Also update the description.
        ui_.descriptionLabel->setText(kStrStartupDialogBinaryDescription);
    }
    else
    {
        // Should not get here.
        assert(false);
        selected_api_ = RgProjectAPI::kUnknown;
    }
}

void RgStartupDialog::HandleListWidgetItemDoubleClicked(QListWidgetItem* item)
{
    if (item->text().compare(kStrApiNameOpencl) == 0)
    {
        selected_api_ = RgProjectAPI::kOpenCL;

        // Also update the description.
        ui_.descriptionLabel->setText(kStrStartupDialogOpenclDescription);

        // Start RGA.
        HandleStartRGAButtonClicked(false);
    }
    else if (item->text().compare(kStrApiNameVulkan) == 0)
    {
        selected_api_ = RgProjectAPI::kVulkan;

        // Also update the description.
        ui_.descriptionLabel->setText(kStrStartupDialogVulkanDescription);

        // Start RGA.
        HandleStartRGAButtonClicked(false);
    }
    else if (item->text().compare(kStrApiNameBinary) == 0)
    {
        selected_api_ = RgProjectAPI::kBinary;

        // Also update the description.
        ui_.descriptionLabel->setText(kStrStartupDialogBinaryDescription);

        // Start RGA.
        HandleStartRGAButtonClicked(false);
    }
    else
    {
        // Should not get here.
        assert(false);
        selected_api_ = RgProjectAPI::kUnknown;
    }
}

void RgStartupDialog::HandleListWidgetItemSelected(int current_row)
{
    // Extract the QListWidgetItem that the user just selected and process it.
    HandleListWidgetItemClicked(ui_.apiListWidget->item(current_row));
}

void RgStartupDialog::SetDescriptionLength()
{
    for (const char* text : kDescription)
    {
        const QFont  font = ui_.descriptionLabel->font();
        QFontMetrics fm(font);
        const int    text_width = fm.horizontalAdvance(text);

        if (text_width > maxDescriptionWidth)
        {
            maxDescriptionWidth = text_width;
        }
    }
    ui_.descriptionLabel->setMinimumWidth(maxDescriptionWidth);
}

void RgStartupDialog::ScaleDialog()
{
    // Scale the dialog.
    QSize size;
    size.setWidth(width());
    size.setHeight(height());
    setFixedSize(size);

    int left_margin;
    int top_margin;
    int right_margin;
    int bottom_margin;
    this->layout()->getContentsMargins(&left_margin, &top_margin, &right_margin, &bottom_margin);
    left_margin   = left_margin;
    top_margin    = top_margin;
    right_margin  = right_margin;
    bottom_margin = bottom_margin;
    this->layout()->setContentsMargins(left_margin, top_margin, right_margin, bottom_margin);

    // Scale the description header label.
    size.setWidth(ui_.descriptionLabel_1->width());
    size.setHeight(ui_.descriptionLabel_1->height());
    ui_.descriptionLabel_1->setFixedSize(size);

    // Scale the list widget.
    size.setWidth(ui_.apiListWidget->width());
    size.setHeight(ui_.apiListWidget->height());
    ui_.apiListWidget->setFixedSize(size);

    // Scale the description text label.
    size.setWidth(ui_.descriptionLabel->width());
    size.setHeight(ui_.descriptionLabel->height());
    ui_.descriptionLabel->setFixedSize(size);

    // Scale the buttons.
    ui_.exitPushButton->setFixedWidth(ui_.exitPushButton->width());
    ui_.startRGAPushButton->setFixedWidth(ui_.startRGAPushButton->width());

    // Recalculate and set the width.
    setFixedWidth(maxDescriptionWidth + ui_.apiListWidget->width() + kHorizontalLayoutSpacing + kLeftAndRightMargin + kSpacingAfterLabel);
}
