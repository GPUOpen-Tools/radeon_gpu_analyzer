#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_RECENT_PROJECT_WIDGET_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_RECENT_PROJECT_WIDGET_H_

// Qt.
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>

/// This class is used to bundle together the project name
/// and the API icon (e.g. OpenCL or Vulkan) together in one
/// single widget. This widget is used to display the projects
/// under the recent projects list upon startup.

class RgRecentProjectWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RgRecentProjectWidget(QWidget* parent = nullptr);
    virtual ~RgRecentProjectWidget() = default;

    // Set the project name.
    void SetProjectName(const QString& file);

    // Set icon.
    void SetIcon(QIcon icon);

    // Set the icon project type.
    void SetIconProjectType(const char* project_type);

    // Return the project name.
    QString GetProjectName() const;

    // Return the push button.
    QPushButton* GetPushButton() const { return project_button_; }

    // Return the icon button name.
    QString GetIconProjectType() const;

    // Update the link button stylesheets after the color theme has been updated.
    void UpdateLinkButtonStyleSheet();

private:
    // The project name button.
    QPushButton* project_button_ = nullptr;

    // The icon button.
    QPushButton* icon_button_ = nullptr;

    // The horizontal layout to hold the project button and the icon button.
    QHBoxLayout* horizontal_layout_ = nullptr;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_RECENT_PROJECT_WIDGET_H_
