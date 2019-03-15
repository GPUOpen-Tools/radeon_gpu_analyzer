#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgMenuGraphics.h>

class rgMenuVulkan : public rgMenuGraphics
{
    Q_OBJECT

public:
    rgMenuVulkan(QWidget* pParent);
    virtual ~rgMenuVulkan() = default;

    // Connect the signals for the view's default menu items.
    virtual void ConnectDefaultItemSignals() override;

    // Connect signals for the menu file item.
    virtual void ConnectMenuFileItemSignals(rgMenuFileItem* pMenuItem) override;

    // Deselect all menu items.
    virtual void DeselectItems() override;

    // Clear the stage's shader source file.
    void ClearStageSourceFile(rgPipelineStage stage);

    // Set the stage's shader source file.
    // Returns "true" if the file has been added succesfully or "false" otherwise.
    bool SetStageSourceFile(rgPipelineStage stage, const std::string& fullPath, rgVulkanInputType fileType, bool isNewFileItem);

    // Replace the stage's shader file with another file.
    // This function should be used to replace a SPIR-V file with its disassembly version and vice versa.
    bool ReplaceStageFile(rgPipelineStage stage, const std::string& newFilePath, rgVulkanInputType fileType);

    // Check to see if any of the buttons are pressed.
    bool IsButtonPressed() const;

    // Update the focus indices.
    void UpdateFocusIndex();

public slots:
    // Handler invoked when the user presses the activate key (Enter).
    // This is used to handle pressing add/create buttons when in focus.
    virtual void HandleActivateItemAction() override;

    // Handler invoked when the user changes the currently selected file item.
    virtual void HandleSelectedFileChanged(rgMenuFileItem* pSelected) override;

private:
    // Process sub button actions.
    virtual void ProcessSubButtonAction() override;
};