#pragma once

// Qt.
#include <QObject>
#include <QListWidget>
#include <QPushButton>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/rgDataTypes.h>

class ArrowIconWidget;

// An event filter responsible for hiding list widgets on loss of focus.
class rgHideListWidgetEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit rgHideListWidgetEventFilter(QListWidget* pListWidget, ArrowIconWidget* pButton);
    virtual ~rgHideListWidgetEventFilter() = default;

    // API to allow addition to the object name list.
    static void AddObjectName(const QString& objectName);

    // Click the push button.
    void ClickPushButton();

protected:
    // The overridden event filter.
    virtual bool eventFilter(QObject* pObject, QEvent* pEvent) override;

signals:
    // A signal to give focus to disassembly view.
    void FrameInFocusSignal();

    // A signal to open the GPU list widget.
    void OpenGpuListWidget();

    // A signal to open the columns list widget.
    void OpenColumnListWidget();

    // A signal to update the current sub widget.
    void UpdateCurrentSubWidget(DisassemblyViewSubWidgets subWidget);

    // A signal to give focus to cli output window.
    void FocusCliOutputWindow();

    // A signal to indicate list widget status.
    void EnumListWidgetStatusSignal(bool isOpen);

private:
    // The list widget losing focus.
    QListWidget*        m_pListWidget;

    // The push button used to open the list widget.
    ArrowIconWidget*    m_pButton;

    // The string list of widget names.
    static QStringList  m_stringList;
};