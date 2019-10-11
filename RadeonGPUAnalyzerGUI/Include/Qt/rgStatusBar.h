#pragma once

// C++.
#include <memory>

// Qt.
#include <QtWidgets/QWidget>
#include <QPushButton>
#include <QTreeWidget>

// Forward declarations.
class QMenu;
class rgAppState;
class rgRecentProjectWidget;
class QHBoxLayout;
class QStatusBar;
class rgModePushButton;
enum class rgProjectAPI : char;

class rgStatusBar : public QWidget
{
    Q_OBJECT

public:
    rgStatusBar(QStatusBar* pStatusBar = nullptr, QWidget* pParent = nullptr);
    virtual ~rgStatusBar() = default;

    // Set the visibility of API tree widget.
    void SetApiListVisibility(bool isVisible) const;

    // Set the visibility of the status bar.
    void SetStatusBarVisibility(bool isVisible);

signals:
    // Signal to indicate API change.
    void ChangeAPIModeSignal(rgProjectAPI api);

    // A signal to prompt saving any pending changes.
    void SavePendingChanges();

protected slots:
    // Handler for when the mode button is clicked.
    void HandleModePushButtonClicked(bool /*checked */);

    // Handler for when the user selects an API mode from the tree widget.
    void HandleTreeWidgetItemClicked(QTreeWidgetItem* pItem, const int column);

    // Handler for when the user hovers over an item in the tree widget.
    void HandleTreeWidgetItemEntered(QTreeWidgetItem* pItem, const int column);

private:
    // Create Mode push button.
    void CreateModeButton();

    // Create API push buttons.
    void CreateApiTreeWidget();

    // Initialize the API tree widget.
    void InitializeTreeWidget();

    // Set style sheets for mode and API push buttons.
    virtual void SetStylesheets(QStatusBar* pStatusBar) = 0;

    // Set widget dimensions.
    void SetDimensions();

    // Connect signals.
    void ConnectSignals() const;

    // Set the cursor to pointing hand cursor.
    void SetCursor() const;

    // Show a confirmation dialog box close to the status bar.
    bool ShowConfirmationDialogBox(const char* pMessage) const;

    // Place the current mode at the top.
    void ReorderCurrentMode() const;

    // Add the check box icon to the current API row.
    void AddCheckBoxIcon(QTreeWidgetItem* pItem) const;

protected:
    // Re-implement event filter.
    virtual bool eventFilter(QObject* pObject, QEvent* pEvent) override;

    // The mode push button.
    rgModePushButton* m_pModePushButton = nullptr;

    // A tree widget to show API modes.
    QTreeWidget* m_pApiModeTreeWidget = nullptr;

    // A handle to the status bar.
    QStatusBar* m_pStatusBar = nullptr;

    // The parent widget.
    QWidget* m_pParent = nullptr;

    // The horizontal layout for app notification label.
    QHBoxLayout* m_pHLayout = nullptr;
};