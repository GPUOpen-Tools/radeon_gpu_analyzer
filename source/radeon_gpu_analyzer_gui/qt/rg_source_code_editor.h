//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for source code editor.
//=============================================================================
#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SOURCE_CODE_EDITOR_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SOURCE_CODE_EDITOR_H_

// Qt.
#include <QPlainTextEdit>
#include <QObject>
#include <QAction>

// Infra.
#include "qt_isa_gui/widgets/shader_source_code_viewer.h"

// Forward declarations.
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class LineNumberArea;

class RgSourceCodeEditor : public ShaderSourceCodeViewer
{
    Q_OBJECT

public:
    // Ctor
    RgSourceCodeEditor(QWidget* parent = nullptr, ShaderSourceLanguage lang = ShaderSourceLanguage::Unknown);

    // Returns true if this editor is a placeholder for a missing source file.
    bool IsPlaceholder() const { return is_placeholder_editor_; }

    // Mark this editor as a placeholder for a missing source file.
    void SetIsPlaceholder(bool is_placeholder) { is_placeholder_editor_ = is_placeholder; }

signals:
    // A signal emitted when the source code editor gets the focus.
    void SourceCodeEditorFocusInEvent();

    // A signal to disable table scroll bar signals.
    void DisableScrollbarSignals();

    // A signal to enable table scroll bar signals.
    void EnableScrollbarSignals();

    // A signal that requests the system to open a header file.
    void OpenHeaderFileRequested(const QString& header_file_path);

    // Emitted when a placeholder editor detects a click on the "Click here" link text.
    void OpenSourceSearchDirectoriesRequested();

protected:
    // The overridden mousePressEvent.
    virtual void mousePressEvent(QMouseEvent* event) override;

    // Show this source view's context menu to the user.
    void ShowContextMenu(const QPoint& location) override;

    // Override the parent functions to disable line highlighting when the editor is in read-only mode.
    void HighlightCursorLine(QList<QTextEdit::ExtraSelection>& selections) override;

    // Override the parent functions to disable source correlation when the editor is in read-only mode.
    void HighlightCorrelatedSourceLines(QList<QTextEdit::ExtraSelection>& selections) override;

    // Override to show a hand cursor over the "Click here" line in placeholder editors.
    void mouseMoveEvent(QMouseEvent* event) override;

private slots:

    // Handler for opening a header file.
    void HandleOpenHeaderFile();

protected:
    // Action for opening a header file.
    QAction* open_header_file_action_ = nullptr;

    // Action for cutting text.
    QAction* cut_text_action_ = nullptr;

    // Action for pasting text.
    QAction* paste_text_action_ = nullptr;

    // True if this editor is a placeholder for a missing source file.
    bool is_placeholder_editor_ = false;
};
#endif  // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_SOURCE_CODE_EDITOR_H_
