#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgOrderedListDialog.h>

class rgPreprocessorDirectivesDialog : public rgOrderedListDialog
{
    Q_OBJECT

public:
    rgPreprocessorDirectivesDialog(const char* pDelimiter, QWidget* pParent = nullptr);
    virtual ~rgPreprocessorDirectivesDialog() = default;

protected:
    // An overridden virtual responsible for determining if an edited list item is valid.
    virtual void OnListItemChanged(QListWidgetItem* pItem) override;
};