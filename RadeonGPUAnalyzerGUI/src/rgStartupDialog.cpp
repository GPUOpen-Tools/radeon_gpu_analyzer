// C++.
#include <assert.h>
#include <QKeyEvent>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesVulkan.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgStartupDialog.h>
#include "ui_rgStartupDialog.h"

static const int s_HORIZONTAL_LAYOUT_SPACING = 15;
static const int s_LEFT_AND_RIGHT_MARGIN     = 23;
static const int s_SPACING_AFTER_LABEL       = 10;
static const std::vector<const char*> s_DESCRIPTION_STRINGS = { STR_STARTUP_DIALOG_OPENCL_DESCRIPTION,
                                                                STR_STARTUP_DIALOG_VULKAN_DESCRIPTION };

rgStartupDialog::rgStartupDialog(QWidget* pParent) :
    QDialog(pParent),
    m_selectedApi(rgProjectAPI::Vulkan),
    m_shouldNotAskAgain(false)
{
    ui.setupUi(this);

    // Set the background color to white.
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::white);
    setAutoFillBackground(true);
    setPalette(pal);

    // Disable the help button in the title bar, and disable resizing of this dialog.
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::MSWindowsFixedSizeDialogHint);

    // Shift the initial focus to the Do Not Ask Me Again check box.
    ui.DoNotAskMeAgainCheckBox->setFocus();

    // Connect signals.
    ConnectSignals();

    // Populate the API list widget. (Starting with Vulkan, and going backwards).
    for (int apiIndex = static_cast<int>(rgProjectAPI::Vulkan); apiIndex > 0; --apiIndex)
    {
        rgProjectAPI currentApi = static_cast<rgProjectAPI>(apiIndex);
        std::string apiString;
        rgUtils::ProjectAPIToString(currentApi, apiString);
        ui.apiListWidget->addItem(QString::fromStdString(apiString));
    }

    // Select Vulkan selection in the list widget by default.
    ui.apiListWidget->setCurrentRow(0);
    ui.apiListWidget->setFocus();

    // Set the description for Vulkan selection.
    ui.descriptionLabel->setText(STR_STARTUP_DIALOG_VULKAN_DESCRIPTION);

    clearFocus();

    // Set the cursor to pointing hand cursor.
    SetCursor();

    // Set description field length.
    SetDescriptionLength();

    // Set button shortcuts.
    ui.exitPushButton->setShortcut(Qt::AltModifier + Qt::Key_X);
    ui.startRGAPushButton->setShortcut(Qt::AltModifier + Qt::Key_R);

    //Scale the dialog and the widget inside it.
    ScaleDialog();
}

void rgStartupDialog::ConnectSignals() const
{
    // Connect the handler invoked when the user clicks the Exit push button.
    bool isConnected = connect(ui.exitPushButton, &QPushButton::clicked, this, &rgStartupDialog::HandleExitButtonClicked);
    assert(isConnected);

    // Connect the handler invoked when the user clicks the Start RGA push button.
    isConnected = connect(ui.startRGAPushButton, &QPushButton::clicked, this, &rgStartupDialog::HandleStartRGAButtonClicked);
    assert(isConnected);

    // Connect the handler invoked when the user clicks an item in the API list widget.
    isConnected = connect(ui.apiListWidget, &QListWidget::itemClicked, this, &rgStartupDialog::HandleListWidgetItemClicked);
    assert(isConnected);

    // Connect the handler invoked when the user double clicks an item in the API list widget.
    isConnected = connect(ui.apiListWidget, &QListWidget::itemDoubleClicked, this, &rgStartupDialog::HandleListWidgetItemDoubleClicked);
    assert(isConnected);

    // Connect the handler invoked when the user selects the API by using up/down arrow keys.
    isConnected = connect(ui.apiListWidget, &QListWidget::currentRowChanged, this, &rgStartupDialog::HandleListWidgetItemSelected);
    assert(isConnected);
}

void rgStartupDialog::SetCursor() const
{
    // Set the cursor to pointing hand cursor for various widgets.
    ui.DoNotAskMeAgainCheckBox->setCursor(Qt::PointingHandCursor);
    ui.exitPushButton->setCursor(Qt::PointingHandCursor);
    ui.startRGAPushButton->setCursor(Qt::PointingHandCursor);
    ui.apiListWidget->setCursor(Qt::PointingHandCursor);
}

rgProjectAPI rgStartupDialog::SelectedApi() const
{
    return m_selectedApi;
}

bool rgStartupDialog::ShouldNotAskAgain() const
{
    return m_shouldNotAskAgain;
}

void rgStartupDialog::keyPressEvent(QKeyEvent* pEvent)
{
    // Disable the escape key from closing the dialog
    if (pEvent->key() != Qt::Key_Escape)
    {
        QDialog::keyPressEvent(pEvent);
    }
}

void rgStartupDialog::HandleExitButtonClicked(bool /* checked */)
{
    reject();
}

void rgStartupDialog::HandleStartRGAButtonClicked(bool /* checked */)
{
    m_shouldNotAskAgain = ui.DoNotAskMeAgainCheckBox->isChecked();

    accept();
}

void rgStartupDialog::HandleListWidgetItemClicked(QListWidgetItem* pItem)
{
    if (pItem->text().compare(STR_API_NAME_OPENCL) == 0)
    {
        m_selectedApi = rgProjectAPI::OpenCL;

        // Also update the description.
        ui.descriptionLabel->setText(STR_STARTUP_DIALOG_OPENCL_DESCRIPTION);
    }
    else if (pItem->text().compare(STR_API_NAME_VULKAN) == 0)
    {
        m_selectedApi = rgProjectAPI::Vulkan;

        // Also update the description.
        ui.descriptionLabel->setText(STR_STARTUP_DIALOG_VULKAN_DESCRIPTION);
    }
    else
    {
        // Should not get here.
        assert(false);
        m_selectedApi = rgProjectAPI::Unknown;
    }
}

void rgStartupDialog::HandleListWidgetItemDoubleClicked(QListWidgetItem* pItem)
{
    if (pItem->text().compare(STR_API_NAME_OPENCL) == 0)
    {
        m_selectedApi = rgProjectAPI::OpenCL;

        // Also update the description.
        ui.descriptionLabel->setText(STR_STARTUP_DIALOG_OPENCL_DESCRIPTION);

        // Start RGA.
        HandleStartRGAButtonClicked(false);
    }
    else if (pItem->text().compare(STR_API_NAME_VULKAN) == 0)
    {
        m_selectedApi = rgProjectAPI::Vulkan;

        // Also update the description.
        ui.descriptionLabel->setText(STR_STARTUP_DIALOG_VULKAN_DESCRIPTION);

        // Start RGA.
        HandleStartRGAButtonClicked(false);
    }
    else
    {
        // Should not get here.
        assert(false);
        m_selectedApi = rgProjectAPI::Unknown;
    }
}

void rgStartupDialog::HandleListWidgetItemSelected(int currentRow)
{
    // Extract the QListWidgetItem that the user just selected and process it.
    HandleListWidgetItemClicked(ui.apiListWidget->item(currentRow));
}

void rgStartupDialog::SetDescriptionLength()
{
    int maxWidth = 0;

    for (const char* text : s_DESCRIPTION_STRINGS)
    {
        const QFont font = ui.descriptionLabel->font();
        QFontMetrics fm(font);
        const int textWidth = fm.width(text);

        if (textWidth > maxWidth)
        {
            maxWidth = textWidth;
        }
    }
    ui.descriptionLabel->setMinimumWidth(maxWidth);
}

void rgStartupDialog::ScaleDialog()
{
    ScalingManager& scalingManager = ScalingManager::Get();
    const double scaleFactor = scalingManager.GetScaleFactor();

    // Scale the dialog.
    QSize size;
    size.setWidth(width() * scaleFactor);
    size.setHeight(height() * scaleFactor);
    setFixedSize(size);

    int leftMargin;
    int topMargin;
    int rightMargin;
    int bottomMargin;
    this->layout()->getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);
    leftMargin = leftMargin * scaleFactor;
    topMargin = topMargin * scaleFactor;
    rightMargin = rightMargin * scaleFactor;
    bottomMargin = bottomMargin * scaleFactor;
    this->layout()->setContentsMargins(leftMargin, topMargin, rightMargin, bottomMargin);

    // Scale the description header label.
    size.setWidth(ui.descriptionLabel_1->width() * scaleFactor);
    size.setHeight(ui.descriptionLabel_1->height() * scaleFactor);
    ui.descriptionLabel_1->setFixedSize(size);

    // Scale the list widget.
    size.setWidth(ui.apiListWidget->width() * scaleFactor);
    size.setHeight(ui.apiListWidget->height() * scaleFactor);
    ui.apiListWidget->setFixedSize(size);

    // Scale the description text label.
    size.setWidth(ui.descriptionLabel->width() * scaleFactor);
    size.setHeight(ui.descriptionLabel->height() * scaleFactor);
    ui.descriptionLabel->setFixedSize(size);

    // Scale the buttons.
    ui.exitPushButton->setFixedWidth(ui.exitPushButton->width() * scaleFactor);
    ui.startRGAPushButton->setFixedWidth(ui.startRGAPushButton->width() * scaleFactor);

    // Recalculate and set the width.
    QFontMetrics fontMetrics(ui.descriptionLabel->font());
    QRect boundingRect = fontMetrics.boundingRect(ui.descriptionLabel->text());
    const int width = boundingRect.width();
    setFixedWidth(width + ui.apiListWidget->width()
                  + s_HORIZONTAL_LAYOUT_SPACING * ScalingManager::Get().GetScaleFactor()
                  + s_LEFT_AND_RIGHT_MARGIN * ScalingManager::Get().GetScaleFactor()
                  + s_SPACING_AFTER_LABEL * ScalingManager::Get().GetScaleFactor()
                  );

}
