#pragma once

// Qt.
#include <QTreeView>
#include <QHeaderView>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>

// Forward declarations.
class QLabel;
class rgIsaDisassemblyTableModel;

// A custom QTreeView implementation used to present rows of disassembled instructions.
// This widget makes use of a custom row paint function to customize individual row style.
class rgIsaDisassemblyCustomTableView : public QTreeView
{
    Q_OBJECT

public:
    rgIsaDisassemblyCustomTableView(QWidget* pParent = nullptr);
    virtual ~rgIsaDisassemblyCustomTableView() = default;

    // Reimplement the event filter.
    bool eventFilter(QObject* pObject, QEvent* pEvent) override;

    // Provide a vector of QLabels that have been added to the disassembly table.
    void SetLabelLinkWidgets(const std::vector<QLabel*>& labelLinks);

    // Set the source model containing the data to be presented.
    void SetModel(rgIsaDisassemblyTableModel* pModel);

public slots:
    // A slot to handle disabling of scroll bar signals.
    void DisableScrollbarSignals();

    // A slot to handle enabling of scroll bar signals.
    void EnableScrollbarSignals();

    // A slot to handle updating the current sub widget.
    void HandleUpdateCurrentSubWidget(DisassemblyViewSubWidgets currentWidget);

protected:
    // Override the row-painting function so that correlated lines can be highlighted.
    virtual void drawRow(QPainter* pPainter, const QStyleOptionViewItem& options, const QModelIndex& index) const override;

    // Override the mouse handler function to handle embedded link labels correctly.
    virtual void mousePressEvent(QMouseEvent* pEvent) override;

    // Override the focus out event to color the frame black.
    virtual void focusOutEvent(QFocusEvent* pEvent) override;

    // Override the focus in event to color the frame red.
    virtual void focusInEvent(QFocusEvent* pEvent) override;

signals:
    // A signal to indicate frame lost focus.
    void FrameFocusOutSignal();

    // A signal to indicate frame gained focus.
    void FrameFocusInSignal();

    // A signal to indicate that the target GPU push button gained the focus.
    void FocusTargetGpuPushButton();

    // A signal to indicate that the column push button gained the focus.
    void FocusColumnPushButton();

    // A signal to give focus to output window.
    void FocusCliOutputWindow();

    // A signal to give focus to source window.
    void FocusSourceWindow();

    // A signal to switch the disassembly view window size.
    void SwitchDisassemblyContainerSize();

private slots:
    // Handler to color the container frame red when the user clicks on a header section.
    void HandleHeaderClicked(int sectionNumber);

    // Handler to color the container frame red when the user clicks on a scroll bar.
    void HandleScrollBarClicked();

private:
    // Connect signals method.
    void ConnectSignals();

    // Actions for tab and shift+tab.
    QAction* m_pTabKeyAction = nullptr;
    QAction* m_pShiftTabKeyAction = nullptr;

    // Keep track of the current sub widget.
    DisassemblyViewSubWidgets m_currentSubWidget = DisassemblyViewSubWidgets::TableView;

    // The model containing the data that the view is presenting.
    rgIsaDisassemblyTableModel* m_pModel = nullptr;

    // A vector of all Label links embedded in the disassembly table.
    std::vector<QLabel*> m_labelLinks;
};