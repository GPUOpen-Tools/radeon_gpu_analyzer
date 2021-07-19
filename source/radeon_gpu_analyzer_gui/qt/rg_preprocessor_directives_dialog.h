#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PREPROCESSOR_DIRECTIVES_DIALOG_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PREPROCESSOR_DIRECTIVES_DIALOG_H_

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_ordered_list_dialog.h"

class RgPreprocessorDirectivesDialog : public RgOrderedListDialog
{
    Q_OBJECT

public:
    RgPreprocessorDirectivesDialog(const char* delimiter, QWidget* parent = nullptr);
    virtual ~RgPreprocessorDirectivesDialog() = default;

protected:
    // An overridden virtual responsible for determining if an edited list item is valid.
    virtual void OnListItemChanged(QListWidgetItem* item) override;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PREPROCESSOR_DIRECTIVES_DIALOG_H_
