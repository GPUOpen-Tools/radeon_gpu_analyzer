//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for shader ISA Disassembly item model delegate.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_ITEM_DELEGATE_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_ITEM_DELEGATE_H_

// Qt.
#include <QModelIndex>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>

// Infra.
#include "qt_isa_gui/widgets/isa_item_delegate.h"
#include "qt_isa_gui/widgets/isa_tree_view.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_isa_item_model.h"

// RgIsaItemDelegate is a styled delegate to be used with the SharedIsaTreeView.
// It custom paints isa text and handles user interaction.
class RgIsaItemDelegate : public IsaItemDelegate
{
    Q_OBJECT

public:
    RgIsaItemDelegate(IsaTreeView* view, QObject* parent = nullptr);

    ~RgIsaItemDelegate();

    // Override editor event in order to track mouse moves and mouse clicks over code block labels and selectable tokens.
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) Q_DECL_OVERRIDE;

    // Override paint to custom render isa text.
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& model_index) const Q_DECL_OVERRIDE;

    // Override sizeHint to cache text width to improve performance.
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const Q_DECL_OVERRIDE;
};

#endif  // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_ITEM_DELEGATE_H_
