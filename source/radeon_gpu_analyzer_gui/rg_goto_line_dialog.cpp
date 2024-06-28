// C++.
#include <cassert>

// Qt.
#include <QFileDialog>
#include <QSignalMapper>
#include <QValidator>

// Local.
#include "radeon_gpu_analyzer_gui/qt/rg_go_to_line_dialog.h"

RgGoToLineDialog::RgGoToLineDialog(int maxLineNumber, QWidget* parent) :
    QDialog(parent),
    line_number_(0),
    max_line_number_(maxLineNumber)
{
    // Setup the UI.
    ui_.setupUi(this);

    // Disable the help button in the title bar.
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Set the background to white.
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    // Set the validator for the line number edit box.
    QValidator *validator = new QIntValidator(0, max_line_number_, this);

    // The line number edit box will only accept integers between 0 and m_maxLineNumber.
    ui_.lineNumberEdit->setValidator(validator);

    // Update the label with line number range.
    ui_.lineNumberLabel->setText(QString("Line number (1-") + QString::number(max_line_number_) + QString("): "));

    // Connect the signals.
    ConnectSignals();
}

RgGoToLineDialog::~RgGoToLineDialog()
{
}

void RgGoToLineDialog::ConnectSignals()
{
    // Create a signal mapper to map the button clicks to the done(int) slot
    // with appropriate result values.
    QSignalMapper* button_signal_mapper = new QSignalMapper(this);

    // Yes button.
    bool is_connected = connect(ui_.okPushButton, SIGNAL(clicked()), button_signal_mapper, SLOT(map()));
    assert(is_connected);
    button_signal_mapper->setMapping(ui_.okPushButton, RgGoToLineDialog::kOk);

    // Cancel button.
    is_connected = connect(ui_.cancelPushButton, SIGNAL(clicked()), button_signal_mapper, SLOT(map()));
    assert(is_connected);
    button_signal_mapper->setMapping(ui_.cancelPushButton, RgGoToLineDialog::kCancel);

    // Signal mapper.
    is_connected = connect(button_signal_mapper, SIGNAL(mappedInt(int)), this, SLOT(done(int)));
    assert(is_connected);

    // The line number edit box.
    is_connected = connect(ui_.lineNumberEdit, &QLineEdit::textChanged, this, &RgGoToLineDialog::HandleLineNumberEntered);
}

void RgGoToLineDialog::HandleLineNumberEntered(const QString& text)
{
    line_number_ = text.toInt();
}

int RgGoToLineDialog::GetLineNumber() const
{
    return line_number_;
}
