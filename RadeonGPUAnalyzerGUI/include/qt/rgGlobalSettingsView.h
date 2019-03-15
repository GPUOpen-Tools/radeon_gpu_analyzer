#pragma once

// C++.
#include <memory>

// Infra.
#include <QtCommon/CustomWidgets/ListWidget.h>
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include "ui_rgGlobalSettingsView.h"
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyTableModel.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgBuildSettingsView.h>

// Forward declarations.
class QWidget;

class rgGlobalSettingsView : public rgBuildSettingsView
{
    Q_OBJECT

public:
    rgGlobalSettingsView(QWidget* pParent, const rgGlobalSettings& globalSettings);
    virtual ~rgGlobalSettingsView();
    void CloseListWidget();

public:
    virtual bool GetHasPendingChanges() const override;
    virtual bool RevertPendingChanges() override;
    virtual void RestoreDefaultSettings() override;
    virtual bool SaveSettings() override;
    virtual std::string GetTitleString() override;
    virtual void SetInitialWidgetFocus() override;

public slots:
    // Handler for when the pending changes state has changed.
    void HandlePendingChangesStateChanged(bool hasPendingChanges);

    // Handler for when the log file location browse button is clicked.
    void HandleLogFileLocationBrowseButtonClick(bool checked);

    // Handler for when the include files viewer browse button is clicked.
    void HandleIncludeFilesViewerBrowseButtonClick(bool checked);

    // Handler for when the column visibility combo box item is clicked.
    void HandleColumnVisibilityComboBoxItemClicked(const QString& text, bool checked);

    // Handler for when the column visibility filter state has changed.
    void HandleColumnVisibilityFilterStateChanged(bool checked);

    // Handler for when the view column button is clicked.
    void HandleViewColumnsButtonClick(bool checked);

    // Handler for when the project name check box state has changed.
    void HandleProjectNameCheckboxStateChanged(int checked);

    // Handler for when log file editing is finished.
    void HandleLogFileEditingFinished();

    // Handler for when the log file edit box changed.
    void HandleLogFileEditBoxChanged(const QString& text);

    // Handler for any text box which signals about pending changes.
    void HandleTextBoxChanged(const QString& text);

    // Handler for any combo box which signals about pending changes.
    void HandleComboBoxChanged(int index);

    // Handler for when the font family has changed.
    void HandleFontFamilyChanged(const QFont& font);

    // Handler for when include files viewer text box editing is finished.
    void HandleIncludeFilesViewerEditingFinished();

protected:
    virtual void showEvent(QShowEvent* pEvent) override;

private:
    // Make the UI reflect the values in the supplied global settings struct.
    void PushToWidgets(const rgGlobalSettings& globalSettings);

    // Use the values in the UI to create a global settings struct which
    // contains the pending changes.
    rgGlobalSettings PullFromWidgets() const;

    // Remove all items from the given list widget.
    void ClearListWidget(ListWidget* &pListWidget);

    // Connect the signals.
    void ConnectSignals();

    // Create the controls responsible for picking the visible columns in the disassembly table.
    void CreateColumnVisibilityControls();

    // Get the name of the given disassembly column as a string.
    std::string GetDisassemblyColumnName(rgIsaDisassemblyTableColumns column) const;

    // Populate the names in the column visibility list.
    void PopulateColumnVisibilityList();

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // Set the tooltip for the default program name check box.
    void SetCheckboxToolTip(const std::string& text);

    // Set the tooltips for the edit boxes.
    void SetEditBoxToolTips();

    // Update the "All" checkbox text color to grey if it is already checked,
    // to black otherwise.
    void UpdateAllCheckBoxText();

    // The widget used to display all columns available for display in the disassembly table.
    ListWidget* m_pDisassemblyColumnsListWidget = nullptr;

    // A custom event filter for the disassembly columns list widget.
    QObject* m_pDisassemblyColumnsListEventFilter = nullptr;

    // A pointer to the parent widget.
    QWidget* m_pParent = nullptr;

    // Initial version of the settings that the view was created with.
    // Note: They will also get updated when the user clicks 'Save', so it can't
    // be const.
    rgGlobalSettings m_initialSettings;

    // The generated interface view object.
    Ui::rgGlobalSettingsView ui;
};
