// C++.
#include <cassert>

// Qt.
#include <QHBoxLayout>
#include <QColor>

// QtCommon.
#include "qt_common/utils/common_definitions.h"
#include "qt_common/utils/qt_util.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_recent_project_widget.h"

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

    if (QtCommon::QtUtils::ColorTheme::Get().GetColorTheme() == ColorThemeType::kColorThemeTypeDark)
    {
        project_button_->setStyleSheet(kDarkLinkButtonStylesheet);
        icon_button_->setStyleSheet(kDarkLinkButtonStylesheet);
    }
    else
    {
        project_button_->setStyleSheet(kLinkButtonStylesheet);
        icon_button_->setStyleSheet(kLinkButtonStylesheet);
    }

    connect(&QtCommon::QtUtils::ColorTheme::Get(), &QtCommon::QtUtils::ColorTheme::ColorThemeUpdated, this, &RgRecentProjectWidget::UpdateLinkButtonStyleSheet);

    icon_button_->setFocusPolicy(Qt::NoFocus);

    // Set the icon size.
    QSize size(kApiIconWidth, kApiIconHeight);
    icon_button_->setIconSize(size);
}

void RgRecentProjectWidget::UpdateLinkButtonStyleSheet()
{
    if (QtCommon::QtUtils::ColorTheme::Get().GetColorTheme() == ColorThemeType::kColorThemeTypeDark)
    {
        project_button_->setStyleSheet(kDarkLinkButtonStylesheet);
        icon_button_->setStyleSheet(kDarkLinkButtonStylesheet);
    }
    else
    {
        project_button_->setStyleSheet(kLinkButtonStylesheet);
        icon_button_->setStyleSheet(kLinkButtonStylesheet);
    }
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
