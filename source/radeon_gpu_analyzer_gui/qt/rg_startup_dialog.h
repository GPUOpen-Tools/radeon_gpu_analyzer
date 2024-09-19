#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_STARTUP_DIALOG_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_STARTUP_DIALOG_H_

#include <QDialog>
#include "source/radeon_gpu_analyzer_gui/rg_data_types.h"

#include "ui_rg_startup_dialog.h"

class RgStartupDialog : public QDialog
{
    Q_OBJECT

public:
    // Constructor.
    explicit RgStartupDialog(QWidget* parent = nullptr);

    // Default destructor.
    virtual ~RgStartupDialog() = default;

    // Expose the selected API.
    RgProjectAPI SelectedApi() const;

    // Expose whether the user does not want to be prompted again.
    bool ShouldNotAskAgain() const;

    // Override the keyPressEvent.
    virtual void keyPressEvent(QKeyEvent* event) override;

private slots:
    // Handler for when the Exit button is clicked.
    void HandleExitButtonClicked(bool /* checked */);

    // Handler for when the Start RGA button is clicked.
    void HandleStartRGAButtonClicked(bool /* checked */);

    // Handler for when the list widget item is clicked.
    void HandleListWidgetItemClicked(QListWidgetItem* item);

    // Handler for when the list widget item is double clicked.
    void HandleListWidgetItemDoubleClicked(QListWidgetItem* item);

    // Handler for when the user selects the API by using up/down arrow keys.
    void HandleListWidgetItemSelected(int current_row);

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    /// @brief Handle Color scheme changed in the OS.
    ///
    /// color_scheme The color scheme selected by the OS.
    void HandleOsColorSchemeChanged(Qt::ColorScheme color_scheme);
#endif

private:
    // Set the cursor to pointing hand cursor.
    void SetCursor() const;

    // Connect signals.
    void ConnectSignals() const;

    // Set the minimum length of description field.
    void SetDescriptionLength();

    // Scale this dialog.
    void ScaleDialog();

protected:
    // The Ui elements.
    Ui::RgStartupDialog ui_;

    // Store the selected API.
    RgProjectAPI selected_api_;

    // Expose whether the user opted to not prompt for an API again.
    bool should_not_ask_again_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_STARTUP_DIALOG_H_
