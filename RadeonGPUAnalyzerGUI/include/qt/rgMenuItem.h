#pragma once

// Qt.
#include <QWidget>

// Forward declarations.
class rgMenu;

class rgMenuItem : public QWidget
{
    Q_OBJECT

public:
    rgMenuItem(rgMenu* pParent = nullptr);
    virtual ~rgMenuItem() = default;

    // The parent menu that this item lives within.
    rgMenu* GetParentMenu() const;

private:
    // The parent menu for the item.
    rgMenu* m_pParentMenu = nullptr;
};