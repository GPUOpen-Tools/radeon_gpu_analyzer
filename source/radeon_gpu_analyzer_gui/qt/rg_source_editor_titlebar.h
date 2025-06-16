//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for source code editor titlebar.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SOURCE_EDITOR_TITLEBAR_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SOURCE_EDITOR_TITLEBAR_H_

// Qt.
#include <QFrame>

// Local.
#include "ui_rg_source_editor_titlebar.h"

// A title bar inserted above the source editor that displays the correlation status.
class RgSourceEditorTitlebar : public QFrame
{
    Q_OBJECT

public:
    RgSourceEditorTitlebar(QWidget* parent = nullptr);
    virtual ~RgSourceEditorTitlebar() = default;

    // Update the title bar view based on whether or not a file's correlation state is enabled.
    void SetIsCorrelationEnabled(bool is_enabled);

    // Show the required message in the titlebar.
    void ShowMessage(const std::string& msg);

    // Change the visibility of the title bar contents.
    void SetTitlebarContentsVisibility(bool is_visible);

signals:
    void DismissMsgButtonClicked();

private slots:
    // Handler to process the dismiss message button click.
    void HandleDismissMessagePushButtonClicked(/* bool checked */);

private:
    // Connect the signals.
    void ConnectSignals();

    // Set the mouse cursor to pointing hand cursor.
    void SetCursor();

    // The generated view object for the title bar.
    Ui::RgSourceEditorTitlebar ui_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SOURCE_EDITOR_TITLEBAR_H_
