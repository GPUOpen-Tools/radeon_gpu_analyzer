#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgStartTab.h>

// A Vulkan-specific implementation of the start tab.
class rgStartTabVulkan : public rgStartTab
{
    Q_OBJECT

public:
    rgStartTabVulkan(QWidget* pParent);
    virtual ~rgStartTabVulkan() = default;

signals:
    // Signal emitted when the user clicks the "Create Graphics pipeline" button.
    void CreateGraphicsPipelineEvent();

    // Signal emitted when the user clicks the "Create Compute pipeline" button.
    void CreateComputePipelineEvent();

protected:
    // Apply API-specific string constants to the view object's widgets.
    virtual void ApplyApiStringConstants() override;

    // Get a list of buttons to insert into the start page's "Start" section.
    virtual void GetStartButtons(std::vector<QPushButton*>& startButtons) override;

private:
    // Initialize the start buttons.
    void InitializeStartButtons();

    // Connect signals for the API-specific start page items.
    void ConnectSignals();

    // A QPushButton used to create a new graphics pipeline project.
    QPushButton* m_pCreateGraphicsPipelinePushButton = nullptr;

    // A QPushButton used to create a new compute pipeline project.
    QPushButton* m_pCreateComputePipelinePushButton = nullptr;
};