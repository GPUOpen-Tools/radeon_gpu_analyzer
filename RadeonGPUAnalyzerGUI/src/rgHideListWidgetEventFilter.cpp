// C++.
#include <cassert>

// Qt.
#include <QCheckBox>
#include <QEvent>
#include <QKeyEvent>
#include <QListWidget>
#include <QObject>

// Infra.
#include <QtCommon/CustomWidgets/ArrowIconWidget.h>
#include <QtCommon/Util/CommonDefinitions.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgHideListWidgetEventFilter.h>

// Object name associated with the column dropdown list.
static const char* STR_DISASSEMBLY_COLUMN_VISIBILITY_LIST = "DisassemblyColumnVisibilityList";

// Object name associated with the target GPU dropdown list.
static const char* STR_DISASSEMBLY_TARGET_GPU_LIST = "TargetGpuList";

// The object names for UI objects that the user can click on.
static const char* STR_COLUMN_VISIBLITY_ARROW_PUSH_BUTTON = "columnVisibilityArrowPushButton";
static const char* STR_DISASSEMBLY_TARGET_GPU_PUSH_BUTTON = "targetGpuPushButton";
static const char* STR_ISA_LIST_ARROW_PUSH_BUTTON         = "isaListArrowPushButton";
static const char* STR_QT_SCROLL_AREA_VIEWPORT            = "qt_scrollarea_viewport";
static const char* STR_VIEW_TITLE_BAR                     = "viewTitlebar";
static const char* STR_VIEW_OPEN_CL_SETTINGS_VIEW         = "rgOpenClSettingsView";
static const char* STR_SETTINGS_TAB                       = "settingsTab";
static const char* STR_START_TAB                          = "startTab";
static const char* STR_TAB_WIDGET_STACKED_WIDGET          = "qt_tabwidget_stackedwidget";
static const char* STR_MAIN_TAB_WIDGET                    = "mainTabWidget";
static const char* STR_HOME_PAGE                          = "homePage";
static const char* STR_STACKED_WIDGET                     = "stackedWidget";
static const char* STR_CENTRAL_WIDGET                     = "centralWidget";
static const char* STR_RG_MAIN_WINDOW                     = "rgMainWindow";
static const char* STR_RG_MAIN_WINDOW_TAB_BAR             = "rgMainWindowTabBar";
static const char* STR_RG_MAIN_WINDOW_MENU_BAR            = "menuBar";
static const char* STR_ENUM_COMBO_PUSH_BUTTON             = "enumComboPushButton";
static const char* STR_PSO_EDITOR_ADD_ELEMENT_BUTTON      = "addElementButton";
static const char* STR_PSO_EDITOR_DELETE_ELEMENT_BUTTON   = "deleteElementButton";
static const char* STR_PSO_EDITOR_LINE_EDIT               = "lineEdit";
static const char* STR_FILE_MENU_ADD_FILE_BUTTON          = "addFilePushButton";
static const char* STR_BUILD_SETTINGS_BUTTON_NAME         = "buildSettingsButton";
static const char* STR_FILE_MENU_NAME_GRAPHICS            = "fileMenuGraphics";
static const char* STR_FILE_MENU_ITEM_NAME_GRAPHICS       = "fileMenuItemGraphics";
static const char* STR_PSO_EDITOR_LOAD_BUTTON             = "loadButton";
static const char* STR_PSO_EDITOR_SAVE_BUTTON             = "saveButton";

QStringList rgHideListWidgetEventFilter::m_stringList =
{
    STR_COLUMN_VISIBLITY_ARROW_PUSH_BUTTON,
    STR_DISASSEMBLY_TARGET_GPU_PUSH_BUTTON,
    STR_ISA_LIST_ARROW_PUSH_BUTTON,
    STR_QT_SCROLL_AREA_VIEWPORT,
    STR_VIEW_TITLE_BAR,
    STR_VIEW_OPEN_CL_SETTINGS_VIEW,
    STR_SETTINGS_TAB,
    STR_START_TAB,
    STR_TAB_WIDGET_STACKED_WIDGET,
    STR_MAIN_TAB_WIDGET,
    STR_HOME_PAGE,
    STR_STACKED_WIDGET,
    STR_CENTRAL_WIDGET,
    STR_RG_MAIN_WINDOW,
    STR_RG_MAIN_WINDOW_TAB_BAR,
    STR_RG_MAIN_WINDOW_MENU_BAR,
    STR_ENUM_COMBO_PUSH_BUTTON,
    STR_PSO_EDITOR_ADD_ELEMENT_BUTTON,
    STR_PSO_EDITOR_DELETE_ELEMENT_BUTTON,
    STR_PSO_EDITOR_LINE_EDIT,
    STR_FILE_MENU_ADD_FILE_BUTTON,
    STR_BUILD_SETTINGS_BUTTON_NAME,
    STR_FILE_MENU_NAME_GRAPHICS,
    STR_FILE_MENU_ITEM_NAME_GRAPHICS,
    STR_PSO_EDITOR_LOAD_BUTTON,
    STR_PSO_EDITOR_SAVE_BUTTON,
};

rgHideListWidgetEventFilter::rgHideListWidgetEventFilter(QListWidget* pListWidget, ArrowIconWidget* pButton) :
    QObject(pListWidget),
    m_pListWidget(pListWidget),
    m_pButton(pButton)
{
}

bool rgHideListWidgetEventFilter::eventFilter(QObject* pObject, QEvent* pEvent)
{
    assert(m_pListWidget != nullptr);

    bool filtered = false;

    if (m_pListWidget != nullptr)
    {
        if (pEvent->type() == QEvent::MouseButtonPress)
        {
            // If the user has clicked on a widget in the list and the object clicked on is not the pushbutton registered
            // with this object (since it will be cleaned up by its slot later), then hide the list widget.
            if (m_stringList.contains(pObject->objectName()) && m_pButton->objectName().compare(pObject->objectName()) != 0)
            {
                // Hide the list widget.
                m_pListWidget->hide();

                // Set the button icon to down arrow.
                m_pButton->SetDirection(ArrowIconWidget::Direction::DownArrow);

                // Emit the list widget status changed signal.
                emit EnumListWidgetStatusSignal(false);
            }
        }
        else if (pEvent->type() == QEvent::KeyPress && m_pListWidget->isVisible())
        {
            QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(pEvent);
            assert(pKeyEvent != nullptr);
            if (pKeyEvent != nullptr)
            {
                int currentRow = m_pListWidget->currentRow();
                int keyPressed = pKeyEvent->key();
                if (keyPressed == Qt::Key_Down)
                {
                    if (currentRow < m_pListWidget->count() - 1)
                    {
                        currentRow++;
                    }
                }
                else if (keyPressed == Qt::Key_Up)
                {
                    if (currentRow > 0)
                    {
                        currentRow--;
                    }
                }
                else if (keyPressed == Qt::Key_Enter || keyPressed == Qt::Key_Return || keyPressed == Qt::Key_Space)
                {
                    // If this is the columns list widget, check/un-check the check box,
                    // else just emit the currentRowChanged signal.
                    if (m_pListWidget->objectName().compare(STR_DISASSEMBLY_TARGET_GPU_LIST) == 0)
                    {
                        emit m_pListWidget->currentRowChanged(currentRow);

                        // Close the list widget.
                        m_pListWidget->hide();

                        // Set the button icon to down arrow.
                        m_pButton->SetDirection(ArrowIconWidget::Direction::DownArrow);

                        // Emit the list widget status changed signal.
                        emit EnumListWidgetStatusSignal(false);
                    }
                    else if (m_pListWidget->objectName().compare(STR_DISASSEMBLY_COLUMN_VISIBILITY_LIST) == 0)
                    {
                        // Toggle the check state in the ISA column visibility dropdown.
                        QListWidgetItem* pItem = m_pListWidget->currentItem();
                        if (pItem != nullptr)
                        {
                            QCheckBox* pCheckBox = qobject_cast<QCheckBox*>(m_pListWidget->itemWidget(pItem));
                            if (pCheckBox != nullptr)
                            {
                                if (pCheckBox->isChecked())
                                {
                                    pCheckBox->setCheckState(Qt::Unchecked);
                                }
                                else
                                {
                                    pCheckBox->setCheckState(Qt::Checked);
                                }

                                emit pCheckBox->clicked();
                            }
                        }
                    }
                }
                else if (keyPressed == Qt::Key_Tab)
                {
                    // Close the list widget.
                    m_pListWidget->hide();

                    // Set the button icon to down arrow.
                    m_pButton->SetDirection(ArrowIconWidget::Direction::DownArrow);

                    // Emit the list widget status changed signal.
                    emit EnumListWidgetStatusSignal(false);

                    // Pop open the column list widget if this is gpu list widget,
                    // else if this is the column list widget, give focus to output window.
                    if (m_pListWidget->objectName().compare(STR_DISASSEMBLY_TARGET_GPU_LIST) == 0)
                    {
                        emit OpenColumnListWidget();

                        // Update the rgIsaDisassemblyCustomTableView::m_currentSubWidget as well.
                        emit UpdateCurrentSubWidget(DisassemblyViewSubWidgets::ColumnPushButton);
                    }
                    else if (m_pListWidget->objectName().compare(STR_DISASSEMBLY_COLUMN_VISIBILITY_LIST) == 0)
                    {
                        emit FocusCliOutputWindow();

                        // Update the rgIsaDisassemblyCustomTableView::m_currentSubWidget as well.
                        emit UpdateCurrentSubWidget(DisassemblyViewSubWidgets::OutputWindow);
                    }
                }
                else if (keyPressed == Qt::Key_Backtab)
                {
                    // Close the list widget.
                    m_pListWidget->hide();

                    // Set the button icon to down arrow.
                    m_pButton->SetDirection(ArrowIconWidget::Direction::DownArrow);

                    // Emit the list widget status changed signal.
                    emit EnumListWidgetStatusSignal(false);

                    // Pop open the gpu list widget if this is column list widget,
                    // else if this is the gpu list widget, give focus to disassembly view window.
                    if (m_pListWidget->objectName().compare(STR_DISASSEMBLY_COLUMN_VISIBILITY_LIST) == 0)
                    {
                        emit OpenGpuListWidget();

                        // Update the rgIsaDisassemblyCustomTableView::m_currentSubWidget as well.
                        emit UpdateCurrentSubWidget(DisassemblyViewSubWidgets::TargetGpuPushButton);
                    }
                    else if (m_pListWidget->objectName().compare(STR_DISASSEMBLY_TARGET_GPU_LIST) == 0)
                    {
                        emit FrameInFocusSignal();

                        // Update the rgIsaDisassemblyCustomTableView::m_currentSubWidget as well.
                        emit UpdateCurrentSubWidget(DisassemblyViewSubWidgets::TableView);
                    }
                }
                else if (keyPressed == Qt::Key_Escape)
                {
                    m_pListWidget->close();

                    // Set the button icon to down arrow.
                    m_pButton->SetDirection(ArrowIconWidget::Direction::DownArrow);

                    // Emit the list widget status changed signal.
                    emit EnumListWidgetStatusSignal(false);
                }
                m_pListWidget->setCurrentRow(currentRow);

                // Do not let Qt process this event any further.
                filtered = true;
            }
        }
    }
    return filtered;
}

void rgHideListWidgetEventFilter::AddObjectName(const QString& objectName)
{
    m_stringList.push_back(objectName);
}

void rgHideListWidgetEventFilter::ClickPushButton()
{
    assert(m_pButton != nullptr);
    if (m_pButton != nullptr)
    {
        m_pButton->clicked();
    }
}