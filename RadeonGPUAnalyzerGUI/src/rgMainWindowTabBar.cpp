// C++.
#include <cassert>

// Qt.
#include <QTabBar>
#include <QMouseEvent>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QPainter>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgMainWindowTabBar.h>
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgUnsavedItemsDialog.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>

// Infra.
#include <QtCommon/Scaling/ScalingManager.h>

rgMainWindowTabBar::rgMainWindowTabBar(QWidget* pParent) :
    QTabBar(pParent),
    m_spacerIndex(-1),
    m_mouseHoverLastTabIndex(-1),
    m_pParent(pParent)
{
    setMouseTracking(true);
    setObjectName("rgMainWindowTabBar");
}

void rgMainWindowTabBar::mouseMoveEvent(QMouseEvent* pEvent)
{
    int tabIndex = QTabBar::tabAt(pEvent->pos());

    // Only change mouse cursor if the mouse is hovering over a different tab.
    if (m_mouseHoverLastTabIndex != tabIndex)
    {
        if (isTabEnabled(tabIndex))
        {
            setCursor(Qt::PointingHandCursor);
        }
        else
        {
            setCursor(Qt::ArrowCursor);
        }
        m_mouseHoverLastTabIndex = tabIndex;
    }
}

void rgMainWindowTabBar::mousePressEvent(QMouseEvent* pEvent)
{
    int tabIndex = QTabBar::tabAt(pEvent->pos());

    // Check if the user is switching to the "START" tab.
    switch (tabIndex)
    {
    case (TabItem::Start):
    {
        if (m_hasBuildPendingChanges || m_hasApplicationPendingChanges)
        {
            rgUnsavedItemsDialog::UnsavedFileDialogResult result = SaveSettings();

            switch (result)
            {
            case rgUnsavedItemsDialog::Yes:
                // Save all data.
                emit SaveBuildSettingsChangesSignal(true);

                // Let the tab switch occur.
                // Pass the event onto the base class.
                QTabBar::mousePressEvent(pEvent);

                break;
            case rgUnsavedItemsDialog::No:
                // Do not save the data
                emit SaveBuildSettingsChangesSignal(false);

                // Let the tab switch occur.
                // Pass the event onto the base class.
                QTabBar::mousePressEvent(pEvent);

                break;
            case rgUnsavedItemsDialog::Cancel:
                // Do not save anything if the user hit the cancel button.
                // Also, do not change the tab.
                break;
            default:
                // Shouldn't get here.
                assert(false);
            }
        }
        else
        {
            // Let the tab switch occur.
            // Pass the event onto the base class.
            QTabBar::mousePressEvent(pEvent);
        }
        break;
    }
    case (TabItem::Settings):
    {
        // The user is switching to settings tab, allow it.
        QTabBar::mousePressEvent(pEvent);
        break;
    }
    default:
    {
        // Should not get here.
        assert(false);
    }
    }
}

void rgMainWindowTabBar::SetTabEnabled(int index, bool enable)
{
    // Force next mouseMoveEvent to set mouse cursor even if mouse is hovering over
    // the same tab as the last event call.
    m_mouseHoverLastTabIndex = -1;

    QTabBar::setTabEnabled(index, enable);
}

QSize rgMainWindowTabBar::minimumTabSizeHint(int index) const
{
    if (index == SpacerIndex() || tabText(index).isEmpty())
    {
        return QSize(0, QTabBar::tabSizeHint(index).height());
    }
    else
    {
        return QTabBar::minimumTabSizeHint(index);
    }
}

QSize rgMainWindowTabBar::tabSizeHint(int index) const
{
    int height = QTabBar::tabSizeHint(index).height();
    if (index == SpacerIndex())
    {
        return QSize(CalcSpacerWidth(), height);
    }
    else if (tabText(index).isEmpty())
    {
        int width = 0;
        QWidget* pWidget = tabButton(index, QTabBar::ButtonPosition::LeftSide);
        if (pWidget)
        {
            width += pWidget->width();
        }

        pWidget = tabButton(index, QTabBar::ButtonPosition::RightSide);
        if (pWidget)
        {
            width += pWidget->width();
        }
        return QSize(width, height);
    }
    else
    {
        return QSize(QTabBar::tabSizeHint(index).width(), height);
    }
}

void rgMainWindowTabBar::paintEvent(QPaintEvent* pEvent)
{
    QTabBar::paintEvent(pEvent);
    if (count() > 0)
    {
        QPainter painter;
        painter.begin(this);
        painter.setPen(tabTextColor(0));
        painter.drawLine(pEvent->rect().topLeft(), pEvent->rect().topRight());
        painter.end();
    }
}

void rgMainWindowTabBar::SetSpacerIndex(int index)
{
    if (index != -1)
    {
        setTabEnabled(index, false);
        setTabText(index, "");
    }
    m_spacerIndex = index;
}

void rgMainWindowTabBar::SetTabTool(int index, QWidget* pWidget)
{
    setTabText(index, "");
    setTabEnabled(index, false);
    setTabButton(index, QTabBar::LeftSide, pWidget);
}

int rgMainWindowTabBar::SpacerIndex() const
{
    return m_spacerIndex;
}

int rgMainWindowTabBar::CalcSpacerWidth() const
{
    int spacerWidth = -1;

    if ( (count() == 0) || (m_spacerIndex < 0) )
    {
        spacerWidth = 0;
    }
    else
    {
        // Figure out the length of the spacer between the last left tab
        // and the right tab.
        QRect leftTabRect;
        int tabMargin = contentsMargins().left();
        if (count() > 1)
        {
            leftTabRect = tabRect(m_spacerIndex - 1);
            tabMargin = ScalingManager::Get().Scaled(4);
        }

        int rightTabsWidth = leftTabRect.width() * (count() - (m_spacerIndex + 1));
        spacerWidth = parentWidget()->width() - (leftTabRect.right() + rightTabsWidth);
        spacerWidth -= (tabMargin * 2);
    }

    return spacerWidth;
}

void rgMainWindowTabBar::UpdateApplicationPendingChanges(bool pendingChanges)
{
    m_hasApplicationPendingChanges = pendingChanges;
}

void rgMainWindowTabBar::UpdateBuildPendingChanges(bool pendingChanges)
{
    m_hasBuildPendingChanges = pendingChanges;
}

void rgMainWindowTabBar::SetParentWidget(QWidget* pParent)
{
    m_pParent = pParent;
}

rgUnsavedItemsDialog::UnsavedFileDialogResult rgMainWindowTabBar::SaveSettings()
{
    rgUnsavedItemsDialog::UnsavedFileDialogResult result = rgUnsavedItemsDialog::No;

    if (m_hasBuildPendingChanges || m_hasApplicationPendingChanges)
    {
        // Create a modal unsaved file dialog.
        rgUnsavedItemsDialog* pUnsavedChangesDialog = new rgUnsavedItemsDialog(this);
        pUnsavedChangesDialog->setModal(true);
        pUnsavedChangesDialog->setWindowTitle(STR_UNSAVED_ITEMS_DIALOG_TITLE);

        // Add a message string to the dialog list.
        if (m_hasApplicationPendingChanges)
        {
            pUnsavedChangesDialog->AddFile(STR_SETTINGS_CONFIRMATION_APPLICATION_SETTINGS);
        }
        if (m_hasBuildPendingChanges)
        {
            pUnsavedChangesDialog->AddFile(STR_DEFAULT_OPENCL_BUILD_SETTINGS);
        }

        // Register the dialog with the scaling manager.
        ScalingManager::Get().RegisterObject(pUnsavedChangesDialog);

        // Center the dialog on the view (registering with the scaling manager
        // shifts it out of the center so we need to manually center it).
        rgUtils::CenterOnWidget(pUnsavedChangesDialog, m_pParent);

        // Execute the dialog and get the result.
        result = static_cast<rgUnsavedItemsDialog::UnsavedFileDialogResult>(pUnsavedChangesDialog->exec());
    }

    return result;
}
