#ifndef RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_EDITOR_ELEMENT_H_
#define RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_EDITOR_ELEMENT_H_

// C++
#include <functional>

// Qt.
#include <QVariant>
#include <QWidget>

// Infra.
#include "qt_common/custom_widgets/arrow_icon_combo_box.h"
// Local.
#include "ui_rg_editor_element.h"

// The following style bit flags can be applied to each row in order to alter the background color.
enum class RgStyleFlags : char
{
    // No special style is applied. The row is drawn with default coloring.
    None = 0,

    // The row is displayed as "current," because the user selected the row.
    CurrentRow = 1 << 0,

    // The row is displayed as a "Search Result Occurrence," because it was flagged as containing
    // a search result, but it's not the current result being focused on.
    SearchResultOccurrence = 1 << 1,

    // The row is displayed as the "Current Search Result," because it's flagged as containing a
    // search result, and it's the current result being focused on.
    SearchResultCurrent = 1 << 2
};

// This enum identifies each row's data.
enum class RgRowData
{
    kRowDataMemberName,
    kRowDataMemberValue,
    kRowDataCount
};

// An enumeration used to track the expanded/collapsed state of a row.
enum class RgRowExpansionState
{
    kExpanded,
    kCollapsed
};

// A list of value types that the user is able to edit.
enum class RgEditorDataType : char
{
    // This type is used for editor elements that cannot be edited.
    kVoid,

    // Edit true/false type.
    kBool,

    // Edit numeric primitive types.
    kInt8,
    kUInt8,
    kInt16,
    kUInt16,
    kInt32,
    kUInt32,
    kFloat,
    kDouble,

    // Edit elements with a limited number of possible values.
    kEnum,

    // The type used for array root elements. These elements can be resized to alter
    // the dimension of an array of child element items.
    kArray,

    // A single element of an array.
    kArrayElement,
};

// Forward declarations.
class RgPipelineStateEditorWidget;
class RgPipelineStateTree;

// The base class for typed editor elements.
class RgEditorElement : public QWidget
{
    Q_OBJECT

public:
    explicit RgEditorElement(QWidget* parent, const std::string& member_name,
        RgEditorDataType data_type = RgEditorDataType::kVoid,
        std::function<void()> value_changed_callback = nullptr);
    virtual ~RgEditorElement() = default;

    // Get the data held within this item.
    virtual QVariant Data(int column) const;

    // Retrieve the editor widget used by the row. If the row is not editable, it won't contain an
    // editor widget, and this function will return nullptr.
    virtual RgPipelineStateEditorWidget* GetEditorWidget();

    // Add the given style flag to the row's style bit field.
    void AddStyleFlag(RgStyleFlags style_flag);

    // Remove the given style flag from the row's style bit field.
    void RemoveStyleFlag(RgStyleFlags style_flag);

    // Append a new child element to the item.
    void AppendChildItem(RgEditorElement* item);

    // Remove all children from the element.
    void ClearChildren();

    // Recursively find the last visible descendant row.
    RgEditorElement* FindLastVisibleDescendant(RgEditorElement* current_element) const;

    // Get the next ancestor to the given element by walking up the tree of parent rows.
    RgEditorElement* FindNextAnscestor(RgEditorElement* current_element = nullptr);

    // Get the last descendant row of the element tree.
    // If there are no remaining descendants return this element.
    RgEditorElement* GetLastVisibleDescendant();

    // Get the parent RgPipelineStateTree that this widget hierarchy is rooted to.
    RgPipelineStateTree* GetParentStateTree() const;

    // Set this row's parent RgPipelineStateTree.
    void SetParentStateTree(RgPipelineStateTree* parent_tree);

    // Get a child item by row index.
    RgEditorElement* GetChild(int row_index) const;

    // Returns true if the value should be editable, and false if not.
    bool GetIsEditable() const;

    // Returns the height of the row itself- not including children.
    int GetRowHeight() const;

    // Return the style flags bit field for the row.
    uint32_t GetStyleFlags() const;

    // Get the number of children for this item.
    int ChildCount() const;

    // Get the number of columns displayed for an item.
    int ColumnCount() const;

    // Get the row index of this child relative to the parent's children list.
    int GetRowIndex() const;

    // Get the parent item for this item.
    RgEditorElement* GetParentItem() const;

    // Get the type of data being edited by this element.
    RgEditorDataType GetType() const;

    // Set the expansion state of the row.
    void SetExpansionState(RgRowExpansionState state, bool update_children = false);

    // Get the expanded/collapsed state.
    RgRowExpansionState GetExpansionState() const;

    // Set the element's tooltip text.
    void SetTooltipText();

    // Initialize the row controls.
    void InitializeControls();

    // Recursively initialize tree rows, starting at the given element.
    void InitializeRows(RgEditorElement* root_element = nullptr);

    // Retrieve the member name.
    std::string GetMemberName() const { return member_name_; }

    // Clear the substring search data.
    // This method clears any stored data related to
    // substring start location, end location, string value
    // and the color to be used to highlight the substring.
    void ClearSearchStringData();

    // Clear the substring search data storage vector.
    // This method clears the substring information
    // in between highlighting substring between
    // widgets within the same row.
    void ClearSearchStringDataVector();

    // Highlight the line edit matching substring.
    void HighlightLineEditSubString(int start_location, const std::string& search_string, bool is_current_match);

    // Reset the line edit matching substring.
    void ResetLineEditSubString();

    // Reset the label matching substring.
    void ResetLabelSubString();

    // Highlight the requested substring in the button.
    void HighlightButtonSubString(int character_location, const std::string& search_string, bool is_current_match);

    // Reset the requested substring in the button.
    void ResetButtonSubString();

    // Highlight the requested substring in the label.
    void HighlightLabelSubString(int character_location, const std::string& search_string, bool is_current_match);

    // Reset the requested substring in the label.
    void ResetLabelSubString(int character_location, const std::string& search_string);

    // Focus on the given column widget.
    void SetFocusedColumn(RgRowData column);

    // Set the callback that's invoked when the value is changed.
    void SetValueChangedCallback(std::function<void()> value_changed_callback);

    // A flag indicating if the row is currently filtered out due to a user search.
    bool is_filtered_out_;

public slots:
    // Handler invoked when the user clicks an editor widget in the value column of the row.
    void HandleEditorFocusIn();

    // Handler invoked when editor widget loses focus.
    void HandleEditorFocusOut();

    // Handler invoked when the user clicks the row's Expand/Collapse button.
    void HandleExpandClicked();

    // A handler invoked when an element's value has changed.
    void HandleValueChanged();

protected slots:
    // Handler invoked when the user clicks the row's name label in the left column.
    void HandleNameLabelFocusIn();

signals:
    // A signal emitted to indicate list widget status change.
    void EnumListWidgetStatusSignal(bool is_open);

protected:
    // A handler invoked when the user has changed the editable value.
    virtual void ValueChangedHandler() {};

    // Compute the indent width for the given element.
    int ComputeIndentationWidth(RgEditorElement* element);

    // Connect internal signals to slots within the row element.
    void ConnectSignals();

    // Expand the tree entry.
    void ExpandTreeEntry();

    // Set the highlight color for the matched string.
    void SetHighlightColor(StringHighlightData& string_highlight_data);

    // Set the style flag bits for the row.
    void SetStyleFlags(uint32_t style_flags);

    // Compute the indentation for each row in the tree.
    void UpdateIndentation();

    // Update the button substring to highlight.
    void UpdateButtonSubString();

    // Update the location for the matching substring location.
    void UpdateStringMatchingLocation(int start_location, int length, const std::string& search_string);

    // A callback invoked when the element value is changed.
    std::function<void()> value_changed_callback_ = nullptr;

    // A vector of child nodes for this item.
    std::vector<std::shared_ptr<RgEditorElement>> child_items_;

    // The name of the member being edited.
    std::string member_name_;

    // The type of value being edited.
    RgEditorDataType type_;

    // The current expanded/collapsed state of the row.
    RgRowExpansionState row_state_;

    // The parent item for this item.
    RgEditorElement* parent_item_ = nullptr;

    // The parent RgPipelineStateTree that this instance is rooted to.
    // This pointer will only be valid if it's the root element in the tree.
    RgPipelineStateTree* parent_tree_ = nullptr;

    // A bit field of style flags for the row.
    uint32_t style_flags_ = 0;

    // A vector to store substring highlight data.
    QVector<StringHighlightData> string_highlight_data_ = {};

    // Boolean to indicate if this is the current match.
    bool is_current_match_ = false;

    // A spacer that can be inserted to indent the row.
    QSpacerItem* spacer_item_ = nullptr;

    // The generated view object.
    Ui::RgEditorElement ui_;
};
#endif // RGA_RADEONGPUANALYZERGUI_INCLUDE_QT_RG_EDITOR_ELEMENT_H_
