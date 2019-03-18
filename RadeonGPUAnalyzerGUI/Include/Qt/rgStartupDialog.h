#pragma once

#include <QDialog>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>

#include "ui_rgStartupDialog.h"

class rgStartupDialog : public QDialog
{
    Q_OBJECT

public:
    // Constructor.
    explicit rgStartupDialog(QWidget* pParent = nullptr);

    // Default destructor.
    virtual ~rgStartupDialog() = default;

    // Expose the selected API.
    rgProjectAPI SelectedApi() const;

    // Expose whether the user does not want to be prompted again.
    bool ShouldNotAskAgain() const;

    // Override the keyPressEvent.
    virtual void keyPressEvent(QKeyEvent* pEvent) override;

private slots:
    // Handler for when the Exit button is clicked.
    void HandleExitButtonClicked(bool /* checked */);

    // Handler for when the Start RGA button is clicked.
    void HandleStartRGAButtonClicked(bool /* checked */);

    // Handler for when the list widget item is clicked.
    void HandleListWidgetItemClicked(QListWidgetItem* pItem);

    // Handler for when the list widget item is double clicked.
    void HandleListWidgetItemDoubleClicked(QListWidgetItem* pItem);

    // Handler for when the user selects the API by using up/down arrow keys.
    void HandleListWidgetItemSelected(int currentRow);

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
    Ui::rgStartupDialog ui;

    // Store the selected API.
    rgProjectAPI m_selectedApi;

    // Expose whether the user opted to not prompt for an API again.
    bool m_shouldNotAskAgain;
};
