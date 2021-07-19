#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_EDITOR_WIDGET_ENUM_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_EDITOR_WIDGET_ENUM_H_

// Infra.
#include "QtCommon/CustomWidgets/ListWidget.h"
#include "QtCommon/Scaling/ScalingManager.h"

// Local.
#include "source/radeon_gpu_analyzer_gui/qt/rg_pipeline_state_editor_widget.h"
#include "source/radeon_gpu_analyzer_gui/qt/rg_editor_element_enum.h"
#include "ui_rg_pipeline_state_editor_widget_enum.h"

// Forward declarations.
class ListWidget;
namespace Ui {
class RgPipelineStateEditorWidgetEnum;
}

class RgPipelineStateEditorWidgetEnum : public RgPipelineStateEditorWidget
{
    Q_OBJECT

public:
    explicit RgPipelineStateEditorWidgetEnum(bool is_bit_flags_enum, QWidget* parent = nullptr);
    virtual ~RgPipelineStateEditorWidgetEnum();

    // Return a string displaying the enabled flags in decimal and bit flags.
    void GetFlagBitsString(std::string& flag_bits);

    // Get the state of the editor checkbox.
    uint32_t GetValue() const;

    // Set the list of possible values to set the element to.
    void SetEnumerators(const RgEnumValuesVector& enumerators);

    // Set the state of the editor checkbox.
    void SetValue(uint32_t value);

signals:
    // A signal to indicate list widget status change.
    void EnumListWidgetStatusSignal(bool is_opened);

public slots:
    // Handler for the hotkey pressed signal.
    void HandleHotKeyPressedSignal();

protected:
    // The widget used to display all enum values.
    ListWidget* enum_list_widget_ = nullptr;

    // The generated UI object.
    Ui::RgPipelineStateEditorWidgetEnum ui_;

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

private:
    // Connect internal signals.
    void ConnectSignals();

    // Create the enum control.
    void CreateEnumControls();

    // Return the enum push button counter.
    static int GetEnumPushButtonCounter();

    // Return a string displaying the selected enum/bitwise'd flag strings.
    void GetTooltipString(std::string& tooltip_text);

    // Hide the list widget.
    void HideListWidget();

    // Set the selected row in an index-based enumeration.
    void SetSelectedListRow(int row_index);

    // Bump up the enum push button counter.
    static void UpdateEnumPushButtonCounter();

    // Update the selected enum.
    void UpdateSelectedEnum(const std::string& new_value);

    // A vector of the enumeration values that are possible to set in this element.
    RgEnumValuesVector enumerators_;

    // A custom event filter for the enum list widget.
    QObject* enum_list_event_filter_ = nullptr;

    // A flag used to indicate if flags or a single enumeration is being edited.
    bool is_bit_flags_enum_ = false;

    // The currently selected enumeration index.
    int current_index_ = 0;

    // The parent widget.
    QWidget* parent_ = nullptr;

    // Counter to keep track of the number of enum push buttons created.
    static int enum_combo_push_button_counter_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_PIPELINE_STATE_EDITOR_WIDGET_ENUM_H_
