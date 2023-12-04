#pragma once

// Qt.
#include <QWidget>

// Local.
#include "ui_rg_link_source_menu_item.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_menu_item.h"

// Forward declarations:
class QPushButton;

class RgLinkSourceMenuItem :
    public RgMenuItem
{
    Q_OBJECT

public:
    explicit RgLinkSourceMenuItem(RgMenu* parent = nullptr);
    virtual ~RgLinkSourceMenuItem() = default;

    // Getter for the Load button.
    QPushButton* GetLoadCodeObjButton() const;

    // Getter for the Add button.
    QPushButton* GetLinkSourceButton() const;

    // Toggle Visiblity for Horizontal line separators.
    void ToggleLineSeparatorVisibilty(bool visible) const;

    // Toggle Visiblity for Load Code Object Button.
    void ToggleLoadCodeObjectButtonVisibilty(bool visible) const;

private:
    // Set the cursor to pointing hand cursor for various widgets.
    void SetCursor();

    // The generated UI object for this view.
    Ui::RgLinkSourceMenuItem ui_;
};
