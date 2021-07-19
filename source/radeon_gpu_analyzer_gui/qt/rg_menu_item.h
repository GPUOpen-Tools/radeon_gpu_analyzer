#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_ITEM_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_ITEM_H_

// Qt.
#include <QWidget>

// Forward declarations.
class RgMenu;

class RgMenuItem : public QWidget
{
    Q_OBJECT

public:
    RgMenuItem(RgMenu* parent = nullptr);
    virtual ~RgMenuItem() = default;

    // The parent menu that this item lives within.
    RgMenu* GetParentMenu() const;

private:
    // The parent menu for the item.
    RgMenu* parent_menu_ = nullptr;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_ITEM_H_
