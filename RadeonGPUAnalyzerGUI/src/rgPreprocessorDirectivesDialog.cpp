// C++.
#include <sstream>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPreprocessorDirectivesDialog.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>
#include <RadeonGPUAnalyzerGUI/Include/rgUtils.h>

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

    m_editingInvalidEntry = false;

    // Process the newly-entered data.
    if (pItem != nullptr)
    {
        QString newMacro = pItem->text();

        bool isDuplicateItem = m_itemsList.contains(newMacro);
        bool isContainsWhitespace = rgUtils::IsContainsWhitespace(newMacro.toStdString());

        // If the new macro exists, and it is not a duplicate entry, update local data.
        if (newMacro.isEmpty())
        {
            // The user has emptied out the entry, so delete it.
            // Simulate a click on the delete button to remove the entry from the UI.
            ui.deletePushButton->click();
        }
        else
        {
            if (isDuplicateItem || isContainsWhitespace)
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

            // Update local data.
            int itemRow = ui.itemsList->row(pItem);
            if (itemRow < m_itemsList.count())
            {
                m_itemsList[itemRow] = newMacro;
            }
            else
            {
                m_itemsList.append(newMacro);
            }
        }

        // Update tool tips.
        UpdateToolTips();

        // Unblock signals from the list widget.
        ui.itemsList->blockSignals(false);

        // Update buttons.
        UpdateButtons();
    }
}