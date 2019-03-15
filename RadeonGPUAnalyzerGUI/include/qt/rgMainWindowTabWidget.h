#pragma once

// Qt.
#include <QTabWidget>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMainWindowTabBar.h>

/// Support for the custom Tab Widget.
class rgMainWindowTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit rgMainWindowTabWidget(QWidget* pParent = nullptr);
    virtual ~rgMainWindowTabWidget() {};
    void SetTabEnabled(int index, bool);
    void SetSpacerIndex(const int index);
    void SetTabTool(int index, QWidget* pToolWidget);
    int TabHeight() const;
    rgMainWindowTabBar* GetTabBar();

protected:
    // Get a pointer to the tab bar.
    QTabBar* tabBar() const;

    // The overridden resizeEvent.
    virtual void resizeEvent(QResizeEvent* pResizeEvent) override;

private:
    // Custom tab bar.
    rgMainWindowTabBar* m_pTabBar = nullptr;
};
