#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_STATUS_BAR_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_STATUS_BAR_H_

// C++.
#include <memory>

// Qt.
#include <QtWidgets/QWidget>
#include <QPushButton>
#include <QTreeWidget>

// Forward declarations.
class QMenu;
class RgAppState;
class RgRecentProjectWidget;
class QHBoxLayout;
class QStatusBar;
class RgModePushButton;
enum class RgProjectAPI : char;

class RgStatusBar : public QWidget
{
    Q_OBJECT

public:
    RgStatusBar(QStatusBar* status_bar = nullptr, QWidget* parent = nullptr);
    virtual ~RgStatusBar() = default;

    // Set the visibility of API tree widget.
    void SetApiListVisibility(bool is_visible) const;

    // Set the visibility of the status bar.
    void SetStatusBarVisibility(bool is_visible);

    // Get the mode push button.
    RgModePushButton* GetModePushButton();

    // Get the api list tree widget.
    QTreeWidget* GetApiListTreeWidget();

    // Status Bar Status - BEGIN.
    enum class StatusType
    {
        kUnknown,
        kStarted,
        kFailed,
        kCanceled,
        kSucceeded
    };

    // Returns status message string.
    virtual bool ConstructStatusMessageString(StatusType type, std::string& status_msg_str) const;

signals:
    // Signal to indicate API change.
    void ChangeAPIModeSignal(RgProjectAPI api);

    // A signal to prompt saving any pending changes.
    void SavePendingChanges();

public slots:
    // Handler for when the user selects an API mode from the tree widget.
    void HandleTreeWidgetItemClicked(QTreeWidgetItem* item, const int column);

protected slots:
    // Handler for when the mode button is clicked.
    void HandleModePushButtonClicked(bool /*checked */);

    // Handler for when the user hovers over an item in the tree widget.
    void HandleTreeWidgetItemEntered(QTreeWidgetItem* item, const int column);

private:
    // Create Mode push button.
    void CreateModeButton();

    // Create API push buttons.
    void CreateApiTreeWidget();

    // Initialize the API tree widget.
    void InitializeTreeWidget();

    // Set style sheets for mode and API push buttons.
    virtual void SetStylesheets(QStatusBar* status_bar) = 0;

    // Set widget dimensions.
    void SetDimensions();

    // Connect signals.
    void ConnectSignals() const;

    // Set the cursor to pointing hand cursor.
    void SetCursor() const;

    // Show a confirmation dialog box close to the status bar.
    bool ShowConfirmationDialogBox(const char* message) const;

    // Place the current mode at the top.
    void ReorderCurrentMode() const;

    // Add the check box icon to the current API row.
    void AddCheckBoxIcon(QTreeWidgetItem* item) const;

protected:
    // Re-implement event filter.
    virtual bool eventFilter(QObject* object, QEvent* event) override;

    // The mode push button.
    RgModePushButton* mode_push_button_ = nullptr;

    // A tree widget to show API modes.
    QTreeWidget* api_mode_tree_widget_ = nullptr;

    // A handle to the status bar.
    QStatusBar* status_bar_ = nullptr;

    // The parent widget.
    QWidget* parent_ = nullptr;

    // The horizontal layout for app notification label.
    QHBoxLayout* h_layout_ = nullptr;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_STATUS_BAR_H_
