// C++.
#include <cassert>

// Qt.
#include <QApplication>
#include <QItemDelegate>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_custom_table_view.h"
#include "radeon_gpu_analyzer_gui/qt/rg_isa_disassembly_table_model.h"
#include "radeon_gpu_analyzer_gui/rg_data_types.h"
#include "radeon_gpu_analyzer_gui/rg_definitions.h"
#include "radeon_gpu_analyzer_gui/rg_string_constants.h"

// The highlight color to use for correlated source lines.
static QColor kCorrelationHighlightColor = QColor(Qt::yellow).lighter(170);
static QColor kLightBlueSelectionColor = QColor(229, 243, 255);

// Colors for VGPR pressure ranges.
static const QColor kVgprRangeZeroColor    = QColor(3, 252, 152);
static const QColor kVgprRangeOneColor     = QColor(20, 252, 3);
static const QColor kVgprRangeTwoColor     = QColor(252, 244, 3);
static const QColor kVgprRangeThreeColor   = QColor(252, 215, 3);
static const QColor kVgprRangeFourColor    = QColor(252, 152, 3);
static const QColor kVgprRangeFiveColor    = QColor(252, 115, 3);
static const QColor kVgprRangeSixColor     = QColor(252, 74, 3);
static const QColor kVgprRangeSevenColor   = QColor(252, 3, 3);

static const QColor kVgprRangeColors[] =
    {
        kVgprRangeZeroColor,
        kVgprRangeOneColor,
        kVgprRangeTwoColor,
        kVgprRangeThreeColor,
        kVgprRangeFourColor,
        kVgprRangeFiveColor,
        kVgprRangeSixColor,
        kVgprRangeSevenColor
    };

// Begin and end for color range indexes.
static const int kBeginRange = 0;
static const int kEndRange   = 7;

// Live VGPR register allocation block.
static const int kLiveVgprAllocationBlock = 8;

static void DrawVgprWidget(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& model_index)
{
    int num_live_registers = 0;
    int block_allocation_value = 0;
    int allocated_rect_width   = 0;
    QFont font;

    // Save the painter object.
    painter->save();

    // Set the background highlight color for the cell when the row is selected.
    if (option.state & QStyle::State_Selected)
    {
        painter->fillRect(option.rect, kCorrelationHighlightColor);
    }

    // Get the number of VGPR registers.
    QString num_live_registers_str = model_index.data().toString();

    // If this is a label, do not do anything.
    if (num_live_registers_str.compare(kLiveVgprLabelString) == 0)
    {
        painter->restore();
        return;
    }
    else
    {
        // Process the string to get the live VGPR number and
        // the allocation block granularity.
        QStringList values = num_live_registers_str.split(",");
        if (values.size() == 2)
        {
            num_live_registers     = values.at(0).toInt();
            block_allocation_value = values.at(1).toInt();
        }
    }

    // If the number of live registers is greater than zero,
    // process it.
    if (num_live_registers > 0)
    {
        // Set painter options.
        font = painter->font();
        font.setBold(false);

        // Get and set the global font size.
        std::shared_ptr<RgGlobalSettings> global_config = RgConfigManager::Instance().GetGlobalConfig();
        font.setPointSize(global_config->font_size);

        // Set pen and font values.
        QPen pen = painter->pen();
        pen.setColor(Qt::GlobalColor::gray);
        painter->setPen(pen);
        painter->setFont(font);

        // Calculate the color swatch rectangle.
        QRect rect = option.rect;
        rect.setWidth(num_live_registers);
        rect.setY(rect.y() + 1);
        if (rect.height() > 1)
        {
            rect.setHeight(rect.height() - 1);
        }
        else
        {
            rect.setHeight(rect.height());
        }

        // Convert the one-based live register number into the range that
        // it falls into to use as an array index.
        int array_index = (num_live_registers - 1) >> 5;

        // If the range array_index comes out to be negative,
        // set it to zero.
        if (array_index < 0)
        {
            array_index = 0;
        }

        // If the range array_index comes out to be more than the max,
        // set it to the max value of 7.
        if (array_index > 7)
        {
            array_index = 7;
        }

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
            rect.setWidth(allocated_rect_width);
            painter->drawRect(rect);

            // Draw the color swatch.
            rect.setWidth(num_live_registers);
            QColor color = kVgprRangeColors[array_index];
            painter->fillRect(rect, color);
        }
        else
        {
            // We should not get here.
            assert(false);
        }
    }

    // Now draw the number of live registers value text.
    QString live_reg_label;
    QRect   r = option.rect;
    QRect   text_rect = painter->fontMetrics().boundingRect(QString::number(num_live_registers));
    if (num_live_registers_str.compare("N/A") == 0)
    {
        // If the number of VGPRs is N/A.
        live_reg_label = "N/A";
    }
    else
    {
        live_reg_label = QString::number(num_live_registers);
        r.setWidth(option.rect.width() + text_rect.width());
        r.setX(r.x() + allocated_rect_width + 10);
    }

    // Set the font.
    font = painter->font();
    font.setBold(false);
    painter->setFont(font);

    // Draw the text.
    qApp->style()->drawItemText(painter, r, option.displayAlignment, option.palette, true, live_reg_label, QPalette::Text);

    // Restore the painter object.
    painter->restore();
}

class RgTreeviewSelectionDelegateVulkan : public QItemDelegate
{
public:
    RgTreeviewSelectionDelegateVulkan(QObject* parent = nullptr)
        : QItemDelegate(parent) {}

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& model_index) const
    {
        // If this is the VGPR pressure column, go ahead and draw the register information.
        QString column_header = model_index.model()->headerData(model_index.column(), Qt::Orientation::Horizontal).toString();
        if (column_header.contains(kStrDisassemblyTableLiveVgprHeaderPart))
        {
            DrawVgprWidget(painter, option, model_index);
        }
        else
        {
            QStyleOptionViewItem view_option(option);

            QColor item_foreground_color = model_index.data(Qt::ForegroundRole).value<QColor>();

            view_option.palette.setColor(QPalette::HighlightedText, item_foreground_color);
            QColor background_color(kCorrelationHighlightColor);
            view_option.palette.setColor(QPalette::Highlight, background_color);

            QItemDelegate::paint(painter, view_option, model_index);
        }
    }
};

class RgTreeviewSelectionDelegateOpencl : public QItemDelegate
{
public:
    RgTreeviewSelectionDelegateOpencl(QObject* parent = nullptr) : QItemDelegate(parent) {}

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& model_index) const
    {
        // If this is the VGPR pressure column, go ahead and draw the register information.
        QString column_header = model_index.model()->headerData(model_index.column(), Qt::Orientation::Horizontal).toString();
        if (column_header.contains(kStrDisassemblyTableLiveVgprHeaderPart))
        {
            DrawVgprWidget(painter, option, model_index);
        }
        else
        {
            QStyleOptionViewItem view_option(option);

            QColor item_foreground_color = model_index.data(Qt::ForegroundRole).value<QColor>();

            view_option.palette.setColor(QPalette::HighlightedText, item_foreground_color);
            QColor background_color(kCorrelationHighlightColor);
            view_option.palette.setColor(QPalette::Highlight, background_color);

            QItemDelegate::paint(painter, view_option, model_index);
        }
    }
};

RgIsaDisassemblyCustomTableView::RgIsaDisassemblyCustomTableView(QWidget* parent) :
    QTreeView(parent)
{
    // Enable treeview's header's click event.
    QHeaderView* header_view = this->header();
    assert(header_view != nullptr);
    header_view->setSectionsClickable(true);

    // Connect signals.
    ConnectSignals();

    // Set the fonts.
    QFont font = this->font();
    font.setFamily(kStrBuildViewFontFamily);
    font.setPointSize(kBuildViewFontSize);
    this->setFont(font);

    RgProjectAPI current_api = RgConfigManager::Instance().GetCurrentAPI();
    if (current_api == RgProjectAPI::kVulkan)
    {
        setItemDelegate(new RgTreeviewSelectionDelegateVulkan(this));
    }
    else if (current_api == RgProjectAPI::kOpenCL)
    {
        setItemDelegate(new RgTreeviewSelectionDelegateOpencl(this));
    }
    else
    {
        // Should not get here.
        assert(false);
    }

    // Set focus to no focus to avoid a rectangle around the cell clicked on.
    setFocusPolicy(Qt::NoFocus);

    // Install the event filter to process tab and shift+tab key presses.
    installEventFilter(this);
}

void RgIsaDisassemblyCustomTableView::ConnectSignals()
{
    // Connect the header view.
    QHeaderView* header_view = this->header();
    assert(header_view != nullptr);

    if (header_view != nullptr)
    {
        bool is_connected = connect(header_view, &QHeaderView::sectionClicked, this, &RgIsaDisassemblyCustomTableView::HandleHeaderClicked);
        assert(is_connected);
    }

    // Connect the vertical scroll bar's sliderPressed and valueChanged signals,
    // so when the user clicks on the vertical scroll bar, or scrolls the view
    // by clicking on the scroll bar, the frame container gets the focus.
    QScrollBar* scroll_bar = this->verticalScrollBar();
    assert(scroll_bar != nullptr);

    if (scroll_bar != nullptr)
    {
        bool is_connected = connect(scroll_bar, &QScrollBar::sliderPressed, this, &RgIsaDisassemblyCustomTableView::HandleScrollBarClicked);
        assert(is_connected);

        is_connected = connect(scroll_bar, &QScrollBar::valueChanged, this, &RgIsaDisassemblyCustomTableView::HandleScrollBarClicked);
        assert(is_connected);
    }

    // Connect the horizontal scroll bar's sliderPressed and valueChanged signals,
    // so when the user clicks on the horizontal scroll bar, or scrolls the view
    // by clicking on the scroll bar, the frame container gets the focus.
    scroll_bar = this->horizontalScrollBar();
    assert(scroll_bar != nullptr);

    if (scroll_bar != nullptr)
    {
        bool is_connected = connect(scroll_bar, &QScrollBar::sliderPressed, this, &RgIsaDisassemblyCustomTableView::HandleScrollBarClicked);
        assert(is_connected);

        is_connected = connect(scroll_bar, &QScrollBar::valueChanged, this, &RgIsaDisassemblyCustomTableView::HandleScrollBarClicked);
        assert(is_connected);
    }
}

void RgIsaDisassemblyCustomTableView::SetLabelLinkWidgets(const std::vector<QLabel*>& label_links)
{
    label_links_ = label_links;
}

void RgIsaDisassemblyCustomTableView::SetModel(RgIsaDisassemblyTableModel* model)
{
    model_ = model;
}

void RgIsaDisassemblyCustomTableView::drawRow(QPainter* painter, const QStyleOptionViewItem& options, const QModelIndex& index) const
{
    if (model_ != nullptr)
    {
        // Painting with this delegate can be completely disabled depending on the incoming index.
        bool is_paint_enabled = true;

        QStyleOptionViewItem new_options(options);
        new_options.palette.setBrush(QPalette::Base, Qt::white);

        bool is_index_valid = index.isValid();
        assert(is_index_valid);
        if (is_index_valid)
        {
            // Is the current instruction row correlated with the currently selected line in the input file?
            bool is_line_correlated_with_input_file = model_->IsIsaLineCorrelated(index.row());
            if (is_line_correlated_with_input_file)
            {
                // Paint the row background with the highlight color.
                painter->fillRect(options.rect, kCorrelationHighlightColor);

                // If the row background gets painted manually, reset the row's background brush to be transparent.
                new_options.palette.setBrush(QPalette::Base, Qt::transparent);
            }

            // If the index is a branch operand item, a separate label widget will render a clickable link instead.
            bool is_branch_operation = model_->IsBranchOperandItem(index);
            if (is_branch_operation)
            {
                is_paint_enabled = false;
            }
        }

        if (is_paint_enabled)
        {
            // Invoke the default item paint implementation.
            QTreeView::drawRow(painter, new_options, index);
        }
    }
}

void RgIsaDisassemblyCustomTableView::mousePressEvent(QMouseEvent* event)
{
    // Reset the current sub widget to be the table view.
    current_sub_widget_ = DisassemblyViewSubWidgets::kTableView;

    // Detect if the user has clicked on any of the embedded Label links within the table.
    bool is_click_on_label = false;
    for (auto label : label_links_)
    {
        if (label->underMouse())
        {
            is_click_on_label = true;
            break;
        }
    }

    QItemSelectionModel* selection_model = selectionModel();
    if (is_click_on_label)
    {
        // Block the tree's selection model from emitting signals while the label click is handled.
        // This will prevent the "row has changed" signal from firing, since the user only intended to click on the Label link.
        if (selection_model != nullptr)
        {
            selection_model->blockSignals(true);
        }
    }

    // Pass the event onto the base class.
    QTreeView::mousePressEvent(event);

    // Always unblock signals to the selection model before returning.
    if (selection_model != nullptr)
    {
        selection_model->blockSignals(false);
    }

    // Highlight the frame container.
    emit FrameFocusInSignal();
}

void RgIsaDisassemblyCustomTableView::focusOutEvent(QFocusEvent* event)
{
    emit FrameFocusOutSignal();
}

void RgIsaDisassemblyCustomTableView::focusInEvent(QFocusEvent* event)
{
    emit FrameFocusInSignal();
}

void RgIsaDisassemblyCustomTableView::HandleHeaderClicked(int /* sectionNumber */)
{
    // Highlight the frame container.
    emit FrameFocusInSignal();
}

void RgIsaDisassemblyCustomTableView::HandleScrollBarClicked()
{
    // Highlight the frame container.
    emit FrameFocusInSignal();
}

void RgIsaDisassemblyCustomTableView::DisableScrollbarSignals()
{
    QScrollBar* scroll_bar = this->verticalScrollBar();
    assert(scroll_bar != nullptr);

    if (scroll_bar != nullptr)
    {
        scroll_bar->blockSignals(true);
    }

    scroll_bar = this->horizontalScrollBar();
    assert(scroll_bar != nullptr);

    if (scroll_bar != nullptr)
    {
        scroll_bar->blockSignals(true);
    }
}

void RgIsaDisassemblyCustomTableView::EnableScrollbarSignals()
{
    QScrollBar* scroll_bar = this->verticalScrollBar();
    assert(scroll_bar != nullptr);

    if (scroll_bar != nullptr)
    {
        scroll_bar->blockSignals(false);
    }

    scroll_bar = this->horizontalScrollBar();
    assert(scroll_bar != nullptr);

    if (scroll_bar != nullptr)
    {
        scroll_bar->blockSignals(false);
    }
}

void RgIsaDisassemblyCustomTableView::HandleUpdateCurrentSubWidget(DisassemblyViewSubWidgets currentWidget)
{
    current_sub_widget_ = currentWidget;
}

bool RgIsaDisassemblyCustomTableView::eventFilter(QObject* object, QEvent* event)
{
    bool is_filtered = false;

    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
        Qt::KeyboardModifiers keyboard_modifiers = QApplication::keyboardModifiers();
        assert(key_event != nullptr);
        if (key_event != nullptr)
        {
            if (key_event->key() == Qt::Key_Tab)
            {
                if (current_sub_widget_ == DisassemblyViewSubWidgets::kSourceWindow)
                {
                    emit FrameFocusInSignal();
                    current_sub_widget_ = DisassemblyViewSubWidgets::kTableView;
                }
                else if (current_sub_widget_ == DisassemblyViewSubWidgets::kTableView)
                {
                    emit FocusTargetGpuPushButton();
                    current_sub_widget_ = DisassemblyViewSubWidgets::kTargetGpuPushButton;
                }
                else if (current_sub_widget_ == DisassemblyViewSubWidgets::kTargetGpuPushButton)
                {
                    current_sub_widget_ = DisassemblyViewSubWidgets::kColumnPushButton;
                    emit FocusColumnPushButton();

                }
                else if (current_sub_widget_ == DisassemblyViewSubWidgets::kColumnPushButton)
                {
                    current_sub_widget_ = DisassemblyViewSubWidgets::kOutputWindow;
                    emit FocusCliOutputWindow();
                }

                is_filtered = true;
            }
            else if (key_event->key() == Qt::Key_Backtab)
            {
                if (current_sub_widget_ == DisassemblyViewSubWidgets::kOutputWindow)
                {
                    emit FocusColumnPushButton();
                    current_sub_widget_ = DisassemblyViewSubWidgets::kColumnPushButton;
                }
                else if (current_sub_widget_ == DisassemblyViewSubWidgets::kTableView)
                {
                    emit FocusSourceWindow();
                    emit FrameFocusOutSignal();
                    current_sub_widget_ = DisassemblyViewSubWidgets::kSourceWindow;
                }
                else if (current_sub_widget_ == DisassemblyViewSubWidgets::kTargetGpuPushButton)
                {
                    current_sub_widget_ = DisassemblyViewSubWidgets::kTableView;
                    emit FrameFocusInSignal();

                }
                else if (current_sub_widget_ == DisassemblyViewSubWidgets::kColumnPushButton)
                {
                    current_sub_widget_ = DisassemblyViewSubWidgets::kTargetGpuPushButton;
                    emit FocusTargetGpuPushButton();
                }

                is_filtered = true;
            }
            else if ((keyboard_modifiers & Qt::ControlModifier) && (key_event->key() == Qt::Key_R))
            {
                emit SwitchDisassemblyContainerSize();
            }
    }
    }

    return is_filtered;
}
