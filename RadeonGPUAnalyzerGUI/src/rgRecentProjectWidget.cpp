// C++.
#include <cassert>

// Qt.
#include <QHBoxLayout>

// Local.
#include "rgRecentProjectWidget.h"

const static QString s_PROJECT_BUTTON_STYLESHEET(
    "QPushButton"
    "{"
    "color: rgb(0, 0, 255);"
    "padding: 0 0 0 0;"
    "border: none;"
    "text-align: left;"
    "font: 10pt;"
    "spacing : 10;"
    "}"
    "QPushButton:hover"
    "{"
    "color: rgb(255, 128, 0);"
    "}"
);

static const int s_API_ICON_WIDTH = 50;
static const int s_API_ICON_HEIGHT = 25;

rgRecentProjectWidget::rgRecentProjectWidget(QWidget* pParent) :
    QWidget(pParent),
    m_pProjectButton(new QPushButton(this))
{
    m_pHorizontalLayout = new QHBoxLayout(this);
    m_pHorizontalLayout->setContentsMargins(0, 0, 0, 0);
    m_pHorizontalLayout->setSpacing(5);

    // Additional settings for the project button.
    QSizePolicy policy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_pProjectButton->setSizePolicy(policy);
    m_pProjectButton->setCursor(Qt::PointingHandCursor);

    // Create a button to hold the icon.
    m_pIconButton = new QPushButton(this);

    m_pHorizontalLayout->addWidget(m_pProjectButton);
    m_pHorizontalLayout->addWidget(m_pIconButton);

    m_pProjectButton->setStyleSheet(s_PROJECT_BUTTON_STYLESHEET);
    m_pIconButton->setStyleSheet(s_PROJECT_BUTTON_STYLESHEET);
    m_pIconButton->setFocusPolicy(Qt::NoFocus);

    // Set the icon size.
    QSize size(s_API_ICON_WIDTH, s_API_ICON_HEIGHT);
    m_pIconButton->setIconSize(size);
}

void rgRecentProjectWidget::SetProjectName(const QString& fileName)
{
    // Set project button text.
    assert(m_pProjectButton != nullptr);
    if (m_pProjectButton != nullptr)
    {
        m_pProjectButton->setText(fileName);
    }
}

void rgRecentProjectWidget::SetIcon(QIcon icon)
{
    // Set the icon.
    assert(m_pIconButton != nullptr);
    if (m_pIconButton != nullptr)
    {
        m_pIconButton->setIcon(QIcon(icon));
    }
}

void rgRecentProjectWidget::SetIconProjectType(const char* projectType)
{
    QString projectTypeString = projectType;

    // Set the icon project type.
    assert(m_pIconButton != nullptr);
    if (m_pIconButton != nullptr)
    {
        m_pIconButton->setObjectName(projectTypeString);
    }
}

QString rgRecentProjectWidget::GetProjectName() const
{
    QString projectName = "";

    assert(m_pProjectButton != nullptr);
    if (m_pProjectButton != nullptr)
    {
        projectName = m_pProjectButton->text();
    }

    return projectName;
}

QString rgRecentProjectWidget::GetIconProjectType() const
{
    QString projectType = "";

    assert(m_pIconButton != nullptr);
    if (m_pIconButton != nullptr)
    {
        projectType = m_pIconButton->objectName();
    }

    return projectType;
}
