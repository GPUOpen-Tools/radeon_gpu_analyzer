#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgStatusBar.h>

class rgStatusBarOpenCL : public rgStatusBar
{
    Q_OBJECT

public:
    rgStatusBarOpenCL(QStatusBar* pStatusBar, QWidget* pParent = nullptr);
    virtual ~rgStatusBarOpenCL() = default;

private:
    // Set style sheets for mode and API push buttons.
    virtual void SetStylesheets(QStatusBar* pStatusBar) override;

    // The parent widget.
    QWidget* m_pParent = nullptr;
};