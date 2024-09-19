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

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_main_window.h"
#include "radeon_gpu_analyzer_gui/qt/rg_mode_push_button.h"
#include "radeon_gpu_analyzer_gui/qt/rg_status_bar.h"
#include "radeon_gpu_analyzer_gui/qt/rg_tree_widget.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_data_types_binary.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"

// Infra.
#include "qt_common/utils/qt_util.h"

static const int kWidgetMinimumHeight = 25;
static const int kModeButtonMinimumWidth = 150;
static const int kPushButtonFontSize = 11;
static const int kTreeWidgetItemHeight = 20;
static const int kModeButtonIconHeight = 15;
static const int kTreeWidgetIconColumnId = 0;
static const int kTreeWidgetApiColumnId = 1;
static const int kTreeWidgetIconColumnWidth = 40;
static const int kModeApiTreeWidgetColumnCount = 2;
static const int kOffsetAboveStatusBar = 3;
static const int kCurrentApiIndex = 0;
static const char* kStrMode = "Mode: ";
static const char* kStrCustomStatusBarStylesheet = "border: none";
static const char* kStrApiButtonTooltipA = "Switch to ";
static const char* kStrApiButtonTooltipB = " mode.";
static const char* kStrConfirmationMessage = "RGA will switch to %1 mode. Are you sure?";
static const char* kStrConfirmationMessageBoxTitle = "Switch application mode";
static const char* kStrApiModeTreeWidgetStylesheet = "border: 1px solid black";
static const char* kStrApiModeTreeWidgetObjectName = "modeAPIList";
static const char* kStrCheckBoxDisabledIconFile = ":/icons/checked_disabled_icon.svg";
static const char* kStrCheckBoxIconLabelStylesheet = "border: none";

class RgApiTreeWidgetItemStyleDelegate : public QStyledItemDelegate
{
public:
    RgApiTreeWidgetItemStyleDelegate(QObject* parent = nullptr)
    : QStyledItemDelegate(parent) {}

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& model_index) const
    {
        // Draw the text for column one manually since we don't want to call the base class' paint method.
        if (model_index.column() == 1)
        {
            // Save the painter object.
            painter->save();

            // Get the current API.
            RgConfigManager& config_manager = RgConfigManager::Instance();
            RgProjectAPI current_api = config_manager.GetCurrentAPI();
            
            // Create the API string.
            std::string api_string = "";
            RgUtils::ProjectAPIToString(current_api, api_string, false);      

            // Set painter options.
            QFont font = painter->font();
            font.setBold(false);
            font.setPointSize(9);
            QPen pen = painter->pen();
            pen.setColor(Qt::GlobalColor::gray);
            painter->setPen(pen);
            painter->setFont(font);

            // Get the text height.
            QRect     text_bounding_rect = painter->boundingRect(QRect(0, 0, 0, 0), Qt::AlignLeft, QString::fromStdString(api_string));
            const int text_height = text_bounding_rect.height();
            const int text_width = text_bounding_rect.width();

            // Calculate the text rect.
            QRect rect = option.rect;
            int text_offset = (rect.height() - text_height) / 2;
            QRect text_rect;
            text_rect.setLeft(rect.left());
            text_rect.setWidth(text_width);
            text_rect.setTop(rect.y() + text_offset);
            text_rect.setHeight(text_height);

            // Draw the current API text.
            painter->drawText(text_rect, QString::fromStdString(api_string));

            // Restore the painter object.
            painter->restore();
        }
    }
};

RgStatusBar::RgStatusBar(QStatusBar* status_bar, QWidget* parent) :
    status_bar_(status_bar),
    parent_(parent),
    QWidget(parent)
{
    // Create the mode button.
    CreateModeButton();

    // Get the supported API names and create a tree widget.
    CreateApiTreeWidget();

    // Set contents margins.
    setContentsMargins(5, 0, 5000, 0);

    // Remove the borders.
    setStyleSheet(kStrCustomStatusBarStylesheet);

    // Create a horizontal layout.
    h_layout_ = new QHBoxLayout();
    h_layout_->setContentsMargins(0, 0, 0, 0);
    h_layout_->setSpacing(0);

    // Set layout to horizontal layout.
    setLayout(h_layout_);

    // Add the mode button to the layout.
    h_layout_->addWidget(mode_push_button_);
    h_layout_->setSizeConstraint(QLayout::SetDefaultConstraint);

    // Set the cursor to pointing hand cursor.
    SetCursor();

    // Connect signals.
    ConnectSignals();

    // Set dimensions.
    SetDimensions();

    // Install an event filter on tree widget to handle up/down arrow keys.
    api_mode_tree_widget_->installEventFilter(this);

    // Set the delegate for the first row of tree widget.
    RgApiTreeWidgetItemStyleDelegate* item_delegate = new RgApiTreeWidgetItemStyleDelegate();
    api_mode_tree_widget_->setItemDelegateForRow(0, item_delegate);
}

bool RgStatusBar::ConstructStatusMessageString(StatusType type, std::string& status_msg_str) const
{
    bool ret = true;
    switch (type)
    {
    case StatusType::kStarted:
        status_msg_str = std::string{kStrStatusBarBuild} + std::string{kStrStatusBarStarted};
        break;
    case StatusType::kFailed:
        status_msg_str = std::string{kStrStatusBarBuild} + std::string{kStrStatusBarFailed};
        break;
    case StatusType::kCanceled:
        status_msg_str = std::string{kStrStatusBarBuild} + std::string{kStrStatusBarCanceled};
        break;
    case StatusType::kSucceeded:
        status_msg_str = std::string{kStrStatusBarBuild} + std::string{kStrStatusBarSucceeded};
        break;
    case StatusType::kUnknown:
    default:
        // We shouldn't get here.
        ret = false;
        assert(ret);
        break;
    }
    return ret;
}

void RgStatusBar::CreateModeButton()
{
    // Get the startup API.
    RgConfigManager& config_manager = RgConfigManager::Instance();
    RgProjectAPI current_api = config_manager.GetCurrentAPI();

    // Create the string for the mode button.
    std::string api_string = "";
    RgUtils::ProjectAPIToString(current_api, api_string, false);
    QString mode_push_button_string = QString(kStrMode) + QString::fromStdString(api_string);

    // Create the mode button.
    mode_push_button_ = new RgModePushButton(this);

    // Set text.
    mode_push_button_->SetText(mode_push_button_string.toStdString());

    // Set the font color.
    mode_push_button_->SetColor(Qt::GlobalColor::white);

    // Set the focus policy.
    mode_push_button_->setFocusPolicy(Qt::StrongFocus);
}

void RgStatusBar::CreateApiTreeWidget()
{
    // Get the list of supported APIs.
    std::vector<std::string> supported_apis;
    RgConfigManager& config_manager = RgConfigManager::Instance();
    config_manager.GetSupportedApis(supported_apis);

    // Create a tree widget to show API modes.
    InitializeTreeWidget();

    assert(api_mode_tree_widget_ != nullptr);
    if (api_mode_tree_widget_ != nullptr)
    {
        for (const std::string& api_string : supported_apis)
        {
            RgProjectAPI api = RgUtils::ProjectAPIToEnum(api_string);
            std::string  display_string;
            RgUtils::ProjectAPIToString(api, display_string, false);

            QTreeWidgetItem* item = new QTreeWidgetItem();

            // If this item is current, disable it so the user cannot select it.
            if (api == config_manager.GetCurrentAPI())
            {
                // Set the item text.
                item->setText(kTreeWidgetApiColumnId, QString::fromStdString(display_string));

                // Add the check box icon in the first column of this item.
                AddCheckBoxIcon(item);

                // Do not show a tooltip for the current mode.
                item->setToolTip(kTreeWidgetIconColumnId, QString());
                item->setToolTip(kTreeWidgetApiColumnId, QString());
            }
            else
            {
                item->setText(kTreeWidgetApiColumnId, QString::fromStdString(display_string));
                api_mode_tree_widget_->addTopLevelItem(item);

                // Set the tooltip.
                QString toolTip = kStrApiButtonTooltipA + QString::fromStdString(display_string) + kStrApiButtonTooltipB;
                item->setToolTip(kTreeWidgetIconColumnId, toolTip);
                item->setToolTip(kTreeWidgetApiColumnId, toolTip);
            }
        }
        api_mode_tree_widget_->resizeColumnToContents(kTreeWidgetApiColumnId);
        api_mode_tree_widget_->setColumnWidth(kTreeWidgetIconColumnId, kTreeWidgetIconColumnWidth);
        api_mode_tree_widget_->setSelectionMode(QAbstractItemView::NoSelection);
        api_mode_tree_widget_->setFocusPolicy(Qt::NoFocus);

        api_mode_tree_widget_->hide();
    }
}

void RgStatusBar::InitializeTreeWidget()
{
    api_mode_tree_widget_ = new RgTreeWidget(parent_);
    api_mode_tree_widget_->setColumnCount(kModeApiTreeWidgetColumnCount);
    api_mode_tree_widget_->setColumnWidth(kTreeWidgetIconColumnId, kTreeWidgetIconColumnWidth);
    api_mode_tree_widget_->setStyleSheet(kStrApiModeTreeWidgetStylesheet);
    api_mode_tree_widget_->setWindowFlags(Qt::FramelessWindowHint);
    api_mode_tree_widget_->setObjectName(kStrApiModeTreeWidgetObjectName);
    api_mode_tree_widget_->setHeaderHidden(true);
    api_mode_tree_widget_->setContentsMargins(0, 0, 0, 0);
    api_mode_tree_widget_->setIndentation(0);
    api_mode_tree_widget_->setAllColumnsShowFocus(true);

    // Disable scrollbars on this tree widget.
    api_mode_tree_widget_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    api_mode_tree_widget_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void RgStatusBar::SetDimensions()
{
    assert(mode_push_button_ != nullptr);
    if (mode_push_button_ != nullptr)
    {
        // Mode push button dimensions.
        mode_push_button_->setMinimumHeight(kWidgetMinimumHeight);
        mode_push_button_->setMaximumHeight(kWidgetMinimumHeight);
        mode_push_button_->setMinimumWidth(kModeButtonMinimumWidth);

        // Gear icon widget dimensions.
        QSize icon_size = mode_push_button_->iconSize();
        icon_size.setHeight(kModeButtonIconHeight);
        mode_push_button_->setIconSize(icon_size);
    }

    // Status bar dimensions.
    setMinimumHeight(kWidgetMinimumHeight);
    setMaximumHeight(kWidgetMinimumHeight);
}

void RgStatusBar::ConnectSignals() const
{
    // Connect the mode push button's clicked signal.
    bool is_connected = connect(mode_push_button_, &QPushButton::clicked, this, &RgStatusBar::HandleModePushButtonClicked);
    assert(is_connected);

    // Connect the API tree widget's clicked signal.
    is_connected = connect(api_mode_tree_widget_, &QTreeWidget::itemClicked, this, &RgStatusBar::HandleTreeWidgetItemClicked);
    assert(is_connected);

    // Connect the API tree widget's itemEntered signal.
    is_connected = connect(api_mode_tree_widget_, &QTreeWidget::itemEntered, this, &RgStatusBar::HandleTreeWidgetItemEntered);
    assert(is_connected);
}

void RgStatusBar::SetCursor() const
{
    // Set the cursor to pointing hand cursor.
    mode_push_button_->setCursor(Qt::PointingHandCursor);
}

void RgStatusBar::SetStatusBarVisibility(bool is_visible)
{
    if (is_visible)
    {
        show();
    }
    else
    {
        hide();
    }
}

void RgStatusBar::HandleModePushButtonClicked(bool /* checked */)
{
    const int kTreeWidgetHeight = kTreeWidgetItemHeight * (static_cast<int>(RgProjectAPI::kApiCount)-1);

    // Process the mode button click.
    assert(api_mode_tree_widget_ != nullptr);
    if (api_mode_tree_widget_ != nullptr)
    {
        if (api_mode_tree_widget_->isVisible())
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
            pos = mode_push_button_->mapTo(parent_, pos);
            const int y_position   = pos.y() - (kTreeWidgetHeight) - kOffsetAboveStatusBar;
            const int height       = kTreeWidgetHeight;
            api_mode_tree_widget_->setGeometry(pos.x(), y_position, mode_push_button_->width(), height);
            api_mode_tree_widget_->setFocus();

            // Remove highlight from all rows.
            QTreeWidgetItemIterator it(api_mode_tree_widget_);
            while (*it)
            {
                (*it)->setBackground(kTreeWidgetIconColumnId, Qt::GlobalColor::transparent);
                (*it)->setBackground(kTreeWidgetApiColumnId, Qt::GlobalColor::transparent);
                ++it;
            }
        }
    }
}

void RgStatusBar::ReorderCurrentMode() const
{
    assert(api_mode_tree_widget_ != nullptr);
    if (api_mode_tree_widget_ != nullptr)
    {
        for (int row = 0; row < api_mode_tree_widget_->topLevelItemCount(); row++)
        {
            QTreeWidgetItem* item = api_mode_tree_widget_->topLevelItem(row);
            bool is_item_enabled = item->flags() & Qt::ItemIsSelectable;
            if (!is_item_enabled)
            {
                // Move this item to the top and break out of the loop.
                item = api_mode_tree_widget_->takeTopLevelItem(row);
                api_mode_tree_widget_->insertTopLevelItem(0, item);

                // Add the check box icon in the first column of this item.
                AddCheckBoxIcon(item);

                break;
            }
        }
    }
}

void RgStatusBar::AddCheckBoxIcon(QTreeWidgetItem* item) const
{
    QIcon icon(kStrCheckBoxDisabledIconFile);
    QLabel* icon_label = new QLabel();
    icon_label->setAutoFillBackground(true);
    QPixmap pixmap = icon.pixmap(QSize(kTreeWidgetItemHeight, kTreeWidgetItemHeight));
    icon_label->setPixmap(pixmap);
    icon_label->setContentsMargins(10, 0, 0, 0);
    icon_label->setStyleSheet(kStrCheckBoxIconLabelStylesheet);
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
    api_mode_tree_widget_->addTopLevelItem(item);
    api_mode_tree_widget_->setItemWidget(item, kTreeWidgetIconColumnId, icon_label);
}

bool RgStatusBar::ShowConfirmationDialogBox(const char* message) const
{
    // Create a custom confirmation dialog box.
    QMessageBox confirmation_dialog;
    confirmation_dialog.setWindowIcon(QIcon(kIconResourceRgaLogo));
    confirmation_dialog.setWindowTitle(kStrConfirmationMessageBoxTitle);
    confirmation_dialog.setText(message);
    confirmation_dialog.setIcon(QMessageBox::Question);
    confirmation_dialog.setModal(true);
    confirmation_dialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmation_dialog.setFocus();

    // Set button cursor to pointing hand cursor.
    QAbstractButton* button = confirmation_dialog.button(QMessageBox::Button::Yes);
    assert(button != nullptr);
    if (button != nullptr)
    {
        button->setCursor(Qt::PointingHandCursor);
    }

    button = confirmation_dialog.button(QMessageBox::Button::No);
    assert(button != nullptr);
    if (button != nullptr)
    {
        button->setCursor(Qt::PointingHandCursor);
    }

    // Display the dialog box just above the status bar.
    QPoint global_cursor_pos = QCursor::pos();
    const int dialog_height = confirmation_dialog.height();
    global_cursor_pos.setY(global_cursor_pos.y() - dialog_height/2);
    confirmation_dialog.move(global_cursor_pos);

    // Return true if the user clicked yes, otherwise return false.
    return (confirmation_dialog.exec() == QMessageBox::Yes);
}

void RgStatusBar::HandleTreeWidgetItemClicked(QTreeWidgetItem* item, const int column)
{
    Q_UNUSED(column);

    assert(item != nullptr);
    if (item != nullptr && (item->flags() & Qt::ItemIsSelectable))
    {
        // If the user clicked on the first item, do nothing.
        const int row = api_mode_tree_widget_->indexOfTopLevelItem(item);

        if (row != kCurrentApiIndex)
        {
            QString text = item->text(kTreeWidgetApiColumnId);

            // Put up a confirmation dialog box.
            QString message_string = QString(kStrConfirmationMessage).arg(text);

            // Hide the API list widget.
            SetApiListVisibility(false);

            // Show a confirmation dialog box.
            bool status = ShowConfirmationDialogBox(message_string.toStdString().c_str());

            // Set the focus to the mode push button.
            mode_push_button_->setFocus();

            if (status)
            {
                // Save any pending changes.
                bool is_not_cancelled = false;
                RgMainWindow* main_window = static_cast<RgMainWindow*>(parent_);
                if (main_window != nullptr)
                {
                    is_not_cancelled = main_window->HandleSavePendingChanges();
                }

                // Emit a signal to indicate API change.
                if (is_not_cancelled)
                {
                    emit ChangeAPIModeSignal(RgUtils::ProjectAPIToEnum(text.toStdString()));

                    // Disable the selected item so the user cannot select it next time around.
                    item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
                }
            }
        }
    }
    else
    {
        QTreeWidgetItem* second_item = api_mode_tree_widget_->topLevelItem(1);
        api_mode_tree_widget_->setCurrentItem(second_item);
    }
}

void RgStatusBar::SetApiListVisibility(bool is_visible) const
{
    if (is_visible)
    {
        api_mode_tree_widget_->show();
    }
    else
    {
        api_mode_tree_widget_->hide();
    }
}

bool RgStatusBar::eventFilter(QObject* object, QEvent* event)
{
    bool status = false;

    if ((object == api_mode_tree_widget_) && (event->type() == QEvent::KeyPress))
    {
        QTreeWidgetItem* item = api_mode_tree_widget_->currentItem();
        if (item != nullptr)
        {
            QString api_string = item->text(kTreeWidgetApiColumnId);
            const int current_index = api_mode_tree_widget_->indexOfTopLevelItem(item);
            if (event->type() == QEvent::KeyPress)
            {
                QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
                if (key_event->key() == Qt::Key_Up)
                {
                    if (current_index == 1 || current_index == 0)
                    {
                        const int numRows = api_mode_tree_widget_->topLevelItemCount();
                        item = api_mode_tree_widget_->topLevelItem(numRows - 1);
                        api_mode_tree_widget_->setCurrentItem(item);
                        HandleTreeWidgetItemEntered(item, 0);
                    }
                    else
                    {
                        item = api_mode_tree_widget_->topLevelItem(current_index-1);
                        api_mode_tree_widget_->setCurrentItem(item);
                        HandleTreeWidgetItemEntered(item, 0);
                    }

                   status = true;
                }
                else if (key_event->key() == Qt::Key_Down)
                {
                    if (current_index == (api_mode_tree_widget_->topLevelItemCount() - 1))
                    {
                        item = api_mode_tree_widget_->topLevelItem(1);
                        api_mode_tree_widget_->setCurrentItem(item);
                        HandleTreeWidgetItemEntered(item, 0);
                    }
                    else
                    {
                        item = api_mode_tree_widget_->topLevelItem(current_index+1);
                        api_mode_tree_widget_->setCurrentItem(item);
                        HandleTreeWidgetItemEntered(item, 0);
                    }

                    status = true;
                }
                else if ((key_event->key() == Qt::Key_Enter) || (key_event->key() == Qt::Key_Return))
                {
                    item = api_mode_tree_widget_->currentItem();
                    HandleTreeWidgetItemClicked(item, kTreeWidgetApiColumnId);
                    HandleTreeWidgetItemEntered(item, 0);

                    status = true;
                }
                else if (key_event->key() == Qt::Key_Escape)
                {
                    SetApiListVisibility(false);

                    // Set the focus to the mode push button.
                    mode_push_button_->setFocus();

                    status = true;
                }
            }
        }
    }

    return status;
}

void RgStatusBar::HandleTreeWidgetItemEntered(QTreeWidgetItem* item, const int column)
{
    Q_UNUSED(column);

    // Set background color for all items to transparent.
    QTreeWidgetItemIterator it(api_mode_tree_widget_);
    while (*it)
    {
        (*it)->setBackground(kTreeWidgetIconColumnId, Qt::GlobalColor::transparent);
        (*it)->setBackground(kTreeWidgetApiColumnId, Qt::GlobalColor::transparent);
        ++it;
    }

    // Set the background color for the current item to light blue.
    assert(item != nullptr);
    if (item != nullptr)
    {
        const int column_count = item->columnCount();
        for (int column_number = 0; column_number < column_count; column_number++)
        {
            item->setBackground(column_number, qApp->palette().color(QPalette::Highlight));
        }
    }
}

RgModePushButton* RgStatusBar::GetModePushButton()
{
    return mode_push_button_;
}

QTreeWidget* RgStatusBar::GetApiListTreeWidget()
{
    return api_mode_tree_widget_;
}
