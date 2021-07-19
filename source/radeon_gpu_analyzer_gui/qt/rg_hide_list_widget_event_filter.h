#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_HIDE_LIST_WIDGET_EVENT_FILTER_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_HIDE_LIST_WIDGET_EVENT_FILTER_H_

// Qt.
#include <QObject>
#include <QListWidget>
#include <QPushButton>

// Local.
#include "source/radeon_gpu_analyzer_gui/rg_data_types.h"

class ArrowIconWidget;

// An event filter responsible for hiding list widgets on loss of focus.
class RgHideListWidgetEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit RgHideListWidgetEventFilter(QListWidget* list_widget, ArrowIconWidget* button);
    virtual ~RgHideListWidgetEventFilter() = default;

    // API to allow addition to the object name list.
    static void AddObjectName(const QString& object_name);

    // Click the push button.
    void ClickPushButton();

protected:
    // The overridden event filter.
    virtual bool eventFilter(QObject* object, QEvent* event) override;

signals:
    // A signal to give focus to disassembly view.
    void FrameInFocusSignal();

    // A signal to open the GPU list widget.
    void OpenGpuListWidget();

    // A signal to open the columns list widget.
    void OpenColumnListWidget();

    // A signal to update the current sub widget.
    void UpdateCurrentSubWidget(DisassemblyViewSubWidgets sub_widget);

    // A signal to give focus to cli output window.
    void FocusCliOutputWindow();

    // A signal to indicate list widget status.
    void EnumListWidgetStatusSignal(bool is_open);

private:
    // The list widget losing focus.
    QListWidget*        list_widget_;

    // The push button used to open the list widget.
    ArrowIconWidget*    button_;

    // The string list of widget names.
    static QStringList  string_list_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_HIDE_LIST_WIDGET_EVENT_FILTER_H_
