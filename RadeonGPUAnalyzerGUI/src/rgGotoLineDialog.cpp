// C++.
#include <cassert>

// Qt.
#include <QFileDialog>
#include <QSignalMapper>
#include <QValidator>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgGoToLineDialog.h>


rgGoToLineDialog::rgGoToLineDialog(int maxLineNumber, QWidget* pParent) :
    QDialog(pParent),
    m_lineNumber(0),
    m_maxLineNumber(maxLineNumber)
{
    // Setup the UI.
    ui.setupUi(this);

    // Disable the help button in the title bar.
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Set the background to white.
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    // Set the validator for the line number edit box.
    QValidator *validator = new QIntValidator(0, m_maxLineNumber, this);

    // The line number edit box will only accept integers between 0 and m_maxLineNumber.
    ui.lineNumberEdit->setValidator(validator);

    // Update the label with line number range.
    ui.lineNumberLabel->setText(QString("Line number (1-") + QString::number(m_maxLineNumber) + QString("): "));

    // Connect the signals.
    ConnectSignals();
}

rgGoToLineDialog::~rgGoToLineDialog()
{
}

void rgGoToLineDialog::ConnectSignals()
{
    // Create a signal mapper to map the button clicks to the done(int) slot
    // with appropriate result values.
    QSignalMapper* pButtonSignalMapper = new QSignalMapper(this);

    // Yes button.
    bool isConnected = connect(ui.okPushButton, SIGNAL(clicked()), pButtonSignalMapper, SLOT(map()));
    assert(isConnected);
    pButtonSignalMapper->setMapping(ui.okPushButton, rgGoToLineDialog::Ok);

    // Cancel button.
    isConnected = connect(ui.cancelPushButton, SIGNAL(clicked()), pButtonSignalMapper, SLOT(map()));
    assert(isConnected);
    pButtonSignalMapper->setMapping(ui.cancelPushButton, rgGoToLineDialog::Cancel);

    // Signal mapper.
    isConnected = connect(pButtonSignalMapper, SIGNAL(mapped(int)), this, SLOT(done(int)));
    assert(isConnected);

    // The line number edit box.
    isConnected = connect(ui.lineNumberEdit, &QLineEdit::textChanged, this, &rgGoToLineDialog::HandleLineNumberEntered);
}

void rgGoToLineDialog::HandleLineNumberEntered(const QString& text)
{
    m_lineNumber = text.toInt();
}

int rgGoToLineDialog::GetLineNumber() const
{
    return m_lineNumber;
}



