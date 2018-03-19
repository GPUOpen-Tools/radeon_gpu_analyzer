#pragma once

// Qt.
#include <QTreeView>

// Forward declarations.
class QLabel;
class rgIsaDisassemblyTableModel;

// A custom QTreeView implementation used to present rows of disassembled instructions.
// This widget makes use of a custom row paint function to customize individual row style.
class rgIsaDisassemblyCustomTableView : public QTreeView
{
    Q_OBJECT

public:
    rgIsaDisassemblyCustomTableView(QWidget* pParent = nullptr) : QTreeView(pParent) {}
    virtual ~rgIsaDisassemblyCustomTableView() = default;

    // Provide a vector of QLabels that have been added to the disassembly table.
    void SetLabelLinkWidgets(const std::vector<QLabel*>& labelLinks);

    // Set the source model containing the data to be presented.
    void SetModel(rgIsaDisassemblyTableModel* pModel);

protected:
    // Override the row-painting function so that correlated lines can be highlighted.
    virtual void drawRow(QPainter* pPainter, const QStyleOptionViewItem& options, const QModelIndex& index) const override;

    // Override the mouse handler function to handle embedded link labels correctly.
    virtual void mousePressEvent(QMouseEvent* pEvent) override;

private:
    // The model containing the data that the view is presenting.
    rgIsaDisassemblyTableModel* m_pModel = nullptr;

    // A vector of all Label links embedded in the disassembly table.
    std::vector<QLabel*> m_labelLinks;
};