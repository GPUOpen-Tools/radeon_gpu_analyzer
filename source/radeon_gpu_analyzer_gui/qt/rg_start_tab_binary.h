//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for a Binary-specific implementation of the start tab.
//=============================================================================
#pragma once

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_start_tab.h"

// A Binary-specific implementation of the start tab.
class RgStartTabBinary : public RgStartTab
{
    Q_OBJECT

public:
    RgStartTabBinary(QWidget* parent);
    virtual ~RgStartTabBinary() = default;

signals:
    // A signal emitted when the user clicks the "Create new CL File" button.
    void OpenExistingCodeObjFileEvent();

protected:
    // Apply API-specific string constants to the view object's widgets.
    virtual void ApplyApiStringConstants() override;

    // Get the list of buttons to insert into the start page's "Start" section.
    virtual void GetStartButtons(std::vector<QPushButton*>& start_buttons) override;

private:
    // Initialize the start buttons.
    void InitializeStartButtons();

    // Connect signals for the API-specific start page items.
    void ConnectSignals();

    // A QPushButton used to add an existing .bin codeobj file.
    QPushButton* add_existing_code_obj_file_ = nullptr;
};
