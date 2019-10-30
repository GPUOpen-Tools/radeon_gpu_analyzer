// C++.
#include <cassert>
#include <sstream>

// Qt.
#include <QApplication>
#include <QCheckBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QWidget>
#include <QAction>

// Infra.
#include <QtCommon/CustomWidgets/ArrowIconWidget.h>
#include <QtCommon/Util/CommonDefinitions.h>
#include <QtCommon/CustomWidgets/ListWidget.h>
#include <QtCommon/Util/QtUtil.h>
#include <QtCommon/Util/RestoreCursorPosition.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgConfigManager.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgSettingsButtonsView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgHideListWidgetEventFilter.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyTableModel.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>


rgSettingsButtonsView::rgSettingsButtonsView(QWidget* pParent) :
    QWidget(pParent)
{
    // Setup the UI.
    ui.setupUi(this);

    // Set the background to white.
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    // Connect the signals.
    ConnectSignals();

    // Set the mouse cursor to the pointing hand cursor for various widgets.
    SetCursor();

    // Disable the "Save" button located on build settings view initially.
    ui.saveSettingsButton->setEnabled(false);
}

rgSettingsButtonsView::~rgSettingsButtonsView()
{
}

void rgSettingsButtonsView::ConnectSignals()
{
    // Restore default settings button.
    bool isConnected = connect(this->ui.defaultSettingsPushButton, &QPushButton::clicked, this, &rgSettingsButtonsView::HandleRestoreDefaultSettingsButtonClick);
    assert(isConnected);

    // Save settings button.
    isConnected = connect(this->ui.saveSettingsButton, &QPushButton::clicked, this, &rgSettingsButtonsView::HandleSaveSettingsButtonClick);
    assert(isConnected);

    m_pRestoreSettingsAction = new QAction(tr(STR_RESTORE_DEFAULT_SETTINGS), this);
    assert(m_pRestoreSettingsAction != nullptr);
    if (m_pRestoreSettingsAction != nullptr)
    {
        // Configure the hot key for the Restore default settings action.
        m_pRestoreSettingsAction->setShortcut(QKeySequence(gs_RESTORE_DEFAULT_SETTINGS));

        // Connect the handler for the "Restore default settings" button hot key action.
        isConnected = connect(m_pRestoreSettingsAction, &QAction::triggered, this, &rgSettingsButtonsView::HandleRestoreDefaultSettingsButtonClick);
        assert(isConnected);

        // Add a hot key action to the button.
        ui.defaultSettingsPushButton->addAction(m_pRestoreSettingsAction);
    }
}


void rgSettingsButtonsView::SetCursor()
{
    // Set the cursor to pointing hand cursor.
    ui.defaultSettingsPushButton->setCursor(Qt::PointingHandCursor);
    ui.saveSettingsButton->setCursor(Qt::PointingHandCursor);
}

void rgSettingsButtonsView::HandleRestoreDefaultSettingsButtonClick()
{
    // Emit the signal to indicate clicking of "Save" button.
    emit RestoreDefaultSettingsButtonClickedSignal();
}

void rgSettingsButtonsView::HandleSaveSettingsButtonClick()
{
    // Emit the signal to indicate clicking of "Save" button.
    emit SaveSettingsButtonClickedSignal();
}

void rgSettingsButtonsView::EnableSaveButton(bool isEnabled)
{
    ui.saveSettingsButton->setEnabled(isEnabled);
}

void rgSettingsButtonsView::HideRestoreDefaultSettingsButton(bool isHidden)
{
    ui.defaultSettingsPushButton->setHidden(isHidden);
}


void rgSettingsButtonsView::mousePressEvent(QMouseEvent *pEvent)
{
    emit SettingsButtonsViewClickedSignal();
}