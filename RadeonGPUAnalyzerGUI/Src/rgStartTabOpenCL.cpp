// C++.
#include <cassert>

// Qt.
#include <QAction>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgStartTabOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgAppStateOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypesOpenCL.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

rgStartTabOpenCL::rgStartTabOpenCL(QWidget* pParent)
    : rgStartTab(pParent)
{
    // Initialize the OpenCL start buttons.
    InitializeStartButtons();

    // Connect OpenCL start page signals.
    ConnectSignals();
}

void rgStartTabOpenCL::ApplyApiStringConstants()
{
    // Set label/button text.
    m_pCreateNewCLSourceFile->setText(STR_MENU_BAR_CREATE_NEW_FILE_OPENCL);
    m_pAddExistingCLSourceFile->setText(STR_MENU_BAR_OPEN_EXISTING_FILE_OPENCL);

    // Set tooltips and status tips.
    rgUtils::SetToolAndStatusTip(STR_MENU_BAR_CREATE_NEW_FILE_TOOLTIP_OPENCL, m_pCreateNewCLSourceFile);
    rgUtils::SetToolAndStatusTip(STR_MENU_BAR_OPEN_EXISTING_FILE_TOOLTIP_OPENCL, m_pAddExistingCLSourceFile);
}

void rgStartTabOpenCL::GetStartButtons(std::vector<QPushButton*>& startButtons)
{
    startButtons.push_back(m_pCreateNewCLSourceFile);
    startButtons.push_back(m_pAddExistingCLSourceFile);
}

void rgStartTabOpenCL::InitializeStartButtons()
{
    // Create the "Create .cl file" button.
    m_pCreateNewCLSourceFile = new QPushButton(this);
    m_pCreateNewCLSourceFile->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_pCreateNewCLSourceFile->setText(STR_MENU_BAR_CREATE_NEW_FILE_OPENCL);
    m_pCreateNewCLSourceFile->setToolTip(STR_MENU_BAR_CREATE_NEW_FILE_TOOLTIP_OPENCL);

    // Create the "Add existing .cl file" button.
    m_pAddExistingCLSourceFile = new QPushButton(this);
    m_pAddExistingCLSourceFile->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_pAddExistingCLSourceFile->setText(STR_MENU_BAR_OPEN_EXISTING_FILE_OPENCL);
    m_pAddExistingCLSourceFile->setToolTip(STR_MENU_BAR_OPEN_EXISTING_FILE_TOOLTIP_OPENCL);
}

void rgStartTabOpenCL::ConnectSignals()
{
    // Create New File action.
    bool isConnected = connect(m_pCreateNewCLSourceFile, &QPushButton::clicked, this, &rgStartTabOpenCL::CreateNewCLFileEvent);
    assert(isConnected);

    // Open a file.
    isConnected =  connect(m_pAddExistingCLSourceFile, &QPushButton::clicked, this, &rgStartTabOpenCL::OpenExistingFileEvent);
    assert(isConnected);
}