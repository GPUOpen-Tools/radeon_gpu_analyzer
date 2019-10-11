// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QHBoxLayout>
#include <QMessageBox>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QStyledItemDelegate>
#include <QPainter>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMainWindow.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgModePushButton.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgStatusBar.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgTreeWidget.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <QtCommon/Util/QtUtil.h>

static const int s_WIDGET_MINIMUM_HEIGHT = 25;
static const int s_MODE_BUTTON_MINIMUM_WIDTH = 110;
static const int s_PUSH_BUTTON_FONT_SIZE = 11;
static const int s_TREE_WIDGET_ITEM_HEIGHT = 20;
static const int s_MODE_BUTTON_ICON_HEIGHT = 15;
static const int s_TREE_WIDGET_ICON_COLUMN_ID = 0;
static const int s_TREE_WIDGET_API_COLUMN_ID = 1;
static const int s_TREE_WIDGET_ICON_COLUMN_WIDTH = 40;
static const int s_MODE_API_TREE_WIDGET_COLUMN_COUNT = 2;
static const int s_OFFSET_ABOVE_STATUS_BAR = 3;
static const int s_CURRENT_API_INDEX = 0;
static const char* s_STR_MODE = "Mode: ";
static const char* s_CUSTOM_STATUS_BAR_STYLESHEET = "border: none";
static const char* s_API_BUTTON_TOOLTIP_A = "Switch to ";
static const char* s_API_BUTTON_TOOLTIP_B = " mode.";
static const char* s_CONFIRMATION_MESSAGE = "RGA will switch to %1 mode. Are you sure?";
static const char* s_CONFIRMATION_MESSAGE_BOX_TITLE = "Switch application mode";
static const char* s_API_MODE_TREE_WIDGET_STYLESHEET = "border: 1px solid black";
static const char* s_API_MODE_TREE_WIDGET_OBJECT_NAME = "modeAPIList";
static const char* s_CHECK_BOX_DISABLED_ICON_FILE = ":/icons/checkedDisabledIcon.svg";
static const char* s_CHECK_BOX_ICON_LABEL_STYLESHEET = "border: none";
static const QColor s_API_MODE_TREE_WIDGET_FIRST_COLUMN_BACKGROUND_COLOR = QColor(240, 240, 240);

class rgApiTreeWidgetItemStyleDelegate : public QStyledItemDelegate
{
public:
    rgApiTreeWidgetItemStyleDelegate(QObject* pParent = nullptr)
    : QStyledItemDelegate(pParent) {}


    void paint(QPainter* pPainter, const QStyleOptionViewItem& option,
        const QModelIndex& modelIndex) const
    {
        // Draw the text for column one manually since we don't want to call the base class' paint method.
        if (modelIndex.column() == 1)
        {
            // Save the painter object.
            pPainter->save();

            // Get the current API.
            rgConfigManager& configManager = rgConfigManager::Instance();
            rgProjectAPI currentAPI = configManager.GetCurrentAPI();

            // Create the API string.
            std::string apiString = "";
            rgUtils::ProjectAPIToString(currentAPI, apiString);

            // Set painter options.
            QFont font = pPainter->font();
            font.setBold(false);
            font.setPointSize(9);
            QPen pen = pPainter->pen();
            pen.setColor(Qt::GlobalColor::gray);
            pPainter->setPen(pen);
            pPainter->setFont(font);

            // Get the text height.
            QRect textBoundingRect = pPainter->boundingRect(QRect(0, 0, 0, 0), Qt::AlignLeft, QString::fromStdString(apiString));
            const int textHeight = textBoundingRect.height();
            const int textWidth = textBoundingRect.width();

            // Calculate the text rect.
            QRect rect = option.rect;
            int textOffset = (rect.height() - textHeight) / 2;
            QRect textRect;
            textRect.setLeft(rect.left());
            textRect.setWidth(textWidth);
            textRect.setTop(rect.y() + textOffset);
            textRect.setHeight(textHeight);

            // Draw the current API text.
            pPainter->drawText(textRect, QString::fromStdString(apiString));

            // Restore the painter object.
            pPainter->restore();
        }
    }
};

rgStatusBar::rgStatusBar(QStatusBar* pStatusBar, QWidget* pParent) :
    m_pStatusBar(pStatusBar),
    m_pParent(pParent),
    QWidget(pParent)
{
    // Create the mode button.
    CreateModeButton();

    // Get the supported API names and create a tree widget.
    CreateApiTreeWidget();

    // Set contents margins.
    setContentsMargins(5, 0, 5000, 0);

    // Remove the borders.
    setStyleSheet(s_CUSTOM_STATUS_BAR_STYLESHEET);

    // Create a horizontal layout.
    m_pHLayout = new QHBoxLayout();
    m_pHLayout->setContentsMargins(0, 0, 0, 0);
    m_pHLayout->setSpacing(0);

    // Set layout to horizontal layout.
    setLayout(m_pHLayout);

    // Add the mode button to the layout.
    m_pHLayout->addWidget(m_pModePushButton);
    m_pHLayout->setSizeConstraint(QLayout::SetDefaultConstraint);

    // Set the cursor to pointing hand cursor.
    SetCursor();

    // Connect signals.
    ConnectSignals();

    // Set dimensions.
    SetDimensions();

    // Install an event filter on tree widget to handle up/down arrow keys.
    m_pApiModeTreeWidget->installEventFilter(this);

    // Set the delegate for the first row of tree widget.
    rgApiTreeWidgetItemStyleDelegate* pItemDelegate = new rgApiTreeWidgetItemStyleDelegate();
    m_pApiModeTreeWidget->setItemDelegateForRow(0, pItemDelegate);
}

void rgStatusBar::CreateModeButton()
{
    // Get the startup API.
    rgConfigManager& configManager = rgConfigManager::Instance();
    rgProjectAPI currentAPI = configManager.GetCurrentAPI();

    // Create the string for the mode button.
    std::string apiString = "";
    rgUtils::ProjectAPIToString(currentAPI, apiString);
    QString modePushButtonString = QString(s_STR_MODE) + QString::fromStdString(apiString);

    // Create the mode button.
    m_pModePushButton = new rgModePushButton(this);

    // Set text.
    m_pModePushButton->SetText(modePushButtonString.toStdString());

    // Set the font color.
    m_pModePushButton->SetColor(Qt::GlobalColor::white);

    // Set the focus policy.
    m_pModePushButton->setFocusPolicy(Qt::StrongFocus);
}

void rgStatusBar::CreateApiTreeWidget()
{
    // Get the list of supported APIs.
    std::vector<std::string> supportedAPIs;
    rgConfigManager& configManager = rgConfigManager::Instance();
    configManager.GetSupportedApis(supportedAPIs);

    // Create a tree widget to show API modes.
    InitializeTreeWidget();

    assert(m_pApiModeTreeWidget != nullptr);
    if (m_pApiModeTreeWidget != nullptr)
    {
        for (const std::string& api : supportedAPIs)
        {
            QTreeWidgetItem* pItem = new QTreeWidgetItem();

            // If this item is current, disable it so the user cannot select it.
            rgProjectAPI currentAPI = configManager.GetCurrentAPI();
            std::string currentAPIString = "";
            rgUtils::ProjectAPIToString(currentAPI, currentAPIString);
            if (api.compare(currentAPIString) == 0)
            {
                // Set the item text.
                pItem->setText(s_TREE_WIDGET_API_COLUMN_ID, QString::fromStdString(api));

                // Add the check box icon in the first column of this item.
                AddCheckBoxIcon(pItem);

                // Do not show a tooltip for the current mode.
                pItem->setToolTip(s_TREE_WIDGET_ICON_COLUMN_ID, QString());
                pItem->setToolTip(s_TREE_WIDGET_API_COLUMN_ID, QString());
            }
            else
            {
                pItem->setText(s_TREE_WIDGET_API_COLUMN_ID, QString::fromStdString(api));
                m_pApiModeTreeWidget->addTopLevelItem(pItem);

                // Set the tooltip.
                QString toolTip = s_API_BUTTON_TOOLTIP_A + QString::fromStdString(api) + s_API_BUTTON_TOOLTIP_B;
                pItem->setToolTip(s_TREE_WIDGET_ICON_COLUMN_ID, toolTip);
                pItem->setToolTip(s_TREE_WIDGET_API_COLUMN_ID, toolTip);
                pItem->setBackgroundColor(s_TREE_WIDGET_ICON_COLUMN_ID, s_API_MODE_TREE_WIDGET_FIRST_COLUMN_BACKGROUND_COLOR);
            }
        }
        m_pApiModeTreeWidget->resizeColumnToContents(s_TREE_WIDGET_API_COLUMN_ID);
        m_pApiModeTreeWidget->setColumnWidth(s_TREE_WIDGET_ICON_COLUMN_ID, s_TREE_WIDGET_ICON_COLUMN_WIDTH);
        m_pApiModeTreeWidget->setSelectionMode(QAbstractItemView::NoSelection);
        m_pApiModeTreeWidget->setFocusPolicy(Qt::NoFocus);

        m_pApiModeTreeWidget->hide();
    }
}

void rgStatusBar::InitializeTreeWidget()
{
    m_pApiModeTreeWidget = new rgTreeWidget(m_pParent);
    m_pApiModeTreeWidget->setColumnCount(s_MODE_API_TREE_WIDGET_COLUMN_COUNT);
    m_pApiModeTreeWidget->setColumnWidth(s_TREE_WIDGET_ICON_COLUMN_ID, s_TREE_WIDGET_ICON_COLUMN_WIDTH);
    m_pApiModeTreeWidget->setStyleSheet(s_API_MODE_TREE_WIDGET_STYLESHEET);
    m_pApiModeTreeWidget->setWindowFlags(Qt::FramelessWindowHint);
    m_pApiModeTreeWidget->setObjectName(s_API_MODE_TREE_WIDGET_OBJECT_NAME);
    m_pApiModeTreeWidget->setHeaderHidden(true);
    m_pApiModeTreeWidget->setContentsMargins(0, 0, 0, 0);
    m_pApiModeTreeWidget->setIndentation(0);
    m_pApiModeTreeWidget->setAllColumnsShowFocus(true);

    // Disable scrollbars on this tree widget.
    m_pApiModeTreeWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pApiModeTreeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void rgStatusBar::SetDimensions()
{
    assert(m_pModePushButton != nullptr);
    if (m_pModePushButton != nullptr)
    {
        // Mode push button dimensions.
        m_pModePushButton->setMinimumHeight(s_WIDGET_MINIMUM_HEIGHT);
        m_pModePushButton->setMaximumHeight(s_WIDGET_MINIMUM_HEIGHT);
        m_pModePushButton->setMinimumWidth(s_MODE_BUTTON_MINIMUM_WIDTH);

        // Gear icon widget dimensions.
        QSize iconSize = m_pModePushButton->iconSize();
        iconSize.setHeight(s_MODE_BUTTON_ICON_HEIGHT);
        m_pModePushButton->setIconSize(iconSize);
    }

    // Status bar dimensions.
    setMinimumHeight(s_WIDGET_MINIMUM_HEIGHT);
    setMaximumHeight(s_WIDGET_MINIMUM_HEIGHT);
}

void rgStatusBar::ConnectSignals() const
{
    // Connect the mode push button's clicked signal.
    bool isConnected = connect(m_pModePushButton, &QPushButton::clicked, this, &rgStatusBar::HandleModePushButtonClicked);
    assert(isConnected);

    // Connect the API tree widget's clicked signal.
    isConnected = connect(m_pApiModeTreeWidget, &QTreeWidget::itemClicked, this, &rgStatusBar::HandleTreeWidgetItemClicked);
    assert(isConnected);

    // Connect the API tree widget's itemEntered signal.
    isConnected = connect(m_pApiModeTreeWidget, &QTreeWidget::itemEntered, this, &rgStatusBar::HandleTreeWidgetItemEntered);
    assert(isConnected);
}

void rgStatusBar::SetCursor() const
{
    // Set the cursor to pointing hand cursor.
    m_pModePushButton->setCursor(Qt::PointingHandCursor);
}

void rgStatusBar::SetStatusBarVisibility(bool isVisible)
{
    if (isVisible)
    {
        show();
    }
    else
    {
        hide();
    }
}

void rgStatusBar::HandleModePushButtonClicked(bool /* checked */)
{
    const int s_TREE_WIDGET_HEIGHT = s_TREE_WIDGET_ITEM_HEIGHT * (static_cast<int>(rgProjectAPI::ApiCount)-1);

    // Process the mode button click.
    assert(m_pApiModeTreeWidget != nullptr);
    if (m_pApiModeTreeWidget != nullptr)
    {
        if (m_pApiModeTreeWidget->isVisible())
        {
            // Hide the API tree widget.
            SetApiListVisibility(false);
        }
        else
        {
            // Sort the mode string so the current mode is at the top.
            ReorderCurrentMode();

            // Show the API tree widget.
            SetApiListVisibility(true);
            QPoint pos(0, 0);
            pos = m_pModePushButton->mapTo(m_pParent, pos);
            const int yPosition = pos.y() - (s_TREE_WIDGET_HEIGHT * ScalingManager::Get().GetScaleFactor()) - s_OFFSET_ABOVE_STATUS_BAR;;
            const int height = s_TREE_WIDGET_HEIGHT * ScalingManager::Get().GetScaleFactor();
            m_pApiModeTreeWidget->setGeometry(pos.x(), yPosition, m_pModePushButton->width(), height);
            m_pApiModeTreeWidget->setFocus();

            // Remove highlight from all rows.
            QTreeWidgetItemIterator it(m_pApiModeTreeWidget);
            while (*it)
            {
                (*it)->setBackgroundColor(s_TREE_WIDGET_ICON_COLUMN_ID, Qt::GlobalColor::transparent);
                (*it)->setBackgroundColor(s_TREE_WIDGET_API_COLUMN_ID, Qt::GlobalColor::transparent);
                ++it;
            }
        }
    }
}

void rgStatusBar::ReorderCurrentMode() const
{
    assert(m_pApiModeTreeWidget != nullptr);
    if (m_pApiModeTreeWidget != nullptr)
    {
        for (int row = 0; row < m_pApiModeTreeWidget->topLevelItemCount(); row++)
        {
            QTreeWidgetItem* pItem = m_pApiModeTreeWidget->topLevelItem(row);
            bool isItemEnabled = pItem->flags() & Qt::ItemIsSelectable;
            if (!isItemEnabled)
            {
                // Move this item to the top and break out of the loop.
                pItem = m_pApiModeTreeWidget->takeTopLevelItem(row);
                m_pApiModeTreeWidget->insertTopLevelItem(0, pItem);

                // Add the check box icon in the first column of this item.
                AddCheckBoxIcon(pItem);

                break;
            }
        }
    }
}

void rgStatusBar::AddCheckBoxIcon(QTreeWidgetItem* pItem) const
{
    QIcon icon(s_CHECK_BOX_DISABLED_ICON_FILE);
    QLabel* pIconLabel = new QLabel();
    pIconLabel->setAutoFillBackground(true);
    QPixmap pixmap = icon.pixmap(QSize(s_TREE_WIDGET_ITEM_HEIGHT * ScalingManager::Get().GetScaleFactor(), s_TREE_WIDGET_ITEM_HEIGHT * ScalingManager::Get().GetScaleFactor()));
    pIconLabel->setPixmap(pixmap);
    pIconLabel->setContentsMargins(10, 0, 0, 0);
    pIconLabel->setStyleSheet(s_CHECK_BOX_ICON_LABEL_STYLESHEET);
    pItem->setFlags(pItem->flags() & ~Qt::ItemIsSelectable);
    m_pApiModeTreeWidget->addTopLevelItem(pItem);
    m_pApiModeTreeWidget->setItemWidget(pItem, s_TREE_WIDGET_ICON_COLUMN_ID, pIconLabel);
}

bool rgStatusBar::ShowConfirmationDialogBox(const char* pMessage) const
{
    // Create a custom confirmation dialog box.
    QMessageBox confirmationDialog;
    confirmationDialog.setWindowIcon(QIcon(gs_ICON_RESOURCE_RGA_LOGO));
    confirmationDialog.setWindowTitle(s_CONFIRMATION_MESSAGE_BOX_TITLE);
    confirmationDialog.setText(pMessage);
    confirmationDialog.setIcon(QMessageBox::Question);
    confirmationDialog.setModal(true);
    confirmationDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

    // Set button cursor to pointing hand cursor.
    QAbstractButton* pButton = confirmationDialog.button(QMessageBox::Button::Yes);
    assert(pButton != nullptr);
    if (pButton != nullptr)
    {
        pButton->setCursor(Qt::PointingHandCursor);
    }

    pButton = confirmationDialog.button(QMessageBox::Button::No);
    assert(pButton != nullptr);
    if (pButton != nullptr)
    {
        pButton->setCursor(Qt::PointingHandCursor);
    }

    // Display the dialog box just above the status bar.
    QPoint globalCursorPos = QCursor::pos();
    const int dialogHeight = confirmationDialog.height();
    globalCursorPos.setY(globalCursorPos.y() - dialogHeight/2);
    confirmationDialog.move(globalCursorPos);

    // Return true if the user clicked yes, otherwise return false.
    return (confirmationDialog.exec() == QMessageBox::Yes);
}

void rgStatusBar::HandleTreeWidgetItemClicked(QTreeWidgetItem* pItem, const int column)
{
    Q_UNUSED(column);

    assert(pItem != nullptr);
    if (pItem != nullptr && (pItem->flags() & Qt::ItemIsSelectable))
    {
        // If the user clicked on the first item, do nothing.
        const int row = m_pApiModeTreeWidget->indexOfTopLevelItem(pItem);

        if (row != s_CURRENT_API_INDEX)
        {
            QString text = pItem->text(s_TREE_WIDGET_API_COLUMN_ID);

            // Put up a confirmation dialog box.
            QString messageString = QString(s_CONFIRMATION_MESSAGE).arg(text);

            // Hide the API list widget.
            SetApiListVisibility(false);

            // Show a confirmation dialog box.
            bool status = ShowConfirmationDialogBox(messageString.toStdString().c_str());

            // Set the focus to the mode push button.
            m_pModePushButton->setFocus();

            if (status)
            {
                // Save any pending changes.
                bool isNotCancelled = false;
                rgMainWindow* pMainWindow = static_cast<rgMainWindow*>(m_pParent);
                if (pMainWindow != nullptr)
                {
                    isNotCancelled = pMainWindow->HandleSavePendingChanges();
                }

                // Emit a signal to indicate API change.
                if (isNotCancelled)
                {
                    emit ChangeAPIModeSignal(rgUtils::ProjectAPIToEnum((text.toStdString())));

                    // Disable the selected item so the user cannot select it next time around.
                    pItem->setFlags(pItem->flags() & ~Qt::ItemIsSelectable);
                }
            }
        }
    }
    else
    {
        QTreeWidgetItem* pSecondItem = m_pApiModeTreeWidget->topLevelItem(1);
        m_pApiModeTreeWidget->setCurrentItem(pSecondItem);
    }
}

void rgStatusBar::SetApiListVisibility(bool isVisible) const
{
    if (isVisible)
    {
        m_pApiModeTreeWidget->show();
    }
    else
    {
        m_pApiModeTreeWidget->hide();
    }
}

bool rgStatusBar::eventFilter(QObject* pObject, QEvent* pEvent)
{
    bool status = false;

    if ((pObject == m_pApiModeTreeWidget) && (pEvent->type() == QEvent::KeyPress))
    {
        QTreeWidgetItem* pItem = m_pApiModeTreeWidget->currentItem();
        if (pItem != nullptr)
        {
            QString apiString = pItem->text(s_TREE_WIDGET_API_COLUMN_ID);
            const int currentIndex = m_pApiModeTreeWidget->indexOfTopLevelItem(pItem);
            if (pEvent->type() == QEvent::KeyPress)
            {
                QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(pEvent);
                if (pKeyEvent->key() == Qt::Key_Up)
                {
                    if (currentIndex == 1 || currentIndex == 0)
                    {
                        const int numRows = m_pApiModeTreeWidget->topLevelItemCount();
                        QTreeWidgetItem* pItem = m_pApiModeTreeWidget->topLevelItem(numRows - 1);
                        m_pApiModeTreeWidget->setCurrentItem(pItem);
                        HandleTreeWidgetItemEntered(pItem, 0);
                    }
                    else
                    {
                        QTreeWidgetItem* pItem = m_pApiModeTreeWidget->topLevelItem(currentIndex-1);
                        m_pApiModeTreeWidget->setCurrentItem(pItem);
                        HandleTreeWidgetItemEntered(pItem, 0);
                    }

                   status = true;
                }
                else if (pKeyEvent->key() == Qt::Key_Down)
                {
                    if (currentIndex == (m_pApiModeTreeWidget->topLevelItemCount() - 1))
                    {
                        QTreeWidgetItem* pItem = m_pApiModeTreeWidget->topLevelItem(1);
                        m_pApiModeTreeWidget->setCurrentItem(pItem);
                        HandleTreeWidgetItemEntered(pItem, 0);
                    }
                    else
                    {
                        QTreeWidgetItem* pItem = m_pApiModeTreeWidget->topLevelItem(currentIndex+1);
                        m_pApiModeTreeWidget->setCurrentItem(pItem);
                        HandleTreeWidgetItemEntered(pItem, 0);
                    }

                    status = true;
                }
                else if ((pKeyEvent->key() == Qt::Key_Enter) || (pKeyEvent->key() == Qt::Key_Return))
                {
                    QTreeWidgetItem* pItem = m_pApiModeTreeWidget->currentItem();
                    HandleTreeWidgetItemClicked(pItem, s_TREE_WIDGET_API_COLUMN_ID);
                    HandleTreeWidgetItemEntered(pItem, 0);

                    status = true;
                }
                else if (pKeyEvent->key() == Qt::Key_Escape)
                {
                    SetApiListVisibility(false);

                    // Set the focus to the mode push button.
                    m_pModePushButton->setFocus();

                    status = true;
                }
            }
        }
    }

    return status;
}

void rgStatusBar::HandleTreeWidgetItemEntered(QTreeWidgetItem* pItem, const int column)
{
    // Set background color for all items to transparent.
    QTreeWidgetItemIterator it(m_pApiModeTreeWidget);
    while (*it)
    {
        (*it)->setBackgroundColor(s_TREE_WIDGET_ICON_COLUMN_ID, Qt::GlobalColor::transparent);
        (*it)->setBackgroundColor(s_TREE_WIDGET_API_COLUMN_ID, Qt::GlobalColor::transparent);
        ++it;
    }

    // Set the background color for the current item to light blue.
    QColor lightBlue(229, 243, 255);
    assert(pItem != nullptr);
    if (pItem != nullptr)
    {
        const int columnCount = pItem->columnCount();
        for (int columnNumber = 0; columnNumber < columnCount; columnNumber++)
        {
            pItem->setBackgroundColor(columnNumber, lightBlue);
        }
    }
}