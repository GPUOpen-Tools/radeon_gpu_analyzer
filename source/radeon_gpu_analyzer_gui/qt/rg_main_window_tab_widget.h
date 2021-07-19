#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MAIN_WINDOW_TAB_WIDGET_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MAIN_WINDOW_TAB_WIDGET_H_

// Qt.
#include <QTabWidget>

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_main_window_tab_bar.h"

/// Support for the custom Tab Widget.
class RgMainWindowTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit RgMainWindowTabWidget(QWidget* parent = nullptr);
    virtual ~RgMainWindowTabWidget() {};
    void SetTabEnabled(int index, bool);
    void SetSpacerIndex(const int index);
    void SetTabTool(int index, QWidget* tool_widget);
    int TabHeight() const;
    RgMainWindowTabBar* GetTabBar();

protected:
    // Get a pointer to the tab bar.
    QTabBar* tabBar() const;

    // The overridden resizeEvent.
    virtual void resizeEvent(QResizeEvent* resize_event) override;

private:
    // Custom tab bar.
    RgMainWindowTabBar* tab_bar_ = nullptr;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MAIN_WINDOW_TAB_WIDGET_H_
