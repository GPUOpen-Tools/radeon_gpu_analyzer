#pragma once

// Qt.
#include <QTabBar>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgUnsavedItemsDialog.h>

// Forward declarations.
class QWidget;

/// Support for the custom Tab Bar.
class rgMainWindowTabBar : public QTabBar
{
    Q_OBJECT
public:
    explicit rgMainWindowTabBar(QWidget* pParent = nullptr);
    virtual ~rgMainWindowTabBar() = default;
    void SetTabEnabled(int index, bool enable);
    void SetSpacerIndex(int index);
    int SpacerIndex() const;
    int CalcSpacerWidth() const;
    void SetTabTool(int index, QWidget* pWidget);
    void UpdateBuildPendingChanges(bool pendingChanges);
    void UpdateApplicationPendingChanges(bool pendingChanges);
    void SetParentWidget(QWidget* pParent);
    rgUnsavedItemsDialog::UnsavedFileDialogResult SaveSettings();

protected:
    virtual void mouseMoveEvent(QMouseEvent* pEvent) override;
    virtual QSize tabSizeHint(int index) const override;
    virtual QSize minimumTabSizeHint(int index) const override;
    virtual void paintEvent(QPaintEvent* pEvent) override;
    virtual void mousePressEvent(QMouseEvent* pEvent) override;

signals:
    void SaveBuildSettingsChangesSignal(bool saveChanges);

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

    // Indicates that the user has build pending changes on Settings pane.
    bool m_hasBuildPendingChanges = false;

    // Indicates that the user has application pending changes on Settings pane.
    bool m_hasApplicationPendingChanges = false;

    // The parent widget to use when displaying the dialog box.
    QWidget* m_pParent;
};

