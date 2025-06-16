//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for Titlebar widget in RGA Build view's File Menu.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_TITLEBAR_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_TITLEBAR_H_

// Qt.
#include <QWidget>

// Forward declarations:
class QStackedLayout;
class QVBoxLayout;
class QLabel;
class QLineEdit;

class RgMenuTitlebar : public QWidget
{
    Q_OBJECT

public:
    explicit RgMenuTitlebar(QWidget* parent = nullptr);
    ~RgMenuTitlebar();

    // Set the displayed title.
    void SetTitle(const std::string& title);

    // Start editing the title text.
    void StartEditing();

    // Stop editing the title text.
    void StopEditing();

protected:
    // Event handler for mouse double click event.
    void mouseDoubleClickEvent(QMouseEvent* event) override;

    // Handler invoked when the user hits a key.
    virtual void keyPressEvent(QKeyEvent* event) override;

private:
    // Refresh the tooltip to reflect the new label text.
    void RefreshTooltip(const std::string& tooltip);

    // Set mouse cursor to pointing hand cursor.
    void SetCursor();

    // Widget layout stack.
    QStackedLayout* stacked_layout_ = nullptr;

    // Main layout for this widget.
    QVBoxLayout* main_layout_ = nullptr;

    // Displayed title label.
    QLabel* title_label_ = nullptr;

    // Line text editor shown when renaming.
    QLineEdit* title_text_edit_ = nullptr;

private slots:
    void HandleReturnPressed();

signals:
    void TitleChanged(const std::string& title);

};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_MENU_TITLEBAR_H_
