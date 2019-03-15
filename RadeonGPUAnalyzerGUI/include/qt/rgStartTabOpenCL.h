#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgStartTab.h>

// An OpenCL-specific implementation of the start tab.
class rgStartTabOpenCL : public rgStartTab
{
    Q_OBJECT

public:
    rgStartTabOpenCL(QWidget* pParent);
    virtual ~rgStartTabOpenCL() = default;

signals:
    // A signal emitted when the user clicks the "Create new CL File" button.
    void CreateNewCLFileEvent();

    // A signal emitted when the user clicks the "Open project" button.
    void OpenExistingFileEvent();

protected:
    // Apply API-specific string constants to the view object's widgets.
    virtual void ApplyApiStringConstants() override;

    // Get the list of buttons to insert into the start page's "Start" section.
    virtual void GetStartButtons(std::vector<QPushButton*>& startButtons) override;

private:
    // Initialize the start buttons.
    void InitializeStartButtons();

    // Connect signals for the API-specific start page items.
    void ConnectSignals();

    // A QPushButton used to create a new .cl source file.
    QPushButton* m_pCreateNewCLSourceFile = nullptr;

    // A QPushButton used to add an existing .cl source file.
    QPushButton* m_pAddExistingCLSourceFile = nullptr;
};