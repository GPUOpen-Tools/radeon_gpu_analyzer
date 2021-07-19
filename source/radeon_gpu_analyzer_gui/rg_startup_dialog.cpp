// C++.
#include <assert.h>
#include <QKeyEvent>

// Infra.
#include "QtCommon/Scaling/ScalingManager.h"

// Local.
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_opencl.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_vulkan.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/qt/rg_startup_dialog.h"
#include "ui_rg_startup_dialog.h"

static const int kHorizontalLayoutSpacing = 15;
static const int kLeftAndRightMargin     = 23;
static const int kSpacingAfterLabel       = 10;
static const std::vector<const char*> kDescription = { kStrStartupDialogOpenclDescription,
                                                                kStrStartupDialogVulkanDescription };

RgStartupDialog::RgStartupDialog(QWidget* parent) :
    QDialog(parent),
    selected_api_(RgProjectAPI::kVulkan),
    should_not_ask_again_(false)
{
    ui_.setupUi(this);

    // Set the background color to white.
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::white);
    setAutoFillBackground(true);
    setPalette(pal);

    // Disable the help button in the title bar, and disable resizing of this dialog.
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::MSWindowsFixedSizeDialogHint);

    // Shift the initial focus to the Do Not Ask Me Again check box.
    ui_.DoNotAskMeAgainCheckBox->setFocus();

    // Connect signals.
    ConnectSignals();

    // Populate the API list widget. (Starting with Vulkan, and going backwards).
    for (int api_index = static_cast<int>(RgProjectAPI::kVulkan); api_index > 0; --api_index)
    {
        RgProjectAPI current_api = static_cast<RgProjectAPI>(api_index);
        std::string api_string;
        RgUtils::ProjectAPIToString(current_api, api_string);
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
    ui_.exitPushButton->setShortcut(Qt::AltModifier + Qt::Key_X);
    ui_.startRGAPushButton->setShortcut(Qt::AltModifier + Qt::Key_R);

    //Scale the dialog and the widget inside it.
    ScaleDialog();
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
}

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
    int max_width = 0;

    for (const char* text : kDescription)
    {
        const QFont font = ui_.descriptionLabel->font();
        QFontMetrics fm(font);
        const int text_width = fm.width(text);

        if (text_width > max_width)
        {
            max_width = text_width;
        }
    }
    ui_.descriptionLabel->setMinimumWidth(max_width);
}

void RgStartupDialog::ScaleDialog()
{
    ScalingManager& scaling_manager = ScalingManager::Get();
    const double scale_factor = scaling_manager.GetScaleFactor();

    // Scale the dialog.
    QSize size;
    size.setWidth(width() * scale_factor);
    size.setHeight(height() * scale_factor);
    setFixedSize(size);

    int left_margin;
    int top_margin;
    int right_margin;
    int bottom_margin;
    this->layout()->getContentsMargins(&left_margin, &top_margin, &right_margin, &bottom_margin);
    left_margin = left_margin * scale_factor;
    top_margin = top_margin * scale_factor;
    right_margin = right_margin * scale_factor;
    bottom_margin = bottom_margin * scale_factor;
    this->layout()->setContentsMargins(left_margin, top_margin, right_margin, bottom_margin);

    // Scale the description header label.
    size.setWidth(ui_.descriptionLabel_1->width() * scale_factor);
    size.setHeight(ui_.descriptionLabel_1->height() * scale_factor);
    ui_.descriptionLabel_1->setFixedSize(size);

    // Scale the list widget.
    size.setWidth(ui_.apiListWidget->width() * scale_factor);
    size.setHeight(ui_.apiListWidget->height() * scale_factor);
    ui_.apiListWidget->setFixedSize(size);

    // Scale the description text label.
    size.setWidth(ui_.descriptionLabel->width() * scale_factor);
    size.setHeight(ui_.descriptionLabel->height() * scale_factor);
    ui_.descriptionLabel->setFixedSize(size);

    // Scale the buttons.
    ui_.exitPushButton->setFixedWidth(ui_.exitPushButton->width() * scale_factor);
    ui_.startRGAPushButton->setFixedWidth(ui_.startRGAPushButton->width() * scale_factor);

    // Recalculate and set the width.
    QFontMetrics font_metrics(ui_.descriptionLabel->font());
    QRect bounding_rect = font_metrics.boundingRect(ui_.descriptionLabel->text());
    const int width = bounding_rect.width();
    setFixedWidth(width + ui_.apiListWidget->width()
                  + kHorizontalLayoutSpacing * ScalingManager::Get().GetScaleFactor()
                  + kLeftAndRightMargin * ScalingManager::Get().GetScaleFactor()
                  + kSpacingAfterLabel * ScalingManager::Get().GetScaleFactor()
                  );

}
