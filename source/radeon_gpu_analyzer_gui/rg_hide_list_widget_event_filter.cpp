// C++.
#include <cassert>

// Qt.
#include <QCheckBox>
#include <QEvent>
#include <QKeyEvent>
#include <QListWidget>
#include <QObject>

// Infra.
#include "QtCommon/CustomWidgets/ArrowIconWidget.h"
#include "QtCommon/Util/CommonDefinitions.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_hide_list_widget_event_filter.h"

// Object name associated with the column dropdown list.
static const char* kStrDisassemblyColumnVisibilityList = "DisassemblyColumnVisibilityList";

// Object name associated with the target GPU dropdown list.
static const char* kStrDisassemblyTargetGpuList = "TargetGpuList";

// The object names for UI objects that the user can click on.
static const char* kStrColumnVisiblityArrowPushButton = "columnVisibilityArrowPushButton";
static const char* kStrDisassemblyTargetGpuPushButton = "targetGpuPushButton";
static const char* kStrIsaListArrowPushButton         = "isaListArrowPushButton";
static const char* kStrQtScrollAreaViewport            = "qt_scrollarea_viewport";
static const char* kStrViewTitleBar                     = "viewTitlebar";
static const char* kStrViewOpenClSettingsView         = "rgOpenClSettingsView";
static const char* kStrSettingsTab                       = "settingsTab";
static const char* kStrStartTab                          = "start_tab";
static const char* kStrTabWidgetStackedWidget          = "qt_tabwidget_stackedwidget";
static const char* kStrMainTabWidget                    = "mainTabWidget";
static const char* kStrHomePage                          = "homePage";
static const char* kStrStackedWidget                     = "stackedWidget";
static const char* kStrCentralWidget                     = "centralWidget";
static const char* kStrRgMainWindow                     = "rgMainWindow";
static const char* kStrRgMainWindowTabBar             = "rgMainWindowTabBar";
static const char* kStrRgMainWindowMenuBar            = "menuBar";
static const char* kStrEnumComboPushButton             = "enumComboPushButton";
static const char* kStrPsoEditorAddElementButton      = "addElementButton";
static const char* kStrPsoEditorDeleteElementButton   = "deleteElementButton";
static const char* kStrPsoEditorLineEdit               = "lineEdit";
static const char* kStrFileMenuAddFileButton          = "addFilePushButton";
static const char* kStrBuildSettingsButtonName         = "buildSettingsButton";
static const char* kStrFileMenuNameGraphics            = "fileMenuGraphics";
static const char* kStrFileMenuItemNameGraphics       = "fileMenuItemGraphics";
static const char* kStrPsoEditorLoadButton             = "loadButton";
static const char* kStrPsoEditorSaveButton             = "saveButton";

QStringList RgHideListWidgetEventFilter::string_list_ =
{
    kStrColumnVisiblityArrowPushButton,
    kStrDisassemblyTargetGpuPushButton,
    kStrIsaListArrowPushButton,
    kStrQtScrollAreaViewport,
    kStrViewTitleBar,
    kStrViewOpenClSettingsView,
    kStrSettingsTab,
    kStrStartTab,
    kStrTabWidgetStackedWidget,
    kStrMainTabWidget,
    kStrHomePage,
    kStrStackedWidget,
    kStrCentralWidget,
    kStrRgMainWindow,
    kStrRgMainWindowTabBar,
    kStrRgMainWindowMenuBar,
    kStrEnumComboPushButton,
    kStrPsoEditorAddElementButton,
    kStrPsoEditorDeleteElementButton,
    kStrPsoEditorLineEdit,
    kStrFileMenuAddFileButton,
    kStrBuildSettingsButtonName,
    kStrFileMenuNameGraphics,
    kStrFileMenuItemNameGraphics,
    kStrPsoEditorLoadButton,
    kStrPsoEditorSaveButton,
};

RgHideListWidgetEventFilter::RgHideListWidgetEventFilter(QListWidget* list_widget, ArrowIconWidget* button) :
    QObject(list_widget),
    list_widget_(list_widget),
    button_(button)
{
}

bool RgHideListWidgetEventFilter::eventFilter(QObject* object, QEvent* event)
{
    assert(list_widget_ != nullptr);

    bool filtered = false;

    if (list_widget_ != nullptr)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            // If the user has clicked on a widget in the list and the object clicked on is not the pushbutton registered
            // with this object (since it will be cleaned up by its slot later), then hide the list widget.
            if (string_list_.contains(object->objectName()) && button_->objectName().compare(object->objectName()) != 0)
            {
                // Hide the list widget.
                list_widget_->hide();

                // Set the button icon to down arrow.
                button_->SetDirection(ArrowIconWidget::Direction::DownArrow);

                // Emit the list widget status changed signal.
                emit EnumListWidgetStatusSignal(false);
            }
        }
        else if (event->type() == QEvent::KeyPress && list_widget_->isVisible())
        {
            QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
            assert(key_event != nullptr);
            if (key_event != nullptr)
            {
                int current_row = list_widget_->currentRow();
                int key_pressed = key_event->key();
                if (key_pressed == Qt::Key_Down)
                {
                    if (current_row < list_widget_->count() - 1)
                    {
                        current_row++;
                    }
                }
                else if (key_pressed == Qt::Key_Up)
                {
                    if (current_row > 0)
                    {
                        current_row--;
                    }
                }
                else if (key_pressed == Qt::Key_Enter || key_pressed == Qt::Key_Return || key_pressed == Qt::Key_Space)
                {
                    // If this is the columns list widget, check/un-check the check box,
                    // else just emit the currentRowChanged signal.
                    if (list_widget_->objectName().compare(kStrDisassemblyTargetGpuList) == 0)
                    {
                        emit list_widget_->currentRowChanged(current_row);

                        // Close the list widget.
                        list_widget_->hide();

                        // Set the button icon to down arrow.
                        button_->SetDirection(ArrowIconWidget::Direction::DownArrow);

                        // Emit the list widget status changed signal.
                        emit EnumListWidgetStatusSignal(false);
                    }
                    else if (list_widget_->objectName().compare(kStrDisassemblyColumnVisibilityList) == 0)
                    {
                        // Toggle the check state in the ISA column visibility dropdown.
                        QListWidgetItem* item = list_widget_->currentItem();
                        if (item != nullptr)
                        {
                            QCheckBox* check_box = qobject_cast<QCheckBox*>(list_widget_->itemWidget(item));
                            if (check_box != nullptr)
                            {
                                if (check_box->isChecked())
                                {
                                    check_box->setCheckState(Qt::Unchecked);
                                }
                                else
                                {
                                    check_box->setCheckState(Qt::Checked);
                                }

                                emit check_box->clicked();
                            }
                        }
                    }
                }
                else if (key_pressed == Qt::Key_Tab)
                {
                    // Close the list widget.
                    list_widget_->hide();

                    // Set the button icon to down arrow.
                    button_->SetDirection(ArrowIconWidget::Direction::DownArrow);

                    // Emit the list widget status changed signal.
                    emit EnumListWidgetStatusSignal(false);

                    // Pop open the column list widget if this is gpu list widget,
                    // else if this is the column list widget, give focus to output window.
                    if (list_widget_->objectName().compare(kStrDisassemblyTargetGpuList) == 0)
                    {
                        emit OpenColumnListWidget();

                        // Update the RgIsaDisassemblyCustomTableView::current_SubWidget as well.
                        emit UpdateCurrentSubWidget(DisassemblyViewSubWidgets::kColumnPushButton);
                    }
                    else if (list_widget_->objectName().compare(kStrDisassemblyColumnVisibilityList) == 0)
                    {
                        emit FocusCliOutputWindow();

                        // Update the RgIsaDisassemblyCustomTableView::current_SubWidget as well.
                        emit UpdateCurrentSubWidget(DisassemblyViewSubWidgets::kOutputWindow);
                    }
                }
                else if (key_pressed == Qt::Key_Backtab)
                {
                    // Close the list widget.
                    list_widget_->hide();

                    // Set the button icon to down arrow.
                    button_->SetDirection(ArrowIconWidget::Direction::DownArrow);

                    // Emit the list widget status changed signal.
                    emit EnumListWidgetStatusSignal(false);

                    // Pop open the gpu list widget if this is column list widget,
                    // else if this is the gpu list widget, give focus to disassembly view window.
                    if (list_widget_->objectName().compare(kStrDisassemblyColumnVisibilityList) == 0)
                    {
                        emit OpenGpuListWidget();

                        // Update the RgIsaDisassemblyCustomTableView::current_SubWidget as well.
                        emit UpdateCurrentSubWidget(DisassemblyViewSubWidgets::kTargetGpuPushButton);
                    }
                    else if (list_widget_->objectName().compare(kStrDisassemblyTargetGpuList) == 0)
                    {
                        emit FrameInFocusSignal();

                        // Update the RgIsaDisassemblyCustomTableView::current_SubWidget as well.
                        emit UpdateCurrentSubWidget(DisassemblyViewSubWidgets::kTableView);
                    }
                }
                else if (key_pressed == Qt::Key_Escape)
                {
                    list_widget_->close();

                    // Set the button icon to down arrow.
                    button_->SetDirection(ArrowIconWidget::Direction::DownArrow);

                    // Emit the list widget status changed signal.
                    emit EnumListWidgetStatusSignal(false);
                }
                list_widget_->setCurrentRow(current_row);

                // Do not let Qt process this event any further.
                filtered = true;
            }
        }
    }
    return filtered;
}

void RgHideListWidgetEventFilter::AddObjectName(const QString& object_name)
{
    string_list_.push_back(object_name);
}

void RgHideListWidgetEventFilter::ClickPushButton()
{
    assert(button_ != nullptr);
    if (button_ != nullptr)
    {
        button_->clicked();
    }
}
