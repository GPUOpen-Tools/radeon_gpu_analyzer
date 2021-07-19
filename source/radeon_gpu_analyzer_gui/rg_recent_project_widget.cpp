// C++.
#include <cassert>

// Qt.
#include <QHBoxLayout>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_recent_project_widget.h"

const static QString kStrProjectButtonStylesheet(
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

static const int kApiIconWidth = 50;
static const int kApiIconHeight = 25;

RgRecentProjectWidget::RgRecentProjectWidget(QWidget* parent) :
    QWidget(parent),
    project_button_(new QPushButton(this))
{
    horizontal_layout_ = new QHBoxLayout(this);
    horizontal_layout_->setContentsMargins(0, 0, 0, 0);
    horizontal_layout_->setSpacing(5);

    // Additional settings for the project button.
    QSizePolicy policy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    project_button_->setSizePolicy(policy);
    project_button_->setCursor(Qt::PointingHandCursor);

    // Create a button to hold the icon.
    icon_button_ = new QPushButton(this);

    horizontal_layout_->addWidget(project_button_);
    horizontal_layout_->addWidget(icon_button_);

    project_button_->setStyleSheet(kStrProjectButtonStylesheet);
    icon_button_->setStyleSheet(kStrProjectButtonStylesheet);
    icon_button_->setFocusPolicy(Qt::NoFocus);

    // Set the icon size.
    QSize size(kApiIconWidth, kApiIconHeight);
    icon_button_->setIconSize(size);
}

void RgRecentProjectWidget::SetProjectName(const QString& filename)
{
    // Set project button text.
    assert(project_button_ != nullptr);
    if (project_button_ != nullptr)
    {
        project_button_->setText(filename);
    }
}

void RgRecentProjectWidget::SetIcon(QIcon icon)
{
    // Set the icon.
    assert(icon_button_ != nullptr);
    if (icon_button_ != nullptr)
    {
        icon_button_->setIcon(QIcon(icon));
    }
}

void RgRecentProjectWidget::SetIconProjectType(const char* project_type)
{
    QString projectTypeString = project_type;

    // Set the icon project type.
    assert(icon_button_ != nullptr);
    if (icon_button_ != nullptr)
    {
        icon_button_->setObjectName(projectTypeString);
    }
}

QString RgRecentProjectWidget::GetProjectName() const
{
    QString project_name = "";

    assert(project_button_ != nullptr);
    if (project_button_ != nullptr)
    {
        project_name = project_button_->text();
    }

    return project_name;
}

QString RgRecentProjectWidget::GetIconProjectType() const
{
    QString project_type = "";

    assert(icon_button_ != nullptr);
    if (icon_button_ != nullptr)
    {
        project_type = icon_button_->objectName();
    }

    return project_type;
}
