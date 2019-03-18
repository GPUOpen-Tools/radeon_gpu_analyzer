#pragma once

// Qt.
#include <QTabBar>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgUnsavedItemsDialog.h>

// Forward declarations.
class QWidget;
class rgSettingsTab;

/// Support for the custom Tab Bar.
class rgMainWindowTabBar : public QTabBar
{
    Q_OBJECT
public:
    // Constructor
    explicit rgMainWindowTabBar(QWidget* pParent = nullptr);

    // Default destructor
    virtual ~rgMainWindowTabBar() = default;

    // Enable / Disable a specific tab.
    void SetTabEnabled(int index, bool enable);

    // Set the index of a spacer tab.
    void SetSpacerIndex(int index);

    // Get the index of the spacer tab.
    int SpacerIndex() const;

    // Calculate the width of the spacer tab.
    int CalcSpacerWidth() const;

    // Set the button widget for the tab.
    void SetTabTool(int index, QWidget* pWidget);

    // Set the parent widget so that dialogs are centered correctly.
    void SetParentWidget(QWidget* pParent);

    // Set the settings tab.
    void SetSettingsTab(rgSettingsTab* pSettingsTab);

protected:
    // Override mouseMoveEvent to customize cursor on hover.
    virtual void mouseMoveEvent(QMouseEvent* pEvent) override;

    // Override tabSizeHint to account for tab button size.
    virtual QSize tabSizeHint(int index) const override;

    // Override minimumTabSizeHint to allow Spacer Tab to shrink.
    virtual QSize minimumTabSizeHint(int index) const override;

    // Override paintEvent to draw a line above the tab.
    virtual void paintEvent(QPaintEvent* pEvent) override;

    // Override mousePressEvent to prompt to save settings if user
    // is switching away from the settings tab.
    virtual void mousePressEvent(QMouseEvent* pEvent) override;

    // Override eventFilter to prompt to save settings if user
    // is using a keyboard combo to switch away from the settings tab.
    virtual bool eventFilter(QObject* pObject, QEvent* pEvent) override;

private:
    enum TabItem
    {
        Start,
        Settings
    };

    // Index of the hidden tab between the left justified tabs and the right justified tabs.
    int m_spacerIndex;

    // The index of the tab that the mouse was hovering over when the last mouseMoveEvent was processed.
    int m_mouseHoverLastTabIndex;

    // The parent widget to use when displaying the dialog box.
    QWidget* m_pParent;

    // The settings tab widget that will prompt to save the settings when being switched away from.
    rgSettingsTab* m_pSettingsTab = nullptr;
};

