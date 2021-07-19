#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ADD_CREATE_MENU_ITEMS_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ADD_CREATE_MENU_ITEMS_H_

// Qt.
#include <QWidget>

// Local.
#include "ui_rg_add_create_menu_item.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_menu_item.h"

// Forward declarations:
class QPushButton;

class RgAddCreateMenuItem :
    public RgMenuItem
{
    Q_OBJECT

public:
    explicit RgAddCreateMenuItem(RgMenu* parent = nullptr);
    virtual ~RgAddCreateMenuItem() = default;

    // Getter for the Add button.
    QPushButton* GetAddButton() const;

    // Getter for the Create button.
    QPushButton* GetCreateButton() const;

private:
    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // The generated UI object for this view.
    Ui::RgAddCreateMenuItem ui_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ADD_CREATE_MENU_ITEMS_H_
