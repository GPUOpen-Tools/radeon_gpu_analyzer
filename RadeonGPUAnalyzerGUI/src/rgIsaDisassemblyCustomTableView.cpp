// C++.
#include <cassert>

// Qt.
#include <QLabel>
#include <QPainter>

// Local.
#include <RadeonGPUAnalyzerGUI/include/qt/rgIsaDisassemblyTableModel.h>
#include <RadeonGPUAnalyzerGUI/include/qt/rgIsaDisassemblyCustomTableView.h>

// The highlight color to use for correlated source lines.
static QColor s_CORRELATION_HIGHLIGHT_COLOR = QColor(Qt::yellow).lighter(170);

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
        pSelectionModel->blockSignals(true);
        QTreeView::mousePressEvent(pEvent);
    }
    else
    {
        // Handle the mouse click normally, since the user didn't click on a Label link in the table.
        QTreeView::mousePressEvent(pEvent);
    }

    // Always unblock signals to the selection model before returning.
    pSelectionModel->blockSignals(false);
}