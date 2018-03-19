#pragma once

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgOrderedListDialog.h>

class rgIncludeDirectoriesView : public rgOrderedListDialog
{
    Q_OBJECT

public:
    rgIncludeDirectoriesView(const char* pDelimiter, QWidget* pParent = nullptr);
    virtual ~rgIncludeDirectoriesView() = default;

private slots:
    // Handler when the include file browse button is clicked.
    void HandleIncludeFileLocationBrowseButtonClick(bool /* checked */);

protected:
    // An overridden virtual responsible for determining if an edited list item is valid.
    virtual void OnListItemChanged(QListWidgetItem* pItem) override;

private:
    // Connect the signals.
    void ConnectSignals();

    // Initialize the view and add a "Browse" button to browse for new directories to add.
    void InitializeBrowseButton();

    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // Set the button fonts.
    void SetButtonFonts();

    // Set button shortcuts.
    void SetButtonShortcuts();

    // Button keyboard actions.
    QAction* m_pBrowseAction = nullptr;

    // A "Browse" QPushButton used to browse new directories to add.
    QPushButton* m_pBrowsePushButton = nullptr;
};