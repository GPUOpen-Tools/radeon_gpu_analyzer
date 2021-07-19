#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_INCLUDE_DIRECTORIES_VIEW_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_INCLUDE_DIRECTORIES_VIEW_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_ordered_list_dialog.h"

class RgIncludeDirectoriesView : public RgOrderedListDialog
{
    Q_OBJECT

public:
    RgIncludeDirectoriesView(const char* delimiter, QWidget* parent = nullptr);
    virtual ~RgIncludeDirectoriesView() = default;

private slots:
    // Handler when the include file browse button is clicked.
    void HandleIncludeFileLocationBrowseButtonClick(bool /* checked */);

protected:
    // An overridden virtual responsible for determining if an edited list item is valid.
    virtual void OnListItemChanged(QListWidgetItem* item) override;

private:
    // Connect the signals.
    void ConnectSignals();

    // Initialize the view and add a "Browse" button to browse for new directories to add.
    void InitializeBrowseButton();

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // Set the button fonts.
    void SetButtonFonts();

    // A "Browse" QPushButton used to browse new directories to add.
    QPushButton* browse_push_button_ = nullptr;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_INCLUDE_DIRECTORIES_VIEW_H_
