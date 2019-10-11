#pragma once

// C++.
#include <memory>

// Infra.
#include <QtCommon/CustomWidgets/ListWidget.h>
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include "ui_rgSettingsButtonsView.h"
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>

// Forward declarations.
class QWidget;
class rgOpenCLBuildSettingsModel;

class rgSettingsButtonsView : public QWidget
{
    Q_OBJECT

public:
    rgSettingsButtonsView(QWidget* pParent);
    virtual ~rgSettingsButtonsView();

    // Enable/disable save button.
    void EnableSaveButton(bool isEnabled);

    // Hide / show the Restore defaultSettings button.
    void HideRestoreDefaultSettingsButton(bool isHidden);

    // Re-implement mousePressEvent method.
    virtual void mousePressEvent(QMouseEvent *pEvent) override;

signals:
    void RestoreDefaultSettingsButtonClickedSignal();
    void SaveSettingsButtonClickedSignal();
    void SettingsButtonsViewClickedSignal();

private slots:
    void HandleRestoreDefaultSettingsButtonClick();
    void HandleSaveSettingsButtonClick();

private:
    // Connect the signals.
    void ConnectSignals();

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // The generated interface view object.
    Ui::rgSettingsButtonsView ui;

    // The action used to Restore settings.
    QAction* m_pRestoreSettingsAction = nullptr;
};