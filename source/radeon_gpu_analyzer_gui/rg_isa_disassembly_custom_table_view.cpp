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

class RgTreeviewSelectionDelegateVulkan : public QItemDelegate
{
public:
    RgTreeviewSelectionDelegateVulkan(QObject* parent = nullptr) : QItemDelegate(parent) {}

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        QStyleOptionViewItem view_option(option);

        QColor item_foreground_color = index.data(Qt::ForegroundRole).value<QColor>();

        view_option.palette.setColor(QPalette::HighlightedText, item_foreground_color);
        QColor background_color(kCorrelationHighlightColor);
        view_option.palette.setColor(QPalette::Highlight, background_color);

        QItemDelegate::paint(painter, view_option, index);
    }
};

class RgTreeviewSelectionDelegateOpencl : public QItemDelegate
{
public:
    RgTreeviewSelectionDelegateOpencl(QObject* parent = nullptr) : QItemDelegate(parent) {}

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        QStyleOptionViewItem view_option(option);

        QColor item_foreground_color = index.data(Qt::ForegroundRole).value<QColor>();

        view_option.palette.setColor(QPalette::HighlightedText, item_foreground_color);
        QColor background_color(kLightBlueSelectionColor);
        view_option.palette.setColor(QPalette::Highlight, background_color);

        QItemDelegate::paint(painter, view_option, index);
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
