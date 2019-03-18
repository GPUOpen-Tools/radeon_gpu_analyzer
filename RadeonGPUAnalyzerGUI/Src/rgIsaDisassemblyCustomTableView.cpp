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
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyCustomTableView.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgIsaDisassemblyTableModel.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>
#include <RadeonGPUAnalyzerGUI/Include/rgDefinitions.h>
#include <RadeonGPUAnalyzerGUI/Include/rgStringConstants.h>

// The highlight color to use for correlated source lines.
static QColor s_CORRELATION_HIGHLIGHT_COLOR = QColor(Qt::yellow).lighter(170);
static QColor s_LIGHT_BLUE_SELECTION_COLOR = QColor(229, 243, 255);

class rgTreeviewSelectionDelegateVulkan : public QItemDelegate
{
public:
    rgTreeviewSelectionDelegateVulkan(QObject* pParent = nullptr) : QItemDelegate(pParent) {}

    virtual void paint(QPainter* pPainter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        QStyleOptionViewItem viewOption(option);

        QColor itemForegroundColor = index.data(Qt::ForegroundRole).value<QColor>();

        viewOption.palette.setColor(QPalette::HighlightedText, itemForegroundColor);
        QColor backgroundColor(s_CORRELATION_HIGHLIGHT_COLOR);
        viewOption.palette.setColor(QPalette::Highlight, backgroundColor);

        QItemDelegate::paint(pPainter, viewOption, index);
    }
};

class rgTreeviewSelectionDelegateOpenCL : public QItemDelegate
{
public:
    rgTreeviewSelectionDelegateOpenCL(QObject* pParent = nullptr) : QItemDelegate(pParent) {}

    virtual void paint(QPainter* pPainter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        QStyleOptionViewItem viewOption(option);

        QColor itemForegroundColor = index.data(Qt::ForegroundRole).value<QColor>();

        viewOption.palette.setColor(QPalette::HighlightedText, itemForegroundColor);
        QColor backgroundColor(s_LIGHT_BLUE_SELECTION_COLOR);
        viewOption.palette.setColor(QPalette::Highlight, backgroundColor);

        QItemDelegate::paint(pPainter, viewOption, index);
    }
};

rgIsaDisassemblyCustomTableView::rgIsaDisassemblyCustomTableView(QWidget* pParent) :
    QTreeView(pParent)
{
    // Enable treeview's header's click event.
    QHeaderView* pHeaderView = this->header();
    assert(pHeaderView != nullptr);
    pHeaderView->setSectionsClickable(true);

    // Connect signals.
    ConnectSignals();

    // Set the fonts.
    QFont font = this->font();
    font.setFamily(STR_BUILD_VIEW_FONT_FAMILY);
    font.setPointSize(gs_BUILD_VIEW_FONT_SIZE);
    this->setFont(font);

    rgProjectAPI currentAPI = rgConfigManager::Instance().GetCurrentAPI();
    if (currentAPI == rgProjectAPI::Vulkan)
    {
        setItemDelegate(new rgTreeviewSelectionDelegateVulkan(this));
    }
    else if (currentAPI == rgProjectAPI::OpenCL)
    {
        setItemDelegate(new rgTreeviewSelectionDelegateOpenCL(this));
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

void rgIsaDisassemblyCustomTableView::ConnectSignals()
{
    // Connect the header view.
    QHeaderView* pHeaderView = this->header();
    assert(pHeaderView != nullptr);

    if (pHeaderView != nullptr)
    {
        bool isConnected = connect(pHeaderView, &QHeaderView::sectionClicked, this, &rgIsaDisassemblyCustomTableView::HandleHeaderClicked);
        assert(isConnected);
    }

    // Connect the vertical scroll bar's sliderPressed and valueChanged signals,
    // so when the user clicks on the vertical scroll bar, or scrolls the view
    // by clicking on the scroll bar, the frame container gets the focus.
    QScrollBar* pScrollBar = this->verticalScrollBar();
    assert(pScrollBar != nullptr);

    if (pScrollBar != nullptr)
    {
        bool isConnected = connect(pScrollBar, &QScrollBar::sliderPressed, this, &rgIsaDisassemblyCustomTableView::HandleScrollBarClicked);
        assert(isConnected);

        isConnected = connect(pScrollBar, &QScrollBar::valueChanged, this, &rgIsaDisassemblyCustomTableView::HandleScrollBarClicked);
        assert(isConnected);
    }

    // Connect the horizontal scroll bar's sliderPressed and valueChanged signals,
    // so when the user clicks on the horizontal scroll bar, or scrolls the view
    // by clicking on the scroll bar, the frame container gets the focus.
    pScrollBar = this->horizontalScrollBar();
    assert(pScrollBar != nullptr);

    if (pScrollBar != nullptr)
    {
        bool isConnected = connect(pScrollBar, &QScrollBar::sliderPressed, this, &rgIsaDisassemblyCustomTableView::HandleScrollBarClicked);
        assert(isConnected);

        isConnected = connect(pScrollBar, &QScrollBar::valueChanged, this, &rgIsaDisassemblyCustomTableView::HandleScrollBarClicked);
        assert(isConnected);
    }
}

void rgIsaDisassemblyCustomTableView::SetLabelLinkWidgets(const std::vector<QLabel*>& labelLinks)
{
    m_labelLinks = labelLinks;
}

void rgIsaDisassemblyCustomTableView::SetModel(rgIsaDisassemblyTableModel* pModel)
{
    m_pModel = pModel;
}

void rgIsaDisassemblyCustomTableView::drawRow(QPainter* pPainter, const QStyleOptionViewItem& options, const QModelIndex& index) const
{
    if (m_pModel != nullptr)
    {
        // Painting with this delegate can be completely disabled depending on the incoming index.
        bool isPaintEnabled = true;

        QStyleOptionViewItem newOptions(options);
        newOptions.palette.setBrush(QPalette::Base, Qt::white);

        bool isIndexValid = index.isValid();
        assert(isIndexValid);
        if (isIndexValid)
        {
            // Is the current instruction row correlated with the currently selected line in the input file?
            bool isLineCorrelatedWithInputFile = m_pModel->IsIsaLineCorrelated(index.row());
            if (isLineCorrelatedWithInputFile)
            {
                // Paint the row background with the highlight color.
                pPainter->fillRect(options.rect, s_CORRELATION_HIGHLIGHT_COLOR);

                // If the row background gets painted manually, reset the row's background brush to be transparent.
                newOptions.palette.setBrush(QPalette::Base, Qt::transparent);
            }

            // If the index is a branch operand item, a separate label widget will render a clickable link instead.
            bool isBranchOperation = m_pModel->IsBranchOperandItem(index);
            if (isBranchOperation)
            {
                isPaintEnabled = false;
            }
        }

        if (isPaintEnabled)
        {
            // Invoke the default item paint implementation.
            QTreeView::drawRow(pPainter, newOptions, index);
        }
    }
}

void rgIsaDisassemblyCustomTableView::mousePressEvent(QMouseEvent* pEvent)
{
    // Reset the current sub widget to be the table view.
    m_currentSubWidget = DisassemblyViewSubWidgets::TableView;

    // Detect if the user has clicked on any of the embedded Label links within the table.
    bool isClickOnLabel = false;
    for (auto pLabel : m_labelLinks)
    {
        if (pLabel->underMouse())
        {
            isClickOnLabel = true;
            break;
        }
    }

    QItemSelectionModel* pSelectionModel = selectionModel();
    if (isClickOnLabel)
    {
        // Block the tree's selection model from emitting signals while the label click is handled.
        // This will prevent the "row has changed" signal from firing, since the user only intended to click on the Label link.
        if (pSelectionModel != nullptr)
        {
            pSelectionModel->blockSignals(true);
        }
    }

    // Pass the event onto the base class.
    QTreeView::mousePressEvent(pEvent);

    // Always unblock signals to the selection model before returning.
    if (pSelectionModel != nullptr)
    {
        pSelectionModel->blockSignals(false);
    }

    // Highlight the frame container.
    emit FrameFocusInSignal();
}

void rgIsaDisassemblyCustomTableView::focusOutEvent(QFocusEvent* pEvent)
{
    emit FrameFocusOutSignal();
}

void rgIsaDisassemblyCustomTableView::HandleHeaderClicked(int /* sectionNumber */)
{
    // Highlight the frame container.
    emit FrameFocusInSignal();
}

void rgIsaDisassemblyCustomTableView::HandleScrollBarClicked()
{
    // Highlight the frame container.
    emit FrameFocusInSignal();
}

void rgIsaDisassemblyCustomTableView::DisableScrollbarSignals()
{
    QScrollBar* pScrollBar = this->verticalScrollBar();
    assert(pScrollBar != nullptr);

    if (pScrollBar != nullptr)
    {
        pScrollBar->blockSignals(true);
    }

    pScrollBar = this->horizontalScrollBar();
    assert(pScrollBar != nullptr);

    if (pScrollBar != nullptr)
    {
        pScrollBar->blockSignals(true);
    }
}

void rgIsaDisassemblyCustomTableView::EnableScrollbarSignals()
{
    QScrollBar* pScrollBar = this->verticalScrollBar();
    assert(pScrollBar != nullptr);

    if (pScrollBar != nullptr)
    {
        pScrollBar->blockSignals(false);
    }

    pScrollBar = this->horizontalScrollBar();
    assert(pScrollBar != nullptr);

    if (pScrollBar != nullptr)
    {
        pScrollBar->blockSignals(false);
    }
}

void rgIsaDisassemblyCustomTableView::HandleUpdateCurrentSubWidget(DisassemblyViewSubWidgets currentWidget)
{
    m_currentSubWidget = currentWidget;
}

bool rgIsaDisassemblyCustomTableView::eventFilter(QObject* pObject, QEvent* pEvent)
{
    bool filtered = false;

    if (pEvent->type() == QEvent::KeyPress)
    {
        QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(pEvent);
        Qt::KeyboardModifiers keyboardModifiers = QApplication::keyboardModifiers();
        assert(pKeyEvent != nullptr);
        if (pKeyEvent != nullptr)
        {
            if (pKeyEvent->key() == Qt::Key_Tab)
            {
                if (m_currentSubWidget == DisassemblyViewSubWidgets::SourceWindow)
                {
                    emit FrameFocusInSignal();
                    m_currentSubWidget = DisassemblyViewSubWidgets::TableView;
                }
                else if (m_currentSubWidget == DisassemblyViewSubWidgets::TableView)
                {
                    emit FocusTargetGpuPushButton();
                    m_currentSubWidget = DisassemblyViewSubWidgets::TargetGpuPushButton;
                }
                else if (m_currentSubWidget == DisassemblyViewSubWidgets::TargetGpuPushButton)
                {
                    m_currentSubWidget = DisassemblyViewSubWidgets::ColumnPushButton;
                    emit FocusColumnPushButton();

                }
                else if (m_currentSubWidget == DisassemblyViewSubWidgets::ColumnPushButton)
                {
                    m_currentSubWidget = DisassemblyViewSubWidgets::OutputWindow;
                    emit FocusCliOutputWindow();
                }

                filtered = true;
            }
            else if (pKeyEvent->key() == Qt::Key_Backtab)
            {
                if (m_currentSubWidget == DisassemblyViewSubWidgets::OutputWindow)
                {
                    emit FocusColumnPushButton();
                    m_currentSubWidget = DisassemblyViewSubWidgets::ColumnPushButton;
                }
                else if (m_currentSubWidget == DisassemblyViewSubWidgets::TableView)
                {
                    emit FocusSourceWindow();
                    emit FrameFocusOutSignal();
                    m_currentSubWidget = DisassemblyViewSubWidgets::SourceWindow;
                }
                else if (m_currentSubWidget == DisassemblyViewSubWidgets::TargetGpuPushButton)
                {
                    m_currentSubWidget = DisassemblyViewSubWidgets::TableView;
                    emit FrameFocusInSignal();

                }
                else if (m_currentSubWidget == DisassemblyViewSubWidgets::ColumnPushButton)
                {
                    m_currentSubWidget = DisassemblyViewSubWidgets::TargetGpuPushButton;
                    emit FocusTargetGpuPushButton();
                }

                filtered = true;
            }
            else if ((keyboardModifiers & Qt::ControlModifier) && (pKeyEvent->key() == Qt::Key_R))
            {
                emit SwitchDisassemblyContainerSize();
            }
    }
    }

    return filtered;
}