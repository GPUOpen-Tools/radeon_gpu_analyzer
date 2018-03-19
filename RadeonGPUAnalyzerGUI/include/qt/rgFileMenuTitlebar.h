#pragma once

// Qt.
#include <QWidget>

// Forward declarations:
class QStackedLayout;
class QVBoxLayout;
class QLabel;
class QLineEdit;

class rgFileMenuTitlebar : public QWidget
{
    Q_OBJECT

public:
    explicit rgFileMenuTitlebar(QWidget* pParent = nullptr);
    ~rgFileMenuTitlebar();

    // Set the displayed title.
    void SetTitle(const std::string& title);

    // Start editing the title text.
    void StartEditing();

    // Stop editing the title text.
    void StopEditing();

protected:
    // Event handler for mouse double click event.
    void mouseDoubleClickEvent(QMouseEvent* pEvent) override;

    // Handler invoked when the user hits a key.
    virtual void keyPressEvent(QKeyEvent* pEvent) override;

private:
    // Refresh the tooltip to reflect the new label text.
    void RefreshTooltip(const std::string& tooltip);

    // Resize the project name tooltip box.
    void ResizeTooltipBox(const std::string& tooltip);

    // Set mouse cursor to pointing hand cursor.
    void SetCursor();

    // Widget layout stack.
    QStackedLayout* m_pStackedLayout = nullptr;

    // Main layout for this widget.
    QVBoxLayout* m_pMainLayout = nullptr;

    // Displayed title label.
    QLabel* m_pTitleLabel = nullptr;

    // Line text editor shown when renaming.
    QLineEdit* m_pTitleTextEdit = nullptr;

private slots:
    void HandleReturnPressed();

signals:
    void TitleChanged(const std::string& title);

};