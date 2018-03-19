// Qt.
#include <QListWidget>
#include <QObject>
#include <QEvent>

// Infra.
#include <QtCommon/CustomWidgets/ArrowIconWidget.h>
#include <QtCommon/Util/CommonDefinitions.h>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgHideListWidgetEventFilter.h>

// The object names for UI objects that the user can click on.
static const char* STR_COLUMN_VISIBLITY_ARROW_PUSH_BUTTON   = "columnVisibilityArrowPushButton";
static const char* STR_DISASSEMBLY_TARGET_GPU_PUSH_BUTTON   = "targetGpuPushButton";
static const char* STR_ISA_LIST_ARROW_PUSH_BUTTON           = "isaListArrowPushButton";
static const char* STR_QT_SCROLL_AREA_VIEWPORT              = "qt_scrollarea_viewport";
static const char* STR_VIEW_TITLE_BAR                       = "viewTitlebar";
static const char* STR_VIEW_OPEN_CL_SETTINGS_VIEW           = "rgOpenClSettingsView";
static const char* STR_SETTINGS_TAB                         = "settingsTab";
static const char* STR_START_TAB                            = "startTab";
static const char* STR_TAB_WIDGET_STACKED_WIDGET            = "qt_tabwidget_stackedwidget";
static const char* STR_MAIN_TAB_WIDGET                      = "mainTabWidget";
static const char* STR_HOME_PAGE                            = "homePage";
static const char* STR_STACKED_WIDGET                       = "stackedWidget";
static const char* STR_CENTRAL_WIDGET                       = "centralWidget";
static const char* STR_RG_MAIN_WINDOW                       = "rgMainWindow";
static const char* STR_RG_MAIN_WINDOW_TAB_BAR               = "rgMainWindowTabBar";
static const char* STR_RG_MAIN_WINDOW_MENU_BAR              = "menuBar";

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
};

rgHideListWidgetEventFilter::rgHideListWidgetEventFilter(QListWidget* pListWidget, ArrowIconWidget* pButton) :
    QObject(pListWidget),
    m_pListWidget(pListWidget),
    m_pButton(pButton)
{
}

bool rgHideListWidgetEventFilter::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        // if the user has clicked on a widget in the list and the object clicked on is not the pushbutton registered
        // with this object (since it will be cleaned up by its slot later), then hide the list widget
        if (m_stringList.contains(object->objectName()) && m_pButton->objectName().compare(object->objectName()) != 0)
        {
            // Hide the list widget.
            m_pListWidget->hide();

            // Set the button icon to down arrow.
            m_pButton->SetDirection(ArrowIconWidget::Direction::DownArrow);
        }
    }
    return false;
}