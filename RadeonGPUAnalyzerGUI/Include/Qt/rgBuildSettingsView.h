#pragma once

// C++.
#include <memory>

// Qt.
#include <QWidget>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSettingsView.h>

// Forward declarations.
struct rgBuildSettings;

class rgBuildSettingsView : public rgSettingsView
{
    Q_OBJECT

public:
    rgBuildSettingsView(QWidget* pParent, bool isGlobalSettings);
    virtual ~rgBuildSettingsView() = default;

public:
    // Does the settings view have any pending changes that need to be saved?
    virtual bool GetHasPendingChanges() const = 0;

    // Revert the pending changes.
    virtual bool RevertPendingChanges() = 0;

    // Restore the current setting values to defaults.
    virtual void RestoreDefaultSettings() = 0;

    // Save all pending settings to disk.
    virtual bool SaveSettings() = 0;

    // Get the title string.
    virtual std::string GetTitleString() = 0;

public slots:
    // Update the generated command line text.
    virtual void UpdateCommandLineText() = 0;

signals:
    // A signal emitted when the "has pending changes" state changes for the view.
    void PendingChangesStateChanged(bool hasPendingChanges);

protected:
    // Subclasses can use this to set whether they have pending changes;
    // this will emit the pending changes signal as needed.
    void SetHasPendingChanges(bool hasPendingChanges);

    // Flag used to indicate if the view relates to global or project-specific settings.
    bool m_isGlobalSettings = false;

    // Flag used to store whether a signal was emited regarding pending changes.
    bool m_hasPendingChanges = false;
};