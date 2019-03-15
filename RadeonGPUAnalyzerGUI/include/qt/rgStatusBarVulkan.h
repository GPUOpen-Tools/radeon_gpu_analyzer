#pragma once

// C++.
#include <memory>

// Qt.
#include <QtWidgets/QWidget>
#include <QPushButton>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgStatusBar.h>

class rgStatusBarVulkan : public rgStatusBar
{
    Q_OBJECT

public:
    rgStatusBarVulkan(QStatusBar* pStatusBar, QWidget* pParent = nullptr);
    virtual ~rgStatusBarVulkan() = default;

private:
    // Set style sheets for mode and API push buttons.
    virtual void SetStylesheets(QStatusBar* pStatusBar) override;

    // The parent widget.
    QWidget* m_pParent = nullptr;
};