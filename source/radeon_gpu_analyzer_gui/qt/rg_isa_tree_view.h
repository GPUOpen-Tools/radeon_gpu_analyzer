//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Header for shader ISA Disassembly tree view.
//=============================================================================

#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_TREE_VIEW_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_TREE_VIEW_H_

// Qt.
#include <QAction>
#include <QMenu>
#include <QKeyEvent>

// Infra.
#include "qt_common/custom_widgets/scaled_tree_view.h"
#include "qt_isa_gui/widgets/isa_item_model.h"
#include "qt_isa_gui/widgets/isa_tree_view.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_isa_item_model.h"

// Forward declaration to prevent a circular dependency.
class IsaWidget;
class RgIsaDisassemblyView;

// RgIsaTreeView is a tree view intended to be attached to a RgIsaItemModel
// to display isa in a tree structure. It instantiates and uses a RgIsaItemDelegate
// to do any custom rendering and handle user interaction.
class RgIsaTreeView : public IsaTreeView
{
    Q_OBJECT

public:
    // Constructor; set default properties on tree and header, create delegate and scroll bar.
    explicit RgIsaTreeView(QWidget* parent = nullptr, RgIsaDisassemblyView* disassembly_view = nullptr);

    // Destructor.
    virtual ~RgIsaTreeView();

    // Respond to a request to update line correlation wrt. given index.
    bool UpdateLineCorrelation(const QModelIndex source_index, bool update_source_code_editor);

public slots:
    // A handler to update the context menu next maximum live VGPR line option.
    void HandleEnableShowMaxVgprOptionSignal(bool is_enabled);

protected:
    // Override drawRow to manually paint alternating background colors to assist painting labels and comments across columns.
    /// In order to paint code block labels and comments such that they span multiple columns, we have to manually paint the alternating background color in the tree.
    /// If we let Qt paint the background color, it will paint over our attempt to span multiple columns.
    virtual void drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const Q_DECL_OVERRIDE;
    
    // Connect signals used for the right-click context menu.
    void ConnectContextMenuSignals();

    // Initialize the context menu attached to the disassembly table.
    void InitializeContextMenu();

    // Connect signals used for actions in the top level disassembly_view.
    void ConnectDisassemblyViewSignals();

signals:
    // A signal emitted when the user changes the correlated ISA row. Parameter is the input file's line number to select.
    void HighlightedIsaRowChanged(int input_src_line_number);

    // A signal emitted when the user clicks the context menu to open disassembly file in the Browser.
    void OpenDisassemblyInFileBrowserSignal();

private slots:
    // Handler invoked when the copy item is clicked in the table's context menu.
    void HandleCopyDisassemblyClicked();

    // Handler invoked when the table's context menu should be opened.
    void HandleOpenContextMenu(const QPoint& widget_click_position);

    // Handler invoked when the input src line was changed.
    void HandleHighlightedInputSrcLineChanged(int src_line_index);

    // Handler invoked when the clicks the context menu to show next max vgpr line.
    void HandleShowNextMaxVgpr();

    // Handler invoked when the clicks the context menu to show prev max vgpr line.
    void HandleShowPrevMaxVgpr();

    // Handler invoked when the color theme is changed.
    void HandleColorThemeChanged();

    // Handler invoked when the tree view programmatically selects and scrolls to a new index. 
    // Updates line correlation and notifies the source code editor that it needs to select a new line.
    void HandleScrolledToIndex(const QModelIndex source_index);

private:
    // Top level disassembly view.
    RgIsaDisassemblyView* parent_disassembly_view_ = nullptr;

    // The context menu displayed when the user left-clicks on the disassembly table.
    QMenu* context_menu_ = nullptr;

    // The context menu item used to copy the selected disassembly rows.
    QAction* copy_selected_disassembly_ = nullptr;

    // The context menu item used to open the current disassembly table's data in the file browser.
    QAction* open_disassembly_in_file_browser_ = nullptr;

    // The context menu item used to navigate to the maximum VGPR line.
    QAction* show_maximum_vgpr_lines_ = nullptr;
};

#endif  // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_TREE_VIEW_H_
