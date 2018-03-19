#pragma once

// Qt.
#include <QObject>
#include <QListWidget>
#include <QPushButton>

class ArrowIconWidget;

// An event filter responsible for hiding list widgets on loss of focus.
class rgHideListWidgetEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit rgHideListWidgetEventFilter(QListWidget* pListWidget, ArrowIconWidget* pButton);
    virtual ~rgHideListWidgetEventFilter() = default;

private:
    // The overridden event filter.
    virtual bool eventFilter(QObject *object, QEvent *event) override;

private:
    // The list widget losing focus.
    QListWidget*        m_pListWidget;

    // The push button used to open the list widget.
    ArrowIconWidget*    m_pButton;

    // The string list of widget names.
    static QStringList  m_stringList;
};