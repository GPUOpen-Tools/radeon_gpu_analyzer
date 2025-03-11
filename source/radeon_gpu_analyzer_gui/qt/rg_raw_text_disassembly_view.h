#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_RAW_TEXT_DISASSEMBLY_VIEW_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_RAW_TEXT_DISASSEMBLY_VIEW_H_

// Qt.
#include <QAction>
#include <QMenu>
#include <QObject>
#include <QPlainTextEdit>

class RgRawTextDisassemblyView : public QPlainTextEdit
{
    Q_OBJECT

public:
    // Ctor
    RgRawTextDisassemblyView(QWidget* parent = nullptr);

    // Sets the contents of the source code view.
    void setText(const QString& txt);

    // Clears the contents of the source code view.
    void clearText(const QString& txt);

private slots:
    // Show this source view's context menu to the user.
    void ShowContextMenu(const QPoint& location);

protected:
    // Action for copying text.
    QAction* copy_text_action_ = nullptr;

    // Action for selecting all text.
    QAction* select_all_text_action_ = nullptr;

    // This source view's context menu.
    QMenu* context_menu_ = nullptr;
};

#endif  // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_RAW_TEXT_DISASSEMBLY_VIEW_H_
