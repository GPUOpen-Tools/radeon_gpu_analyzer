#pragma once

// C++.
#include <memory>

// Infra.
#include <QtCommon/CustomWidgets/ListWidget.h>
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include "ui_rgGlobalSettingsView.h"
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgIsaDisassemblyTableModel.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgBuildSettingsView.h>

// Forward declarations.
class QWidget;
class rgGlobalSettingsModel;

class rgGlobalSettingsView : public rgBuildSettingsView
{
    Q_OBJECT

public:
    rgGlobalSettingsView(QWidget* pParent, std::shared_ptr<rgGlobalSettings> pBuildSettings);
    virtual ~rgGlobalSettingsView();
    void CloseListWidget();

public:
    virtual bool GetHasPendingChanges() const override;
    virtual bool RevertPendingChanges() override;
    virtual void RestoreDefaultSettings() override;
    virtual void SaveSettings() override;

public slots:
    void HandlePendingChangesStateChanged(bool hasPendingChanges);
    void HandleLogFileLocationBrowseButtonClick(bool checked);
    void HandleColumnVisibilityComboBoxItemClicked(const QString& text, const bool checked);
    void HandleColumnVisibilityFilterStateChanged(bool checked);
    void HandleViewColumnsButtonClick(bool checked);
    void HandleProjectNameCheckboxStateChanged(int checked);
    void HandleLogFileEditingFinished();
    void HandleLogFileEditBoxChanged(const QString& text);
    void HandleDefaultProjectNameCheckboxUpdate();

private:
    // Remove all items from the given list widget.
    void ClearListWidget(ListWidget* &pListWidget);

    // Connect the signals.
    void ConnectSignals();

    // Create the controls responsible for picking the visible columns in the disassembly table.
    void CreateColumnVisibilityControls();

    // Get the name of the given disassembly column as a string.
    std::string GetDisassemblyColumnName(rgIsaDisassemblyTableColumns column) const;

    // Initialize the view with values in the incoming global settings structure.
    void InitializeModel();

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

    // The settings model bound to this view.
    rgGlobalSettingsModel* m_pSettingsModel = nullptr;

    // The generated interface view object.
    Ui::rgGlobalSettingsView ui;
};