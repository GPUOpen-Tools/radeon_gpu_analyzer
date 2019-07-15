#pragma once

// Qt.
#include <QPushButton>

class rgBrowseButton : public QPushButton
{
    Q_OBJECT

public:
    rgBrowseButton(QWidget* pParent);
    virtual ~rgBrowseButton() = default;

protected:
    // Re-implement focusInEvent.
    virtual void focusInEvent(QFocusEvent* pEvent) override;

    // Re-implement focusOutEvent.
    virtual void focusOutEvent(QFocusEvent* pEvent) override;

signals:
    // Signal the focus in event.
    void BrowseButtonFocusInEvent();

    // Signal the focus out event.
    void BrowseButtonFocusOutEvent();
};

