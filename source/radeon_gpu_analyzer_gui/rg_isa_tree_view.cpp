//=============================================================================
/// Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief Implementation for shader ISA Disassembly tree view.
//=============================================================================

// Qt.
#include <QtWidgets/QApplication>

// QtCommon.
#include "qt_common/utils/qt_util.h"

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_proxy_model.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_tree_view.h"

// The style for the tooltip background color.
static const QString kStyleSheetText = "QToolTip {background-color: palette(window); border: 1px solid palette(text);}";

RgIsaTreeView::RgIsaTreeView(QWidget* parent, RgIsaDisassemblyView* disassembly_view)
    : IsaTreeView(parent)
    , parent_disassembly_view_(disassembly_view)
{
    RgIsaItemDelegate* rg_isa_item_delegate = new RgIsaItemDelegate(this);
    ReplaceDelegate(rg_isa_item_delegate);

     // Explicitly set style for the tooltip background color, and connect it to theme color update.
    HandleColorThemeChanged();
    connect(&QtCommon::QtUtils::ColorTheme::Get(), &QtCommon::QtUtils::ColorTheme::ColorThemeUpdated, this, &RgIsaTreeView::HandleColorThemeChanged);

    // Initialize the context menu.
    InitializeContextMenu();

    // Connect signals used for actions in the top level disassembly view.
    ConnectDisassemblyViewSignals();

    // Connect signal for when the isa tree view selects and scrolls to a new line.
    connect(this, &IsaTreeView::ScrolledToIndex, this, &RgIsaTreeView::HandleScrolledToIndex);
}

RgIsaTreeView::~RgIsaTreeView()
{
}

void RgIsaTreeView::HandleScrolledToIndex(const QModelIndex source_index)
{
    UpdateLineCorrelation(source_index, true);
}

bool RgIsaTreeView::UpdateLineCorrelation(const QModelIndex source_index, bool update_source_code_editor)
{
    bool ret = false;

    if (parent_disassembly_view_ != nullptr && parent_disassembly_view_->IsLineCorrelationSupported())
    {
        const RgIsaProxyModel* proxy = qobject_cast<const RgIsaProxyModel*>(this->model());
        RgIsaItemModel*        model = nullptr;

        QModelIndex isa_tree_view_index = source_index;

        if (proxy != nullptr)
        {
            isa_tree_view_index = proxy->mapFromSource(isa_tree_view_index);
            model               = qobject_cast<RgIsaItemModel*>(proxy->sourceModel());
        }
        else
        {
            model = qobject_cast<RgIsaItemModel*>(model);
        }

        isa_tree_view_index = isa_tree_view_index.siblingAtColumn(IsaItemModel::kLineNumber);

        if (model != nullptr && parent_disassembly_view_ != nullptr)
        {
            if (isa_tree_view_index.isValid())
            {
                // Line in the input src correlated with the current line in the isa.
                int correlated_src_line_index = isa_tree_view_index.data(RgIsaItemModel::UserRoles::kIsaRowToSrcLineRole).toInt();

                RgIsaItemModel::EntryData entry_data{};
                entry_data.input_source_line_index = correlated_src_line_index;
                entry_data.operation               = RgIsaItemModel::EntryData::Operation::kUpdateLineCorrelation;
                model->UpdateData(&entry_data);

                if (update_source_code_editor)
                {
                    emit HighlightedIsaRowChanged(correlated_src_line_index);
                }

                ret = true;
            }
            else
            {
                RgIsaItemModel::EntryData entry_data{};
                entry_data.input_source_line_index = kInvalidCorrelationLineIndex;
                entry_data.operation               = RgIsaItemModel::EntryData::Operation::kUpdateLineCorrelation;
                model->UpdateData(&entry_data);

                viewport()->update();
            }
        }
    }
    
    return ret;
}

void RgIsaTreeView::drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // Paint the rows contents.
    IsaTreeView::drawRow(painter, option, index);

    int row_height = option.rect.height();
    if (row_height == 0)
    {
        // If the row height is zero we wouldn't paint anything anyway.
        return;
    }

    // Save the painter object.
    painter->save();

    bool is_index_valid = index.isValid();
    assert(is_index_valid);
    if (is_index_valid)
    {
        // Is the current instruction row correlated with the currently selected line in the input file?
        bool is_line_correlated_with_input_file = index.data(RgIsaItemModel::UserRoles::kSrcLineToIsaRowRole).toBool();
        if (is_line_correlated_with_input_file)
        {
            // Paint the row background with the highlight color.
            const QColor row_highlight_color = QtCommon::QtUtils::ColorTheme::Get().GetCurrentThemeColors().line_correlation_highlight_color;
            painter->fillRect(option.rect, row_highlight_color);
        }

        const bool is_max_vgpr_row = index.data(RgIsaItemModel::UserRoles::kMaxVgprLineRole).toBool();
        if (is_max_vgpr_row)
        {
            // Initialize painter variables.
            QPen  pen  = painter->pen();
            QRect rect = option.rect;

            // Color the bounds of the current maximum VGPR line.
            pen.setColor(Qt::GlobalColor::red);
            pen.setWidth(2);
            painter->setPen(pen);
            painter->drawRect(rect);
        }

        // Restore the painter object.
        painter->restore();
    }
}

void RgIsaTreeView::HandleEnableShowMaxVgprOptionSignal(bool is_enabled)
{
    show_maximum_vgpr_lines_->setEnabled(is_enabled);
}

void RgIsaTreeView::ConnectContextMenuSignals()
{
    // Connect the handler responsible for triggering a copy to clipboard from the table.
    bool is_connected = connect(copy_selected_disassembly_, &QAction::triggered, this, &RgIsaTreeView::HandleCopyDisassemblyClicked);
    assert(is_connected);

    // Connect the handler responsible for opening the current disassembly build output folder in the system file browser.
    is_connected = connect(open_disassembly_in_file_browser_, &QAction::triggered, this, &RgIsaTreeView::OpenDisassemblyInFileBrowserSignal);
    assert(is_connected);

    // Connect the handler responsible for showing the max VGPR line.
    is_connected = connect(show_maximum_vgpr_lines_, &QAction::triggered, this, &RgIsaTreeView::HandleShowNextMaxVgpr);
    assert(is_connected);

    // Use a custom context menu for the table.
    this->setContextMenuPolicy(Qt::CustomContextMenu);

    // Connect the handler responsible for showing the table's context menu.
    is_connected = connect(this, &QWidget::customContextMenuRequested, this, &RgIsaTreeView::HandleOpenContextMenu);
    assert(is_connected);
}

void RgIsaTreeView::InitializeContextMenu()
{
    // Create the context menu instance.
    context_menu_ = new QMenu(this);

    // Set the cursor for the context menu.
    context_menu_->setCursor(Qt::PointingHandCursor);

    // Create the menu items to insert into the context menu.
    copy_selected_disassembly_ = new QAction(kStrDisassemblyTableContextMenuCopy, this);
    context_menu_->addAction(copy_selected_disassembly_);

    // Create an item allowing the user to jump to highest VGPR pressure line.
    show_maximum_vgpr_lines_ = new QAction(kStrDisassemblyTableContextMenuGoToMaxVgpr, this);
    context_menu_->addAction(show_maximum_vgpr_lines_);

    // Create an item allowing the user to browse to the disassembly build output folder.
    open_disassembly_in_file_browser_ = new QAction(kStrDisassemblyTableContextMenuOpenInFileBrowser, this);
    context_menu_->addAction(open_disassembly_in_file_browser_);

    // Connect the context menu signals.
    ConnectContextMenuSignals();
}

void RgIsaTreeView::ConnectDisassemblyViewSignals()
{
    if (parent_disassembly_view_ != nullptr)
    {
        // Connect the handler responsible for responding to a change in input src line change.
        bool is_connected = connect(
            parent_disassembly_view_, &RgIsaDisassemblyView::InputSourceHighlightedLineChanged, this, &RgIsaTreeView::HandleHighlightedInputSrcLineChanged);
        assert(is_connected);

        is_connected = connect(parent_disassembly_view_, &RgIsaDisassemblyView::ShowNextMaxVgprClickedSignal, this, &RgIsaTreeView::HandleShowNextMaxVgpr);
        assert(is_connected);

        is_connected = connect(parent_disassembly_view_, &RgIsaDisassemblyView::ShowPrevMaxVgprClickedSignal, this, &RgIsaTreeView::HandleShowPrevMaxVgpr);
        assert(is_connected);
    }
}

void RgIsaTreeView::HandleCopyDisassemblyClicked()
{
    CopyRowsToClipboard();
}

void RgIsaTreeView::HandleOpenContextMenu(const QPoint& widget_click_position)
{
    // Convert the widget's local click position to the global screen position.
    const QPoint click_point = mapToGlobal(widget_click_position);

    // Open the context menu at the user's click position.
    context_menu_->exec(click_point);
}

void RgIsaTreeView::HandleHighlightedInputSrcLineChanged(int src_line_index)
{
    const RgIsaProxyModel* proxy = qobject_cast<const RgIsaProxyModel*>(this->model());
    RgIsaItemModel*        model = nullptr;

    if (proxy != nullptr)
    {
        model = qobject_cast<RgIsaItemModel*>(proxy->sourceModel());
    }
    else
    {
        model = qobject_cast<RgIsaItemModel*>(model);
    }

    if (model != nullptr)
    {
        QModelIndex isa_tree_view_index = model->GetFirstLineCorrelatedIndex(src_line_index);
        if (isa_tree_view_index.isValid())
        {
            ScrollToIndex(isa_tree_view_index, false, false, false);
        }

        UpdateLineCorrelation(isa_tree_view_index, false);
    }
}

void RgIsaTreeView::HandleShowNextMaxVgpr()
{
    const RgIsaProxyModel* proxy = qobject_cast<const RgIsaProxyModel*>(this->model());
    RgIsaItemModel*        model = nullptr;

    if (proxy != nullptr)
    {
        model = qobject_cast<RgIsaItemModel*>(proxy->sourceModel());
    }
    else
    {
        model = qobject_cast<RgIsaItemModel*>(model);
    }

    if (model != nullptr)
    {
        RgIsaItemModel::EntryData entry_data{};
        entry_data.operation = RgIsaItemModel::EntryData::Operation::kGoToNextMaxVgpr;
        model->UpdateData(&entry_data);

        QModelIndex isa_tree_view_index = model->GetMaxVgprIndex();
        if (isa_tree_view_index.isValid())
        {
            ScrollToIndex(isa_tree_view_index, false, false, false);
        }

        UpdateLineCorrelation(isa_tree_view_index, true);
    }
}

void RgIsaTreeView::HandleShowPrevMaxVgpr()
{
    const RgIsaProxyModel* proxy = qobject_cast<const RgIsaProxyModel*>(this->model());
    RgIsaItemModel*        model = nullptr;

    if (proxy != nullptr)
    {
        model = qobject_cast<RgIsaItemModel*>(proxy->sourceModel());
    }
    else
    {
        model = qobject_cast<RgIsaItemModel*>(model);
    }

    if (model != nullptr)
    {
        RgIsaItemModel::EntryData entry_data{};
        entry_data.operation = RgIsaItemModel::EntryData::Operation::kGoToPrevMaxVgpr;
        model->UpdateData(&entry_data);

        QModelIndex isa_tree_view_index = model->GetMaxVgprIndex();
        if (isa_tree_view_index.isValid())
        {
            ScrollToIndex(isa_tree_view_index, false, false, false);
        }

        UpdateLineCorrelation(isa_tree_view_index, true);
    }
}

void RgIsaTreeView::HandleColorThemeChanged()
{
    setStyleSheet(kStyleSheetText);
}
