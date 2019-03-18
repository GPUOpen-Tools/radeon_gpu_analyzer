#pragma once

// Qt.
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>

/// This class is used to bundle together the project name
/// and the API icon (e.g. OpenCL or Vulkan) together in one
/// single widget. This widget is used to display the projects
/// under the recent projects list upon startup.

class rgRecentProjectWidget : public QWidget
{
    Q_OBJECT

public:
    explicit rgRecentProjectWidget(QWidget* pParent = nullptr);
    virtual ~rgRecentProjectWidget() = default;

    // Set the project name.
    void SetProjectName(const QString& file);

    // Set icon.
    void SetIcon(QIcon icon);

    // Set the icon project type.
    void SetIconProjectType(const char* projectType);

    // Return the project name.
    QString GetProjectName() const;

    // Return the push button.
    QPushButton* GetPushButton() const { return m_pProjectButton; }

    // Return the icon button name.
    QString GetIconProjectType() const;

private:
    // The project name button.
    QPushButton* m_pProjectButton = nullptr;

    // The icon button.
    QPushButton* m_pIconButton = nullptr;

    // The horizontal layout to hold the project button and the icon button.
    QHBoxLayout* m_pHorizontalLayout = nullptr;
};
