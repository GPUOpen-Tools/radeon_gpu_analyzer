//=============================================================================
/// Copyright (c) 2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for Reveal Kernel Name dialog.
//=============================================================================

// C++.
#include <cassert>

// Qt.
#include <QSignalMapper>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_reveal_kernel_name_dialog.h"

RgRevealKernelNameDialog::RgRevealKernelNameDialog(const QString& kernel_name, QWidget* parent)
    : QDialog(parent)
{
    // Setup the UI.
    ui_.setupUi(this);

    // Disable the help button in the title bar.
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Set the kernel name text.
    ui_.kernelNameTextEdit->setPlainText(kernel_name);

    // Set a fixed size based on the initial geometry.
    setFixedSize(size());

    // Connect the signals.
    ConnectSignals();
}

void RgRevealKernelNameDialog::ConnectSignals()
{
    // Create a signal mapper to map the button click to the done(int) slot.
    QSignalMapper* button_signal_mapper = new QSignalMapper(this);

    // Close button.
    bool is_connected = connect(ui_.closePushButton, SIGNAL(clicked()), button_signal_mapper, SLOT(map()));
    assert(is_connected);
    button_signal_mapper->setMapping(ui_.closePushButton, QDialog::Accepted);

    // Signal mapper.
    is_connected = connect(button_signal_mapper, SIGNAL(mappedInt(int)), this, SLOT(done(int)));
    assert(is_connected);
}
