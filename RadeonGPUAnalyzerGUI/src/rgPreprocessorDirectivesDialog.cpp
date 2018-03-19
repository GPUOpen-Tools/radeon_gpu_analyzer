// C++.
#include <sstream>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgPreprocessorDirectivesDialog.h>
#include <RadeonGPUAnalyzerGUI/include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/include/rgUtils.h>

rgPreprocessorDirectivesDialog::rgPreprocessorDirectivesDialog(const char* pDelimiter, QWidget* pParent) :
    rgOrderedListDialog(pDelimiter, pParent)
{
    // Set the window title.
    setWindowTitle(STR_PREPROCESSOR_DIRECTIVES_DIALOG_TITLE);

    // Update various buttons.
    UpdateButtons();
}

void rgPreprocessorDirectivesDialog::OnListItemChanged(QListWidgetItem* pItem)
{
    // Block signals from the list widget.
    ui.itemsList->blockSignals(true);

    // Process the newly-entered data.
    if (pItem != nullptr)
    {
        QString newMacro = pItem->text();

        bool isDuplicateItem = m_itemsList.contains(newMacro);
        bool isContainsWhitespace = rgUtils::IsContainsWhitespace(newMacro.toStdString());

        // If the new directory exists, and it is not a duplicate entry, update local data.
        if (!isDuplicateItem && !isContainsWhitespace)
        {
            if (!m_editingInvalidEntry)
            {
                // Update local data.
                m_itemsList << newMacro;
            }

            // Update tool tips.
            UpdateToolTips();

            m_editingInvalidEntry = false;
        }
        else
        {
            // Display an error message box.
            std::stringstream errorString;
            if (isDuplicateItem)
            {
                errorString << STR_PREPROCESSOR_DIRECTIVES_DIALOG_DIRECTIVE_IS_DUPLICATE;
                errorString << newMacro.toStdString().c_str();
            }
            else if (isContainsWhitespace)
            {
                errorString << STR_PREPROCESSOR_DIRECTIVES_DIALOG_DIRECTIVE_CONTAINS_WHITESPACE;
                errorString << newMacro.toStdString().c_str();
            }

            rgUtils::ShowErrorMessageBox(errorString.str().c_str(), this);
            m_editingInvalidEntry = true;
        }

        // Unblock signals from the list widget.
        ui.itemsList->blockSignals(false);

        // Update buttons.
        UpdateButtons();
    }
}