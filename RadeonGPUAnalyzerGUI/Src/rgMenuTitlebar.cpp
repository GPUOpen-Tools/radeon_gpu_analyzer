// C++.
#include <sstream>
#include <cassert>

// Qt.
#include <QWidget>
#include <QStackedLayout>
#include <QLabel>
#include <QLineEdit>
#include <QKeyEvent>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>

// Local
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuTitlebar.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>

// Fixed height of the project title item.
static const int s_PROJECT_TITLE_ITEM_HEIGHT = 28;

// File menu bar stylesheets.
static const char* s_STR_FILEMENU_TITLE_BAR_TOOLTIP_WIDTH = "min-width: %1px; width: %2px;";
static const char* s_STR_FILEMENU_TITLE_BAR_TOOLTIP_HEIGHT = "min-height: %1px; height: %2px; max-height: %3px;";

rgMenuTitlebar::rgMenuTitlebar(QWidget* pParent) :
    QWidget(pParent)
{
    // Create basic layout to hold the background.
    QVBoxLayout* pLayout = new QVBoxLayout();
    pLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(pLayout);

    // Create the background widget.
    QWidget* pBackground = new QWidget();
    pBackground->setObjectName("itemBackground");
    pLayout->addWidget(pBackground);

    // Create main layout.
    m_pMainLayout = new QVBoxLayout();
    m_pMainLayout->setContentsMargins(6, 6, 6, 6);
    pBackground->setLayout(m_pMainLayout);

    // Create stacked layout.
    m_pStackedLayout = new QStackedLayout();
    m_pMainLayout->addLayout(m_pStackedLayout);

    // Create label and editor.
    m_pTitleLabel = new QLabel();
    m_pTitleTextEdit = new QLineEdit();

    // Add widgets to layout.
    m_pStackedLayout->addWidget(m_pTitleLabel);
    m_pStackedLayout->addWidget(m_pTitleTextEdit);

    // Connect the handler for when texting editing is finished.
    bool isConnected = connect(m_pTitleTextEdit, &QLineEdit::editingFinished, this, &rgMenuTitlebar::StopEditing);
    assert(isConnected);
    isConnected = connect(m_pTitleTextEdit, &QLineEdit::returnPressed, this, &rgMenuTitlebar::HandleReturnPressed);
    assert(isConnected);

    // Force size for this item.
    setMinimumSize(0, s_PROJECT_TITLE_ITEM_HEIGHT);
    setMaximumSize(QWIDGETSIZE_MAX, s_PROJECT_TITLE_ITEM_HEIGHT);

    // Set mouse cursor to pointing hand cursor.
    SetCursor();
}

rgMenuTitlebar::~rgMenuTitlebar()
{
    m_pTitleLabel->deleteLater();
    m_pTitleTextEdit->deleteLater();
    m_pStackedLayout->deleteLater();
}

void rgMenuTitlebar::SetTitle(const std::string& title)
{
    // Set widget text.
    m_pTitleLabel->setText(title.c_str());
    m_pTitleTextEdit->setText(title.c_str());

    // Refresh the tooltip.
    RefreshTooltip(title.c_str());
}

void rgMenuTitlebar::StartEditing()
{
    // Set the initial line edit text from the title label text.
    std::string labelText = m_pTitleLabel->text().toStdString();
    labelText = labelText.substr(labelText.find("</b>") + 4);

    m_pTitleTextEdit->setText(labelText.c_str());

    // Show the line edit widget.
    m_pStackedLayout->setCurrentWidget(m_pTitleTextEdit);

    // Default focus on the line edit.
    m_pTitleTextEdit->setFocus();
    m_pTitleTextEdit->setSelection(0, static_cast<int>(m_pTitleLabel->text().length()));
}

void rgMenuTitlebar::StopEditing()
{
    m_pTitleTextEdit->blockSignals(true);

    // Trim the leading and trailing whitespace characters from the project name.
    std::string updatedProjectName = m_pTitleTextEdit->text().toStdString();
    rgUtils::TrimLeadingAndTrailingWhitespace(updatedProjectName, updatedProjectName);

    if (rgUtils::IsValidFileName(updatedProjectName))
    {
        // Check if the text has changed during editing.
        std::string labelText = m_pTitleLabel->text().toStdString();
        labelText = labelText.substr(labelText.find("</b>") + 4);

        if (labelText.compare(updatedProjectName) != 0)
        {
            // Set the new title text from the edited line text.
            std::stringstream updatedTitle;
            updatedTitle << rgUtils::GetProjectTitlePrefix(rgProjectAPI::OpenCL) << updatedProjectName;
            m_pTitleLabel->setText(updatedTitle.str().c_str());

            // Indicate the title text has been changed.
            emit TitleChanged(updatedProjectName);

            // Update the tooltip.
            std::stringstream updatedToolTip;
            updatedToolTip << STR_FILE_MENU_PROJECT_TITLE_TOOLTIP_A << updatedProjectName;

            // Refresh the tooltip.
            RefreshTooltip(updatedToolTip.str());
        }

        // Show the title label widget.
        m_pStackedLayout->setCurrentWidget(m_pTitleLabel);
    }
    else
    {
        // Notify the user that the project name is illegal.
        std::stringstream msg;
        msg << STR_ERR_ILLEGAL_PROJECT_NAME << " \"";
        msg << updatedProjectName << "\".";
        rgUtils::ShowErrorMessageBox(msg.str().c_str());

        // Keep focus on the line edit.
        m_pTitleTextEdit->setFocus();
        m_pTitleTextEdit->setSelection(0, static_cast<int>(m_pTitleLabel->text().length()));
    }

    m_pTitleTextEdit->blockSignals(false);
}

void rgMenuTitlebar::mouseDoubleClickEvent(QMouseEvent* pEvent)
{
    if (pEvent != nullptr && pEvent->button() == Qt::LeftButton)
    {
        StartEditing();
    }
}

void rgMenuTitlebar::keyPressEvent(QKeyEvent* pEvent)
{
    if (pEvent->key() == Qt::Key_Escape)
    {
        // Block signals from the text edit before
        // switching the index on the stacked widget.
        // Not blocking the signals here will cause
        // an editingFinished/StopEditing signal/slot
        // to execute when the text edit loses focus,
        // causing undesired behavior.
        const QSignalBlocker signalBlocker(m_pTitleTextEdit);

        // Switch the stacked widget to the title label.
        assert(m_pStackedLayout != nullptr);
        if (m_pStackedLayout != nullptr)
        {
            m_pStackedLayout->setCurrentWidget(m_pTitleLabel);
        }
    }
    else
    {
        // Pass the event onto the base class
        QWidget::keyPressEvent(pEvent);
    }
}

void rgMenuTitlebar::RefreshTooltip(const std::string& updatedTooltip)
{
    // Make full tooltip string.
    std::string tooltip = updatedTooltip + STR_FILE_MENU_PROJECT_TITLE_TOOLTIP_B;

    // Set tooltip.
    rgUtils::SetToolAndStatusTip(tooltip, this);

    // Resize tooltip box.
    ResizeTooltipBox(tooltip);
}

void rgMenuTitlebar::HandleReturnPressed()
{
    m_pTitleTextEdit->blockSignals(true);

    // When return is pressed to end editing, redirect the focus.
    rgUtils::FocusOnFirstValidAncestor(this);

    m_pTitleTextEdit->blockSignals(false);
}

void rgMenuTitlebar::SetCursor()
{
    // Set mouse cursor to pointing hand cursor.
    setCursor(Qt::PointingHandCursor);
}

void rgMenuTitlebar::ResizeTooltipBox(const std::string& tooltip)
{
    const static int s_TOOLTIP_VERTICAL_MARGIN = 7;

    // Get the font metrics.
    QFontMetrics fontMetrics(font());

    // Calculate the width of the tooltip string.
    QRect boundingRect = fontMetrics.boundingRect(QString::fromStdString(tooltip));
    int width = boundingRect.width();
    width = width * ScalingManager::Get().GetScaleFactor();

    // Calculate the height of the tooltip string.
    const int height = (boundingRect.height() + s_TOOLTIP_VERTICAL_MARGIN) * ScalingManager::Get().GetScaleFactor();

    // Create a width and a height string.
    const QString widthString = QString(s_STR_FILEMENU_TITLE_BAR_TOOLTIP_WIDTH).arg(width).arg(width);
    const QString heightString = QString(s_STR_FILEMENU_TITLE_BAR_TOOLTIP_HEIGHT).arg(height).arg(height).arg(height);

    // Set the stylesheet.
    setStyleSheet("QToolTip {" + widthString + heightString + "}");
}