// C++.
#include <sstream>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_preprocessor_directives_dialog.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"
#include "radeon_gpu_analyzer_gui/rg_utils.h"

RgPreprocessorDirectivesDialog::RgPreprocessorDirectivesDialog(const char* delimiter, QWidget* parent) :
    RgOrderedListDialog(delimiter, parent)
{
    // Set the window title.
    setWindowTitle(kStrPreprocessorDirectivesDialogTitle);

    // Update various buttons.
    UpdateButtons();
}

void RgPreprocessorDirectivesDialog::OnListItemChanged(QListWidgetItem* item)
{
    // Block signals from the list widget.
    ui_.itemsList->blockSignals(true);

    editing_invalid_entry_ = false;

    // Process the newly-entered data.
    if (item != nullptr)
    {
        QString new_macro = item->text();

        bool is_duplicate_item = items_list_.contains(new_macro);
        bool is_contains_whitespace = RgUtils::IsContainsWhitespace(new_macro.toStdString());

        // If the new macro exists, and it is not a duplicate entry, update local data.
        if (new_macro.isEmpty())
        {
            // The user has emptied out the entry, so delete it.
            // Simulate a click on the delete button to remove the entry from the UI.
            ui_.deletePushButton->click();
        }
        else
        {
            if (is_duplicate_item || is_contains_whitespace)
            {
                // Display an error message box.
                std::stringstream error_string;
                if (is_duplicate_item)
                {
                    error_string << kStrPreprocessorDirectivesDialogDirectiveIsDuplicate;
                    error_string << new_macro.toStdString().c_str();
                }
                else if (is_contains_whitespace)
                {
                    error_string << kStrPreprocessorDirectivesDialogDirectiveContainsWhitespace;
                    error_string << new_macro.toStdString().c_str();
                }

                RgUtils::ShowErrorMessageBox(error_string.str().c_str(), this);
                editing_invalid_entry_ = true;
            }

            // Update local data.
            int item_row = ui_.itemsList->row(item);
            if (item_row < items_list_.count())
            {
                items_list_[item_row] = new_macro;
            }
            else
            {
                items_list_.append(new_macro);
            }
        }

        // Update tool tips.
        UpdateToolTips();

        // Unblock signals from the list widget.
        ui_.itemsList->blockSignals(false);

        // Update buttons.
        UpdateButtons();
    }
}
