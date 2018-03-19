#pragma once

// Qt.
#include <QWidget>

// Forward declarations.
class rgFileMenu;

class rgFileMenuItem : public QWidget
{
    Q_OBJECT

public:
    rgFileMenuItem(rgFileMenu* pParent = nullptr);
    virtual ~rgFileMenuItem() = default;

    // The parent menu that this item lives within.
    rgFileMenu* GetParentMenu() const;

private:
    // The parent menu for the item.
    rgFileMenu* m_pParentMenu = nullptr;
};