#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_START_TAB_OPENCL_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_START_TAB_OPENCL_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_start_tab.h"

// An OpenCL-specific implementation of the start tab.
class RgStartTabOpencl : public RgStartTab
{
    Q_OBJECT

public:
    RgStartTabOpencl(QWidget* parent);
    virtual ~RgStartTabOpencl() = default;

signals:
    // A signal emitted when the user clicks the "Create new CL File" button.
    void CreateNewCLFileEvent();

    // A signal emitted when the user clicks the "Open project" button.
    void OpenExistingFileEvent();

protected:
    // Apply API-specific string constants to the view object's widgets.
    virtual void ApplyApiStringConstants() override;

    // Get the list of buttons to insert into the start page's "Start" section.
    virtual void GetStartButtons(std::vector<QPushButton*>& start_buttons) override;

private:
    // Initialize the start buttons.
    void InitializeStartButtons();

    // Connect signals for the API-specific start page items.
    void ConnectSignals();

    // A QPushButton used to create a new .cl source file.
    QPushButton* create_new_cl_source_file_ = nullptr;

    // A QPushButton used to add an existing .cl source file.
    QPushButton* add_existing_cl_source_file_ = nullptr;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_START_TAB_OPENCL_H_
