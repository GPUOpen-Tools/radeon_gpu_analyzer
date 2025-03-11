// C++.
#include <algorithm>

// Qt.
#include <QColor>
#include <QEvent>
#include <QGuiApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QSortFilterProxyModel>

// Infra.
#include "qt_common/utils/common_definitions.h"
#include "qt_isa_gui/utility/isa_dictionary.h"
#include "qt_common/utils/qt_util.h"

// Local.
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_item_delegate.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_proxy_model.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_tree_view.h"

// Colors for VGPR pressure ranges.
static const QColor kVgprRangeZeroColor  = QColor(3, 252, 152);
static const QColor kVgprRangeOneColor   = QColor(20, 252, 3);
static const QColor kVgprRangeTwoColor   = QColor(252, 244, 3);
static const QColor kVgprRangeThreeColor = QColor(252, 215, 3);
static const QColor kVgprRangeFourColor  = QColor(252, 152, 3);
static const QColor kVgprRangeFiveColor  = QColor(252, 115, 3);
static const QColor kVgprRangeSixColor   = QColor(252, 74, 3);
static const QColor kVgprRangeSevenColor = QColor(252, 3, 3);

static const QColor kVgprRangeColors[] = {kVgprRangeZeroColor,
                                          kVgprRangeOneColor,
                                          kVgprRangeTwoColor,
                                          kVgprRangeThreeColor,
                                          kVgprRangeFourColor,
                                          kVgprRangeFiveColor,
                                          kVgprRangeSixColor,
                                          kVgprRangeSevenColor};

// Begin and end for color range indexes.
static const int kBeginRange = 0;
static const int kEndRange   = 7;

// Equivalent to dividing by 2^5 (32).
// 32 is calculated as, max number of vgprs (256) / kVgprRangeColors.size() which is 8.
static const int kVgprBitShift = 5;

// "Unknown" functional group string.
static const int     kFunctionalGroupNameIndexUnknown = 0;
static const QString kFunctionalGroupNameUnknown      = amdisa::kFunctionalGroupName[kFunctionalGroupNameIndexUnknown];

RgIsaItemDelegate::RgIsaItemDelegate(IsaTreeView* view, QObject* parent)
    : IsaItemDelegate(view, parent)
{
}

RgIsaItemDelegate::~RgIsaItemDelegate()
{
}

bool RgIsaItemDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
    bool ret = false;
    switch (event->type())
    {
    case QEvent::MouseButtonRelease:
    {
        ret = IsaItemDelegate::editorEvent(event, model, option, index);
        if (!ret)
        {
            const QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);

            if (mouse_event != nullptr && mouse_event->button() == Qt::LeftButton)
            {
                QModelIndex                  source_index;
                const QSortFilterProxyModel* proxy = qobject_cast<QSortFilterProxyModel*>(model);

                // Get source index.
                if (proxy != nullptr)
                {
                    source_index = proxy->mapToSource(index);
                }
                else
                {
                    source_index = index;
                }

                // Normal bounds checking.
                if (!source_index.isValid())
                {
                    ret = true;
                }

                RgIsaTreeView* view = qobject_cast<RgIsaTreeView*>(view_);
                if (view != nullptr)
                {
                    view->UpdateLineCorrelation(source_index);
                }
            }

        }
        break;
    }
    default:
    {
        ret = IsaItemDelegate::editorEvent(event, model, option, index);
        break;
    }
    }

    return ret;
}

void RgIsaItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& model_index) const
{
    // Bounds checking.
    if (!model_index.isValid())
    {
        return;
    }

    const QSortFilterProxyModel* proxy_model        = qobject_cast<const QSortFilterProxyModel*>(model_index.model());
    const RgIsaItemModel*        model              = nullptr;
    auto                         source_model_index = model_index;

    // Get the source index.
    if (proxy_model == nullptr)
    {
        model = qobject_cast<const RgIsaItemModel*>(model_index.model());
    }
    else
    {
        model              = qobject_cast<const RgIsaItemModel*>(proxy_model->sourceModel());
        source_model_index = proxy_model->mapToSource(model_index);
    }

    // Error checking.
    if (model == nullptr)
    {
        return;
    }

    // Ask the isa delegate to paint shared columns, row highlight, and pinned block labels.
    IsaItemDelegate::paint(painter, option, model_index);

    // Check if there should be a block label pinned to the top of the view.
    int proxy_index_y_position = -1;

    if (BlockLabelPinnedToTop(source_model_index, model_index, proxy_index_y_position))
    {
        // Don't paint anything else if this index is a block label pinned to the top of the view.
        return;
    }

    QStyleOptionViewItem initialized_option = option;
    initStyleOption(&initialized_option, source_model_index);

    painter->save();

    painter->setFont(initialized_option.font);

    // Get a default text color if applicable.
    QVariant color_data = source_model_index.data(Qt::ForegroundRole);
    if (color_data.isValid())
    {
        auto pen = painter->pen();
        pen.setColor(color_data.value<QColor>());
        painter->setPen(pen);
    }

    if (source_model_index.column() < IsaItemModel::Columns::kColumnCount)
    {
        return;
    }
    else if (source_model_index.column() == RgIsaItemModel::Columns::kIsaColumnVgprPressure)
    {
        int   num_live_registers     = 0;
        int   block_allocation_value = 0;
        int   allocated_rect_width   = 0;
        QFont font;

        // Get the number of VGPR registers.
        QString num_live_registers_str = model_index.data().toString();

        // Process the string to get the live VGPR number and
        // the allocation block granularity.
        QStringList values = num_live_registers_str.split(",");
        if (values.size() == 2)
        {
            num_live_registers     = values.at(0).toInt();
            block_allocation_value = values.at(1).toInt();
        }

        // If the number of live registers is greater than zero,
        // process it.
        if (num_live_registers > 0)
        {
            painter->save();

            // Calculate the color swatch rectangle.
            QRect rect = option.rect;
            rect.setWidth(std::min(num_live_registers, option.rect.width()));
            rect.setY(rect.y() + 1);
            rect.setHeight(std::max(1, rect.height() - 1));
            
            // Convert the one-based live register number into the range that it falls into to use as an array index.
            int array_index = std::clamp((num_live_registers - 1) >> kVgprBitShift, kBeginRange, kEndRange);

            // Draw the rectangle indicating VGPR pressure.
            allocated_rect_width = num_live_registers;
            assert((array_index >= kBeginRange) && (array_index <= kEndRange));
            if ((array_index >= kBeginRange) && (array_index <= kEndRange))
            {
                // Draw a rectangle upto the boundary of this range.
                if (num_live_registers % block_allocation_value != 0)
                {
                    allocated_rect_width = (num_live_registers / block_allocation_value + 1) * block_allocation_value;
                }

                rect.setWidth(std::min(allocated_rect_width, option.rect.width()));
                painter->drawRect(rect);

                // Draw the color swatch.
                rect.setWidth(std::min(num_live_registers, option.rect.width()));
                QColor color = kVgprRangeColors[array_index];
                painter->fillRect(rect, color);
            }
            else
            {
                // We should not get here.
                assert(false);
            }

            painter->restore();
        }

        // Draw the number of live registers value text if it fits.
        QString live_reg_label;
        QRect   r         = option.rect;
        QRect   text_rect = painter->fontMetrics().boundingRect(QString::number(num_live_registers));
        if (num_live_registers_str.compare("N/A") == 0)
        {
            live_reg_label = "N/A";
            painter->drawText(r, live_reg_label);
        }
        else
        {
            live_reg_label = QString::number(num_live_registers);
            int adjusted_x = r.x() + std::min(allocated_rect_width, option.rect.width()) + 10;

            // Check if the text fits within the bounds.
            if (adjusted_x + text_rect.width() <= option.rect.right())
            {
                r.setX(adjusted_x);
                r.setWidth(option.rect.width() + text_rect.width());
                painter->drawText(r, live_reg_label);
            }
        }
    }
    else if (source_model_index.column() == RgIsaItemModel::Columns::kIsaColumnFunctionalUnit)
    {   
        QString functional_group_name = kFunctionalGroupNameUnknown;

        const QVariant data = model_index.data();
        if (data.isValid())
        {
            const auto decoded_info     = qvariant_cast<amdisa::InstructionInfo>(data);
            const auto op_code          = QString(decoded_info.instruction_name.c_str()).toLower().toStdString();
            const auto functional_group = decoded_info.functional_group_subgroup_info.IsaFunctionalGroup;
            functional_group_name       = amdisa::kFunctionalGroupName[static_cast<int>(functional_group)];
        }


        painter->drawText(initialized_option.rect, Qt::TextSingleLine, functional_group_name);
    }
    else
    {
        painter->drawText(initialized_option.rect, initialized_option.displayAlignment, source_model_index.data(Qt::DisplayRole).toString());
    }

    painter->restore();
}

QSize RgIsaItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QAbstractProxyModel*  proxy_model  = qobject_cast<const QAbstractProxyModel*>(index.model());
    const RgIsaItemModel* source_model = nullptr;

    if (proxy_model == nullptr)
    {
        source_model = qobject_cast<const RgIsaItemModel*>(index.model());
    }
    else
    {
        source_model = qobject_cast<const RgIsaItemModel*>(proxy_model->sourceModel());
    }

    if (source_model == nullptr)
    {
        return QSize(0, 0);
    }

    QModelIndex source_model_index;

    if (proxy_model == nullptr)
    {
        // No proxy being used.
        source_model_index = index;
    }
    else
    {
        // Proxy being used.
        source_model_index = proxy_model->mapToSource(index);
    }

    if (source_model_index.column() == IsaItemModel::kLineNumber && !source_model->LineNumbersVisible())
    {
        return source_model->ColumnSizeHint(-1, view_);
    }

    if (source_model_index.column() < IsaItemModel::kColumnCount)
    {
        return IsaItemDelegate::sizeHint(option, index);
    }

    return source_model->ColumnSizeHint(source_model_index.column(), view_);
}