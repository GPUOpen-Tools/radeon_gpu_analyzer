#pragma once

// C++.
#include <memory>

// Qt.
#include <QWidget>

// Forward declarations.
struct rgBuildSettings;

class rgBuildSettingsView : public QWidget
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
    virtual void SaveSettings() = 0;

signals:
    // A signal emitted when the "has pending changes" state changes for the view.
    void PendingChangesStateChanged(bool hasPendingChanges);

protected:
    // Flag used to indicate if the view relates to global or project-specific settings.
    bool m_isGlobalSettings = false;
};