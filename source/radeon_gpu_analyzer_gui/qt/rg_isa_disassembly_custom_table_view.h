#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_CUSTOM_TABLE_VIEW_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_CUSTOM_TABLE_VIEW_H_

// Qt.
#include <QTreeView>
#include <QHeaderView>

// Local.
#include "source/radeon_gpu_analyzer_gui/rg_data_types.h"

// Forward declarations.
class QLabel;
class RgIsaDisassemblyTableModel;

// A custom QTreeView implementation used to present rows of disassembled instructions.
// This widget makes use of a custom row paint function to customize individual row style.
class RgIsaDisassemblyCustomTableView : public QTreeView
{
    Q_OBJECT

public:
    RgIsaDisassemblyCustomTableView(QWidget* parent = nullptr);
    virtual ~RgIsaDisassemblyCustomTableView() = default;

    // Reimplement the event filter.
    bool eventFilter(QObject* object, QEvent* event) override;

    // Provide a vector of QLabels that have been added to the disassembly table.
    void SetLabelLinkWidgets(const std::vector<QLabel*>& label_links);

    // Set the source model containing the data to be presented.
    void SetModel(RgIsaDisassemblyTableModel* model);

public slots:
    // A slot to handle disabling of scroll bar signals.
    void DisableScrollbarSignals();

    // A slot to handle enabling of scroll bar signals.
    void EnableScrollbarSignals();

    // A slot to handle updating the current sub widget.
    void HandleUpdateCurrentSubWidget(DisassemblyViewSubWidgets current_widget);

protected:
    // Override the row-painting function so that correlated lines can be highlighted.
    virtual void drawRow(QPainter* painter, const QStyleOptionViewItem& options, const QModelIndex& index) const override;

    // Override the mouse handler function to handle embedded link labels correctly.
    virtual void mousePressEvent(QMouseEvent* event) override;

    // Override the focus out event to color the frame black.
    virtual void focusOutEvent(QFocusEvent* event) override;

    // Override the focus in event to color the frame red.
    virtual void focusInEvent(QFocusEvent* event) override;

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
    void HandleHeaderClicked(int section_number);

    // Handler to color the container frame red when the user clicks on a scroll bar.
    void HandleScrollBarClicked();

private:
    // Connect signals method.
    void ConnectSignals();

    // Actions for tab and shift+tab.
    QAction* tab_key_action_ = nullptr;
    QAction* shift_tab_key_action_ = nullptr;

    // Keep track of the current sub widget.
    DisassemblyViewSubWidgets current_sub_widget_ = DisassemblyViewSubWidgets::kTableView;

    // The model containing the data that the view is presenting.
    RgIsaDisassemblyTableModel* model_ = nullptr;

    // A vector of all Label links embedded in the disassembly table.
    std::vector<QLabel*> label_links_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_ISA_DISASSEMBLY_CUSTOM_TABLE_VIEW_H_
