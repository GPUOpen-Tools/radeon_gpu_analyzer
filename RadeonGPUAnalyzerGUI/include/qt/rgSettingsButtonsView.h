#pragma once

// C++.
#include <memory>

// Infra.
#include <QtCommon/CustomWidgets/ListWidget.h>
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include "ui_rgSettingsButtonsView.h"
#include <RadeonGPUAnalyzerGUI/include/rgDataTypes.h>

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

signals:
    void RestoreDefaultSettingsButtonClickedSignal();
    void SaveSettingsButtonClickedSignal();

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
};