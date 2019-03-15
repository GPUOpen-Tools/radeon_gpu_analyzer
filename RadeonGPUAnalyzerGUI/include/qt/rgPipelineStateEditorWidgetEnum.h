#pragma once

// Infra.
#include <QtCommon/CustomWidgets/ListWidget.h>
#include <QtCommon/Scaling/ScalingManager.h>

// Local.
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgPipelineStateEditorWidget.h>
#include <RadeonGPUAnalyzerGUI/Include/Qt/rgEditorElementEnum.h>
#include <ui_rgPipelineStateEditorWidgetEnum.h>

// Forward declarations.
class ListWidget;
namespace Ui {
class rgPipelineStateEditorWidgetEnum;
}

class rgPipelineStateEditorWidgetEnum : public rgPipelineStateEditorWidget
{
    Q_OBJECT

public:
    explicit rgPipelineStateEditorWidgetEnum(bool isBitFlagsEnum, QWidget* pParent = nullptr);
    virtual ~rgPipelineStateEditorWidgetEnum();

    // Return a string displaying the enabled flags in decimal and bit flags.
    void GetFlagBitsString(std::string& flagBits);

    // Get the state of the editor checkbox.
    uint32_t GetValue() const;

    // Set the list of possible values to set the element to.
    void SetEnumerators(const rgEnumValuesVector& enumerators);

    // Set the state of the editor checkbox.
    void SetValue(uint32_t value);

signals:
    // A signal to indicate list widget status change.
    void EnumListWidgetStatusSignal(bool isOpened);

private slots:
    // Handler for the enum list push button click.
    void HandleEnumPushButtonClick(bool checked);

    // Handler invoked when a check state changes in a flag enumeration.
    void HandleFlagCheckStateChanged(bool checked);

    // Handler invoked when the selected row index is changed (for index-based enumerations).
    void HandleEnumChanged(int index);

    // Handle updating the enum button's text when the selected value changes.
    void HandleUpdateEnumButtonText(const QString& text, bool checked);

    // Handle application lost focus event.
    void HandleApplicationFocusOutEvent(Qt::ApplicationState state);

    // Handle list widget push button focus out event.
    void HandleArrowButtonFocusOutEvent();

private:
    // Connect internal signals.
    void ConnectSignals();

    // Create the enum control.
    void CreateEnumControls();

    // Return the enum push button counter.
    static int GetEnumPushButtonCounter();

    // Return a string displaying the selected enum/bitwise'd flag strings.
    void GetTooltipString(std::string& tooltipText);

    // Hide the list widget.
    void HideListWidget();

    // Set the selected row in an index-based enumeration.
    void SetSelectedListRow(int rowIndex);

    // Bump up the enum push button counter.
    static void UpdateEnumPushButtonCounter();

    // Update the selected enum.
    void UpdateSelectedEnum(const std::string& newValue);

    // A vector of the enumeration values that are possible to set in this element.
    rgEnumValuesVector m_enumerators;

    // The generated UI object.
    Ui::rgPipelineStateEditorWidgetEnum ui;

    // The widget used to display all enum values.
    ListWidget* m_pEnumListWidget = nullptr;

    // A custom event filter for the enum list widget.
    QObject* m_pEnumListEventFilter = nullptr;

    // A flag used to indicate if flags or a single enumeration is being edited.
    bool m_isBitFlagsEnum = false;

    // The currently selected enumeration index.
    int m_currentIndex = 0;

    // The parent widget.
    QWidget* m_pParent = nullptr;

    // Counter to keep track of the number of enum push buttons created.
    static int s_enumComboPushButtonCounter;
};