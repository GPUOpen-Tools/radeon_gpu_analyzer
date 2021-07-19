#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MAIN_WINDOW_TAB_BAR_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MAIN_WINDOW_TAB_BAR_H_

// Qt.
#include <QTabBar>

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_unsaved_items_dialog.h"

// Forward declarations.
class QWidget;
class RgSettingsTab;

/// Support for the custom Tab Bar.
class RgMainWindowTabBar : public QTabBar
{
    Q_OBJECT
public:
    // Constructor
    explicit RgMainWindowTabBar(QWidget* parent = nullptr);

    // Default destructor
    virtual ~RgMainWindowTabBar() = default;

    // Enable / Disable a specific tab.
    void SetTabEnabled(int index, bool enable);

    // Set the index of a spacer tab.
    void SetSpacerIndex(int index);

    // Get the index of the spacer tab.
    int SpacerIndex() const;

    // Calculate the width of the spacer tab.
    int CalcSpacerWidth() const;

    // Set the button widget for the tab.
    void SetTabTool(int index, QWidget* widget);

    // Set the parent widget so that dialogs are centered correctly.
    void SetParentWidget(QWidget* parent);

    // Set the settings tab.
    void SetSettingsTab(RgSettingsTab* settings_tab);

protected:
    // Override mouseMoveEvent to customize cursor on hover.
    virtual void mouseMoveEvent(QMouseEvent* event) override;

    // Override tabSizeHint to account for tab button size.
    virtual QSize tabSizeHint(int index) const override;

    // Override minimumTabSizeHint to allow Spacer Tab to shrink.
    virtual QSize minimumTabSizeHint(int index) const override;

    // Override paintEvent to draw a line above the tab.
    virtual void paintEvent(QPaintEvent* event) override;

    // Override mousePressEvent to prompt to save settings if user
    // is switching away from the settings tab.
    virtual void mousePressEvent(QMouseEvent* event) override;

    // Override eventFilter to prompt to save settings if user
    // is using a keyboard combo to switch away from the settings tab.
    virtual bool eventFilter(QObject* object, QEvent* event) override;

private:
    enum TabItem
    {
        kStart,
        kSettings
    };

    // Index of the hidden tab between the left justified tabs and the right justified tabs.
    int spacer_index_;

    // The index of the tab that the mouse was hovering over when the last mouseMoveEvent was processed.
    int mouse_hover_last_tab_index_;

    // The parent widget to use when displaying the dialog box.
    QWidget* parent_;

    // The settings tab widget that will prompt to save the settings when being switched away from.
    RgSettingsTab* settings_tab_ = nullptr;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MAIN_WINDOW_TAB_BAR_H_

